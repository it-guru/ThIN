#ifndef _lib_LEDdimmer1_h_
#define _lib_LEDdimmer1_h_

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



class LEDdimmer1 : public Pack {
   protected:
   void Connect();
   gpioListen   gpioLst[MAX_GPIO];
   void broadcastNewGPIOstate(long DevicePos,int newstate);
   void monGPIO(int gpio);
   #ifdef packlib_WebSrv
   void progCont();
   #endif


   public:
   virtual void setup();
   virtual void loop();
   virtual int  command(Session *session,Print *cli,char **args,int argn);
   virtual void handleSystemEvent(SysEvent *e,const char *source);
   bool validateVariable(String &var,char *ov,String &val,String &msg);

   void setLEDLevel(int pio,float f);

   private:
   void setGPIOblink(int gpio,long ti,long cnt);
};

#endif

