#include "./pack.h"

void KeepAlive::keepAliveMessage(long cnt,int intervalFlag){
   // ESP Class
   // https://github.com/esp8266/Arduino/blob/master/cores/esp8266/Esp.h
   long load=Controller->load();
   CONS->printf("KeepAlive: FreeHeap:%" PRIu32 " Bytes load=%ld "
                "DeepSleepTime=%ld uptime=%ld (%ld/%d)\n",
                ESP.getFreeHeap()/* uint32_t */ ,load,
                Controller->deepSleepDeadLine,
                Controller->Uptime.getSeconds(),cnt,intervalFlag);
}

bool KeepAlive::validateVariable(String &var,char *ov,String &val,String &msg){
   CONS->printf("KeepAlive::validateVariable=%s\n",var.c_str());
   if (var=="serial"){
      return(true);
   }
   msg="ERROR: invalid variable '"+var+"'";
   return(false);
}


#ifdef packlib_WebSrv
void KeepAlive::keepAliveMessageWeb(ESP8266WebServer *srv,String &path){
   String p="";
   int c;

   p+="<html>";
   p+="<head>";
   p+="<meta http-equiv=\"refresh\" content=\"10\">";
   p+="</head>";
   p+="<body>";
   p+="<h1>KeepAlive</h1>";

   p+="</body>";
   p+="</html>";
   srv->send(200,"text/html",p);
}
const char KeepAlive_ModActionJavaScript[] PROGMEM = 
"define([\"action/SystemKeepAlive\"], function(SystemKeepAlive) {"
"  return {"
"    start: function() {"
"      $(\"#main\").html(\"SystemKeepAlive loaded\");"
"      return true;"
"    },"
"    end: function() {"
"      return true;"
"    }"
"  }"
"});";
#endif


void KeepAlive::setup(){
   i=new Interval(10000,[&](long cnt,int intervalFlag)->long{
      this->keepAliveMessage(cnt,intervalFlag);
      return(cnt);
   },50);
   CONS->printf("KeepAlive: Monitor setup() done\n");


   char *serial=Controller->cfg->getVar(Name(),"serial");
   if (serial==NULL){
      CONS->printf("KeepAlive: serial not set - setting it\n");
      serial=Controller->cfg->setVar(Name(),"serial","11833");
   }

 //  serial=Controller->cfg->setVar(Name(),"serial9","11x33");
 //  serial=Controller->cfg->setVar(Name(),"serial1","11x33");
 //  serial=Controller->cfg->getVar(Name(),"serial11");
 //  if (serial==NULL){
 //     CONS->printf("KeepAlive: serial11 not set - setting it\n");
 //     serial=Controller->cfg->setVar(Name(),"serial11","11833");
 // }
 //  CONS->printf("KeepAlive: serial = %s\n",Controller->cfg->getVar(Name(),"serial"));
 //  CONS->printf("KeepAlive: serial1 = %s\n",Controller->cfg->getVar(Name(),"serial1"));
  // CONS->printf("KeepAlive: serial11 = %s\n",Controller->cfg->getVar(Name(),"serial11"));
  

   #ifdef packlib_WebSrv
   WebSrv *w=(WebSrv *) Controller->findPack("websrv");
   if (w!=NULL){
      w->regNS("/KeepAlive.html",[&]
               (Session &session,ESP8266WebServer *s,String &p)->bool{
         this->keepAliveMessageWeb(s,p);
         return(true);
      });
      const char *m[] = {"System","KeepAlive", NULL };
      w->regMod("SystemKeepAlive",[&]
                (Session &session,ESP8266WebServer *s,String &p)->bool{
         s->send_P(200,PSTR("text/javascript"),KeepAlive_ModActionJavaScript); 
         return(true);
      },m);
   }
   #endif
}

void KeepAlive::loop(){
   if (i!=NULL){
      i->loop();
   }
}

