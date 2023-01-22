const int RESET_BUTTON           = 2;
const int RESET_BUTTON_GREEN_LED = 3;
const int RESET_BUTTON_RED_LED   = 4;

const int MAIN_BUTTONS_INPUT_LOAD      = 8;
const int MAIN_BUTTONS_INPUT_CLOCK     = 7;
const int MAIN_BUTTONS_INPUT_DATA      = 6;

int loop_count = 0;
int reset_button_state = 0;

uint16_t main_buttons_state = 0;

void printBin(uint16_t b) {
  for (int i = 15; i >= 0; i--) {
    Serial.write(bitRead(b, i) ? '1' : '0');
  }
}

// Read the state of the main nine buttons from the daisy-chained
//two 74HC165 shift registers.
uint16_t readMainButtonsState()
{
  uint16_t data = 0;

  // Output a pulse to the load pin (PL) of the 74HC165 shift registers
  // so that they read the input states.
  digitalWrite(MAIN_BUTTONS_INPUT_LOAD, LOW);
  delayMicroseconds(5);
  digitalWrite(MAIN_BUTTONS_INPUT_LOAD, HIGH);
  delayMicroseconds(5);

  // Read the first bit from the 74HC165 shift registers.
  data = digitalRead(MAIN_BUTTONS_INPUT_DATA);
  delayMicroseconds(5);

  // Read the rest of the bits from the 74HC165 shift reigsters.
  for (int i = 1; i < 16; i++) {
    digitalWrite(MAIN_BUTTONS_INPUT_CLOCK, HIGH);
    delayMicroseconds(5);
    digitalWrite(MAIN_BUTTONS_INPUT_CLOCK, LOW);
    delayMicroseconds(5);
    data = (data << 1) | (digitalRead(MAIN_BUTTONS_INPUT_DATA));
  }

  return data;
}

void setup() {
  Serial.begin(57600);
  Serial.println("setup");

  // Digital I/O setup for the reset button and its LEDs.
  pinMode(RESET_BUTTON,           INPUT);
  pinMode(RESET_BUTTON_GREEN_LED, OUTPUT);
  pinMode(RESET_BUTTON_RED_LED,   OUTPUT);

  // Digital I/O setup for the shift register 74HC165 that
  // reads the states of the main nine buttons.
  pinMode(MAIN_BUTTONS_INPUT_LOAD,  OUTPUT);
  pinMode(MAIN_BUTTONS_INPUT_CLOCK, OUTPUT);
  pinMode(MAIN_BUTTONS_INPUT_DATA,  INPUT);
}

void loop() {
  loop_count = loop_count + 1;
  Serial.print("loop: ");
  Serial.println(loop_count, DEC);

  // Read the status of the reset button.
  reset_button_state = digitalRead(RESET_BUTTON);
  if (reset_button_state == HIGH) {
    Serial.println("Reset button: high");
  } else {
    Serial.println("Reset button: low");
  }

  // Flush LEDs in the reset button.
  if (loop_count % 2 == 0) {
    digitalWrite(RESET_BUTTON_GREEN_LED, HIGH);
    digitalWrite(RESET_BUTTON_RED_LED,   LOW);
  } else {
    digitalWrite(RESET_BUTTON_GREEN_LED, LOW);
    digitalWrite(RESET_BUTTON_RED_LED,   HIGH);
  }

  // Read the state of the main nine buttons.
  main_buttons_state = readMainButtonsState();
  printBin(main_buttons_state);
  Serial.println("");

  delay(1000);
}
