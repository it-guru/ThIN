#include "./pack.h"
#include "./progCont.h"



#ifdef packlib_WebSrv
bool simpleAuth::changePasswordHandler(
                      Session &session,ESP8266WebServer *s,String &p){
  DynamicJsonBuffer jsonBuffer;
  JsonObject& r = jsonBuffer.createObject();
  r["exitcode"]="0";
  r["exitmsg"]="OK password changed";
  r["time"]=millis();
  String callback=s->arg("callback");

  if (s->arg("newpassword")==s->arg("newpassword2")){
     String oldpass(s->arg("oldpassword"));
     String newpass(s->arg("newpassword"));
     bool chpass=this->changeUserPassword(session.user,oldpass,newpass);
     if (!chpass){
        r["exitcode"]="20";
        r["exitmsg"]="password change failed";
     }
  }
  else{
     r["exitcode"]="10";
     r["exitmsg"]="new password repeat is not identical";
  }


  String dstr;
  r.prettyPrintTo(dstr);
  s->send(200, "application/javascript",callback+"("+dstr+");");
  return(true);
}
#endif


void simpleAuth::setup(){
   CONS->printf("simpleAuth: setup() start\n");
   Controller->auth=this;

   #ifdef packlib_WebSrv
   WebSrv *w=(WebSrv *) Controller->findPack("websrv");
   if (w!=NULL){
      const char *m[] = {"System","Change Password", NULL };
      w->regMod("SystemChangePasswd",[&]
                (Session &session,ESP8266WebServer *s,String &p)->bool{
         s->send_P(200,PSTR("text/javascript"),
                   PSTR_SIMPLEAUTH__JSACT_SYSTEMCHANGEPASSWD_JS_DATA(),
                   SIMPLEAUTH__JSACT_SYSTEMCHANGEPASSWD_JS_LENGTH);
         return(true);
      },m,50);
      w->regNS("/jssys/changePassword",[&]
                  (Session &session,ESP8266WebServer *s,String &p)->bool{
         return(this->changePasswordHandler(session,s,p));
      });
   }
   #endif
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



