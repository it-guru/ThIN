#ifndef _lib_Net_h_
#define _lib_Net_h_

#include "../../kernel/PackMaster.h"

// Local default Defines (pre Config)

#include "../../config.h"

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

class Net : public Pack {
   private:
   uint8_t  WpsStep=0;
   unsigned long WpsTimer; 
   unsigned long WpsStart; 


   protected:
   void Connect();

   public:
   virtual void setup();
   virtual void loop();
   virtual void handleWiFiEvent(WiFiEvent_t e);
   virtual int  command(Session *session,Print *cli,char **args,int argn);
   bool validateVariable(String &var,String &val,String &msg);
   void handleSystemEvent(SysEvent *e,const char *source);
};

#endif

