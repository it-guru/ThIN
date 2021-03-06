#include "./pack.h"

void Net::setup(){
  // Connect();
}

void Net::loop(){
   if (WpsStep>0){
      unsigned long t=millis();
      if (WpsStep==1){
         CONS->printf("WpsStep 1 - starting\n");
         WpsStart=t;
         WiFiClient::stopAll();
         WiFiUDP::stopAll();
         WiFi.disconnect();
         CONS->printf("WpsStep 1 - WiFi is disconnected\n");
         delay(100);
         WpsStep++;
         WpsTimer=t;
      }
      if (WpsStep==2 && WiFi.status()!=WL_CONNECTED){
         CONS->printf("WpsStep 2 - WiFi is now disconnected\n");
         WiFi.mode(WIFI_OFF);
         delay(1000);
         WpsStep++;
         WpsTimer=t;
      }
      if (WpsStep==3 && (WpsTimer+4000)<t){
         CONS->printf("WpsStep 3 - now starting WPSConfig()\n");
         WiFi.mode(WIFI_STA);
         delay(1000);
         CONS->printf("WpsStep 3 - begin set\n");
         yield();
         WiFi.beginWPSConfig();
         delay(500);
         CONS->printf("WpsStep 3 - waiting\n");
         WpsStep++;
         WpsTimer=t;
      }
      if (WpsStep==4 && (WpsTimer+2000)<t){
         CONS->printf("WpsStep 4 - checking WL_CONNECT\n");
         if (WiFi.status() == WL_CONNECTED){
            CONS->printf("WpsStep 5 - ---- OK WPS Success -----\n");
            WpsStep++;
         }
         WpsTimer=t;
         CONS->printf("WpsStep 4 - WpsStep=%d WpsTimer=%ld\n",WpsStep,WpsTimer);
      }
      if ((WpsStart+40000)<t){
         CONS->printf("WpsStep - FAIL - giving up\n");
         WpsStep=0;
         SysEvent e;
         e.type=SYS_EVENT_WPS_END;
         Controller->postSystemEvent(&e,PackName.c_str());
      }
      if (WpsStep==5){
         CONS->printf("WpsStep 5 SUCCESS - WPS end\n");
         WpsStep=0;
         SysEvent e;
         e.type=SYS_EVENT_WPS_END;
         Controller->postSystemEvent(&e,PackName.c_str());
      }
   }
}


void Net::handleWiFiEvent(WiFiEvent_t e){
   String ip;
   switch(e) {
      case WIFI_EVENT_STAMODE_GOT_IP:
         ip=WiFi.localIP().toString();
         CONS->printf("Net: got IP '%s'\n",ip.c_str());
         Print *p;
         break;
      case WIFI_EVENT_STAMODE_DISCONNECTED:
         CONS->printf("Net: disconnect\n");
         break;
   }
}

void Net::handleSystemEvent(SysEvent *e,const char *src){
   switch(e->type) {
      case SYS_EVENT_WPS_START:
         if (WpsStep==0){
            WpsStep=1;
            CONS->printf("got request for WPS mode Net::handleSystemEvent\n");
         }
         break;
   }
}


void Net::Connect(){
  if (0){
   //  WiFi.softAP(ssid, password);
     CONS->printf("Configuring access point...\n");

   //  IPAddress apIP = WiFi.softAPIP();
   //  dnsServer.start(DNS_PORT, "*", apIP);
  }
  else{
     CONS->printf("Begin Network config ...\n");
     yield();
     WiFi.begin("","");
     yield();
     CONS->printf("AP Connect ...\n");
  }
}

bool Net::validateVariable(String &var,String &val,String &msg){
   CONS->printf("Net::validateVariable=%s\n",var.c_str());
   if (var=="ssid"){
      return(true);
   }
   if (var=="wpakey"){
      return(true);
   }
   msg="ERROR: invalid variable '"+var+"'";
   return(false);
}



int Net::command(Session *session,Print *cli,char **args,int argn){
   if (!strcmp(args[0],"show") && argn==3 &&
            !strcmp(args[1],PackName.c_str())){
      if (!strcmp(args[2],"status")){
         cli->printf("IP: %s\n",IPAddress(WiFi.localIP()).toString().c_str());
         cli->printf("Hostname: %s\n",Controller->hostname());
         WiFi.printDiag(*cli);
         return(CMD_OK);
      }
      else{
         return(CMD_SYNTAX);
      }
   }
   else if (!strcmp(args[0],"help") &&
       ( (argn==1) ||
         (argn==2 && !strcmp(args[1],PackName.c_str())))){
      cli->printf("\n%s:\n",PackName.c_str());
      cli->printf("show %s status\n",PackName.c_str());
      cli->printf("    Displays the current network status.\n");
      cli->printf("\n","");
      return(CMD_PART);
   }
   return(0);
};


