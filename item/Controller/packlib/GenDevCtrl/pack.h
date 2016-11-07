#ifndef _lib_GenDevCtrl_h_
#define _lib_GenDevCtrl_h_

#include "../../config.h"
#include "../../kernel/PackMaster.h"
#include "../../include/Interval.hpp"


#ifdef packlib_WebSrv
#include "../WebSrv/pack.h"
#endif



typedef struct _pinmon{
   char   *name;
   long   btnDevPos;
   long   swiDevPos;

   

} pinMonStruct;



typedef struct _gpioListen{
   boolean        watch=false;

   boolean        curBlinkState=false;
   Interval       *i;

   long           DevicePos=-1;

   int            oldShortListenState=0;
   bool           curLogicalState=false;
   bool           curLongState=false;
   bool           LongStateListen=false;
   unsigned long  oldShortListenTime=0;

} gpioListen;

#define MAX_GPIO  15



class GenDevCtrl : public Pack {
   protected:
   void Connect();
   gpioListen   gpioLst[MAX_GPIO];
   void broadcastNewGPIOstate(long DevicePos,int newstate);
   void monGPIO(int gpio);

   public:
   virtual void setup();
   virtual void loop();
   virtual void handleWiFiEvent(WiFiEvent_t e);
   virtual int  command(Session *session,Print *cli,char **args,int argn);
   virtual void handleSystemEvent(SysEvent *e,const char *source);

   private:
   void setGPIOblink(int gpio,long ti,long cnt);
};

#endif

