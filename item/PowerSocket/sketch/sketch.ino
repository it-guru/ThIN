// Flash Size "4M (1M SPIFFS)"
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include "./console.h"
#include "./config.h"
#include "./package.h"
//#include "./mqtt_pack.h"
#include "./Pack_Config.h"
#include "./Pack_WebCtrl.h"
#include "./Pack_Dev_PSOCK.h"
#include "./Pack_Console.h"

PackStack obj;
 
void setup() {
  WiFi.onEvent([](WiFiEvent_t e){obj.handleWiFiEvent(e);});
  obj.add(new Pack_Console);
  obj.add(new Pack_Dev_PSOCK);
  obj.add(new Pack_Config);
  obj.add(new Pack);
 // obj.add(new MQTT_Pack);
  obj.add(new Pack_WebCtrl);
  
  obj.setup();
 
  //WiFi.hostname("ThIN_PSOCK_01");

  obj.begin();
 // ((Pack_Config *) obj.find("Config"))->save();
}

void loop() {
  obj.loop();
}
