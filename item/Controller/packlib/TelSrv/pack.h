#ifndef _lib_TelSrv_h_
#define _lib_TelSrv_h_

#include "../../kernel/PackMaster.h"
#include "../../include/DynHashTable.hpp"
#include "./libtelnet.h"

#include <ESP8266WiFi.h>
// Local default Defines (pre Config)
#define TELSRV_MAX_CLIENTS 10

#include "../../config.h"

typedef enum SessionStep_Type
{                                     // Always from the device point of view
   INIT    = 0,
   WELCOME,
   ASKUSER,  
   ASKPASSWORD,  
   ONLINE  
} SessionStepType;


class TelCli: public Print{
   protected:
   String     lineBuffer;
   String     runningCmd;
   String     term;
   bool       inShutdown;
   bool       welcomeSend; 
   SessionStepType     sessionStep;
   bool       isTTY;
   telnet_t * tc;
   WiFiClient fd;
   PackMaster *Controller=NULL;
   public:
   ~TelCli();
   Session    session;
   TelCli(PackMaster *c,WiFiClient newfd);
   bool handleClient();
   void handleTelnet(telnet_event_t *event);
   void sendPrompt();
   virtual size_t write(uint8_t);
   virtual size_t write(const uint8_t *buffer, size_t size);
};

class TelSrv : public Pack {
   protected:
   TelCli *serverClients[TELSRV_MAX_CLIENTS];

   public:
   WiFiServer *srv=NULL;
   TelSrv(){
      memset(serverClients,'\0',sizeof(serverClients));
      srv=new WiFiServer(23);
   };
   void startListener();
   void stopListener();
   virtual void setup();
   virtual void begin();
   virtual void loop();
   virtual void handleWiFiEvent(WiFiEvent_t e);
   virtual int  command(Session *session,Print *cli,char **args,int argn);
};

#endif

