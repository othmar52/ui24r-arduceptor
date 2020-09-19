



void handleEncoderChangeStageChooseInput(uint8_t eIdx, bool doIncrement) {
  // dont mind which encoder is rotated...
  if(doIncrement == true) {
    setNextChooseInput();
    return;
  }
  setPreviousChooseInput();
}

void handleEncoderChangeStageChooseAux(uint8_t eIdx, bool doIncrement) {
  // dont mind which encoder is rotated...
  if(doIncrement == true) {
    setNextChooseAux();
    return;
  }
  setPreviousChooseAux();  
}


uint8_t getPercentCollectedInputs() {
  // we need name + stereoIndex from each input
  uint8_t needValues = 2*INPUTS;
  uint8_t haveValues = collectedInputNames + collectedInputStereoIndex;
  if(haveValues >= needValues) {
    return 100;
  }
  return haveValues * (needValues/100.0);
}


uint8_t getPercentCollectedAux() {
  // we need name + stereoIndex from each aux
  uint8_t needValues = 2*AUX;
  uint8_t haveValues = collectedAuxNames + collectedAuxStereoIndex;
  if(haveValues >= needValues) {
    return 100;
  }
  return haveValues * (needValues/100.0);
}

uint8_t getPercentCollectedAllData() {
  uint16_t needValues = 0;
  uint16_t haveValues = 0;
  needValues += INPUTS; // collectedInputStereoIndex
  needValues += INPUTS; // collectedInputNames
  needValues += INPUTS; // collectedInputMutes
  needValues += INPUTS; // collectedInputToMasterLevels
  needValues += INPUTS*AUX; // collectedInputToAuxLevels
  needValues += AUX; // collectedAuxNames
  needValues += AUX; // collectedAuxStereoIndex
  needValues += AUX; // collectedAuxMutes
  needValues += AUX; // collectedAuxLevels

  
  haveValues += collectedInputStereoIndex;
  haveValues += collectedInputNames;
  haveValues += collectedInputMutes;
  haveValues += collectedInputToMasterLevels;
  haveValues += collectedInputToAuxLevels;
  haveValues += collectedAuxNames;
  haveValues += collectedAuxStereoIndex;
  haveValues += collectedAuxMutes;
  haveValues += collectedAuxLevels;
  ////debug("have: " + String(haveValues) + "\tneed: " + String(needValues));
  if(haveValues >= needValues) {
    return 100;
  }
  return haveValues / (needValues/100.0);
}

/**
 * input channel stuff
 */
void setNextChooseInput() {
  while (true) {
    currentChooseInputIndex++;
    if (currentChooseInputIndex >= INPUTS - 1) {
      currentChooseInputIndex = 0;
    }
    if (inputStereoIndex[currentChooseInputIndex] == 1) {
      continue;
    }
    return;
  }
}

void setPreviousChooseInput() {
  while (true) {
    if (currentChooseInputIndex < 1) {
      currentChooseInputIndex = INPUTS;
    }
    currentChooseInputIndex--;
    if (inputStereoIndex[currentChooseInputIndex] == 1) {
      continue;
    }
    return;
  }
}

String getCurrentChooseInputName() {
  return inputNames[currentChooseInputIndex];
}


String getCurrentChooseInputChannelString() {
  String theChannelString = String(currentChooseInputIndex + 1);
  theChannelString += (inputStereoIndex[currentChooseInputIndex] == 0) ? "+" + String((currentChooseInputIndex + 2)) + "" : "";
  return theChannelString;
}

/**
 * aux channel stuff
 */
void setNextChooseAux() {
  while (true) {
    currentChooseAuxIndex++;
    if (currentChooseAuxIndex >= AUX - 1) {
      currentChooseAuxIndex = 0;
    }
    if (auxStereoIndex[currentChooseAuxIndex] == 1) {
      continue;
    }
    return;
  }
}

void setPreviousChooseAux() {
  while (true) {
    if (currentChooseAuxIndex == 0) {
      currentChooseAuxIndex = AUX;
    }
    currentChooseAuxIndex--;
    if (auxStereoIndex[currentChooseAuxIndex] == 0) {
      continue;
    }
    return;
  }
}

String getCurrentChooseAuxName() {
  return auxNames[currentChooseAuxIndex];
}

// channel numbers which are printed on the mixer hardware
String getCurrentChooseAuxChannelString() {
  String theChannelString = String((currentChooseAuxIndex + 1));
  theChannelString += (auxStereoIndex[currentChooseAuxIndex] == 0) ? "+" + String((currentChooseAuxIndex + 2)) + "" : "";
  return theChannelString;
}
