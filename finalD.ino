#include <LiquidCrystal.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>
#include <EEPROM.h>
#include <Keypad.h>

File EnumbersFile, DateFile, CourseNumberFile, AttendenceFile, LectureHoursFile, DepFile,CountFile;

uint8_t getFingerprintEnroll(int enumber);
int getFingerprintIDez();
char readchar();
int getenum(uint8_t input);
uint8_t deleteFingerprint(uint8_t id);
int putid(int enumber);
int saveAttendence(int attendence);
int getID(int input);

LiquidCrystal lcd(12, 13, 5, 4, 3, 2);
SoftwareSerial mySerial(10, 11);
SoftwareSerial myConnection(8, 9);

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

uint8_t id, temp, reseted = 0, fingercount, temp4;
uint32_t Date = 0;

int id8, idcounter = 0, resetstate = 10, stateaddress = 20, fingercountaddress = 30, enumber1, re = 2, CourseNumber = 0,Countnm=0,
 LectureHours = 0, tempEnum = 0, temp3 = 0, password = 0, waitbluetooth = 0, waitforfinished = 0;
char state = 'D', again = 'C';
boolean enrollmentsetup = true, fingersetup = true, finshedenroll = true,
        fingerexit = false, gotofing = false, entered = true, istrue = true, issuccess = true;

String enumberfound = "", idfound = "", enumid = "", temp6 = "",dep="CO";
String dataStr,encripted;

const int chipSelect = 53;//SD card

char keys[4][4] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'R', '0', 'E', 'D'}
};

byte rowPin[4] = {28, 29, 30, 31};
byte colPin[4] = {32, 33, 34, 35};

Keypad keypad = Keypad(makeKeymap(keys), rowPin, colPin, 4, 4);

int SDsetup() {
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    lcd.print("SOMETHING WRONG.");
     lcd.setCursor(0, 1);
     lcd.print(" MAKE SURE SDCARD");
       lcd.setCursor(0, 2);
      lcd.print(" AVAILABLE....?");
        lcd.setCursor(0, 3);
       lcd.print("RESTART THE DEVICE");
   
    delay(2000);
    
  }
  //Serial.print("CARD INITIALIZED");

  return 0;
}

int getID(int input) {
  idfound = "";

  String names = String();

  names += input;

  EnumbersFile = SD.open("enumbers.txt");
  if (EnumbersFile) {
    int i = 0;
    
    // read from the file until there's nothing else in it:
    while (EnumbersFile.available()) {
      String enumberfoundtemp = EnumbersFile.readStringUntil(',');
      //Serial.println ("ENUM:");
      //Serial.println (enumberfound);
      
      String idst = EnumbersFile.readStringUntil('\n');
      //Serial.print ("ENUM-ID:");
      //Serial.print (idst);

      int temp1 = names.toInt();
      int temp2 = enumberfoundtemp.toInt();

      if (temp1 == temp2) {
        //Serial.println(".............matched...........");
        //put the ID to idfound global varible
        idfound = idst;
        EnumbersFile.close();
       return 0;
      }
      else;
    }

    EnumbersFile.close();
  }
  else {
    // if the file didn't open, print an error:
    //Serial.println("error opening enumbers.txt");
  }

  return -1;
}

int getenum(uint8_t input) {
  enumberfound = "";

  String names = String();

  names += input;
  
  EnumbersFile = SD.open("enumbers.txt");
  if (EnumbersFile) {
    // Serial.println("enumbers.txt:");
    int i = 0;
    while (EnumbersFile.available()) {
      String enumberfoundtemp = EnumbersFile.readStringUntil(',');
      //Serial.println ("ENUM:");
      //Serial.println (enumberfound);
      
      String idst = EnumbersFile.readStringUntil('\n');
      //Serial.print ("ENUM-ID:");
      //Serial.print (idst);

      int temp1 = names.toInt();
      int temp2 = idst.toInt();

      if (temp1 == temp2) {
        //Serial.println(".............matched...........");
        //put enumber to globle varible enumberfound
        enumberfound = enumberfoundtemp;
        EnumbersFile.close();
        return 0;
      }
      else;
    }
    EnumbersFile.close();
  }
  else {
    // if the file didn't open, print an error:
    //Serial.println("error opening enumbers.txt");
  }
  return -1;
}

uint8_t deleteFingerprint(uint8_t id) {
  uint8_t p = -1;
  p = finger.deleteModel(id);
  if (p == FINGERPRINT_OK) {
    lcd.clear();
    lcd.print("Deleted!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    lcdprint("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    lcdprint("Could not delete in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    lcdprint("Error writing to flash");
    return p;
  } else {
    lcd.clear();
    lcd.print("Unknown error: 0x"); lcd.print(p, HEX);
    return p;
  }
}

//put enumber to csv mapping
int putid(int enumber) {
  String dataString = String(enumber);
  dataString += ",";
  id = EEPROM.read(0);

  dataString += String(id);

 EnumbersFile = SD.open("enumbers.txt", FILE_WRITE);
  // if the file is available, write to it:
  if (EnumbersFile) {
    EnumbersFile.println(dataString);
    EnumbersFile.close();
  }
  // if the file isn't open, pop up an error:
  else {
    lcdprint("ERROR OPENING Enumbers.txt");
    state = 'D';
    issuccess = false;
    return -1;

  }
  //Serial.println(dataString);

  return 0;
}

int saveAttendence(int attendence) {
  String dataString = String();
  String datestr;

  DateFile = SD.open("dates.txt");
  if (DateFile) {
    while (DateFile.available()) {
      datestr = DateFile.readStringUntil('\n');
    }

    Date = datestr.toInt();
    DateFile.close();
  }

  else {
    // if the file didn't open, print an error:
    //Serial.println("error opening enumbers.txt");
  }

  //.........................................................................
  CourseNumberFile = SD.open("course.txt");
  if (CourseNumberFile) {
    while (CourseNumberFile.available()) {
      datestr = CourseNumberFile.readStringUntil('\n');
    }

    CourseNumber = datestr.toInt();
    CourseNumberFile.close();
  }

  else {
    // if the file didn't open, print an error:
    //Serial.println("error opening enumbers.txt");
  }

//.........................................................................
 DepFile = SD.open("dep.txt");
  if (DepFile) {
    while (DepFile.available()) {
      datestr = DepFile.readStringUntil('\n');
    }

   dep = datestr;
   DepFile.close();
  }

  else {
    // if the file didn't open, print an error:
    //Serial.println("error opening enumbers.txt");
  }

  //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

  LectureHoursFile = SD.open("hours.txt");
  if (LectureHoursFile) {
    while (LectureHoursFile.available()) {
      datestr = LectureHoursFile.readStringUntil('\n');
    }

    LectureHours = datestr.toInt();
    LectureHoursFile.close();
  }

  else {
    ;
    // if the file didn't open, print an error:
    //Serial.println("error opening enumbers.txt");
  }

  //...................................................................................

 CountFile = SD.open("COUNT.txt");
  if (CountFile) {
    while (CountFile.available()) {
      datestr = CountFile.readStringUntil('\n');
    }

    Countnm = datestr.toInt();
    CountFile.close();
  }

  else {
    ;
    // if the file didn't open, print an error:
    //Serial.println("error opening enumbers.txt");
  }

Countnm=Countnm+1;

 CountFile = SD.open("count.txt", FILE_WRITE);
  // if the file is available, write to it:
  if (CountFile) {
   CountFile.println(Countnm);
    CountFile.close();
  }
  // if the file isn't open, pop up an error:
  else {
    lcd.clear();
    lcdprint("ERROR OPENING Count.txt");
 
  }
 
  dataString +=  Date;
  dataString +=  ",";
  dataString += dep;
  dataString += CourseNumber;
  dataString +=  ",";
  dataString +=  LectureHours;
  dataString +=  ",";
  dataString +=  temp3;
  dataString +=  ",";
  dataString +=  attendence;

  AttendenceFile = SD.open("marked.txt", FILE_WRITE);
  // if the file is available, write to it:
  if (AttendenceFile) {
    AttendenceFile.println(dataString);
    AttendenceFile.close();
  }
  // if the file isn't open, pop up an error:
  else {
    lcdprint("ERROR OPENING attendence.txt");
    state = 'D';
  }
  Serial.println(dataString);
}

void reset() {
  if (EEPROM.read(resetstate) == 1) {
    lcdprint("ALL READY RESET");
    state = 'D';
  }
  else {
    int idcount = EEPROM.read(idcounter);
    for (int id = 1; id < idcount; id++) {
      lcd.clear();
      lcd.print("DELETED :");
      lcd.print(id);
      deleteFingerprint(id);
      delay(50);
    }
    delay(1000);
    EEPROM.update(idcounter, 1);
    lcdprint("SUCCESSFULLY DELETED");
    state = 'D';
  }
}

void lcdprint(String name) {
  lcd.clear();
  lcd.print(name);
  delay(800);
  if (name.length() > 19) {
    for (int positionCounter = 0; positionCounter < name.length() - 19; positionCounter++) {
      // scroll one position left:
      lcd.scrollDisplayLeft();
      // wait a bit:
      delay(150);
    }
  }
  delay(500);
}

void Mainsetup() {
  // set the data rate for the sensor serial port
  finger.begin(57600);
  if (finger.verifyPassword()) {
    Serial.print("PLEASE WAIT...!");
  } else {
    lcdprint("Did not find fingerprint sensor :(");
    while (1);
  }
}

int readnumberint() {
  int num = 0;
  boolean validnum = false;
  while (1) {
    //Serial.println("waiting....");
    char c = readchar();
    if (c == 'D') {
      num = 0;
      return num;
    }

     if (c == 'R') {
      num /= 10;
      lcd.clear();
      lcd.print(num);
    
    }

    else if (isdigit(c)) {
      num *= 10;
      num += c - '0';
      validnum = true;
      lcd.clear();
      lcd.print(num);
      Serial.println(num);
    } else if (c == 'E') {
      return num;
    }
  }
}

uint32_t readDate() {
  uint32_t numD = 0;
  boolean validnumD = false;
  while (1) {
    //Serial.println("waiting....");
    char cD = readchar();
    if (cD == 'D') {
      numD = 0;
      return numD;
    }
     if (cD == 'R') {
      numD /= 10;
      lcd.clear();
      lcd.print(numD);
    
    }

    else if (isdigit(cD)) {
      numD *= 10;
      numD += cD - '0';
      validnumD = true;
      lcd.clear();
      lcd.print(numD);
      Serial.println(numD);
    } else if (cD == 'E') {
      return numD;
    }
  }
}

uint8_t readnumber(void) {
  uint8_t num = 0;
  boolean validnum = false;
  while (1) {
    char c = keypad.getKey();
    if (c == 'D') {
      num = 0;
      return num;//this is to change the state
    }

     if (c == 'R') {
      num /= 10;
      lcd.clear();
      lcd.print(num);
    
    }
    else if (isdigit(c)) {
      num *= 10;
      num += c - '0';
      validnum = true;
    }
    else if (validnum) {
      return num;
    }
  }
}

uint8_t getFingerprintEnroll(int enumber) {

  int p = -1;
  lcd.print("ENROLLMENT ");
  lcd.print(enumber);
  delay(2000);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        lcd.clear();
        lcd.print("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        lcd.clear();
        lcd.print("Put your Finger");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        lcd.clear();
        lcdprint("Commucation error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        lcd.clear();
        lcd.print("Imaging error");
        break;
      default:
        lcd.clear();
        lcd.print("Unknown error");
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      lcd.clear();
      lcd.print("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      lcd.clear();
      lcd.print("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      lcd.clear();
      lcd.println("Comuniction error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      lcd.clear();
      lcd.print("NO FP features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      lcd.clear();
      lcd.print("NO FP features");
      return p;
    default:
      lcd.clear();
      lcd.print("Unknown error");
      return p;
  }
  lcd.clear();
  lcd.print("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }

  p = -1;
  lcd.clear();
  lcd.print("PLACE SAME");
  lcd.setCursor(0, 1);
  lcd.print("FINGER AGAIN..!");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        lcd.clear();
        lcd.print("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        lcd.clear();
        lcd.print("Comuniction error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        lcd.clear();
        lcd.print("Imaging error");
        break;
      default:
        lcd.clear();
        lcd.print("Unknown error");
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      lcd.clear();
      lcd.print("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      lcd.clear();
      lcd.print("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      lcd.clear();
      lcd.print("Comuniction error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      lcd.clear();
      lcd.print("NO FP features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      lcd.clear();
      lcd.print("NO FP features");
      return p;
    default:
      lcd.clear();
      lcd.print("Unknown error");
      return p;
  }

  // OK converted!
  lcd.clear();
  lcd.print("Creating model for #");  lcd.print(id8);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    lcd.clear();
    lcd.print("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    lcd.clear();
    lcd.print("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    lcd.clear();
    lcd.print("Fingerprints ");
    lcd.setCursor(0, 1);
    lcd.print("did not match");

    return p;
  } else {
    lcd.clear();
    lcd.print("Unknown error");
    return p;
  }

  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    lcd.clear();
    lcd.print("Stored! ");
    lcd.print(enumber);
    putid(enumber);
    //Serial.println("Stored ");
    //Serial.println(enumber);

    if (issuccess == true) {
      temp = EEPROM.read(idcounter);
      Serial.println(temp);
      temp++;
      EEPROM.write(idcounter, temp);

    }
    issuccess = true;
    //Serial.println(EEPROM.read(idcounter));
    delay(1000);
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    lcd.print("Comuniction error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    lcd.print("cant store in ");
    lcd.setCursor(0, 1);
    lcd.print("that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    lcd.clear();
    lcd.print("Error writing ");
    lcd.setCursor(0, 1);
    lcd.print("to flash");

    return p;
  } else {
    lcd.clear();
    lcd.print("Unknown error");
    return p;
  }
}
// initialize the library with the numbers of the interface pins


uint8_t getFingerprintID() {
  int p = -1;
  delay(2000);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        lcd.clear();
        lcd.print("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        lcd.clear();
        lcd.print("PLACE YOUR FINGER");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        lcd.clear();
        lcdprint("Commucation error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        lcd.clear();
        lcd.print("Imaging error");
        break;
      default:
        lcd.clear();
        lcd.print("Unknown error");
        break;
    }
  }
  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted.....................................");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  // OK converted!
  p = finger.fingerFastSearch();
  Serial.println("C.............................");
  if (p == FINGERPRINT_OK) {
    //lcdprint("Found a print match!");
    //delay(1000);

  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    lcdprint("Did not find a match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  // found a match!

  lcd.clear();
  lcd.println("YOUR ATTENDANCE");
  lcd.setCursor(0, 1);
  lcd.println("IS MARKED ");
  return finger.fingerID;
}
// returns -1 if failed, otherwise returns ID #

int getFingerprintIDez() {
  delay(3000);
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  {
    Serial.print("OR 'D' TO EXIT 1");
    return -1;
  }
  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) {
    Serial.print("OR 'D' TO EXIT 2");
    return -1;
  }
  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  {
    Serial.print("OR 'D' TO EXIT 5");
    return -1;
  }
  // found a match!

  return finger.fingerID;
}

void Enrollmentloop() {
  Mainsetup();

  lcd.clear();
  lcd.print("TYPE NEW ID OR");
  lcd.setCursor(0, 1);
  lcd.print("'F4' TO EXIT :)");

  int id8 = readnumberint();
  lcd.clear();
  lcd.print(id8);

  delay(1000);

  if (id8 == 0) {
    state = 'D';

  }
  else if (id8 == 1) {
    state = 'D';
  }
  else {

    id = EEPROM.read(0);

    Serial.print(" next number  ");
    Serial.print(id);
    lcd.clear();
    while (!  getFingerprintEnroll(id8) );

    Serial.print("else end ");
  }

  Serial.print("function end");
}

  void Fingerloop()  {
  uint8_t input = getFingerprintIDez();

  //Serial.println("found a match ");
  getenum(input);
  //Serial.print ("enumber is: ");

  if (enumberfound != "") {
    temp3 = 0;
    temp3 = enumberfound.toInt();
    lcd.clear();
    lcd.print("Your Attendance");
    lcd.setCursor(0, 1);
    lcd.print("Is Marked ");

    lcd.print(temp3);
    saveAttendence(1);
  }

  else {
    lcdprint("Does not found the finger");

  }
  delay(2000);
}

void putZero() {

  EnumbersFile = SD.open("enumbers.txt");
  // if the file is available, write to it:
  if (EnumbersFile) {
    while (EnumbersFile.available()) {
      String linetext = AttendenceFile.readStringUntil('\n');
      temp6 = linetext.substring(0, 5);
      int temp9 = temp6.toInt();

      AttendenceFile = SD.open("marked.txt");
      if (AttendenceFile) {
        while (AttendenceFile.available()) {
          enumid = "0";
          String enumberfoundtemp = AttendenceFile.readStringUntil('\n');
          String temp7 =  enumberfoundtemp.substring(13, 18);

          int temp10 = temp7.toInt();

          if (temp9 == temp10) {
            Serial.println(temp9);
            enumid = temp7;

            break;
          }
        }

        int temp12 = enumid.toInt();
        if (temp12 == 0) {

          temp3 = temp12;
          saveAttendence(0);
        }
        AttendenceFile.close();
      }
      else {
        ;
        //
      }
    }
    EnumbersFile.close();
  }

  else {
    lcdprint("ERROR OPENING Enumbers.txt");
    state = 'D';
    issuccess = false;

  }
}

char readchar() {
  boolean validnum = false;
  while (1) {
    char c = keypad.getKey();
    if (c == 'A') {
      return c;//this is to change the state
    }
    else if (c == 'B') {
      return c;//this is to change the state
    }
    else if (c == 'E') {
      return c;
    }
    else if (c == 'C') {
      return c;
    } else if (c == 'D') {
      return c;
    }
    else if (isdigit(c)) {
      return c;
    }
    else if (c == 'R') {
      return c;
    }
  }
}

void setup() {

  Serial.begin(9600);
  lcd.begin(20,4);
  myConnection.begin(9600);
  Mainsetup();
  pinMode(18, OUTPUT);
  SDsetup();

  while (1) {
    switch (state) {

      case '1':
      lcd.clear();
        lcd.print("SET COURSE NUMBER:"); 
        
        CourseNumber = readnumberint();
        dataStr = String(CourseNumber);
        //SDsetup();

        CourseNumberFile = SD.open("course.txt", FILE_WRITE);
        // if the file is available, write to it:
        if (CourseNumberFile) {
          CourseNumberFile.println(dataStr);
          CourseNumberFile.close();
        }
        // if the file isn't open, pop up an error:
        else {
          lcdprint("ERROR OPENING Course_Now.txt");
          state = 'D';
        }

        // Serial.println("grfgdgdfgdg");
        // Serial.println(dataStr);

        lcdprint("SUCCESSS..!");

        break;

      case '2':
       lcd.clear();
        lcd.print("SET LECTURE HOURS");

        LectureHours = readnumberint();

        dataStr = String(LectureHours);
        //SDsetup();

        LectureHoursFile = SD.open("hours.txt", FILE_WRITE);
        // if the file is available, write to it:
        if (LectureHoursFile) {
          LectureHoursFile.println(dataStr);
          LectureHoursFile.close();
        }
        // if the file isn't open, pop up an error:
        else {
          lcdprint("ERROR OPENING hours.txt");
          state = 'D';
        }
        //Serial.println("dfdsfdfsfs");
        //  Serial.println(dataStr);
        lcdprint("SUCCESSS..!");

        break;

      case '3':
       lcd.clear();
        lcd.print("SET DATE:");
        Date = readDate();

        dataStr = String(Date);
        //SDsetup();

        DateFile = SD.open("dates.txt", FILE_WRITE);
        // if the file is available, write to it:
        if (DateFile) {
          DateFile.println(dataStr);
          DateFile.close();
        }
        // if the file isn't open, pop up an error:
        else {
          lcdprint("ERROR OPENING dates.txt");
          state = 'D';
        }
        // Serial.println("sdssfkdsjkfj");
        // Serial.println(dataStr);

        lcdprint("SUCCESSS..!");

        break;

      case '4':
 lcd.clear();
        lcd.print("COMMUNICATION MODE");
        digitalWrite(18, HIGH);
        waitbluetooth = readnumberint();
        myConnection.listen();
        Serial.println("Start");

        // SDsetup();
        AttendenceFile = SD.open("marked.txt");
        if (AttendenceFile) {
          Serial.println("OPEN THE FILE");
          int i = 0;
          // read from the file until there's nothing else in it:
          while (AttendenceFile.available()) {
            lcd.clear();
            lcd.print("Sending ");
            lcd.print(i++);
            String enumberfoundtemp = AttendenceFile.readStringUntil('\n');
            myConnection.println(enumberfoundtemp);
            delay(1000);
          }
          AttendenceFile.close();
        }
        else {
          ;
          lcd.clear();
          lcd.print("Error");
        }
        lcd.clear();
        lcd.print("Finished");
        delay(2000);
        mySerial.listen();
        digitalWrite(18, LOW);

        break;

      case 'A':
        lcd.clear();
        lcd.print("USER ENROLLMENT ");
        lcd.setCursor(0, 1);
        lcd.print("PASSWORD:");
        delay(2000);
        password = readnumberint();
        if (password == 12345) {
          Enrollmentloop() ;
          lcd.clear();
          delay(1000);
        }
        else {
          lcd.print("WRONG PASSWORD:");
          delay(1000);
        }
        break;

      case 'B':
        lcd.clear();
        lcd.print("HOLD THE FINGER");
        lcd.setCursor(0, 1);
        lcd.print("UNTIL IT BLINKS");
        Fingerloop();

        break;

      case 'C':
        lcdprint("RESET MODE");
        reset();
        break;

      case '5':
        lcd.clear();
        lcd.print("ENTER ID TO ");
        lcd.setCursor(0, 1);
        lcd.print("DELETE:");
        id8 = readnumberint();
        getID(id8);
        temp4 = idfound.toInt();
        deleteFingerprint(temp4);

        break;

      case '6':

        lcd.clear();
        lcd.print("WAIT....");
        delay(1000);
        putZero();
        lcd.clear();
        lcd.print("Success...");

        break;
 
      case '7':
        lcd.clear();
        id8=0;
        lcd.print("ENTER DEP ");
        lcd.setCursor(0, 1);
        id8 = readnumberint();
        if(id8==1){
          dep="CO";
        }

        if(id8==2){
          
          dep="EE";
          
        } 

       DepFile = SD.open("dep.txt", FILE_WRITE);
        // if the file is available, write to it:
        if (DepFile) {
          DepFile.println(dep);
         DepFile.close();
        }
        // if the file isn't open, pop up an error:
        else {
          lcdprint("ERROR OPENING dep.txt");
          state = 'D';
        }

        lcdprint("SUCCESSS..!");
      
      default:

      /*-

      String dataString = String();
          String datestr;
        
          DateFile = SD.open("dates.txt");
          if (DateFile) {
            while (DateFile.available()) {
              datestr = DateFile.readStringUntil('\n');
            }
        
            Date = datestr.toInt();
            DateFile.close();
          }
        
          else {
            // if the file didn't open, print an error:
            //Serial.println("error opening enumbers.txt");
          }
          
          //.........................................................................
          CourseNumberFile = SD.open("course.txt");
          if (CourseNumberFile) {
            while (CourseNumberFile.available()) {
              datestr = CourseNumberFile.readStringUntil('\n');
            }
        
            CourseNumber = datestr.toInt();
            CourseNumberFile.close();
          }
        
          else {
            // if the file didn't open, print an error:
            //Serial.println("error opening enumbers.txt");
          }
        
        //.........................................................................
         DepFile = SD.open("dep.txt");
          if (DepFile) {
            while (DepFile.available()) {
              datestr = DepFile.readStringUntil('\n');
            }
        
           dep = datestr;
           DepFile.close();
          }
        
          else {
            // if the file didn't open, print an error:
            //Serial.println("error opening enumbers.txt");
          }
       
          //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
        
          LectureHoursFile = SD.open("hours.txt");
          if (LectureHoursFile) {
            while (LectureHoursFile.available()) {
              datestr = LectureHoursFile.readStringUntil('\n');
            }
        
            LectureHours = datestr.toInt();
            LectureHoursFile.close();
          }
        
          else {
            ;
            // if the file didn't open, print an error:
            //Serial.println("error opening enumbers.txt");
          }
        
           CountFile = SD.open("COUNT.txt");
          if (CountFile) {
            while (CountFile.available()) {
              datestr = CountFile.readStringUntil('\n');
            }
        
            Countnm = datestr.toInt();
            CountFile.close();
          }
        
          else {
            ;
            // if the file didn't open, print an error:
            //Serial.println("error opening enumbers.txt");
          }
*/
        lcd.clear();
      
        lcd.print("HELLO I'M READY");
     
     lcd.setCursor(0, 1);
     lcd.print("DATE : ");
      // lcd.print(Date);
       lcd.setCursor(0, 2);
      lcd.print("COURSE CODE : ");
       //lcd.print(dep);
       // lcd.print(CourseNumber);
         lcd.print(" : ");
        // lcd.print(LectureHours);
          lcd.print("h");
         
        lcd.setCursor(0, 3);
       lcd.print("STUDENT COUNT :");
      // lcd.print(Countnm);
      
        break;
    }
    state = readchar();
    delay(50);
  }
}

void loop() {}
