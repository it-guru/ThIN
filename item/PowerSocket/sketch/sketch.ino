 

// Flash Size "4M (1M SPIFFS)"

#include <stdio.h>
#include <stdarg.h>
#include <time.h>


#include "./index_html.h"
#include "./jquery_js.h"

#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
//#include <WiFiClient.h>
#include <ESP8266WebServer.h>
//#include <ESP8266mDNS.h>
#include <ArduinoJson.h>      //        https://github.com/bblanchon/ArduinoJson/wiki/Encoding%20JSON

#include <ESP8266HTTPClient.h>
#include "./console.h"
#include "./config.h"


 

ESP8266WebServer WebSrv(80);

#define WPS_BUTTON 0
#define SW1_BUTTON 3
#define SW1_RELAIS 2

int WpsTimer=0;
int SW1Timer=0;
int SW1State=0;

int StateTimer=0;
int LoopTimer=10;
//char buffer[255];


Con con;
int WiFiRunning=0;
int WpsButtonConfigured=0;


void handleActSwitchOn(){
   con.printf("SW1 on\n");
   con.setBlink(1);
   digitalWrite(SW1_RELAIS, LOW);
}

int CurrentSwitchState(){
   if (digitalRead(SW1_RELAIS) == LOW){
      return(HIGH);
   }
   return(LOW);
}

void handleActSwitchOff(){
   con.setBlink(0);
   digitalWrite(SW1_RELAIS, HIGH);
   //     if (SW1Timer>1000 && SW1Timer < 3000){
   //        con.printf("SW1 down trigger\n");
   //     }
}

void handleActSwitchToggle(){
   if (CurrentSwitchState()==HIGH){
      handleActSwitchOff();
   }
   else{
      handleActSwitchOn();
   }
}



void setup() {
  config_setup();
  pinMode(SW1_RELAIS,OUTPUT);
  digitalWrite(SW1_RELAIS, HIGH);
  
  
  con.setup(CON_SERIAL);
  pinMode(WPS_BUTTON,INPUT_PULLUP);
  pinMode(SW1_BUTTON,INPUT);
  con.mdelay(1000);
  if (digitalRead(WPS_BUTTON) == HIGH){  // Serial console only in prog-Mode
     con.switchTo(CON_TCP|CON_LED);
     
  }
  else{
     con.printf("Booting dongle ...\n");  
  }
  
  
  WpsButtonConfigured++;
}

void WpsSetup(){

  con.printf("Reset WiFi\n");
  WiFi.disconnect();
  con.printf("Starting WPS config ...\n");
  con.Blink(500);
  con.mdelay(800);
  if (WiFi.status() != WL_CONNECTED) {
      con.mdelay(5000);
      WiFi.beginWPSConfig();
      // Another long delay required.
      con.mdelay(4000);
      if (WiFi.status() == WL_CONNECTED) {
         con.Blink(100);
      }
      else {
         con.Blink(2000);
      }
  }
  if (WiFi.status() == WL_CONNECTED) {
     con.Blink(100);
     WiFiRunning=1;
  }
  con.mdelay(3000);
  
}


void Handle_index_html(){
   HTTPClient http;
   http.begin("http://10.123.100.21:8080/ESP8266/index.html");
   int httpCode = http.GET();
   if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
          String payload = http.getString();
          WebSrv.send(200, "text/html",payload);
          return;
      }   
   }
   WebSrv.send_P(200, "text/html",DATA_index_html);
   return;
}

void handleActLoadConfig(JsonObject *r){

   con.mdelay(1500);
   (*r)["syslog_server"]=Config.syslog_server;
   (*r)["mq_server"]=Config.mq_server;
}

void handleActSaveConfig(ESP8266WebServer *cgi){
   String syslog_server=(*cgi).arg("syslog_server");
   syslog_server.toCharArray(Config.syslog_server,128);

   String mq_server=(*cgi).arg("mq_server");
   mq_server.toCharArray(Config.mq_server,128);
   config_save();
}



void Handle_act(String uri){
  // String s("OK:");
  // String state=WebSrv.arg("state");
  // WebSrv.send(200, "text/plain",s+" state="+state);
  //   String s;
  //   WebSrv.send(200, "text/plain",s);
  String callback=WebSrv.arg("callback");
  DynamicJsonBuffer jsonBuffer;
  JsonObject& r = jsonBuffer.createObject();  

  String op=WebSrv.arg("op");
  if (op=="on"){
     handleActSwitchOn();
     r["exitcode"]="0"; 
     r["exitmsg"]="OK";
  }
  else if (op=="off"){
     handleActSwitchOff();
     r["exitcode"]="0"; 
     r["exitmsg"]="OK";
  }
  else if (op=="toggle"){
     handleActSwitchToggle();
     r["exitcode"]="0"; 
     r["exitmsg"]="OK";
  }
  else if (op=="loadConfig"){
     handleActLoadConfig(&r);
     r["exitcode"]="0"; 
     r["exitmsg"]="OK";
  }
  else if (op=="saveConfig"){
     handleActSaveConfig(&WebSrv);
     r["exitcode"]="0"; 
     r["exitmsg"]="OK";
  }
  else if (op=="status"){
     r["exitcode"]="0"; 
     r["exitmsg"]="OK";
  }
  else{
     r["exitcode"]="1"; 
     String a;
     a="unknown op '";
     a+=op;
     a+="'";
     r["exitmsg"]=a;
  }
  String st;
  st=CurrentSwitchState();
  r["type"]="answer";
  r["state"]=st;
  r["time"]=millis();
  String s;
  r.prettyPrintTo(s);
  WebSrv.send(200, "application/javascript",callback+"("+s+");");
  
  
}

void Handle_jquery_js(){
   WebSrv.send_P(200, "text/javascript",DATA_jquery_js);
}


void Handle_NotFound() {
  String path = WebSrv.uri();
  int index = path.indexOf("/json");
  if (index >= 0) {
    Handle_act(path);
  }
  
}









void loop() {
  int c;
  

//  if (trccli.connected()){
//     sprintf((char *)buffer,"BlinkTimer=%d\n",BlinkTimer);
//     trccli.write((char *)buffer,strlen(buffer));
//  }
  StateTimer+=LoopTimer;
  if (StateTimer>500000){
     StateTimer=0;
     con.printf("time=%ld\n",(long) millis());
  }

  
  if (WpsButtonConfigured){
     if (digitalRead(WPS_BUTTON) == LOW){
        //con.printf("WPS low = %d\n",WpsTimer);
        WpsTimer+=LoopTimer;
     }
     else{
        if ( WpsTimer>1000 && WpsTimer < 3000){
           con.printf("Starting WPS config ...\n");
           WpsSetup();
        }
        WpsTimer=0;
     }
  }

  if (digitalRead(WPS_BUTTON) == HIGH){
     if (digitalRead(SW1_BUTTON) == HIGH){
        SW1Timer+=LoopTimer;
        if (SW1Timer>100){
           if (SW1State!=1){
              handleActSwitchOn();
              SW1State=1;
           }
        }
     }
     else{
        if (SW1State!=0){
           handleActSwitchOff();
           SW1State=0;
        }
        SW1Timer=0;
     }
  }

  if (WiFiRunning==1){
     if (!WpsButtonConfigured){
        con.printf("WLAN Startup 1\n");
     }
     if (WiFi.status() == WL_CONNECTED){
        WiFiRunning++; 
        con.handleWiFiConnect();
        con.printf("WLAN is connected\n");

    // WebSrv.on("/", [](){
    //     WebSrv.send(200, "text/plain", "This is an index page.");
    // });
        //MDNS.begin ( "pug" );
        
        WebSrv.on("/",             Handle_index_html);
        WebSrv.on("/index.html",   Handle_index_html);
        WebSrv.begin();
        //WebSrv.on("/jquery.js",      Handle_jquery_js);
        WebSrv.on("/js/jquery.min.js", Handle_jquery_js);
        
        WebSrv.onNotFound(Handle_NotFound);
        
        
        
     }
  }
 
  
  
  if (WiFiRunning==0){
     con.Blink(100);
     WiFi.mode(WIFI_STA);
     con.mdelay(150);
     WiFi.begin("","");
     for(c=0;c<10;c++){
        if (WiFi.status() == WL_CONNECTED) {
           break;
        }
        con.mdelay(1000);
     }     
     if (WiFi.status() != WL_CONNECTED) {
        con.printf("ERROR: fail to connect to WLAN\n");
        con.Blink(2000);
        WiFiRunning=-1;
     }
     else{
        con.Blink(0);
        WiFiRunning++;
     }
  }
  if (WiFiRunning==2){
     WebSrv.handleClient();        
     con.TCPhandler();
  }
}
