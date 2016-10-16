#ifndef CONSOLE_H
#define CONSOLE_H



#include <stdio.h>
#include <stdarg.h>
#include <time.h>

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




class Con
{
   private:
   int tDelay=10;
   int curType;
   int BlinkSpeed=0;
   int BlinkTimer=0;
   int BlinkState=0;
   int BlinkWhile=0;
   

   virtual void setup_SERIAL();
   virtual void setup_LED();
   virtual void setup_TCP();
   
   public:
   virtual void TCPhandler();
   virtual void handleWiFiConnect();
   virtual void setBlink(int newBlink);
   virtual void mdelay(int n);
   virtual void setup(int t);
   virtual void switchTo(int t);
   virtual void printf(const char *fmt, ...);
   virtual void println(const char *);
   virtual void Blink(int n);
   virtual void Blink(int n,int pBlinkWhile);
};

extern Con con;

#endif
