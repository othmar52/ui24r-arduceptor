
#include "WebSocketClientMod.h"
#include <ESP8266WiFi.h>


const char *ssid =  "foo";     // replace with your wifi ssid and wpa2 key
const char *pass =  "bar";

WiFiClient client;

// @see https://platformio.org/lib/show/6123/ESP8266%20Websocket%20Client/examples
WebSocketClientMod ws(false);


unsigned long websocketAliveLast = 0;
uint16_t websocketInterval = 3000;

void setupNetwork() 
{
 
}

void connectWlan() {
   debug("Connecting to ");
   debug(ssid);
   WiFi.begin(ssid, pass);
   WiFi.setAutoReconnect (true);
}

void reconnectWlan() {
  if(WiFi.status() == WL_CONNECTED) {
    WiFi.disconnect(true);
    delay(500);
  }
  connectWlan();
}

void loopNetwork() {

  if (!ws.isConnected()) {
    debug("connecting to websocket...");
    ws.connect("10.0.1.124", "/", 80);
    return;
  }

  String msg;
  if (ws.getMessage(msg)) {
    handleIncomingMixerMessage(msg);
  }

  if(millis() - websocketAliveLast >= websocketInterval) {
    ws.send("3:::ALIVE");
    websocketAliveLast = millis();
  };
}
