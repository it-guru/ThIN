#ifndef _PACKMASTER_H_
#define _PACKMASTER_H_

#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#include "../config.h"
#include "../include/DynHashTable.hpp"
#include "../include/Interval.hpp"
#include <Ticker.h>

class PackMaster;
class Pack;
class PackStdCons;
class PackStdNet;

extern PackMaster *Controller;

class Session
{
   public:
   String user;
   String proto;
   String ipaddr;
   long   uid=-1;
   int8_t authLevel=0;
};

// Command Handling

#define CMD_OK     1
#define CMD_PART   2
#define CMD_SYNTAX 3
#define CMD_RUNS   4
#define CMD_FAIL  10

// Device Management


typedef enum SysDeviceType
{                                     // Always from the device point of view
   AIO    = 0,                         
   DIN,                                 
   DOT,        // digital (1|0)               Output
   DIO,        // digital = 1 bit     
   IIN,        // integer (-32768 - 32768)    Input
   IOT,        // integer= singed int 2 Byte
   IIO,                               
   AIN,        // analog (1.2E-38 to 3.4E+38) Input
   AOT,        // analog = flot 4 Byte
   SIN,        // commands (string)           Input (no space allowed!)
   SOT,        // commands=max 128 Bytes      Output
   SIO         // commands                    Input/Output (no space allowed!)
} SysDevice_Type;

static const char *SysDeviceName[]={
   "AIO",
   "DIN",
   "DOT",
   "DIO",
   "IIN",
   "IOT",
   "IIO",
   "AIN",
   "AOT",
   "SIN",
   "SOT",
   "SIO" 
};


// Event Management

typedef enum SysEventType
{
   SYS_EVENT_REBOOT    = 100,
   SYS_EVENT_WPS_START,      // request device to go in WPS mode
   SYS_EVENT_WPS_END,        // request device to go in WPS mode
   SYS_EVENT_REQDEEPSLEEP,   // DeepSleep anfordern (vom Pack aus)
   SYS_EVENT_DEEPSLEEP,      // Event kurz vor DeepSleep (500ms dann deppslepp)
   SYS_EVENT_DEVLOCCHANGE,   // local change of Device
   SYS_EVENT_DEVREQCHANGE,   // request Device to change (f.e. from any server)
   SYS_EVENT_DEVQUERY,       // query state of a logical Device
   SYS_EVENT_NET_UP,
   SYS_EVENT_NET_DOWN,
} SysEvent_t;

typedef struct SysEv_d{
   long    devicepos;
   boolean state;
} SysEv_d_Struct;

typedef struct SysEv_dev{
   struct {
      long    devicepos;
      int     cnt=0;
      union {
         SysEv_d    D; 
      };
   };

} SysEv_dev_Struct;

typedef struct SysEv_deepsleep{
   struct {
      long    timer;
      long    sleeptime=3600*24;
   };

} SysEv_deepsleep_Struct;



typedef struct SysEvent{
   SysEvent(){};
   SysEventType type;

   union{
      SysEv_dev_Struct dev;
      SysEv_deepsleep_Struct deepsleep;
   };

} SysEvent_Struct;


// User Management
typedef struct _userEntry{
   char username[16];
   char password[16];
   int8_t authLevel;
} userEntry;

class ConfigPack{
   public:
   virtual void store();

   virtual char *getVar(const char *var);
   char *getVar(String &pack,const char *var){
      String fullvar=pack;
      fullvar+=".";
      fullvar+=var;
      return(getVar((const char *)fullvar.c_str()));
   }

   virtual bool setVar(const char *var,const char *val,String &msg){
      msg="ERROR: setVar error";
      return(false);
   }
   virtual char *setVar(const char *var,const char *val);
   char *setVar(String &pack,const char *var,const char *val){
      String fullvar=pack;
      fullvar+=".";
      fullvar+=var;
      return(setVar((const char *)fullvar.c_str(),val));
   }
   virtual bool getEntry(unsigned long req, String &var,String &val);

   virtual bool setUser(long un,const char *user,const char *pass,int8_t al);

   virtual userEntry *getUser(long unum);
};


class AuthPack{
   public:
   virtual bool authUser(String &u,String &p,long *uid, int8_t *authLevel); 
   virtual bool changeUserPassword(String &u,String &pold,String &pnew); 
};



class PackMaster
{
   private:
   DynHashTable<Pack *> Pkg;
   DynHashTable<int> Dev;
   long              tickscount=0;
   unsigned long     deepSleep=0;
   unsigned long     deepSleepSleeptime=0;
   Interval *pSoftInterval=NULL;
   Interval *pHardInterval=NULL;

   public:
   unsigned long     deepSleepDeadLine=0;
   UptimeController    Uptime;

   ConfigPack     *cfg=NULL; 
   AuthPack       *auth=NULL; 
   PackStdCons    *console=NULL;
   Ticker         *t;
   PackMaster();
   void add(const char *name,Pack *p);

   int  registerDevice(SysDeviceType t,const char *name);
   long  getDeviceCount();
   const char *getDeviceName(long pos);
   SysDeviceType  getDeviceType(long pos);
   long  getDevicePos(const char *name);

   long  load();

   void     IntervalLoop(bool isSoft);
   Interval *addInterval(bool isSoft,Interval *iobj);
   Interval *delInterval(bool isSoft,Interval *pI);

   Pack *findPack(const char *name);
   void setup();
   void begin();
   void loop();
   void DeepsleepUpdate();
   void handleWiFiEvent(WiFiEvent_t e);
   void postSystemEvent(SysEvent *e,const char *source);
   int  command(Session *session,Print *cli,String &cmd);

   // Interface for Packages
   PackStdCons *Console();
   char *hostname();
   void hostname(char *hostname);
};


class Pack{
   protected:
   String     PackName;
   public:
   void Name(char *name){
      PackName=name;
   };
   void Name(const char *name){
      PackName=name;
   };
   void Name(String &name){
      PackName=name;
   };
   String &Name(){
      return(PackName);
   }
   virtual bool validateVariable(String &var,char *ov,String &val,String &msg){
      return(false);
   };
   virtual void begin(){};
   virtual void setup(){};
   virtual void loop(){};
   virtual void handleSystemEvent(SysEvent *e,const char *source){};
   virtual void handleWiFiEvent(WiFiEvent_t e){};
   virtual int  command(Session *session,Print *cli,char **args,int argn){
      return(0);
   };

   char *getLocalVar(char *v){
      String var=Name();
      var+=".";
      var +=v;
      return(Controller->cfg->getVar(var.c_str()));
   }
   char *setLocalVar(char *v,char *val){
      String var=Name();
      var+=".";
      var +=v;
      return(Controller->cfg->setVar(var.c_str(),val));
   }
   bool setLocalVar(char *v,char *val,String &msg){
      String var=Name();
      var+=".";
      var +=v;
      return(Controller->cfg->setVar(var.c_str(),val,msg));
   }

  
};

class ledCtrl;

class PackStdCons : public Pack, public Print{
   public:
   virtual bool installStandardSysControl(int pio){
      return(false);
   };
   virtual void setSysLED(int   level,bool rst);
   virtual void setSysLED(int   level){setSysLED(level,true);};
   virtual void setSysLED(float level,bool rst);
   virtual void setSysLED(float level){setSysLED(level,true);};
   virtual void setSysLED(ledCtrl *p);
 //  virtual void printf(const char *fmt, ...){}; // only for Cons calls
};

class PackStdNet : public Pack{

};


String md5sum(String text);
String byte2hex(byte Zahl);


//#define CONS this->Controller->Console()
#define CONS Controller->Console()

#endif
