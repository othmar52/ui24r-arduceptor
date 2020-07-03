
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
               
   Serial.println("Connecting to ");
   Serial.println(ssid); 

   WiFi.begin(ssid, pass);
   WiFi.setAutoReconnect (true);
   //while (WiFi.status() != WL_CONNECTED) 
   //   {
   //     delay(500);
   //     Serial.print(".");
   //   }
  //Serial.println("");
  //Serial.println("WiFi connected");
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
    Serial.println("connecting to websocket...");
    //ws.connect("echo.websocket.org", "/", 443);
    //ws.connect("10.0.1.251", "/", 2332);
    ws.connect("10.0.1.124", "/", 80);
  } else {
    //Serial.println("websocket is alread connected...");
    //ws.send("3:::ALIVE");

    String msg;
    if (ws.getMessage(msg)) {
      handleIncomingMixerMessage(msg);
    }
  }

  if(millis() - websocketAliveLast >= websocketInterval) {
    ws.send("3:::ALIVE");
    websocketAliveLast = millis();
  };
}
