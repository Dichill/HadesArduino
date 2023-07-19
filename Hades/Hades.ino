#include <DFRobot_DHT11.h>
#include <ArduinoJson.h>
#include <LinkedList.h>
#include <MFRC522.h>
#include <SPI.h>

// LED Pins
#define UNLOCKEDL_PIN 4
#define READYL_PIN 5
#define STANDBYL_PIN 6

// Sensor Pins
#define DHT11PIN 2

// RFID
#define SS_PIN 10
#define RST_PIN 9

// Create Instance
DFRobot_DHT11 DHT;
MFRC522 mfrc522 (SS_PIN, RST_PIN);

String inputString = ""; 
boolean stringComplete = false;
boolean lockMode = false;
boolean registerMode = false;
String commandString = "";
String toRegister = "";

const int BUFFER_SIZE = 256; // Adjust the buffer size based on your JSON data size
char buffer[BUFFER_SIZE];

LinkedList<String> dataList;

void setup() {
  // put your setup code here, to run once:
  
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
      } else
      {
          TurnLed(STANDBYL_PIN, LOW);
      }   
    } else if (commandString.equals("REGI")) {
      lockMode = false;
      registerMode = true;
    }
      inputString = "";
  }

  DHT.read(DHT11PIN);

  float humidity = DHT.humidity;
  float temperature = DHT.temperature;

  if (temperature != -1 && humidity != -1)
    {
        Serial.print(temperature);
        Serial.print(",");

        Serial.print(humidity);
        Serial.print(",");

        if (toRegister != "") {
          Serial.print(toRegister);
          Serial.print(",");
          toRegister = "";
        }
        Serial.println();

        delay(1000);
    }
    else
    {
        Serial.println("Error reading data");
    }

    if (lockMode) {
      LockState();
      // Do not add code below if it does not relate to the RFID
      // Here, the if statements will restart the loop if no card is present
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

            delay(1000);

            lockMode = true;
        }
      }
      

    } else if (registerMode) {
      RegisterState();

      // Do not add code below if it does not relate to the RFID
      // Here, the if statements will restart the loop if no card is present
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
    // else if (inputString.substring(0, 1) == "^") {
    //   String rfid_data = inputString.substring(1, 12);
    //   removeStringFromList(rfid_data);
    //   inputString = "";
    // }
  }
}

void TurnLed(int pin, uint8_t value) {
  digitalWrite(pin, value);
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

void removeStringFromList(const String& value) {
  for (int i = 0; i < dataList.size(); i++) {
    if (dataList.get(i) == value) {
      dataList.remove(i);
      break;
    }
  }
}