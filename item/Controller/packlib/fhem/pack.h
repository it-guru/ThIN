#ifndef _lib_fhem_h_
#define _lib_fhem_h_

#include "../../config.h"
#include "../../kernel/PackMaster.h"
#include "../../include/Interval.hpp"


class fhem : public Pack {
   private:
   Interval *con=NULL;
   Interval *i=NULL;
   bool WiFiOnline=false;

   protected:
   WiFiClient *fhemsrv;
   int        fastConnect=0;

   String     readCommand();

   public:
   void         srvConnect();
   void         srvInitSession();
   void         processSrvMessages();
   void         srvSend();
   void setup();
   void loop();
   void handleWiFiEvent(WiFiEvent_t e);
   int  command(Session *session,Print *cli,char **args,int argn);
   void handleSystemEvent(SysEvent *e,const char *source);
   bool validateVariable(String &var,char *ov,String &val,String &msg);

};

#endif

