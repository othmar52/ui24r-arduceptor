# ui24r-arduceptor
Arduino based project for each participant of synth jam sessions.

## Required hardware
  * Soundcraft Ui24r digital mixing console
  * NodeMCU
  * PCD8544 LCD Display 
  * 4 Rotary push encoders
  * MCP23017 port expander
  
## The Goal
Each musician connects his audio out (mono or stereo) to any input of the ui24r mixing console. The musicians headphones are plugged to one of the ui24r's AUX outputs (mono or stereo).
The functionalitys of the 4 rotary encoders are:  
  * Headphone volume
  * Group mix (volume of all others except the musicians audio) very similar to ui24r's native more-me functionality
  * volume of musicians audio to his headphones
  * volume of musicians audio to master
