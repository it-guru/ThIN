#ifndef _lib_NTPCli_h_
#define _lib_NTPCli_h_

#include "../../kernel/PackMaster.h"
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>


// Local default Defines (pre Config)

#include "../../config.h"

class NTPCli : public Pack {
   protected:
   void Connect();
   NTPClient *ntp;

   public:
   virtual void setup();
   virtual void loop();
   virtual void handleWiFiEvent(WiFiEvent_t e);
   virtual int  command(Session *session,Print *cli,char **args,int argn);
};

#endif

