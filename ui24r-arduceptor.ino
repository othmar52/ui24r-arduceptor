// thanks to https://github.com/maxgerhardt/rotary-encoder-over-mcp23017/issues/1


#include <Bounce2mcp.h>
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <Rotary.h>
#include "lib/RotaryEncOverMCP.h"

#define NUM_BUTTONS 4

/*
      7   6   5   4   3   2   1   0          3V  GND GND GND
      |   |   |   |   |   |   |   |   |   |   |   |   |   |
    ---------------------------------------------------------
    |                                                       |
    Ↄ                       MCP23017                        |
    |                                                       |
    ---------------------------------------------------------
      |   |   |   |   |   |   |   |   |   |   |   |   |   |
      8   9  10  11  12  13  14  15  3V  GND      D1  D2
*/
// pin numbers of MCP23017 for all 4 encoders
#define MCP_PIN_E1_SW 8
#define MCP_PIN_E2_SW 9
#define MCP_PIN_E3_SW 10
#define MCP_PIN_E4_SW 11

#define MCP_PIN_E1_DT 0
#define MCP_PIN_E2_DT 2
#define MCP_PIN_E3_DT 4
#define MCP_PIN_E4_DT 6

#define MCP_PIN_E1_CLK 1
#define MCP_PIN_E2_CLK 3
#define MCP_PIN_E3_CLK 5
#define MCP_PIN_E4_CLK 7

// Pins, used by buttons
const uint8_t BUTTON_PINS[NUM_BUTTONS] = {
  MCP_PIN_E1_SW,
  MCP_PIN_E2_SW,
  MCP_PIN_E3_SW,
  MCP_PIN_E4_SW
};

// Flags to distinguish click from turn
bool BUTTON_ROTATION[NUM_BUTTONS] = {false, false, false, false};

// Helper to track push duration for each encoder
unsigned long BUTTON_LASTDOWN[NUM_BUTTONS] = {0, 0, 0, 0};

// Button debounce wrappers
BounceMcp * buttons = new BounceMcp[NUM_BUTTONS];

Adafruit_MCP23017 mcp1;
Adafruit_MCP23017* mcpr = &mcp1;

void RotaryEncoderChanged(bool clockwise, int id) {
  buttons[id].update();
  Serial.println("Encoder " + String(id) + ": "
                 + (clockwise ? String("clockwise ") : String("counter-clock-wise "))
                 + (buttons[id].read() ? String("") : String("(pressed)")));
  BUTTON_ROTATION[id] = true;

}


RotaryEncOverMCP rotaryEncoders[] = {
  RotaryEncOverMCP(&mcp1, MCP_PIN_E1_DT, MCP_PIN_E1_CLK, &RotaryEncoderChanged, 0),
  RotaryEncOverMCP(&mcp1, MCP_PIN_E2_DT, MCP_PIN_E2_CLK, &RotaryEncoderChanged, 1),
  RotaryEncOverMCP(&mcp1, MCP_PIN_E3_DT, MCP_PIN_E3_CLK, &RotaryEncoderChanged, 2),
  RotaryEncOverMCP(&mcp1, MCP_PIN_E4_DT, MCP_PIN_E4_CLK, &RotaryEncoderChanged, 3)
};

void setup() {
  //Wire.begin(4,5);
  //Wire.begin(D2,D1);
  Serial.begin(19000);
  Serial.println("MCP23007 Polling Test");

  //pinMode(D1, INPUT);
  //pinMode(D2, INPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);
  mcp1.begin(0);

  // init encoder buttons
  for (int i = 0; i < NUM_BUTTONS; i++) {
    mcpr->pinMode(BUTTON_PINS[i], INPUT);
    mcpr->pullUp(BUTTON_PINS[i], HIGH);
    buttons[i].attach(*mcpr, BUTTON_PINS[i], 5);
  }

  // init encoders
  for (int i = 0; i < NUM_BUTTONS; i++) {
    rotaryEncoders[i].init();
  }
}

void pollAll() {
  uint16_t gpioAB = mcpr->readGPIOAB();
  for (int i = 0; i < NUM_BUTTONS; i++) {
    rotaryEncoders[i].feedInput(gpioAB);
  }
}

void loop() {
  pollAll();

  for (int i = 0; i < NUM_BUTTONS; i++)  {
    buttons[i].update();
    // Updating all debouncers
    if ( buttons[i].fell() ) {
      // Clearing the rotation flag
      BUTTON_ROTATION[i] = false;
      BUTTON_LASTDOWN[i] = millis();
    }

    if ( buttons[i].rose() ) {
      // When button was released...
      Serial.println("Button " + String(i) + ": up (push duration: " + String(millis() - BUTTON_LASTDOWN[i]) + " ms)" );
    }
  }
}
