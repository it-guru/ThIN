#ifndef _lib_simpleAuth_h_
#define _lib_simpleAuth_h_

#include "../../kernel/PackMaster.h"
#include "../../config.h"

#ifdef packlib_WebSrv
#include "../WebSrv/pack.h"
#endif


class simpleAuth : public Pack, public AuthPack {
   protected:

   private:
   #ifdef packlib_WebSrv
   void progCont();
   #endif


   public:
   void setup();
   bool authUser(String &u,String &p,long *uid, int8_t *authLevel);
   bool changeUserPassword(String &u,String &pold,String &pnew);
   int  command(Session *session,Print *cli,char **args,int argn);
   #ifdef packlib_WebSrv
   bool changePasswordHandler(Session &session,ESP8266WebServer *s,String &p);
   #endif
};

#endif

