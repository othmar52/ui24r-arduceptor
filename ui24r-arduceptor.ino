

#define STAGE_WELCOME 0
#define STAGE_CONNECTING_WLAN 1
#define STAGE_CONNECTED_WLAN 2
#define STAGE_CONNECTING_MIXER 3
#define STAGE_CONNECTED_MIXER 4
#define STAGE_COLLECTINPUTS 5
#define STAGE_CHOOSEINPUT 6
#define STAGE_COLLECTAUX 7
#define STAGE_CHOOSEAUX 8
#define STAGE_COLLECTALLDATA 9
#define STAGE_RUN 10


uint8_t myInput = 0;
uint8_t myAux = 0;
bool limitToZeroDbMode = true;
bool linkMyLevelMode = true;

uint8_t currentStage = 0;

#include <Wire.h>
#include "Adafruit_MCP23017.h"

Adafruit_MCP23017 mcp1;
Adafruit_MCP23017* mcpr = &mcp1;

void setup() {
  Serial.begin(19200);
  pinMode(D1, INPUT);
  pinMode(D2, INPUT);
  mcp1.begin(0);
  

  setupEncoders();
  setupNetwork();
  //setupEncoder();
  setupDisplaystuff();
}
void loop() {
  loopEncoders();
  loopApp();
  loopDisplayStuff();
  loopNetwork();
}


void debug(String msg) {
  Serial.println(msg);
}
