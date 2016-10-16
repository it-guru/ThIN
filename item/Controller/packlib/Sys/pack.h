#ifndef _lib_SYS_h_
#define _lib_SYS_h_


#include "../../config.h"
#include "../../kernel/PackMaster.h"

#ifdef packlib_WebSrv
#include "../WebSrv/pack.h"
#endif


class Sys : public Pack {
   
   public:
   virtual int  command(Session *session,Print *cli,char **args,int argn);
   virtual void handleSystemEvent(SysEvent *e,const char *source);
   virtual void setup();
   bool validateVariable(String &var,char *ov,String &val,String &msg);
};

#endif

