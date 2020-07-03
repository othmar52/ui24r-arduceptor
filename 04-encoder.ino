


// https://www.brainy-bits.com/arduino-rotary-encoder-ky-040/
/*
#define NUM_ENCODERS 2

const uint8_t inputCLK[NUM_ENCODERS] = { 8, 11 };
const uint8_t inputDT[NUM_ENCODERS] = { 9, 12 };
const uint8_t inputSW[NUM_ENCODERS] = { 10, 13 };
*/

#define NUM_ENCODERS 1

const uint8_t inputCLK[NUM_ENCODERS] = { 8 };
const uint8_t inputDT[NUM_ENCODERS] = { 9 };
const uint8_t inputSW[NUM_ENCODERS] = { 10 };


const int debounceDelayEnc = 0.01;
const int debounceDelaySwitch = 50;

uint8_t previousCLK[NUM_ENCODERS];
uint8_t previousDT[NUM_ENCODERS];

uint8_t previousSW[NUM_ENCODERS];
unsigned long durationSW[NUM_ENCODERS];   // track time how long switch has been pressed before release
uint8_t tmpDirectionIncrement[NUM_ENCODERS]; // for now invert direction on encoder push


//unsigned long lastDebounceTimeEnc[NUM_ENCODERS] = {0, 0};
//unsigned long lastDebounceTimeSwitch[NUM_ENCODERS] = {0, 0};
unsigned long lastDebounceTimeEnc[NUM_ENCODERS] = {0};
unsigned long lastDebounceTimeSwitch[NUM_ENCODERS] = {0};

unsigned long currentMilliSecond = 0;


//int16_t encoderValues[NUM_ENCODERS] = {0, 0};
int16_t encoderValues[NUM_ENCODERS] = {0};


void setupEncoder() {
  for(uint8_t i=0; i< NUM_ENCODERS; i++) {
    //mcp.pinMode (inputCLK[i], INPUT);
    //mcp.pinMode (inputDT[i], INPUT);
    mcp.pinMode (inputSW[i], INPUT);
    previousCLK[i] = mcp.digitalRead(inputCLK[i]);
    previousDT[i] = mcp.digitalRead(inputDT[i]);
    previousSW[i] = mcp.digitalRead(inputSW[i]);
    tmpDirectionIncrement[i] = true;
    //currentStateSW[i] = previousStateSW[i];
  }
}

void loopEncoder() {
  currentMilliSecond = millis();
  readEncoders();
  readEncoderSwitches();
}


void encoderChange(uint8_t encoderIdx, bool clockWise, String initiator) {
  encoderValues[encoderIdx] = (clockWise == false) ? encoderValues[encoderIdx]+1 : encoderValues[encoderIdx]-1;
  debug(" ROTATE encoder " + String(encoderIdx) + " "+String(clockWise) +" "+String(initiator) +" change to: " + String(encoderValues[encoderIdx]));
  handleEncoderChange(encoderIdx, tmpDirectionIncrement[encoderIdx]);
}

void switchReleased(uint8_t switchIdx, int holdTime) {
  tmpDirectionIncrement[switchIdx] = !tmpDirectionIncrement[switchIdx];
  debug(" PUSH encoder " + String(switchIdx) + " released    hold milliseconds was "+String(holdTime));
  handleEncoderPush(switchIdx, holdTime);
}

void readEncoders() {
  int currentCLK;
  int currentDT;
  char mask[5];
  for(uint8_t i=0; i<NUM_ENCODERS; i++) {
    if(millis() - lastDebounceTimeEnc[i] > debounceDelayEnc)  {
      
      // check rotary() begin()
      currentCLK = mcp.digitalRead(inputCLK[i]);
      currentDT = mcp.digitalRead(inputDT[i]);
      //mask[0] = previousCLK[i];
      //mask[1] = previousDT[i];
      //mask[2] = currentCLK;
      //mask[3] = currentDT;

      sprintf_P(mask, (PGM_P)F("%d%d%d%d"), previousCLK[i], previousDT[i], currentCLK, currentDT);

      if(strcmp(mask, "0110") == 0) { encoderChange(i, true,  "A"); }
      if(strcmp(mask, "0111") == 0) { encoderChange(i, false, "B"); }

      if(strcmp(mask, "1001") == 0) { encoderChange(i, true,  "C"); }
      if(strcmp(mask, "1000") == 0) { encoderChange(i, false, "D"); }
      
      if(strcmp(mask, "1101") == 0) { encoderChange(i, true,  "E"); }
      if(strcmp(mask, "1100") == 0) { encoderChange(i, false, "F"); }
      
      if(strcmp(mask, "0010") == 0) { encoderChange(i, true,  "G"); }
      if(strcmp(mask, "0011") == 0) { encoderChange(i, false, "H"); }

      /*
      if((previousCLK[i] == 0) && (previousDT[i] == 1)) {
        if((currentCLK == 1) && (currentDT == 0)) {
          encoderValues[i]++;
          Serial.println(" AAA encoder " + String(i) + " ++ change to: " + String(encoderValues[i]));
        }
        if((currentCLK == 1) && (currentDT == 1)) {
          encoderValues[i]--;
          Serial.println(" BBB encoder " + String(i) + " -- change to: " + String(encoderValues[i]));
        }
      }
      
      if((previousCLK[i] == 1) && (previousDT[i] == 0)) {
        if((currentCLK == 0) && (currentDT == 1)) {
          encoderValues[i]++;
          Serial.println(" CCC encoder " + String(i) + " ++ change to: " + String(encoderValues[i]));
        }
        if((currentCLK == 0) && (currentDT == 0)) {
          encoderValues[i]--;
          Serial.println(" DDD encoder " + String(i) + " -- change to: " + String(encoderValues[i]));
        }
      }
      
      if((previousCLK[i] == 1) && (previousDT[i] == 1)) {
        if((currentCLK == 0) && (currentDT == 1)) {
          encoderValues[i]++;
          Serial.println(" EEE encoder " + String(i) + " ++ change to: " + String(encoderValues[i]));
        }
        if((currentCLK == 0) && (currentDT == 0)) {
          encoderValues[i]--;
          Serial.println(" FFF encoder " + String(i) + " -- change to: " + String(encoderValues[i]));
          Serial.println(mask);
        }
      }
      
      if((previousCLK[i] == 0) && (previousDT[i] == 0)) {
        if((currentCLK == 1) && (currentDT == 0)) {
          encoderValues[i]++;
          Serial.println(" GGG encoder " + String(i) + " ++ change to: " + String(encoderValues[i]));
        }
        if((currentCLK == 1) && (currentDT == 1)) {
          encoderValues[i]--;
          Serial.println(" HHH encoder " + String(i) + " -- change to: " + String(encoderValues[i]));
          Serial.println(mask);
        }
      }
      */

      // check rotary() end
      previousCLK[i] = currentCLK;
      previousDT[i] = currentDT;
      lastDebounceTimeEnc[i] = millis();

    }
  }
}

void readEncoderSwitches() {
  int currentSW;
  //int duration;
  for(uint8_t i=0; i< NUM_ENCODERS; i++) {
    if(currentMilliSecond - lastDebounceTimeSwitch[i] > debounceDelaySwitch)  {
      currentSW = mcp.digitalRead(inputSW[i]);
      if(currentSW != previousSW[i]) {
        if(currentSW == 0) {
          durationSW[i] = currentMilliSecond;
        } else {
          //duration = currentMilliSecond - durationSW[i];
          switchReleased(i, currentMilliSecond - durationSW[i]);
        }
      }
      lastDebounceTimeSwitch[i] = currentMilliSecond;
      previousSW[i] = currentSW;
    }
  }  
}


/* Encoder Library - NoInterrupts Example
 * http://www.pjrc.com/teensy/td_libs_Encoder.html
 *
 * This example code is in the public domain.
 */
/*
// If you define ENCODER_DO_NOT_USE_INTERRUPTS *before* including
// Encoder, the library will never use interrupts.  This is mainly
// useful to reduce the size of the library when you are using it
// with pins that do not support interrupts.  Without interrupts,
// your program must call the read() function rapidly, or risk
// missing changes in position.
#define ENCODER_DO_NOT_USE_INTERRUPTS
#include "EncoderMod.h"

// Beware of Serial.print() speed.  Without interrupts, if you
// transmit too much data with Serial.print() it can slow your
// reading from Encoder.  Arduino 1.0 has improved transmit code.
// Using the fastest baud rate also helps.  Teensy has USB packet
// buffering.  But all boards can experience problems if you print
// too much and fill up buffers.

// Change these two numbers to the pins connected to your encoder.
//   With ENCODER_DO_NOT_USE_INTERRUPTS, no interrupts are ever
//   used, even if the pin has interrupt capability
EncoderMod myEnc(27, 28, mcp);
//   avoid using pins with LEDs attached

void setupEncoder() {
  Serial.println("Basic NoInterrupts Test:");
}

long position  = -999;

void loopEncoder() {
  long newPos = myEnc.read();
  if (newPos != position) {
    position = newPos;
    Serial.println(position);
  }
  // With any substantial delay added, Encoder can only track
  // very slow motion.  You may uncomment this line to see
  // how badly a delay affects your encoder.
  //delay(50);
}



*/


/*
// this approach does not work due to "variable or field 'rotatXXe' declared void"
#include "ESPRotaryMod.h";

#define ROTARY_PIN1 8
#define ROTARY_PIN2 9

#define CLICKS_PER_STEP 4   // this number depends on your rotary encoder 

ESPRotaryMod rotaE = ESPRotaryMod(mcp, ROTARY_PIN1, ROTARY_PIN2, CLICKS_PER_STEP);


/////////////////////////////////////////////////////////////////

// on change
void rotatXXe (ESPRotaryMod& rotaE) {
   Serial.println(rotaE.getPosition());
}

// on left or right rotation
void showDirection (ESPRotaryMod &rotaE) {
  Serial.println(rotaE.directionToString(rotaE.getDirection()));
}

 
void setupEncoder() {
  Serial.println("\n\nSimple Counter");
  delay(50);
  rotaE.setChangedHandler(rotatXXe);
  //rotaE.setLeftRotationHandler(showDirection);
  //rotaE.setRightRotationHandler(showDirection);
}

void loopEncoder() {
  rotaE.loop();
}

*/




/*
 * 
 * 


*/
 


/*





*/




/*
#define NUM_ENCODERS 2

const uint8_t inputCLK[NUM_ENCODERS] = { 8, 11 };
const uint8_t inputDT[NUM_ENCODERS] = { 9, 12 };
const uint8_t inputSW[NUM_ENCODERS] = { 10, 13 };

uint8_t currentStateCLK[NUM_ENCODERS];
uint8_t previousStateCLK[NUM_ENCODERS];
uint8_t clkDirection[NUM_ENCODERS];

uint8_t currentStateSW[NUM_ENCODERS];
uint8_t previousStateSW[NUM_ENCODERS];

unsigned long lastDebounceTime[NUM_ENCODERS] = {0, 0};
unsigned long lastStateChange[NUM_ENCODERS] = {0, 0};

unsigned long debounceDelay = 4000;
unsigned long currentMicroSecond = 0;


uint16_t encoderValues[NUM_ENCODERS] = {0, 0};


void setupEncoder() {
  for(uint8_t i=0; i< NUM_ENCODERS; i++) {
    mcp.pinMode (inputCLK[i], INPUT);
    mcp.pinMode (inputDT[i], INPUT);
    mcp.pinMode (inputSW[i], INPUT);
    previousStateCLK[i] = mcp.digitalRead(inputCLK[i]);
    //currentStateCLK[i] = previousStateCLK[i];
    previousStateSW[i] = mcp.digitalRead(inputSW[i]);
    //currentStateSW[i] = previousStateSW[i];
  }
}

void loopEncoder() {
  currentMicroSecond = micros();
  readEncoders();
  readEncoderSwitches();
}



void readEncoders() {
  //Serial.println("---------------------");
  //Serial.println(mcp.digitalRead(inputCLK[0]));
  //Serial.println(mcp.digitalRead(inputCLK[1]));
  //delay(6);
  //return;
  bool increment;
  uint8_t currRead;
  for(uint8_t i=0; i<NUM_ENCODERS; i++) {
    currRead = mcp.digitalRead(inputCLK[i]);
  
    if(currRead != previousStateCLK[i]) {
      if(clkDirection[i] == 0) {
        clkDirection[i] = (mcp.digitalRead(inputDT[i]) != currRead) ? 1 : 2;
      }
      //increment = (mcp.digitalRead(inputDT[i]) != currRead) ? false : true;
      if(currentMicroSecond - lastDebounceTime[i] > debounceDelay) {
        debug(" encoder movement  " + String(i) + " direction " + String(clkDirection[i]));
        lastDebounceTime[i] = currentMicroSecond;
        clkDirection[i] = 0;
      }
    }
    previousStateCLK[i] = currRead;
  }
}

void readEncoderSwitches() {
  return;
  
  for(uint8_t i=0; i< NUM_ENCODERS; i++) {
    currentStateSW[i] = mcp.digitalRead(inputSW[i]);
    if(currentStateSW[i] != previousStateSW[i]) {
      lastDebounceTime[i] = millis(); //set the current time
    }
  
    //filter out any noise by setting a time buffer
    if ( (millis() - lastDebounceTime[i]) > debounceDelay) {
      if (currentStateSW[i] == LOW && millis() - lastStateChange[i] > 1000) {
        debug(" pushed encoder switch " + String(i));
        lastStateChange[i] = millis();
      }
    }
    previousStateSW[i] = currentStateSW[i];
  }  
}





*/


/*  remove this and encoder.h */
/*
// rotary encoder demo by 'jurs' for Arduino Forum
// This is the code for the main "sketch"

#include <Wire.h>
#include <SPI.h> 
#include "encoder.h"

void setupEncoder() {
  Serial.println("Good night and good luck!"); // print some Test-Message at beginning
  beginEncoders();
}

void printEncoders()
{ // print current count of each encoder to Serial
  for (int i=0; i<NUMENCODERS; i++)
  {
    Serial.print(encoder[i].count);
    Serial.print('\t');
  }
  Serial.println();
}

void loopEncoder() {
  if (updateEncoders()) printEncoders();
}
*/
