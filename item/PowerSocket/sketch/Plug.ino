 

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


 

// int ets_vsprintf(char *str, const char *format, va_list argptr);
// int ets_vsnprintf(char *buffer, size_t sizeOfBuffer,  const char *format, va_list argptr);
int ets_vsprintf(char *str, const char *format, va_list arg);
int ets_vsnprintf(char *buffer, size_t sizeOfBuffer, size_t count, const char *format, va_list arg);

#define CON_LED    1
#define CON_SERIAL 2
#define CON_TCP    4
#define CON_NULL   0

WiFiServer ConSrv(7);
WiFiClient ConTcp;

ESP8266WebServer WebSrv(80);


class Con
{
   private:
   int tDelay=10;
   int curType;
   int BlinkSpeed=0;
   int BlinkTimer=0;
   int BlinkState=0;
   int BlinkWhile=0;
   

   virtual void setup_SERIAL(){
      Serial.begin(9600);
      Serial.println("");
      this->mdelay(100);
   }
   virtual void setup_LED(){
      pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
   }
   virtual void setup_TCP(){
      
   }
   
   public:
   virtual void TCPhandler(){
      if (this->curType&CON_TCP){
         if (ConSrv.hasClient()){
            if (ConTcp.connected()){
               ConTcp.stop();
            }
            ConTcp=ConSrv.available();
            ConTcp.write("Hello\n",6);
         }  
      }
   }
   virtual void handleWiFiConnect(){
     this->printf("handleWiFiConnect %d\n",this->curType&CON_TCP);
     if (this->curType&CON_TCP){
        ConSrv.begin();
        ConSrv.setNoDelay(true);   
     }
   }
   virtual void setBlink(int newBlink){
      if (this->curType&CON_SERIAL){
         if (BlinkState==1 && newBlink==0){
            this->printf("Blink off\n");
         }
         else if (BlinkState==0 && newBlink!=0){
            this->printf("Blink on\n");
         }
      }
      if (curType&CON_LED){
         if (BlinkState==1 && newBlink==0){
            digitalWrite(LED_BUILTIN, HIGH);
         }
         else if (BlinkState==0 && newBlink!=0){
            digitalWrite(LED_BUILTIN, LOW);
         }
      }
      BlinkState=newBlink;
   }
   virtual void mdelay(int n){
      int c;
      for(c=n;c>0;c-=tDelay){
         delay(tDelay);
         if (BlinkSpeed>0){
            BlinkTimer+=tDelay;
         }
         if (BlinkWhile>0){
            if (BlinkTimer>BlinkSpeed*2){
               BlinkTimer=0;
            }
            else if (BlinkTimer>BlinkSpeed){
               this->setBlink(1);
            }
            else{
               this->setBlink(0);
            } 
         }
         //this->printf("BlinkWhile=%d BlinkSpeed=%d BlinkTimer=%d BlinkState=%d\n",BlinkWhile,BlinkSpeed,BlinkTimer,BlinkState);
         if (BlinkWhile>1){
            BlinkWhile-=tDelay;
            if (BlinkWhile==1) BlinkWhile++;
            if (BlinkWhile<=0){
               BlinkWhile=0;
               BlinkSpeed=0;
               this->setBlink(0);
            }
         }
         if (BlinkWhile>0){
            
         }
      }
   }
   virtual void setup(int t){
      curType=t;
      if (this->curType&CON_SERIAL){
         this->setup_SERIAL();
      }
   }
   virtual void switchTo(int t){
      if (this->curType&CON_SERIAL){
         Serial.flush();
         this->mdelay(100);
        // Serial.end();
      }
      curType=t;
      if (curType&CON_SERIAL){
         this->setup_SERIAL();
      }
      if (curType&CON_LED){
         this->setup_LED();
      }
      if (curType&CON_TCP){
         this->setup_TCP();
      }
   }
   virtual void printf(const char *fmt, ...){
      int l;
      char buffer[255];

      l=strlen(fmt);
      va_list ap;
      va_start(ap, fmt);
      ets_vsnprintf(buffer,255, fmt, ap);
      va_end(ap);
      if (curType&CON_SERIAL){ 
         Serial.print(buffer);
      }
      if (this->curType&CON_TCP){ 
         if (ConTcp.connected()){
            ConTcp.print(buffer);
         }
      }
   }
   virtual void Blink(int n){
      BlinkWhile=1;
      BlinkSpeed=n;
      if (n==0){
         BlinkWhile=0;
         this->setBlink(0);
      }
   }
   virtual void Blink(int n,int pBlinkWhile){
      BlinkWhile=pBlinkWhile;
      BlinkSpeed=n;
   }
};

#define WPS_BUTTON 0
#define SW1_BUTTON 3
#define SW1_RELAIS 2

int WpsTimer=0;
int SW1Timer=0;  
int StateTimer=0;
int LoopTimer=10;
//char buffer[255];


Con con;
int WiFiRunning=0;
int WpsButtonConfigured=0;


void setup() {
 
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

void Handle_act(String uri){
  // String s("OK:");
  // String state=WebSrv.arg("state");
  // WebSrv.send(200, "text/plain",s+" state="+state);
  //   String s;
  //   WebSrv.send(200, "text/plain",s);
  String callback=WebSrv.arg("callback");
  DynamicJsonBuffer jsonBuffer;
  JsonObject& r = jsonBuffer.createObject();  
  r["exitcode"]="OK";
  r["type"]="answer";
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
  int index = path.indexOf("/act");
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
           con.printf("SW1 on\n");
           con.setBlink(1);
           digitalWrite(SW1_RELAIS, LOW);
        }
     }
     else{
        con.setBlink(0);
        digitalWrite(SW1_RELAIS, HIGH);
        if (SW1Timer>1000 && SW1Timer < 3000){
           con.printf("SW1 down trigger\n");
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
