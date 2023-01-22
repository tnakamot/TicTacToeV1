/*
 *    IOTest.ino - An Arduino Nano program to test input and output for Tic-Tac-Toe V1.
 *    Copyright (c) 2023 Takashi Nakamoto
 * 
 *    Permission is hereby granted, free of charge, to any person
 *    obtaining a copy of this software and associated documentation
 *    files (the "Software"), to deal in the Software without
 *    restriction, including without limitation the rights to use,
 *    copy, modify, merge, publish, distribute, sublicense, and/or sell
 *    copies of the Software, and to permit persons to whom the
 *    Software is furnished to do so, subject to the following
 *    conditions:
 * 
 *    This permission notice shall be included in all copies or 
 *    substantial portions of the Software.
 * 
 *    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *    OTHER DEALINGS IN THE SOFTWARE.
 */

#include "LedControl.h"

const int RESET_BUTTON           = 2;
const int RESET_BUTTON_GREEN_LED = 3;
const int RESET_BUTTON_RED_LED   = 4;

const int MAIN_BUTTONS_INPUT_LOAD      = 8;
const int MAIN_BUTTONS_INPUT_CLOCK     = 7;
const int MAIN_BUTTONS_INPUT_DATA      = 6;

const int MAX7219_CLK  = 11;
const int MAX7219_LOAD = 10;
const int MAX7219_DIN  = 9;

int loop_count = 0;
int reset_button_state = 0;

uint16_t main_buttons_state = 0;

LedControl leds = LedControl(MAX7219_DIN, MAX7219_CLK, MAX7219_LOAD, 1);

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

  // Pin configuraiton for the shift register 74HC165 that
  // reads the states of the main nine buttons.
  pinMode(MAIN_BUTTONS_INPUT_LOAD,  OUTPUT);
  pinMode(MAIN_BUTTONS_INPUT_CLOCK, OUTPUT);
  pinMode(MAIN_BUTTONS_INPUT_DATA,  INPUT);

  // Set up MAX7219.
  leds.shutdown(0, false);
  leds.setIntensity(0, 12);
  leds.clearDisplay(0);
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

  // Flash LEDs in the reset button.
  if (loop_count % 2 == 0) {
    digitalWrite(RESET_BUTTON_GREEN_LED, HIGH);
    digitalWrite(RESET_BUTTON_RED_LED,   LOW);
  } else {
    digitalWrite(RESET_BUTTON_GREEN_LED, LOW);
    digitalWrite(RESET_BUTTON_RED_LED,   HIGH);
  }

  // Read the state of the main nine buttons.
  main_buttons_state = readMainButtonsState();
  for (int i = 0; i < 9; i++) {
    Serial.print("S");
    Serial.print(i + 1);
    Serial.print(" : ");
    Serial.println(bitRead(main_buttons_state, i) ? "high" : "low");
    
  }

  // Flash LEDs.
  leds.clearDisplay(0);
  if (loop_count % 9 == 0) {
    // Turn on RED on S1 and GREEN on S9.
    leds.setRow(0, 0, B01000000);
    leds.setRow(0, 5, B00000010);
  } else if (loop_count % 9 == 1) {
    // Turn on RED on S2 and GREEN on S1.
    leds.setRow(0, 0, B00100000);
    leds.setRow(0, 3, B00001000);
  } else if (loop_count % 9 == 2) {
    // Turn on RED on S3 and GREEN on S2.
    leds.setRow(0, 0, B00010000);
    leds.setRow(0, 3, B00000100);
  } else if (loop_count % 9 == 3) {
    // Turn on RED on S4 and GREEN on S3.
    leds.setRow(0, 1, B01000000);
    leds.setRow(0, 3, B00000010);
  } else if (loop_count % 9 == 4) {
    // Turn on RED on S5 and GREEN on S4.
    leds.setRow(0, 1, B00100000);
    leds.setRow(0, 4, B00001000);
  } else if (loop_count % 9 == 5) {
    // Turn on RED on S6 and GREEN on S5.
    leds.setRow(0, 1, B00010000);
    leds.setRow(0, 4, B00000100);
  } else if (loop_count % 9 == 6) {
    // Turn on RED on S7 and GREEN on S6.
    leds.setRow(0, 2, B01000000);
    leds.setRow(0, 4, B00000010);
  } else if (loop_count % 9 == 7) {
    // Turn on RED on S8 and GREEN on S7.
    leds.setRow(0, 2, B00100000);
    leds.setRow(0, 5, B00001000);
  } else if (loop_count % 9 == 8) {
    // Turn on RED on S9 and GREEN on S8.
    leds.setRow(0, 2, B00010000);
    leds.setRow(0, 5, B00000100);
  }

  Serial.println("-----");

  delay(1000);
}
