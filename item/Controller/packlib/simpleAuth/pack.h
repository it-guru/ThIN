#ifndef _lib_simpleAuth_h_
#define _lib_simpleAuth_h_

#include "../../kernel/PackMaster.h"
#include "../../config.h"





class simpleAuth : public Pack, public AuthPack {
   protected:

   private:

   public:
   void setup();
   bool authUser(String &u,String &p,long *uid, int8_t *authLevel);
   bool changeUserPassword(String &u,String &pold,String &pnew);
   int  command(Session *session,Print *cli,char **args,int argn);
};

#endif

