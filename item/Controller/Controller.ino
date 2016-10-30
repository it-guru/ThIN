#include "./kernel/PackMaster.h"

#include "./config.h"

#ifdef packlib_cfgEEPROM
#include "./packlib/cfgEEPROM/pack.h"
#endif

#ifdef packlib_Cons
#include "./packlib/Cons/pack.h"
#endif

#ifdef packlib_KeepAlive
#include "./packlib/KeepAlive/pack.h"
#endif

#ifdef packlib_WebSrv
#include "./packlib/WebSrv/pack.h"
#endif

#ifdef packlib_TelSrv
#include "./packlib/TelSrv/pack.h"
#endif

#ifdef packlib_Net
#include "./packlib/Net/pack.h"
#endif

#ifdef packlib_Sys
#include "./packlib/Sys/pack.h"
#endif

#ifdef packlib_NTPCli
#include "./packlib/NTPCli/pack.h"
#endif

#ifdef packlib_GenDevCtrl
#include "./packlib/GenDevCtrl/pack.h"
#endif

#ifdef packlib_fhem
#include "./packlib/fhem/pack.h"
#endif

PackMaster PM;

void setup(){
   WiFi.onEvent([](WiFiEvent_t e){            // needed to dispatch Network
      PM.handleWiFiEvent(e);                  // Events
   }); 

   // Level 0 (very native Modules)

   #ifdef packlib_cfgEEPROM 
      PM.add("cfg",       new cfgEEPROM());
   #endif

   #ifdef packlib_Cons 
      PM.add("console",   new Cons(packlib_Cons_InitialMode));
   #endif


   // Level 1 (Modules which needed a very son init phase, f.e. device modules)

   #ifdef packlib_GenDevCtrl
      PM.add("gendevctrl",new GenDevCtrl());
   #endif


   // Level 3 (all outer modules);

   #ifdef packlib_Net
      PM.add("net",       new Net());
   #endif

   #ifdef packlib_Pack   
      PM.add("default",   new Pack);
   #endif

   #ifdef packlib_KeepAlive
      PM.add("keepalive", new KeepAlive());
   #endif

   #ifdef packlib_WebSrv
      PM.add("websrv",    new WebSrv());
   #endif

   #ifdef packlib_TelSrv
      PM.add("telnet",    new TelSrv());
   #endif
 
   #ifdef packlib_Sys
      PM.add("sys",       new Sys());
   #endif

   #ifdef packlib_NTPCli
      PM.add("ntp",       new NTPCli());
   #endif

   #ifdef packlib_fhem
      PM.add("fhem",      new fhem());
   #endif 
   PM.setup();
   PM.begin();
}

void loop(){
  PM.loop();
}

