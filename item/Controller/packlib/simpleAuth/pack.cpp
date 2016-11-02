#include "./pack.h"


void simpleAuth::setup(){
   CONS->printf("simpleAuth: setup() start\n");
   Controller->auth=this;
}

bool simpleAuth::authUser(String &user,String &pass,long *puid, int8_t *paLev){
   int8_t uid=0;
   userEntry *u=Controller->cfg->getUser(uid);
   bool foundUser=false;
   CONS->printf("simpleAuth::authUser user='%s' pass='%s'\n",
                user.c_str(),pass.c_str());
   while(u!=NULL){
       if (user==u->username){
          if (pass==u->password){
             CONS->printf("simpleAuth::authUser OK\n");
             *puid=uid;
             *paLev=u->authLevel;
             CONS->printf("simpleAuth::authUser return(true);\n");
             return(true);
          }
          else{
             CONS->printf("simpleAuth::authUser return(false);\n");
             return(false);
          }  
          break;
       }
       u=Controller->cfg->getUser(++uid);
   }
   return(false);
}

bool simpleAuth::changeUserPassword(String &u,String &pold,String &pnew){
   CONS->printf("simpleAuth::changeUserPassword user='%s' po='%s' pn='%s'\n",
                u.c_str(),pold.c_str(),pnew.c_str());
   long   uid=-1;
   int8_t authLevel=-1;

   bool foundUser=Controller->auth->authUser(u,pold,&uid,&authLevel);
   if (foundUser){
      if (Controller->cfg->setUser(uid,u.c_str(),pnew.c_str(),authLevel)){
         return(true);
      }
   }

   return(false);
}


int simpleAuth::command(Session *session,Print *cli,char **args,int argn){
   if (!strcmp(args[0],"passwd") && argn==4){
      String username(args[1]);
      String oldpass(args[2]);
      String newpass(args[3]);
      bool chpass=this->changeUserPassword(username,oldpass,newpass);
      if (chpass){
         return(CMD_OK);
      }
      return(CMD_SYNTAX);
   }
   else if (!strcmp(args[0],"help") &&
       ( (argn==1) ||
         (argn==2 && !strcmp(args[1],"auth")))){
      cli->printf("\n%s:\n",PackName.c_str());
      cli->printf("passwd <USER> <OLDPASS> <NEWPASS>\n",PackName.c_str());
      cli->printf("    Sets the password of an user.\n");
      cli->printf("\n","");
      return(CMD_PART);
   }
   return(0);
}



