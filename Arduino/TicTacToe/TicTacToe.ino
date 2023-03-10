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
enum MainState { START, PLAYING, RED_WIN, GREEN_WIN, DRAW } main_state = START;

// A variable to hold the territory state.
enum TerritoryState {RED, GREEN, NONE} territory_state[3][3];

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
 * readButtonsRawState()
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
  bitWrite(state, 9, digitalRead(RESET_BUTTON) == HIGH ? 0 : 1);
  return state;
}

/* ===================================================================
 * readMainButtonsState()
 *
 * This function reads the press/release state of all buttons
 * (S1 - S10). Only the buttons that have been pressed more than
 * five milliseconds are considered pressed. This function effectively
 * removes the bouncing effect.
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
  uint16_t state1 = readButtonsRawState();
  delay(10);
  uint16_t state2 = readButtonsRawState();
  return state1 & state2;
}

/* ===================================================================
 * readResetButtonState()
 *
 * This function reads the press/release state of the reset button
 * (S10). This function effectively removes the bouncing effect.
 * =================================================================== */
int readResetButtonState() {
  int state1 = digitalRead(RESET_BUTTON);
  delay(10);
  int state2 = digitalRead(RESET_BUTTON);

  if (state1 == LOW && state2 == LOW) {
    return 1;
  } else {
    return 0;
  }
}

/* ===================================================================
 * drawTerritory()
 *
 * Turn on/off red and green LEDs of S1 - S9 based on the 
 * territory_state global variable.
 * =================================================================== */
void drawTerritory() {
  for (int row = 0; row < 3; row++) {
    byte red_row = 0;
    byte green_row = 0;
    
    for (int col = 0; col < 3; col++) {
      if (territory_state[row][col] == RED) {
        red_row = red_row | (B01000000 >> col);
      } else if (territory_state[row][col] == GREEN) {
        green_row = green_row | (B00001000 >> col);
      }
    }

    leds.setRow(0, row, red_row);
    leds.setRow(0, row + 3, green_row);
  }
}

/* ===================================================================
 * evaluateTerritory()
 *
 * Evaluate the territory and returns if the red play wins (RED_WIN),
 * the green play wins (GREEN_WIN), draw because all territories are
 * populated (DRAW) or the game can continue (PLAYING).
 * =================================================================== */
MainState evaluateTerritory() {
  // Judge if the red or green player wins
  for (int i = 0; i < 3; i++) {
    if (territory_state[0][i] == RED &&
        territory_state[1][i] == RED &&
        territory_state[2][i] == RED) {
      return RED_WIN;
    } else if (territory_state[i][0] == RED &&
               territory_state[i][1] == RED &&
               territory_state[i][2] == RED) {
      return RED_WIN;
    } else if (territory_state[0][i] == GREEN &&
               territory_state[1][i] == GREEN &&
               territory_state[2][i] == GREEN) {
      return GREEN_WIN;
    } else if (territory_state[i][0] == GREEN &&
               territory_state[i][1] == GREEN &&
               territory_state[i][2] == GREEN) {
      return GREEN_WIN;
    }
  }

  if (territory_state[0][0] == RED &&
      territory_state[1][1] == RED &&
      territory_state[2][2] == RED) {
    return RED_WIN;
  } else if (territory_state[0][2] == RED &&
             territory_state[1][1] == RED &&
             territory_state[2][0] == RED) {
    return RED_WIN;
  } else if (territory_state[0][0] == GREEN &&
             territory_state[1][1] == GREEN &&
             territory_state[2][2] == GREEN) {
    return GREEN_WIN;
  } else if (territory_state[0][2] == GREEN &&
             territory_state[1][1] == GREEN &&
             territory_state[2][0] == GREEN) {
    return GREEN_WIN;
  }

  // Judge if the play can continue.
  for (int row = 0; row < 3; row++) {
    for (int col = 0; col < 3; col++) {
      if (territory_state[row][col] == NONE) {
        return PLAYING;
      }
    }
  }

  return DRAW;
}

/* ===================================================================
 * setup()
 *
 * Initialization function that is called only once before start
 * calling the loop() function.
 * =================================================================== */
void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println("TicTacToeV1");

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
  enum PlayTurn {RED_TURN, GREEN_TURN} turn = RED_TURN;
  uint16_t buttons_state = 0;

  /*
   * Initialize the territory.
   */
  for (int row = 0; row < 3; row++) {
    for (int col = 0; col < 3; col++) {
      territory_state[row][col] = NONE;
    }
  }
  drawTerritory();
  
  while (true) {
    // Detect the button state change (pressed or released).
    uint16_t buttons_state_new = readButtonsState();
    uint16_t buttons_state_changed = buttons_state ^ buttons_state_new;
    buttons_state = buttons_state_new;

    if (bitRead(buttons_state_changed, 9) && !bitRead(buttons_state_new, 9)) {
      // The reset button (S10 button) was pressed and released.
      // Reset the game.
      return START;
    }

    for (int i = 0; i < 9; i++) {
      if (bitRead(buttons_state_changed, i) && !bitRead(buttons_state_new, i)) {
        // Handle the release event of the S1 - S9 buttons.
        int row = i / 3;
        int col = i % 3;

        // If the button has not been activated yet, turn the
        // color of that territory.
        if (territory_state[row][col] == NONE) {
          if (turn == RED_TURN) {
            territory_state[row][col] = RED;
            turn = GREEN_TURN;
          } else {
            territory_state[row][col] = GREEN;
            turn = RED_TURN;
          }
        }

        MainState result = evaluateTerritory();
        if (result != PLAYING) {
          return result;
        }
      }
    }
    
    drawTerritory();
  }
}

/* ===================================================================
 * draw()
 *
 * Sub loop in which the program continuously waits until the reset
 * button is pressed and released. Flash the reset button during that
 * time.
 * =================================================================== */
void draw() {
  drawTerritory();

  int reset_button_state = 0;
  int count = 0;

  while (true) {
    int reset_button_state_new = readResetButtonState();
    int reset_button_state_changed = reset_button_state ^ reset_button_state_new;
    reset_button_state = reset_button_state_new;

    if (reset_button_state_changed && !reset_button_state_new) {
      // The reset button was released. Finish this function.
      return;
    }

    // Flash the reset button.
    count = count + 1;
    if (count == 1) {
      digitalWrite(RESET_BUTTON_RED_LED, HIGH);
    } else if (count == 10) {
      digitalWrite(RESET_BUTTON_RED_LED, LOW);
      digitalWrite(RESET_BUTTON_GREEN_LED, HIGH);
    } else if (count == 20) {
      digitalWrite(RESET_BUTTON_GREEN_LED, LOW);
    } else if (count == 100) {
      count = 0;
    }
  }
}

/* ===================================================================
 * win()
 *
 * Sub loop in which the program continuously waits until the reset
 * button is pressed and released. Flash the reset button during
 * that time.
 * =================================================================== */
void win(int red) {
  drawTerritory();

  int reset_button_state = 0;
  int count = 0;

  int coord_to_flash[3][2];
  TerritoryState win_state = red ? RED : GREEN;

  for (int i = 0; i < 3; i++) {
    if (territory_state[0][i] == win_state &&
        territory_state[1][i] == win_state &&
        territory_state[2][i] == win_state) {
      coord_to_flash[0][0] = 0;
      coord_to_flash[0][1] = i;
      coord_to_flash[1][0] = 1;
      coord_to_flash[1][1] = i;
      coord_to_flash[2][0] = 2;
      coord_to_flash[2][1] = i;
    } else if (territory_state[i][0] == win_state &&
               territory_state[i][1] == win_state &&
               territory_state[i][2] == win_state) {
      coord_to_flash[0][0] = i;
      coord_to_flash[0][1] = 0;
      coord_to_flash[1][0] = i;
      coord_to_flash[1][1] = 1;
      coord_to_flash[2][0] = i;
      coord_to_flash[2][1] = 2;
    }
  }

  if (territory_state[0][0] == win_state &&
      territory_state[1][1] == win_state &&
      territory_state[2][2] == win_state) {
      coord_to_flash[0][0] = 0;
      coord_to_flash[0][1] = 0;
      coord_to_flash[1][0] = 1;
      coord_to_flash[1][1] = 1;
      coord_to_flash[2][0] = 2;
      coord_to_flash[2][1] = 2;
  } else if (territory_state[0][2] == win_state &&
             territory_state[1][1] == win_state &&
             territory_state[2][0] == win_state) {
      coord_to_flash[0][0] = 0;
      coord_to_flash[0][1] = 2;
      coord_to_flash[1][0] = 1;
      coord_to_flash[1][1] = 1;
      coord_to_flash[2][0] = 2;
      coord_to_flash[2][1] = 0;
  }

  while (true) {
    int reset_button_state_new = readResetButtonState();
    int reset_button_state_changed = reset_button_state ^ reset_button_state_new;
    reset_button_state = reset_button_state_new;

    if (reset_button_state_changed && !reset_button_state_new) {
      // The reset button was released. Finish this function.
      return;
    }

    // Flash the reset button.
    count = count + 1;
    if (count == 1) {
      if (red) {
        digitalWrite(RESET_BUTTON_RED_LED, HIGH);
      } else {
        digitalWrite(RESET_BUTTON_GREEN_LED, HIGH);
      }
    } else if (count == 15) {
      digitalWrite(RESET_BUTTON_RED_LED, LOW);
      digitalWrite(RESET_BUTTON_GREEN_LED, LOW);
    } else if (count == 30) {
      count = 0;
    }

    // Flash the territory.
    if (count == 1) {
      for (int i = 0; i < 3; i++) {
        int row = coord_to_flash[i][0];
        int col = coord_to_flash[i][1];

        territory_state[row][col] = win_state;
      }
      drawTerritory();
    } else if (count == 15) {
      for (int i = 0; i < 3; i++) {
        int row = coord_to_flash[i][0];
        int col = coord_to_flash[i][1];

        territory_state[row][col] = NONE;
      }
      drawTerritory();
    }

    if (count == 30) {
      count = 0;
    }
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
    win(1);
    main_state = START;
  } else if (main_state == GREEN_WIN) {
    win(0);
    main_state = START;
  } else if (main_state == DRAW) {
    draw();
    main_state = START;
  } else {
    // This is an unknown state.
    // Flush the LEDs and wait until the user presses the reset button.

    // TODO: implement
  }
}
