// Arduino sketch: Toggle between Sniffer and Receiver mode on a 20x4 I2C LCD
#include <RCSwitch.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

RCSwitch mySwitch = RCSwitch();
LiquidCrystal_I2C lcd(0x27, 20, 4);

const int buttonPin = 7;
int modeCount = 0;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

const uint8_t MAX_CODES = 20;

uint8_t codeCount = 0;
uint32_t codes[MAX_CODES];

void saveCodes() {
  EEPROM.update(0, codeCount);
  for (uint8_t i = 0; i < codeCount; i++) {
    int addr = 1 + i * sizeof(uint32_t);
    EEPROM.put(addr, codes[i]);
  }
}

void loadCodes() {
  codeCount = EEPROM.read(0);
  if (codeCount > MAX_CODES) codeCount = MAX_CODES;
  for (uint8_t i = 0; i < codeCount; i++) {
    int addr = 1 + i * sizeof(uint32_t);
    EEPROM.get(addr, codes[i]);
  }
}

bool isKnown(uint32_t v) {
  for (uint8_t i = 0; i < codeCount; i++) {
    if (codes[i] == v) return true;
  }
  return false;
}

void setup() {
  // Initialize LCD
  lcd.init();
  lcd.backlight();

  // Print static header
  lcd.setCursor(0, 0);
  lcd.print("  RF PROJECT BY T  ");

  // RF receiver setup
  mySwitch.enableReceive(0);  // Interrupt 0 => digital pin 2

  // Button input with pullup
  pinMode(buttonPin, INPUT_PULLUP);
  lastButtonState = digitalRead(buttonPin);
}

void loop() {
  // Read and debounce button
  int reading = digitalRead(buttonPin);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    static bool buttonState = HIGH;
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) {
        modeCount++;
      }
    }
  }
  lastButtonState = reading;

  lcd.setCursor(0, 1);
  if (modeCount % 2 == 0) {
    lcd.print("   RECEIVER MODE    ");
    if(mySwitch.available()) {
      rec();
    }
  } else {
    lcd.print("    SNIFFER MODE    ");
    if(mySwitch.available()) {
      sniff();
    }
  }

  delay(100);
}

void rec() {
  int val = mySwitch.getReceivedValue();
  int bits = mySwitch.getReceivedBitlength();
  int pro = mySwitch.getReceivedProtocol();


  bool known = isKnown(val);
  lcd.setCursor(0, 3);
  if (known) {
    lcd.print("       KNOWN       ");
  } else {
    lcd.print("     NOT FOUND     ");
  }
}

void sniff() {
  int val = mySwitch.getReceivedValue();
  int bits = mySwitch.getReceivedBitlength();
  int pro = mySwitch.getReceivedProtocol();

  if (!isKnown(val)) {
    if (codeCount < MAX_CODES) {
      codes[codeCount++] = val;
      saveCodes();
      lcd.setCursor(0, 3);
      lcd.print("   Learned: ");
      lcd.print(val, HEX);
    } else {
      lcd.setCursor(0, 3);
      lcd.print("  Memory Full!    ");
    }
  }
}