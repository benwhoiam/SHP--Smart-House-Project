#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <list>
#include <algorithm>

#include <ESP32Servo.h>
#define ROW_NUM     4 // four rows
#define COLUMN_NUM  4 // four columns
#define mySerial Serial1

Servo servo;
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial2);
uint8_t id;
const int MAX_SIZE = 20;
char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

class IntList {
  private:
    int myList[MAX_SIZE] = {0};
    int count = 0;

  public:
    int getCout(){
      return this->count;
    }
    void printList() const {
        Serial.print("{");
        for (int i = 0; i < MAX_SIZE; i++) {
            Serial.print(myList[i]);
            if (i < MAX_SIZE - 1) {
                Serial.print(",");
            }
        }
        Serial.print("}\n");
    }

    int countZeros() const {
        int zeroCount = 0;
        for (int i = 0; i < MAX_SIZE; i++) {
            if (myList[i] == 0) {
                zeroCount++;
            }
        }
        return zeroCount;
    }

    void addToList(int value) {
        if (count < MAX_SIZE) {
            myList[count++] = value;
            Serial.println(((String)value) + " added to the list."); 
        } else {
            Serial.println("List is full.");
        }
    }

    bool isInList(int value) const {
        return std::find(myList, myList + count, value) != myList + count;
    }

    void removeFromList(int value) {
        auto it = std::find(myList, myList + count, value);
        if (it != myList + count) {
            std::copy(it + 1, myList + count, it);
            myList[--count] = 0;
            Serial.println(((String)value) + " removed from the list."); 
        } else {
            Serial.println(((String)value) + " not found in the list."); 
        }
    }
};

IntList peopleInRoom;

byte pin_rows[ROW_NUM]      = {18, 5, 12, 14}; 
byte pin_column[COLUMN_NUM] = {4,0,13, 15};   
LiquidCrystal_I2C lcd(0x27,16,2);  
const String Password = "5912";
Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );
void beep(int beeps, int time){
  for(int i = 0; i < beeps; i++){
    digitalWrite(19, HIGH);
    delay(50);
    digitalWrite(19, LOW);
    delay(50);
  }
}



void WriteInLCD(String str){
  lcd.clear();
  delay(100);
  int len = str.length();
  lcd.setCursor(0, 0);
  if(len > 16){
    lcd.print(str.substring(0,16));
    lcd.setCursor(0, 1);
    lcd.print(str.substring(16,len));
    return;
  }
  lcd.print(str);
}

String padInput(){
  String str;
  char key;
  while(true)
  {
    key = keypad.getKey();
    if (key) {
      if(key == '*') break;
      str+=key;
      WriteInLCD(str);
    }
  }
  return str;
}
uint8_t readnumber() {
  
  return (uint8_t) padInput().toInt();
}
void saveFingerPrint(){
  WriteInLCD("Please enter theID followed by *");
  id = readnumber();
  WriteInLCD("Enrolling ID "+((String)id));

  while (!  getFingerprintEnroll() );
}
uint8_t getFingerprintEnroll() {

  int p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      WriteInLCD("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      WriteInLCD("Place Finger");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      WriteInLCD("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      WriteInLCD("Imaging error");
      break;
    default:
      WriteInLCD("Unknown error");
      break;
    }
  }
  delay(500);
  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      WriteInLCD("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      WriteInLCD("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      WriteInLCD("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      WriteInLCD("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      WriteInLCD("Could not find fingerprint features");
      return p;
    default:
      WriteInLCD("Unknown error");
      return p;
  }
  delay(500);
  WriteInLCD("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  p = -1;
  WriteInLCD("Place same      finger again");

  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      WriteInLCD("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      WriteInLCD("Communication error");  delay(1000);
      break;
    case FINGERPRINT_IMAGEFAIL:
      WriteInLCD("Imaging error");  delay(1000);
      break;
    default:
      WriteInLCD("Unknown error");  delay(1000);
      break;
    }
  }delay(500);

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      WriteInLCD("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      WriteInLCD("Image too messy");  delay(1000);
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      WriteInLCD("Communication error");  delay(1000);
      return p;
    case FINGERPRINT_FEATUREFAIL:
      WriteInLCD("Could not find fingerprint features");  delay(1000);
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      WriteInLCD("Could not find fingerprint features");  delay(1000);
      return p;
    default:
      WriteInLCD("Unknown error");
      return p;
  }

  WriteInLCD("Creating model  for # " +((String)id));  

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    WriteInLCD("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    WriteInLCD("Communication error");  delay(1000);
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    WriteInLCD("Fingerprints did not match");
      delay(1000);
    return p;
  } else {
    WriteInLCD("Unknown error");  delay(1000);
    return p;
  }
  delay(1000);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    WriteInLCD(((String)id)+" Stored!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    WriteInLCD("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    WriteInLCD("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    WriteInLCD("Error writing to flash");
    return p;
  } else {
    WriteInLCD("Unknown error");
    return p;
  }
  delay(1000);

  return true;
}
void setupLCD(){
  lcd.init();      
  lcd.backlight();
  lcd.clear();
}
void setupFingerPrint(){
  while (!Serial);  
  delay(100);
  finger.begin(57600);
  if (finger.verifyPassword()) {
  } else {
    while (1) { delay(1); }
  }
}
uint8_t findMatchId() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      WriteInLCD("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      WriteInLCD("No finger detected"); delay(1000);
      return 0;
    case FINGERPRINT_PACKETRECIEVEERR:
      WriteInLCD("Communication error");delay(1000);
      return 0;
    case FINGERPRINT_IMAGEFAIL:
      WriteInLCD("Imaging error");delay(1000);
      return 0;
    default:
      WriteInLCD("Unknown error");delay(1000);
      return 0;
  }
  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      WriteInLCD("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      WriteInLCD("Image too messy");delay(1000);
      return 0;
    case FINGERPRINT_PACKETRECIEVEERR:
      WriteInLCD("Communication error");delay(1000);
      return 0;
    case FINGERPRINT_FEATUREFAIL:
      WriteInLCD("Could not find fingerprint features");delay(1000);
      return 0;
    case FINGERPRINT_INVALIDIMAGE:
      WriteInLCD("Could not find fingerprint features");delay(1000);
      return 0;
    default:
      WriteInLCD("Unknown error");delay(1000);
      return 0;
  }

  // OK converted!
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    WriteInLCD("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    WriteInLCD("Communication error");delay(1000);
    return 0;
  } else if (p == FINGERPRINT_NOTFOUND) {
    WriteInLCD("Did not find a match");delay(1000);
    return 0;
  } else {
    WriteInLCD("Unknown error");delay(1000);
    return 0;
  } 

  delay(1000);
  return finger.fingerID;
}
void choice1(){
 WriteInLCD("Enter Code ");
    String codeInput = padInput();
    if(codeInput == Password){
      saveFingerPrint();
    }
    else{
      WriteInLCD("Incorrect !");beep(3,50);
      delay(500);
    }
}
void choice2(){
   int i=0;
    while(i<6){
      delay(1000);
      WriteInLCD("Scaning ...");
      uint8_t p = findMatchId();
      if(p){
        WriteInLCD("Authorized ID "+((String) p));beep(1,2000);
        for(int j= 90 ; j>0; j--){

        servo.write(j); 
        delay(10);
        }
        if(!peopleInRoom.isInList(p))
          {
            peopleInRoom.addToList(p);
            WriteInLCD("EnterRoom");delay(2000);
            /* if(peopleInRoom.getCout() == 1 ){
              sendJson(true,(int)p,1);
            }  */
            Serial.println("1," +((String)p) +",1" );
            //sendJson(true,(int)p,1);
          }
        else{
          peopleInRoom.removeFromList(p);
          WriteInLCD("LeftRoom");delay(2000);
          if(peopleInRoom.getCout() == 0 ){
            Serial.println("0," +((String)p) +",1" );
            //sendJson(false,(int)p,0);
          }
          Serial.println("1," +((String)p) +",0" );
          //sendJson(true,(int)p,0);
        }
        for(int j= 0 ; j<90; j++){

        servo.write(j); 
        delay(10);
        }
        break;
      };
      beep(3,50);
      i++;}
}














void setup(){
  Serial.begin(9600);
  setupLCD();
  setupFingerPrint(); 
  Serial.println("0,1,2");
  pinMode(19, OUTPUT);
  servo.attach(25);

}
void loop()                     
{
  WriteInLCD("1- Add Finger   2- Open Door");
  delay(1000);
  uint8_t choice = readnumber();
  if(choice ==1){
   choice1();
  }
  else if(choice ==2){
   choice2();
  }
  
  delay(500);
}

