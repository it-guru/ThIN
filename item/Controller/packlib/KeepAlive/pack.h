#ifndef _lib_KeepAlive_h_
#define _lib_KeepAlive_h_

#include "../../config.h"

#include "../../include/Interval.hpp"
#include "../../kernel/PackMaster.h"
#ifdef packlib_WebSrv
#include "../WebSrv/pack.h"
#endif


class KeepAlive : public Pack {
   private:
   Interval *i=NULL;

   public:
   void         keepAliveMessage(long cnt,int intervallFlag);
   #ifdef packlib_WebSrv
   void         keepAliveMessageWeb(ESP8266WebServer *srv,String &p);
   #endif
   virtual bool validateVariable(String &var,char *ov,String &val,String &msg);
   virtual void setup();
   virtual void loop();
};

#endif

