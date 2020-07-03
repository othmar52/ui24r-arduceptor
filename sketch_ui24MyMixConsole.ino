

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

Adafruit_MCP23017 mcp;

void setup() {
  //Serial.begin(9600);
  Serial.begin(19200);


  // Initialize the P.print(
  mcp.begin();
  
  // Define GPA0 (physical pin 21) as output pin
  //mcp.pinMode(0, OUTPUT);

  setupEncoder();
  setupNetwork();
  //setupEncoder();
  setupDisplaystuff();
}
void loop() {
  loopEncoder();
  loopApp();
  loopDisplayStuff();
  loopNetwork();
}


void debug(String msg) {
  Serial.println(msg);
}
