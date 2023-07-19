/*  
 * Code by Dichill Tomarong
 * For HadesArduino Software
*/

/* 
 * Download these libraries:
 * DF_Robot_DHT11 (Optional) [Don't forget to comment it out]
 * Linked List (Required)
 * MFRC522 (Required)
 * SPI (Required)
*/
#include <ArduinoJson.h>
#include <LinkedList.h>
#include <MFRC522.h>
#include <SPI.h>

/* 
 * UNLOCKEDL_PIN = GREEN_LED
 * READYL_PIN = YELLOW_LED
 * STANDBYL_PIN = RED_LED
*/
#define UNLOCKEDL_PIN 4
#define READYL_PIN 5
#define STANDBYL_PIN 6

/* 
 * DHT11 is optional, you can simply comment this one out.
 */
#define DHT11PIN 2

/* 
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 *
 * More pin layouts for other boards can be found here: https://github.com/miguelbalboa/rfid#pin-layout
*/
#define SS_PIN 10
#define RST_PIN 9

/* 
 * Iniate Instance
*/
MFRC522 mfrc522 (SS_PIN, RST_PIN);

/* 
 * These are the required string in order to communicate with HadesArduino
 * Do not make changes unless you know what to modify
*/
String toRecord = "";
String toRegister = "";
String inputString = ""; 
String commandString = "";

/* 
 * Required boolean for checking, you'll see this a lot in the code.
*/
boolean stringComplete = false;
boolean registerMode = false;
boolean lockMode = false;

/*
 * For Json parsing using:
 *  Linkedlist
 *  ArduinoJson
 */
const int BUFFER_SIZE = 256;
char buffer[BUFFER_SIZE];

LinkedList<String> dataList;

void setup() {
  /* 
   * Begin Serial Communication with the Baudrate of 19200
   * 19200 bit => 2400 byte
   * 128 ms / 2400 byte = 5ms of Serial Communication  

   [WARNING]
   Do not change the value of Serial.begin(); as HadesArduino only accepts that value!
   If you change it, it will not be able to read from the Arduino!
  */
  Serial.begin(19200);
  Serial.println("00.00;00.00");

  pinMode(UNLOCKEDL_PIN, OUTPUT);
  pinMode(READYL_PIN, OUTPUT);
  pinMode(STANDBYL_PIN, OUTPUT);

  SPI.begin();
  mfrc522.PCD_Init();

  serialEvent();

  lockMode = true;
  LockState();
}

void loop() {
  if (stringComplete) {
      stringComplete = false;
      
      getCommand();
      commandSys();
      
      inputString = "";
  }

  /* 
   * IMPORTANT
   * Make sure that toRegister & toRecord will be in the 2nd `,`
   * [ToRegister is only called once, thus the reason why they are fine if both of them are at the 2nd comma `,`]
   * e.g. data1, data2, toRegister_value,
   * e.g. data1, data2, toRecord_value,
  */
  Serial.print("0");
  Serial.print(",");

  Serial.print("0");
  Serial.print(",");

  if (toRegister != "") {
    Serial.print(toRegister);
    Serial.print(",");
    toRegister = "";
  }

  if (toRecord != "") {
    Serial.print(toRecord);
    Serial.print(",");
    toRecord = "";
  }

  Serial.println();

  delay(1000);

  /* 
     * Do not add code below if it does not relate to the RFID
     * Here, the if statements will restart the loop if no card is present
    */
  if (lockMode) {
    LockState();
    
    if ( ! mfrc522.PICC_IsNewCardPresent()) return;
    if ( ! mfrc522.PICC_ReadCardSerial()) return;

    String content= "";
    byte letter;
    for (byte i = 0; i < mfrc522.uid.size; i++) 
    {
      content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
      content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }

    content.toUpperCase();

    for (int i = 0; i < dataList.size(); i++) {
      if (dataList.get(i) == content.substring(1)) {
          UnlockState();
          lockMode = false;
          toRecord = content.substring(1);

          delay(1000);

          lockMode = true;
      }
    }
  } else if (registerMode) {
    RegisterState();

    if ( ! mfrc522.PICC_IsNewCardPresent()) return;
    if ( ! mfrc522.PICC_ReadCardSerial()) return;

    String content= "";
    byte letter;
    for (byte i = 0; i < mfrc522.uid.size; i++) 
    {
      content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
      content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }

    content.toUpperCase();

    toRegister = content.substring(1);
    UnlockState();

    delay(1000);

    dataList.add(toRegister);

    lockMode = true;
    registerMode = false;
  }
}


#pragma region LED STATE
void TurnLed(int pin, uint8_t value) {
  digitalWrite(pin, value);
}

void LockState() {
  TurnLed(UNLOCKEDL_PIN, LOW);
  TurnLed(READYL_PIN, LOW);
  TurnLed(STANDBYL_PIN, HIGH);
}

void UnlockState() {
  TurnLed(UNLOCKEDL_PIN, HIGH);
  TurnLed(READYL_PIN, LOW);
  TurnLed(STANDBYL_PIN, LOW);
}

void RegisterState() {
  TurnLed(UNLOCKEDL_PIN, LOW);
  TurnLed(READYL_PIN, HIGH);
  TurnLed(STANDBYL_PIN, LOW);
}

boolean getLedState()
{
  boolean state = false;
  if(inputString.substring(5,7).equals("ON"))
  {
    state = true;
  }else
  {
    state = false;
  }
  return state;
}
#pragma endregion

#pragma region COMMANDSYS
void commandSys() {
  if(commandString.equals("LED1")) {
      boolean LedState = getLedState();

      if(LedState == true)
      {
          TurnLed(UNLOCKEDL_PIN, HIGH);
      } else
      {
          TurnLed(UNLOCKEDL_PIN, LOW);
      }   

  } else if (commandString.equals("LED2")) {
    boolean LedState = getLedState();

    if(LedState == true)
    {
        TurnLed(READYL_PIN, HIGH);
    } else
    {
        TurnLed(READYL_PIN, LOW);
    }   

  } else if (commandString.equals("LED3")) {
      boolean LedState = getLedState();

      if(LedState == true)
      {
          TurnLed(STANDBYL_PIN, HIGH);
      } else {
          TurnLed(STANDBYL_PIN, LOW);
      }   

  } else if (commandString.equals("REGI")) {
    lockMode = false;
    registerMode = true;
  }
}
#pragma endregion

#pragma region SERIAL COMMUNICATION
void getCommand()
{
  if(inputString.length()>0)
  {
    if (inputString.substring(0,1) == "#")
    {
      commandString = inputString.substring(1,5);
    } 
    
    else if (inputString.substring(0,1) == "$") {
      StaticJsonDocument<BUFFER_SIZE> jsonDocument;
      DeserializationError error = deserializeJson(jsonDocument, inputString.c_str() + 1); // Skip the first character '$'

      if (error) {
        Serial.println("Error parsing JSON");
      } else {
        // Clear the existing LinkedList before adding new elements
        dataList.clear();
      } 

      // Extract and store the data values in the LinkedList
      JsonArray dataArray = jsonDocument.as<JsonArray>();
      for (JsonVariant dataValue : dataArray) {
        if (dataValue.is<String>()) {
          String dataString = dataValue.as<String>();
          dataString.trim(); // Remove leading and trailing whitespaces
          // Add the data value to the LinkedList
          dataList.add(dataString);
        }
      }
    }
    
    // Remove RFID
    else if (inputString.substring(0, 1) == "^") {
      String rfid_data = inputString.substring(1, 12);
      removeStringFromList(rfid_data);
    }
  }
}

void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    inputString += inChar;

    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}
#pragma endregion

#pragma region JSON PARSING
void removeStringFromList(const String& value) {
  for (int i = 0; i < dataList.size(); i++) {
    if (dataList.get(i) == value) {
      dataList.remove(i);
      break;
    }
  }
}
#pragma endregion
