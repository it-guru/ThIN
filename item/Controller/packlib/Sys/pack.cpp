#include "./pack.h"

void Sys::handleSystemEvent(SysEvent *e,const char *source){
   char *src="unkonwn";
   if (src!=NULL){
      src=(char *)source;
   }
   switch(e->type) {
      case SYS_EVENT_REBOOT:
         CONS->printf("got SysemEvent reboot event from '%s'\n",src);
         ESP.reset();
         break;
   }
}

#ifdef packlib_WebSrv
const char Sys_ModActionJavaScript[] PROGMEM =
"define([\"action/SysStatus\"], function(SysStatus) {"
"  return {"
"    start: function() {"
"      $(\"#main\").html(\"SysStatus loaded\");"
"      return true;"
"    },"
"    end: function() {"
"      return true;"
"    }"
"  }"
"});";
#endif



void Sys::setup(){
   #ifdef packlib_WebSrv
   WebSrv *w=(WebSrv *) Controller->findPack("websrv");
   if (w!=NULL){
      const char *m[] = {"System","Status", NULL };
      w->regMod("SysStatus",[&]
                (Session &session,ESP8266WebServer *s,String &p)->bool{
         s->send_P(200,PSTR("text/javascript"),Sys_ModActionJavaScript);
         return(true);
      },m);
   }
   #endif
   char *hostname=getLocalVar("hostname");
   if (hostname!=NULL){
      Controller->hostname(hostname);
   }
}

bool Sys::validateVariable(String &var,char *ov,String &val,String &msg){
   CONS->printf("Sys::validateVariable=%s\n",var.c_str());
   if (var=="hostname"){
      return(true);
   }
   msg="ERROR: invalid variable '"+var+"'";
   return(false);
}


int Sys::command(Session *session,Print *cli,char **args,int argn){
   SysEvent e;

   if (!strcmp(args[0],"sysvar") && argn==3){
      unsigned long c=0;
      String var(args[1]);
      String val(args[2]);
      String msg;
      if (!Controller->cfg->setVar(var.c_str(),val.c_str(),msg)){
         cli->printf("%s\n",msg.c_str());
         return(CMD_SYNTAX);
      }
      return(CMD_OK);
   }
   if (!strcmp(args[0],"show") && argn==2 &&
            !strcmp(args[1],"sysvars")){
      unsigned long c=0;
      String var;
      String val;
      while(Controller->cfg->getEntry(c,var,val)){
         cli->printf("%s='%s'\n",var.c_str(),val.c_str());
         c++;
      }
      return(CMD_OK);
   }
   if (!strcmp(args[0],"show") && argn==3 &&
            !strcmp(args[1],PackName.c_str())){
      if (!strcmp(args[2],"status")){
         cli->printf("FreeHeap:    %s\n",
                     String(ESP.getFreeHeap()).c_str());
         cli->printf("ChipId:      %s\n",
                     String(ESP.getChipId()).c_str());
         cli->printf("FlashChipId: %s\n",
                     String(ESP.getFlashChipId()).c_str());
         cli->printf("SketchSize:  %s\n",
                     String(ESP.getSketchSize(),DEC).c_str());
         return(CMD_OK);
      }
      else{
         return(CMD_SYNTAX);
      }
   }
   else if (!strcmp(args[0],"uptime") && argn==1){
      cli->printf("%s seconds\n",
                  String((unsigned long) millis()/1000,DEC).c_str());
      return(CMD_OK);
   }
   else if (!strcmp(args[0],"whoami") && argn==1){
      cli->printf("User: %s\n",session->user.c_str());
      cli->printf("UID: %d\n" ,session->uid);
      return(CMD_OK);
   }
   else if (!strcmp(args[0],"reboot") && argn==1){
      e.type=SYS_EVENT_REBOOT;   
      Controller->postSystemEvent(&e,PackName.c_str());
      return(CMD_OK);
   }
   else if (!strcmp(args[0],"hostname") && argn==1){
      cli->printf("%s\n",Controller->hostname());
      return(CMD_OK);
   }
   else if (!strcmp(args[0],"hostname") && argn==2){
      setLocalVar("hostname",args[1]);
      Controller->hostname(args[1]);
      return(CMD_OK);
   }
   else if (!strcmp(args[0],"help") &&
       ( (argn==1) ||
         (argn==2 && !strcmp(args[1],PackName.c_str())))){
      cli->printf("\n%s:\n",PackName.c_str());
      cli->printf("show %s status\n",PackName.c_str());
      cli->printf("    Displays informations about general system\n");
      cli->printf("    parameters.\n");
      cli->printf("\n","");
      cli->printf("show sysvars\n");
      cli->printf("    Displays all variables in non-volatile memory.\n");
      cli->printf("\n","");
      cli->printf("uptime\n");
      cli->printf("    Displays the count of seconds since last\n");
      cli->printf("    reboot of ThiN device.\n");
      cli->printf("\n","");
      cli->printf("reboot\n");
      cli->printf("    Reboots the ThiN device.\n");
      cli->printf("\n","");
      cli->printf("hostname\n");
      cli->printf("    Displays the name of the system. This name is\n");
      cli->printf("    also the name in DNS.\n");
      cli->printf("\n","");
      return(CMD_PART);
   }
   return(0);
};


