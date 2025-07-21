#define MAX_SLOTS 4
#define COUNT_ADDR 0      // EEPROM address to store count
#define DATA_ADDR_START 1 // EEPROM address where data starts
#include <RCSwitch.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

RCSwitch mySwitch = RCSwitch();
LiquidCrystal_I2C lcd(0x27, 20,4);
int address = 0;

const int buttonPin = 12;
int modeCount = 0;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay1 = 50;

unsigned long lastReceivedTime = 0;
const unsigned long debounceDelay = 1500;  // ms
unsigned long lastReceivedValue = 0;

static int slotIndex = 0;  

void setup() {
  clearEEPROM();
  address = EEPROM.read(COUNT_ADDR);
  if (address == 0xFF) {
    address = 0;
    EEPROM.write(COUNT_ADDR, 0);  // Initialize EEPROM count to 0
  }

  lcd.init();
  lcd.backlight();
  lcd.print("  RF PROJECT BY T  ");
  pinMode(buttonPin, INPUT_PULLUP);
  lastButtonState = digitalRead(buttonPin);
  Serial.begin(9600);
  mySwitch.enableReceive(0);  // Interrupt 0 = digital pin 2 on Uno
}

bool inEEPROM (unsigned long val) {
  byte count = EEPROM.read(COUNT_ADDR);

  for(int i = 0; i < count; i++) {
    byte readback;
    unsigned long storedVal;
    EEPROM.get(DATA_ADDR_START + i * sizeof(unsigned long), storedVal);
    if (storedVal == val) return true;
  }
  return false;
}

void clearEEPROM() {
  for (int i = 0; i < EEPROM.length(); i++) {
    EEPROM.write(i, 0xFF);  // Or 0x00 if you prefer
  }
}

void loop() {
  int reading = digitalRead(buttonPin);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay1) {
    static bool buttonState = HIGH;
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) {
        modeCount++;
      }
    }
  }
  lastButtonState = reading;

  if (modeCount % 2 == 0) {
    lcd.setCursor(0, 3);
    lcd.print("   SNIFFER MODE    ");
    if(mySwitch.available()) {
      if (mySwitch.available()) {
      unsigned long receivedValue = mySwitch.getReceivedValue();
      unsigned long now = millis();
        // Process only if new code or enough time has passed since last same code
        if (receivedValue != lastReceivedValue || (now - lastReceivedTime) > debounceDelay) {
          lastReceivedValue = receivedValue;
          lastReceivedTime = now;
            String message = "  Received " + String(receivedValue) + "  ";
            message += String(' ', 20 - message.length()); // Pad with spaces to fill entire line
            lcd.setCursor(0, 0); 

            float normalized = (float)receivedValue / 2149714.0; 
            byte result = normalized * 255;

            if(address < MAX_SLOTS && !inEEPROM(result)) {
              EEPROM.put(DATA_ADDR_START + address * sizeof(unsigned long), result);
              address++;               
              EEPROM.update(COUNT_ADDR, address); 
            }
            else if (inEEPROM(result)) {
              lcd.setCursor(0, 2); 
              lcd.print("    EEPROM DUPE         ");
            }
            else {
              lcd.setCursor(0, 2); 
              lcd.print("    EEPROM FULL         ");
            }

            lcd.setCursor(0, 1);  // Top-left of LCD
            lcd.print(message);
            }
            mySwitch.resetAvailable();
        }
      }
    } 
    else {
      lcd.setCursor(0, 3);
      lcd.print("    RECIEVER MODE    ");
      if(mySwitch.available()) {
        unsigned long receivedSignal = mySwitch.getReceivedValue();

        if (slotIndex < MAX_SLOTS) {
          EEPROM.put(slotIndex * sizeof(unsigned long), receivedSignal);
          slotIndex++; 
        }
        for (int i = 0; i < MAX_SLOTS; i++) {
          unsigned long storedSignal;
          EEPROM.get(i * sizeof(unsigned long), storedSignal);

          if (receivedSignal == storedSignal) {
              lcd.setCursor(0, 2);
              if (i == 0) {
                  lcd.print("      SIGNAL 1      ");
              } else if (i == 1) {
                  lcd.print("      SIGNAL 2      ");
              } else if (i == 2) {
                  lcd.print("      SIGNAL 3      ");
              }
          }
      }
      mySwitch.resetAvailable(); 
    }
  }
}