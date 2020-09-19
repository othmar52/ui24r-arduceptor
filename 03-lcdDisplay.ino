 
 
#include <SPI.h>               // include SPI library
#include <Adafruit_GFX.h>      // include adafruit graphics library
#include <Adafruit_PCD8544.h>  // include adafruit PCD8544 (Nokia 5110) library

#define LCD_X 84
#define LCD_Y 48
#define inputChannels 1

// Nokia 5110 LCD module connections (CLK, DIN, D/C, CS, RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(D8, D7, D6, D0, D5);



const uint8_t totalBars = (inputChannels*2)+2;

// set all x coordinates for channels
uint8_t bars[totalBars][2] = {};
const uint8_t pixelSpace = 3;
const uint8_t headphoneBarWidth = 6;
const uint8_t barWidth = (LCD_X - headphoneBarWidth - (inputChannels*2 + 1)*pixelSpace) / (inputChannels*2 + 1);


void setupDisplaystuff(){
  calculateBarDimensions();
  display.begin();
  // init done
 
  // you can change the contrast around to adapt the display
  // for the best viewing!
  display.setContrast(50);

  display.setRotation(2);
 
  display.display();
  
 
}
 


void loopDisplayStuff() {

}

void showWelcomeScreen() {
  showText("welcome");
}

void showConnectingToWlanScreen() {
  showText("connect wlan..");
}

void showConnectedToWlanScreen() {
  showText("connected");
}

void showConnectingToMixerScreen() {
  showText("connecting ui24..");
}

void showConnectedToMixerScreen() {
  showText("connected ui24..");
}

void showChooseInputScreen() {
  showText("choose input\n\n" + String(getCurrentChooseInputName()));
}

void showChooseAuxScreen() {
  showText("choose output\n\n" + String(getCurrentChooseAuxName()));
}

void showCollectingInputsScreen() {
  showText("collecting\ninputs...");
  progressBarOverlay(getPercentCollectedInputs());
}

void showCollectingAuxScreen() {
  showText("collecting\noutputs...");
  progressBarOverlay(getPercentCollectedAux());
}

void showCollectingAllDataScreen() {
  showText("collecting\nmixer data...");
  progressBarOverlay(getPercentCollectedAllData());
}

void progressBarOverlay(uint8_t percent) {
  uint8_t progressBarWidth = LCD_X - 20;
  uint8_t progressBarHeight = 6;
  // x, y, width, height, color
  display.drawRect(10, LCD_Y-15, progressBarWidth, progressBarHeight, 1);  
  display.fillRect(10, LCD_Y-15, map(percent, 0, 100, 0, progressBarWidth), progressBarHeight, 1);
  display.display();
}



void showCollectingAllDataScreen___NUMERIC_PERCENT() {
  showText("collecting\nmixer data...\n" + String(getPercentCollectedAllData()) + "%");
}

void showLevelScreen() {
  display.clearDisplay();

  uint8_t barHeight;
  for(uint8_t i = 0; i<totalBars; i ++) {
    
    if (i == 1) {
      // group slider
      barHeight = levelToBarHeight(groupMixValue);
      display.fillRect(bars[i][0], LCD_Y-barHeight, bars[i][1] - bars[i][0], barHeight, 1);
      continue;
    }
    if (i > 0 && i < totalBars - inputChannels) {
      // me out to my headphones
      barHeight = levelToBarHeight(inputToAuxLevels[myInput][myAux]);
      display.drawRect(bars[i][0], LCD_Y-barHeight, bars[i][1] - bars[i][0], barHeight, 1);  
    } else {
      if(i == 0) {
        // my headphone level
        barHeight = levelToBarHeight(auxLevels[myAux]);
      } else {
        barHeight = levelToBarHeight(inputToMasterLevels[myInput]);  
      }
      // me out to master and headphone level
      display.fillRect(bars[i][0], LCD_Y-barHeight, bars[i][1]- bars[i][0], barHeight, 1);  
    }
  }

  display.display();
}

uint8_t levelToBarHeight(uint32_t barValue) {
  const uint32_t maxInternalLevel = (limitToZeroDbMode == true) ? zeroDbPos : levelFactor;
  return map(barValue, 0, maxInternalLevel, 0, LCD_Y);
}

void showText(String textToDisplay) {
  display.clearDisplay();
  
  // text display tests
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(3,3);
  display.println(textToDisplay);
  display.display();
}




void calculateBarDimensions()
{

  bars[0][0] = 0; // first bar is headphone level
  bars[0][1] = headphoneBarWidth; // first bar is headphone level

  // last bars me out
  uint8_t tmp = LCD_X-1;
  for(int i = 0; i < inputChannels; i++) {
    bars[1+(inputChannels*2)-i][1] = tmp;
    tmp -= barWidth;
    bars[1+(inputChannels*2)-i][0] = tmp;
    tmp -= pixelSpace;
  }
  // middle bars me to headphone
  for(int i = 0; i < inputChannels; i++) {
    bars[1+(inputChannels)-i][1] = tmp;
    tmp -= barWidth;
    bars[1+(inputChannels)-i][0] = tmp;
    tmp -= pixelSpace;
  }
  // group bar
  bars[1][1] = tmp;
  tmp -= barWidth;
  bars[1][0] = tmp;

}
