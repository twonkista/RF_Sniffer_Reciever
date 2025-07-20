int buttonState = 0;
int lastButtonState = HIGH;
int count = 0;
bool pressed = false;

void setup() {
  // put your setup code here, to run once:
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(7, INPUT_PULLUP); //button pin
}

void loop() {
  // put your main code here, to run repeatedly:
  buttonState = digitalRead(7);

  if(buttonState == LOW &&  !pressed) {
    pressed = true;
    count++;
    delay(50);  
  }

   if (buttonState == HIGH && pressed) {
    pressed = false;
    delay(50);  // Debounce
  }

  if(count %2 == 0) { 
    work();
  }
  else {
    lean();
  }
}

void work() {
  digitalWrite(12, HIGH);
  digitalWrite(13, LOW);
}

void lean() {
  digitalWrite(13, HIGH);
  digitalWrite(12, LOW);
}
