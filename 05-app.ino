


unsigned long lastWlanConnectAttempt = 0;
uint16_t wlanConnectInterval = 5000;



uint8_t currentChooseInputIndex = INPUTS + 1;
uint8_t currentChooseAuxIndex = AUX + 1;
bool myInputAlreadyDefined = false;
bool myAuxAlreadyDefined = false;

uint32_t stepSize = levelFactor/20;

void loopApp() {
  switch(currentStage) {
    case STAGE_WELCOME:
      showWelcomeScreen();
      delay(1000);
      currentStage++;
      connectWlan();
      return;
    case STAGE_CONNECTING_WLAN:
      showConnectingToWlanScreen();
      while (WiFi.status() != WL_CONNECTED) 
      {
        delay(500);
        Serial.print(".");
      }
      showConnectedToWlanScreen();
      delay(1000);
      currentStage = STAGE_CONNECTING_MIXER;
      return;
    case STAGE_CONNECTING_MIXER:
      showConnectingToMixerScreen();
      while (!ws.isConnected()) 
      {
        delay(500);
        Serial.print(".");
      }
      showConnectedToMixerScreen();
      delay(500);
      currentStage = STAGE_COLLECTINPUTS;
      
      // currentStage = STAGE_COLLECTALLDATA;    // temporary...
      return;
    case STAGE_COLLECTINPUTS: {
      if(getPercentCollectedInputs() != 100) {
        showCollectingInputsScreen();
        return;
      }
      setNextChooseInput();
      currentStage = STAGE_CHOOSEINPUT;
      return;
    }
    case STAGE_CHOOSEINPUT:
      ////////////////////////////////////////////////////////////
      // TODO: configure stuff with encoder.
      // for now set the input hardcoded
      //myInput = 0;
      //myInputAlreadyDefined = true;
      //currentStage = STAGE_CHOOSEAUX;
      //return;
      ////////////////////////////////////////////////////////////
      if(myInputAlreadyDefined == true) {
        currentChooseInputIndex = myInput;
        currentStage = STAGE_COLLECTAUX;
        return;
      }
      showChooseInputScreen();
      return;
    case STAGE_COLLECTAUX: {
      if(getPercentCollectedAux() != 100) {
        showCollectingAuxScreen();
        return;
      }
      setNextChooseAux();
      currentStage = STAGE_CHOOSEAUX;
      return;
    }
    case STAGE_CHOOSEAUX:
      if(myAuxAlreadyDefined == true) {
        currentChooseAuxIndex = myAux;
        currentStage = STAGE_COLLECTALLDATA;
        return;
      }
      showChooseAuxScreen();
      ////////////////////////////////////////////////////////////
      // TODO: configure stuff with encoder.
      // for now set the aux hardcoded
      //myAux = 0;
      //myAuxAlreadyDefined = true;
      //currentStage = STAGE_RUN;
      ////////////////////////////////////////////////////////////
      return;
    case STAGE_COLLECTALLDATA:
      if(getPercentCollectedAllData() != 100) {
        showCollectingAllDataScreen();
        return;
      }
      allDataCollected = true;
      updateGroupMix();
      currentStage = STAGE_RUN;
      return;
    case STAGE_RUN:
      showLevelScreen();
      return;
  }
}

void handleEncoderChange(uint8_t eIdx, bool doIncrement) { 
  switch(currentStage) {
    case STAGE_CHOOSEINPUT:
      return handleEncoderChangeStageChooseInput(eIdx, doIncrement);
    case STAGE_CHOOSEAUX:
      return handleEncoderChangeStageChooseAux(eIdx, doIncrement);
    case STAGE_RUN:
      return handleEncoderChangeStageRun(eIdx, doIncrement);
  }
}


void handleEncoderPush(uint8_t eIdx, int holdTime) { 
  switch(currentStage) {
    case STAGE_CHOOSEINPUT:
      if(holdTime < 300) {
        return;  
      }
      myInput = currentChooseInputIndex;
      myInputAlreadyDefined = true;
      return;

    case STAGE_CHOOSEAUX:
      if(holdTime < 300) {
        return;  
      }
      myAux = currentChooseAuxIndex;
      myAuxAlreadyDefined = true;
      return;
  }
}

void handleEncoderChangeStageRun(uint8_t eIdx, bool doIncrement) {
  // TODO relation between encoder index and function to call
  if(eIdx == 5) {  // enc 0: headphone volume
    return changeHeadphoneVolume(doIncrement);
  }
  
  if(eIdx == 0) { // enc 1: groupMix (all others) to headphone
    return changeGroupMixVolume(doIncrement);
  }
  if(eIdx == 5) { // enc 2: me to headphones
    changeMyInputToHeadphonesVolume(doIncrement);
    if(linkMyLevelMode == true) {
      setMyInputToMasterVolume(inputToAuxLevels[myInput][myAux]);
    }
    return;
  }
  if(eIdx == 5) { // enc 3: me to master volume
    changeMyInputToMasterVolume(doIncrement);
    if(linkMyLevelMode == true) {
      setMyInputToHeadphonesVolume(inputToMasterLevels[myInput]);
    }
    return;
  }



  
  
}

void changeHeadphoneVolume(bool doIncrement) {
  uint32_t newValue = auxLevels[myAux];
  newValue = (doIncrement == true)
    ? incrementInternalLevelBy(newValue, stepSize)
    : decrementInternalLevelBy(newValue, stepSize);


  if(auxLevels[myAux] == newValue) {
    // no change
    return;
  }
  auxLevels[myAux] = newValue;
  String newValueFloat = internalToLevel(newValue);
  sendMixerCommand(
    String("a." + String(myAux) +".mix"),
    newValueFloat
  );
  if(auxStereoIndex[myAux] == 0) {
    auxLevels[myAux+1] = newValue;
    sendMixerCommand(
      String("a." + String(myAux+1) +".mix"),
      newValueFloat
    );
  }
}

void changeMyInputToHeadphonesVolume(bool doIncrement) {
  uint32_t newValue = inputToAuxLevels[myInput][myAux];
  newValue = (doIncrement == true)
    ? incrementInternalLevelBy(newValue, stepSize)
    : decrementInternalLevelBy(newValue, stepSize);

  setMyInputToHeadphonesVolume(newValue);
}

void setMyInputToHeadphonesVolume(uint32_t newValue) {
  if(inputToAuxLevels[myInput][myAux] == newValue) {
    // no change
    return;
  }
  handleMonoStereoCombinationsForAuxMix(myInput, myAux, newValue);
}

/**
 * TODO: change arg(s) to something like "i.%d.aux.%d.value"
 */
void handleMonoStereoCombinationsForAuxMix(uint8_t inIndex, uint8_t auxIndex, uint32_t newValueInternal) {

  const bool inIsStereo = (inputStereoIndex[inIndex] == 0) ? true : false;
  const bool auxIsStereo = (auxStereoIndex[auxIndex] == 0) ? true : false;
  const String newValueFloat = internalToLevel(newValueInternal);
  //debug("inIsStereo " + String(inIsStereo) + " for i." + String(inIndex) );
  //debug("auxIsStereo " + String(auxIsStereo) + " for a." + String(auxIndex) );

  // all scenarios: mono -> mono AND stereo -> stereo  AND stereo -> mono AND mono-> stereo
  if(inputToAuxLevels[inIndex][auxIndex] != newValueInternal) {
    inputToAuxLevels[inIndex][auxIndex] = newValueInternal;
    sendMixerCommand(
      String("i." + String(inIndex) + ".aux." + String(auxIndex) + ".value"),
      newValueFloat
    );
  }

  // scenario stereo -> stereo
  if(inIsStereo == true && auxIsStereo == true) {
    if(inputToAuxLevels[inIndex+1][auxIndex+1] != newValueInternal) {
      inputToAuxLevels[inIndex+1][auxIndex+1] = newValueInternal;
      sendMixerCommand(
        String("i." + String(inIndex+1) + ".aux." + String(auxIndex+1) + ".value"),
        newValueFloat
      );
    }
    if(inputToAuxLevels[inIndex+1][auxIndex] != 0) {
      inputToAuxLevels[inIndex+1][auxIndex] = 0;
      sendMixerCommand(
        String("i." + String(inIndex+1) + ".aux." + String(auxIndex) + ".value"),
        "0"
      );
    }
    if(inputToAuxLevels[inIndex][auxIndex+1] != 0) {
      inputToAuxLevels[inIndex][auxIndex+1] = 0;
      sendMixerCommand(
        String("i." + String(inIndex) + ".aux." + String(auxIndex+1) + ".value"),
        "0"
      );
    }
  }
  // scenario mono -> stereo
  if(inIsStereo == false && auxIsStereo == true) {
    if(inputToAuxLevels[inIndex][auxIndex+1] != newValueInternal) {
      inputToAuxLevels[inIndex][auxIndex+1] = newValueInternal;
      sendMixerCommand(
        String("i." + String(inIndex) + ".aux." + String(auxIndex+1) + ".value"),
        newValueFloat
      );
    }
  }
  // scenario stereo -> mono
  if(inIsStereo == true && auxIsStereo == false) {
    if(inputToAuxLevels[inIndex+1][auxIndex] != newValueInternal) {
      inputToAuxLevels[inIndex+1][auxIndex] = newValueInternal;
      sendMixerCommand(
        String("i." + String(inIndex+1) + ".aux." + String(auxIndex) + ".value"),
        newValueFloat
      );
    }
  }
}


void changeMyInputToMasterVolume(bool doIncrement) {
  uint32_t newValue = inputToMasterLevels[myInput];
  newValue = (doIncrement == true)
    ? incrementInternalLevelBy(newValue, stepSize)
    : decrementInternalLevelBy(newValue, stepSize);

  setMyInputToMasterVolume(newValue);
}

void setMyInputToMasterVolume(uint32_t newValue) {
  if(inputToMasterLevels[myInput] == newValue) {
    // no change
    return;  
  }
  inputToMasterLevels[myInput] = newValue;
  String newValueFloat = internalToLevel(newValue); 
  sendMixerCommand(
    String("i." + String(myInput) +".mix"),
    newValueFloat
  );
  if(inputStereoIndex[myInput] == 0) {
    inputToMasterLevels[myInput+1] = newValue;
    sendMixerCommand(
      String("i." + String(myInput+1) +".mix"),
      newValueFloat
    );
  }
}


void changeGroupMixVolume(bool doIncrement) {
  uint32_t newValue = groupMixValue;
  newValue = (doIncrement == true)
    ? incrementInternalLevelBy(newValue, stepSize)
    : decrementInternalLevelBy(newValue, stepSize);

  groupMixValue = newValue;
  updateGroupMix();
}

void updateGroupMix() {

  const float factor = groupMixValue / (zeroDbPos+0.0);
  uint32_t auxLevelForMyMix;
  //debug("groupMixValue: "+ String(groupMixValue));
  //debug("factor: "+ String(factor));
  //debug("zeroDbPos: "+ String(zeroDbPos));
  for(uint8_t i=0;i<INPUTS; i++) {
    if(i == myInput) {
      // skip my mix to headphone because we have a separate encoder for that
      continue;
    }
    if(inputStereoIndex[i] == 1) {
      // skip linked stereo channel (will be handeled by handleMonoStereoCombinationsForAuxMix() )
      continue;
    }

    auxLevelForMyMix = (inputMutes[i] == false) ? inputToMasterLevels[i] * (factor+0.0) : 0;
    auxLevelForMyMix = (auxLevelForMyMix > levelFactor) ? levelFactor : auxLevelForMyMix;
    //debug("newValueFloat: "+ String(newValueFloat) + " for i." + String(i));
    //if(auxLevelForMyMix == inputToAuxLevels[i][myAux]) {
    //  // no change
    //  continue;
    //}
    handleMonoStereoCombinationsForAuxMix(i, myAux, auxLevelForMyMix);
  }
}

void sendMixerCommand(String paramName, String(paramValue)) {
  String cmd = "3:::SETD^" + paramName + "^" + paramValue;
  debug("SENDING: " + cmd);
  ws.send(cmd);
}


uint32_t incrementInternalLevelBy(uint32_t level, uint32_t factor) {
  const uint32_t maxInternalLevel = (limitToZeroDbMode == true) ? zeroDbPos : levelFactor;
  if (level + factor > maxInternalLevel) {
    return maxInternalLevel;
  }
  return level + factor;
}

uint32_t decrementInternalLevelBy(uint32_t level, uint32_t factor) {
  const uint32_t maxInternalLevel = (limitToZeroDbMode == true) ? zeroDbPos : levelFactor;
  if (level < factor) {
    return 0;
  }
  if (level - factor > maxInternalLevel) {
    return maxInternalLevel;
  }
  return level - factor;
}
