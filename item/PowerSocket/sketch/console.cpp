#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "./console.h"


#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
//#include <WiFiClient.h>
#include <ESP8266WebServer.h>
//#include <ESP8266mDNS.h>

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

void Con::setup_SERIAL(){
   Serial.begin(9600);
   Serial.println("");
   this->mdelay(100);
}


void Con::setup_LED(){
   pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
}

void Con::setup_TCP(){
   
}
   
void Con::TCPhandler(){
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

void Con::handleWiFiConnect(){
  this->printf("handleWiFiConnect %d\n",this->curType&CON_TCP);
  if (this->curType&CON_TCP){
     ConSrv.begin();
     ConSrv.setNoDelay(true);   
  }
}

void Con::setBlink(int newBlink){
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

void Con::mdelay(int n){
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

void Con::setup(int t){
   curType=t;
   if (this->curType&CON_SERIAL){
      this->setup_SERIAL();
   }
}

void Con::switchTo(int t){
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

void Con::printf(const char *fmt, ...){
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

void Con::Blink(int n){
   BlinkWhile=1;
   BlinkSpeed=n;
   if (n==0){
      BlinkWhile=0;
      this->setBlink(0);
   }
}

void Con::Blink(int n,int pBlinkWhile){
   BlinkWhile=pBlinkWhile;
   BlinkSpeed=n;
}


