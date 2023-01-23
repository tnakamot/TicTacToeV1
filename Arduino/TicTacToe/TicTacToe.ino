/*
 *    TicTacToe.ino - Arudino Nano program to run Tic-Tac-Toe game.
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

/* ===================================================================
 * Digital I/O pin assignment of the Arduino Nano.
 * =================================================================== */
const int RESET_BUTTON             = 2;
const int RESET_BUTTON_GREEN_LED   = 3;
const int RESET_BUTTON_RED_LED     = 4;

const int MAIN_BUTTONS_INPUT_LOAD  = 8;
const int MAIN_BUTTONS_INPUT_CLOCK = 7;
const int MAIN_BUTTONS_INPUT_DATA  = 6;

const int MAX7219_CLK              = 11;
const int MAX7219_LOAD             = 10;
const int MAX7219_DIN              = 9;

/* ===================================================================
 * Program settings.
 * =================================================================== */

// Serial port baud rate for debugging.
const int32_t SERIAL_BAUD_RATE = 57600;

/* ===================================================================
 * Global variables.
 * =================================================================== */

// A variable to hold the main state of this program.
enum MainState { START, PLAYING, RED_WIN, GREEN_WIN } main_state = START;

// An object that represents the LED controller (MAX7219).
LedControl leds = LedControl(MAX7219_DIN, MAX7219_CLK, MAX7219_LOAD, 1);

/* ===================================================================
 * readMainButtonsRawState()
 *
 * This function read the state of the main nine buttons (S1 - S9)
 * from the daisy-chained two 74HC165 shift registers.
 *
 * bitRead(state, i) represents the press/release state of the S(i+1)
 * button where "state" is the value returned by this function. If it
 * is 1, it means that the button is currently pressed.
 *
 * Example:
 *   uint16_t state = readMainButtonsRawState();
 *   Serial.println( bitRead(state, 0) ? "Button S1 is pressed." : "Button S1 is released." );
 * =================================================================== */
uint16_t readMainButtonsRawState()
{
  uint16_t state = 0;

  // Output a pulse to the load pin (PL) of the 74HC165 shift registers
  // so that they read the input states.
  digitalWrite(MAIN_BUTTONS_INPUT_LOAD, LOW);
  delayMicroseconds(5);
  digitalWrite(MAIN_BUTTONS_INPUT_LOAD, HIGH);
  delayMicroseconds(5);

  // Read the first bit from the 74HC165 shift registers.
  state = digitalRead(MAIN_BUTTONS_INPUT_DATA);
  delayMicroseconds(5);

  // Read the rest of the bits from the 74HC165 shift reigsters.
  for (int i = 1; i < 16; i++) {
    digitalWrite(MAIN_BUTTONS_INPUT_CLOCK, HIGH);
    delayMicroseconds(5);
    digitalWrite(MAIN_BUTTONS_INPUT_CLOCK, LOW);
    delayMicroseconds(5);
    state = (state << 1) | (digitalRead(MAIN_BUTTONS_INPUT_DATA) ? 0 : 1);
  }

  return state;
}

/* ===================================================================
 * readMainButtonsState()
 *
 * This function reads the press/release state of all buttons
 * (S1 - S10).
 *
 * bitRead(state, i) represents the press/release state of the S(i+1)
 * button where "state" is the value returned by this function. If it
 * is 1, it means that the button is currently pressed.
 *
 * Example:
 *   uint16_t state = readButtonsRawState();
 *   Serial.println( bitRead(state, 0) ? "Button S1 is pressed." : "Button S1 is released." );
 *   Serial.println( bitRead(state, 1) ? "Button S2 is pressed." : "Button S2 is released." );
 *   Serial.println( bitRead(state, 9) ? "Button S10 is pressed." : "Button S10 is released." );
 * =================================================================== */
uint16_t readButtonsRawState() {
  uint16_t state = readMainButtonsRawState();
  bitWrite(state, 9, digitalRead(RESET_BUTTON));
  return state;
}

/* ===================================================================
 * readMainButtonsState()
 *
 * This function reads the press/release state of all buttons
 * (S1 - S10). Only the buttons that have been pressed more than
 * five milliseconds are considered pressed. This function effectively
 * remove the bouncing effect.
 *
 * bitRead(state, i) represents the press/release state of the S(i+1)
 * button where "state" is the value returned by this function. If it
 * is 1, it means that the button is currently pressed.
 *
 * Example:
 *   uint16_t state = readButtonsState();
 *   Serial.println( bitRead(state, 0) ? "Button S1 is pressed." : "Button S1 is released." );
 *   Serial.println( bitRead(state, 1) ? "Button S2 is pressed." : "Button S2 is released." );
 *   Serial.println( bitRead(state, 9) ? "Button S10 is pressed." : "Button S10 is released." );
 * =================================================================== */
uint16_t readButtonsState() {
  uint16_t state1 = readMainButtonsRawState();
  delay(5);
  uint16_t state2 = readMainButtonsRawState();
  return state1 & state2;
}

/* ===================================================================
 * setup()
 *
 * Initialization function that is called only once before start
 * calling the loop() function.
 * =================================================================== */
void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println("setup");

  // Digital I/O pin setup for the reset button and its LEDs.
  pinMode(RESET_BUTTON,           INPUT);
  pinMode(RESET_BUTTON_GREEN_LED, OUTPUT);
  pinMode(RESET_BUTTON_RED_LED,   OUTPUT);

  // Digital I/O pin configuraiton for the shift registers 74HC165
  // that read the states of the main nine buttons.
  pinMode(MAIN_BUTTONS_INPUT_LOAD,  OUTPUT);
  pinMode(MAIN_BUTTONS_INPUT_CLOCK, OUTPUT);
  pinMode(MAIN_BUTTONS_INPUT_DATA,  INPUT);

  // Set up MAX7219.
  leds.shutdown(0, false);
  leds.setIntensity(0, 12);
  leds.clearDisplay(0);
}

/* ===================================================================
 * play()
 *
 * Sub loop in which the program continuously waits for the user
 * inputs, and move forward the game play based on the input.
 *
 * This function returns the next state the main programs should 
 * transition to.
 * =================================================================== */
MainState play() {
  enum {RED, GREEN} turn = RED;
  uint16_t buttons_state = 0;
  
  while (true) {
    // Detect the button state change (pressed or released).
    uint16_t buttons_state_new = readButtonsState();
    uint16_t buttons_state_changed = buttons_state ^ buttons_state_new;
    buttons_state = buttons_state_new;

    for (int i = 0; i < 10; i++) {
      if (bitRead(buttons_state_changed, i)) {
        Serial.print("S");
        Serial.print(i + 1);
        Serial.print(" : ");
        Serial.println(bitRead(buttons_state, i) ? "PRESSED" : "released");
      }
    }
    
    //delay(1000);
  }
}

/* ===================================================================
 * loop()
 *
 * Main loop function which is called repeatedly one after another
 * indefinitely.
 * =================================================================== */
void loop() {
  if (main_state == START) {
    // Turn off all LEDs.
    leds.clearDisplay(0);
    digitalWrite(RESET_BUTTON_GREEN_LED, LOW);
    digitalWrite(RESET_BUTTON_RED_LED,   LOW);

    // Start the game.
    main_state = PLAYING;
  } else if (main_state == PLAYING) {
    main_state = play();
  } else if (main_state == RED_WIN) {
    // TODO: implement
  } else if (main_state == GREEN_WIN) {
    // TODO: implement    
  } else {
    // This is an unknown state.
    // Flush the LEDs and wait until the user presses the reset button.

    // TODO: implement
  }
}
