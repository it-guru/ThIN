#include "./pack.h"


WiFiUDP ntpUDP;

void NTPCli::setup(){
   CONS->printf("NTPCli: NTPCliworkController setup() start\n");
   CONS->printf("NTPCli: NTPCliworkController setup() end\n");
   ntp=new NTPClient(ntpUDP);
}

void NTPCli::loop(){
   ntp->update();
}

void NTPCli::handleWiFiEvent(WiFiEvent_t e){
//   String ip;
//   switch(e) {
//      case WIFI_EVENT_STAMODE_GOT_IP:
//         ip=WiFi.localIP().toString();
//         CONS->printf("NTPCli: got IP '%s'\n",ip.c_str());
//         Print *p;
//         break;
//      case WIFI_EVENT_STAMODE_DISCONNECTED:
//         CONS->printf("NTPCli: disconnect\n");
//         break;
//   }
}


int NTPCli::command(Session *session,Print *cli,char **args,int argn){
   if (!strcmp(args[0],"show") && argn==3 &&
            !strcmp(args[1],PackName.c_str())){
      if (!strcmp(args[2],"time")){
         cli->printf("%s\n",ntp->getFormattedTime().c_str());
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
      cli->printf("show %s time\n",PackName.c_str());
      cli->printf("    Displays the time get from ntp server.\n");
      cli->printf("\n","");
      return(CMD_PART);
   }
   return(0);
};


