

#define INPUTS 22
#define AUX 8

#include <stdlib.h>

const uint32_t levelFactor = 1000000;
//const uint32_t zeroDbPos = levelFactor * 0.7647058823529421;
const uint32_t zeroDbPos =    764705;

uint32_t auxLevels[AUX];
uint32_t inputToMasterLevels[INPUTS];
uint32_t inputToAuxLevels[INPUTS][AUX];
uint32_t groupMixValue = 300000;

int8_t inputStereoIndex[INPUTS];
int8_t auxStereoIndex[AUX];

char inputNames[INPUTS][30];
char auxNames[AUX][30];

bool inputMutes[INPUTS];
bool auxMutes[AUX];

// needed to determine if we have all initial params from mixer
bool allDataCollected = false;
uint8_t collectedInputStereoIndex = 0;
uint8_t collectedInputNames = 0;
uint8_t collectedInputMutes = 0;
uint8_t collectedInputToMasterLevels = 0;
uint16_t collectedInputToAuxLevels = 0;
uint8_t collectedAuxNames = 0;
uint8_t collectedAuxStereoIndex = 0;
uint8_t collectedAuxMutes = 0;
uint8_t collectedAuxLevels = 0;

/**
 * this function splits the message by \n and drops out a huge
 * amount of data that is not relevant for us
 */
void handleIncomingMixerMessage(String msg) {
  String isolateMessage = "";
  bool dropMessage = false;
   // we need to split by new line
  for(unsigned int i = 0; i<msg.length(); i++) {
    if(msg[i] == '\n') {
      if(dropMessage == true) {
        //Serial.println("dropping msg");
        isolateMessage = "";
        dropMessage = false;
        continue;
      }
      parseIncomingMessage(isolateMessage);
      dropMessage = false;
      isolateMessage = "";
      continue;
    }
    // isolateMessage += (msg[i] == '^') ? '=' : msg[i];  // regex lib cant handle circumflex char. at least i want able to write the regex
    isolateMessage += msg[i];
    if(isolateMessage == "2::" || isolateMessage == "3:::") {
      isolateMessage = "";
      continue;  
    }

    
    if(isolateMessage.length() == 4) {
      if(isolateMessage == "SETS" || isolateMessage == "SETD") {
        continue;
      }
      dropMessage = true;
      //Serial.println("going to drop " + isolateMessage);
    }
  }
  if(dropMessage == false) {
    parseIncomingMessage(isolateMessage);
    isolateMessage = "";
  }
}

void incrementDataCollectCounter(String subject) {
  if(allDataCollected == true) {
    updateGroupMix();
    return;
  }
  //debug(String(getPercentCollectedAllData()));
  if(subject == "collectedAuxStereoIndex") { collectedAuxStereoIndex++; return; } 
  if(subject == "collectedAuxLevels") { collectedAuxLevels++; return; } 
  if(subject == "collectedInputToAuxLevels") { collectedInputToAuxLevels++; return; } 
  if(subject == "collectedAuxNames") { collectedAuxNames++; return; } 
  if(subject == "collectedInputToMasterLevels") { collectedInputToMasterLevels++; return; } 
  if(subject == "collectedInputStereoIndex") { collectedInputStereoIndex++; return; } 
  if(subject == "collectedInputNames") { collectedInputNames++; return; } 
  if(subject == "collectedInputMutes") { collectedInputMutes++; return; } 
  if(subject == "collectedAuxMutes") { collectedAuxMutes++; return; }
}

/**
   due to performance reasons we can't use String::indexOf() or Regex lib ( https://github.com/nickgammon/Regexp )
   so we check for specific character positions that are hopefully unique.
   based on those we can filter which params are useful and which can be ignored
*/
void parseIncomingMessage(String msg) {

  bool isInput = false;
  bool isAux = false;
  
  if (msg[6] != '.') {
    // we are only interested in i.* and a.*
    // this will dropout [automix.time]
    return;
  }

  if (msg[5] == 'a') {
    isAux = true;
  }
  if (msg[5] == 'i') {
    isInput = true;
  }

  if (isAux == false && isInput == false) {
    //Serial.print("dropping (no i or a): ");
    //Serial.println(msg);
    return;
  }

  if (msg[8] != 'a' && msg[9] == 'm' && msg[10] == 'i' && msg[11] == 'x') {               // we have mix param with one digit as channel [i.1.mix, a.7.mix]  
    // not to confuse with mtx [a.0.mtx.4.postproc]
    // drop out stuff like a.0.mtx.4.postproc, i.4.amixgroup, i.4.amix
    return handleParamMix(isInput, isAux, false, msg);
  }

  if (msg[9] != 'a' && msg[10] == 'm' && msg[11] == 'i' && msg[12] == 'x') {              // we have mix param with two digit as channel [i.10.mix]
    return handleParamMix(isInput, isAux, true, msg);
  }
  if (msg[15] == 'I' && msg[19] == 'x') {              // we have stereoIndex param with one digit as channel [i.0.stereoIndex, a.7.stereoIndex]
    return handleParamStereoIndex(isInput, isAux, false, msg);
  }

  if (msg[16] == 'I' && msg[20] == 'x') {              // we have stereoIndex param with two digits as channel [i.18.stereoIndex]
    return handleParamStereoIndex(isInput, isAux, true, msg);
  }
  if (msg[10] == 'u' && msg[18] == 'u') {              // we have send aux param with one digit as channel [i.1.aux.2.value]
    return handleParamAuxSend(false, msg);
  }

  if (msg[11] == 'u' && msg[19] == 'u') {              // we have send aux param with two digits as channel [i.14.aux.2.value]
    return handleParamAuxSend(true, msg);
  }

  if (msg[9] == 'n' && msg[12] == 'e') {              // we have name param with one digit as channel [a.0.name]
    return handleParamName(isInput, isAux, false, msg);
  }

  if (msg[10] == 'n' && msg[13] == 'e') {              // we have name param with two digits as channel [i.12.name]
    return handleParamName(isInput, isAux, true, msg);
  }

  if (msg[9] == 'm' && msg[12] == 'e') {              // we have mute param with one digit as channel [a.0.name]
    return handleParamMute(isInput, isAux, false, msg);
  }

  if (msg[10] == 'm' && msg[13] == 'e') {              // we have mute param with two digits as channel [i.12.name]
    return handleParamMute(isInput, isAux, true, msg);
  }
}



void handleParamMix(bool isInput, bool isOutput, bool isTwoDigitChannel, String msg) {
  //Serial.print("handleParamMix() "); Serial.println(msg);
  uint8_t channelNumberInt = extractChannelNumberFromMessage(msg, isTwoDigitChannel);

  if (isInput == true) {
    if (channelNumberInt >= INPUTS) {
      return;  // input index too big
    }
    inputToMasterLevels[channelNumberInt] = levelToInternal(getDelimitedValue(msg, '^', 2));
    incrementDataCollectCounter("collectedInputToMasterLevels");
    return;
  }
  if (channelNumberInt >= AUX) {
    return;  // aux index too big
  }
  auxLevels[channelNumberInt] = levelToInternal(getDelimitedValue(msg, '^', 2));
  collectedAuxLevels++;
  incrementDataCollectCounter("collectedAuxLevels");
}

void handleParamStereoIndex(bool isInput, bool isOutput, bool isTwoDigitChannel, String msg) {
  //Serial.print("handleParamStereoIndex() "); Serial.println(msg);
  uint8_t channelNumberInt = extractChannelNumberFromMessage(msg, isTwoDigitChannel);
  int8_t incomingStereoValue = stereoIndexValue(getDelimitedValue(msg, '^', 2));
  if (isInput == true) {
    if (channelNumberInt >= INPUTS) {
      debug("// input index too big");
      return;  // input index too big
    }
    inputStereoIndex[channelNumberInt] = incomingStereoValue;
    incrementDataCollectCounter("collectedInputStereoIndex");
    return;
  }
  if (channelNumberInt >= AUX) {
    return;  // aux index too big
  }
  auxStereoIndex[channelNumberInt] = incomingStereoValue;
  incrementDataCollectCounter("collectedAuxStereoIndex");
}

void handleParamAuxSend(bool isTwoDigitChannel, String msg) {
  //Serial.print("handleParamAuxSend() "); Serial.println(msg);
  uint8_t channelNumberInt = extractChannelNumberFromMessage(msg, isTwoDigitChannel);
  uint8_t auxChannel = extractAuxSendChannelNumberFromMessage(msg, isTwoDigitChannel);

  if (channelNumberInt >= INPUTS) {
    return;  // input index too big
  }
  if (auxChannel >= AUX) {
    return;  // aux index too big
  }
  //Serial.print("with internal handleParamAuxSend: ");
  //Serial.println(levelToInternal(getDelimitedValue(msg, '^', 2)));

  inputToAuxLevels[channelNumberInt][auxChannel] = levelToInternal(getDelimitedValue(msg, '^', 2));
  incrementDataCollectCounter("collectedInputToAuxLevels");
}

void handleParamName(bool isInput, bool isOutput, bool isTwoDigitChannel, String msg) {
  //Serial.print("handleParamName() "); Serial.println(msg);
  uint8_t channelNumberInt = extractChannelNumberFromMessage(msg, isTwoDigitChannel);

  if (isInput == true) {
    if (channelNumberInt >= INPUTS) {
      return;  // input index too big
    }
    getDelimitedValue(msg, '^', 2).toCharArray(inputNames[channelNumberInt], 30);
    incrementDataCollectCounter("collectedInputNames");
    return;
  }
  if (channelNumberInt >= AUX) {
    return;  // aux index too big
  }
  getDelimitedValue(msg, '^', 2).toCharArray(auxNames[channelNumberInt], 30);
  incrementDataCollectCounter("collectedAuxNames");
}


void handleParamMute(bool isInput, bool isOutput, bool isTwoDigitChannel, String msg) {
  //debug("handleParamMute() " + msg);
  uint8_t channelNumberInt = extractChannelNumberFromMessage(msg, isTwoDigitChannel);

  if (isInput == true) {
    if (channelNumberInt >= INPUTS) {
      return;  // input index too big
    }
    inputMutes[channelNumberInt] = ( getDelimitedValue(msg, '^', 2) == "1") ? true : false;
    incrementDataCollectCounter("collectedInputMutes");
    return;
  }
  if (channelNumberInt >= AUX) {
    return;  // aux index too big
  }
  auxMutes[channelNumberInt] = ( getDelimitedValue(msg, '^', 2) == "1") ? true : false;
  incrementDataCollectCounter("collectedAuxMutes");
}


uint8_t extractChannelNumberFromMessage(String msg, bool isTwoDigitChannel) {
  char channelNumberString[3];
  channelNumberString[0] = msg[7];
  if (isTwoDigitChannel == true) {
    channelNumberString[1] = msg[8];
  }
  return atoi(channelNumberString);
}

uint8_t extractAuxSendChannelNumberFromMessage(String msg, bool isTwoDigitChannel) {
  char channelNumberString[2];
  channelNumberString[0] = (isTwoDigitChannel == true) ? msg[14] : msg[13];
  return atoi(channelNumberString);
}

int8_t stereoIndexValue(String level) {
  char sVal[4];
  level.toCharArray(sVal, 3);
  if (strcmp(sVal, "-1") == 0) {
    return -1;
  }
  if (strcmp(sVal, "1") == 0) {
    return 1;
  }
  return 0;
}

String getDelimitedValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}


/**
   instead of dealing with floatvalues like "SETD=i.11.mix=0.1015118791" our
   internal handling is wit unsigned integers
*/
uint32_t levelToInternal(String level) {
  if (level == "0") {
    return 0;
  }
  uint32_t convertedLevel = level.toFloat() * levelFactor;
  return (convertedLevel >= levelFactor) ? levelFactor : convertedLevel;
}

/**
 * convert the internal level handling (1 - 1000000) to ui24r compatible float value (0-1)
 * thanks to https://forum.arduino.cc/index.php?topic=243660.msg1748437#msg1748437
 */
String internalToLevel(uint32_t level) {
  if(level >= levelFactor) {
    return String("1");  
  }
  if(level <= 0) {
    return String("0");  
  }
  float val = level / (levelFactor + 0.0);
  char buff[10];
  dtostrf(val, 4, 6, buff);  //4 is mininum width, 6 is precision
  
  return String(buff);
}
