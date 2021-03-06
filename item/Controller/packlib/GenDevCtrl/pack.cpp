#include "./pack.h"
#include <stdlib.h>

#ifdef packlib_WebSrv
const char GenDevCtrl_ModActionJavaScript[] PROGMEM =
"define([\"action/MainOverview\"], function(MainOverview) {"
  "return {"
    "start: function() {"
        "$(\"#main\").html("
           "FRM.form('dev',FRM.fset('fset1','Generic Device Controller',"
           "FRM.swi('t','GPIO0',1,'Pin GPIO 0','Device State','12em')+"  
           "FRM.swi('t','GPIO2',0,'Pin GPIO 2','Device State','12em')+"  
           "FRM.swi('t','GPIO3',1,'Pin GPIO 3','Device State','12em')"  
           "))"
        ");"
        "$(\".onoffswitch-checkbox\").change(function(ev){"
           "var devname=$(this).attr(\"id\");"
           "jQuery.ajax({url:\"../js/REST/set/\"+devname+\"/3\","
                        "contentType: \"application/json; charset=utf-8\","
                        "dataType: \"json\","
                        "success:function(data,st,jqXHR) {"
                        "   console.log(\"ajax ok:\",data,st,jqXHR);"
                        "},"
                        "error:function(jqXHR,st) {   "
                        "   console.log(\"ajax NOTOK:\",jqXHR,st);"
                        "}"
           "});"
        "});"
        "$(\".directset\").click(function(ev){"
           "var devst=$(this).attr(\"devst\");"
           "var devname=$(this).attr(\"devname\");"
           "jQuery.ajax({url:\"../js/REST/set/\"+devname+\"/\"+devst,"
                        "contentType: \"application/json; charset=utf-8\","
                        "dataType: \"json\","
                        "success:function(data,st,jqXHR) {   "
                        "   console.log(\"ajax ok:\",data,st,jqXHR);"
                        "},"
                        "error:function(jqXHR,st) {   "
                        "   console.log(\"ajax NOTOK:\",jqXHR,st);"
                        "}"
           "});"
        "});"
        "App.postSystemEvent(\"query:ALL\");"
        "return true;"
        "},"
    "handleSystemEvent: function(evt) {"
       "var myRegexp = /state:C:(.*):(.*)/;"
       "var l=myRegexp.exec(evt.data);"
       "if (l[2]==\"1\"){"
       "   $(\"#\"+l[1]).prop('checked', true);"
       "}if (l[2]==\"0\"){"
       "   $(\"#\"+l[1]).prop('checked', false);"
       "}"
       "return true;"
    "},"
    "end: function() {"
       "return true;"
    "}"
  "}"
"});";


#endif



void GenDevCtrl::setup(){
   
   //CONS->installStandardSysControl(3);

//   Controller->addInterval(new Interval(20,
//      if (digitalRead(3)){
//         Controller->addInterval(new Interval(1000,
//                                [&](long cnt,int intervalFlag)->long{
//            int pio=2;
//            if (gpioLst[pio].curBlinkState){
//               gpioLst[pio].curBlinkState=false;
//               CONS->setSysLED(0);
//            }
//            else{
//               gpioLst[pio].curBlinkState=true;
//               CONS->setSysLED(1023);
//            }
//            return(cnt);
//         }));
//
//      }
//   }


   int pio=0;

   pinMode(pio,INPUT);
   gpioLst[pio].DevicePos=Controller->registerDevice(DIN,"GPIO0");
   gpioLst[pio].i=new Interval(20,[&](long cnt,int intervalFlag)->long{
      this->monGPIO(0);
      return(cnt);
   }); 

   pio=2;
   pinMode(pio,OUTPUT);

   gpioLst[pio].DevicePos=Controller->registerDevice(DOT,"GPIO2");

//   pio=3;
//   pinMode(pio,INPUT_PULLUP);
//   gpioLst[pio].DevicePos=Controller->registerDevice(DIN,"GPIO3");
//   gpioLst[pio].i=new Interval(20,[&](long cnt,int intervalFlag)->long{
//      this->monGPIO(3);
//      return(cnt);
//   }); 

   #ifdef packlib_WebSrv
   WebSrv *w=(WebSrv *) Controller->findPack("websrv");
   if (w!=NULL){
      const char *m[] = {"Main","Overview", NULL };
      w->regMod("MainOverview",[&]
                (Session &session,ESP8266WebServer *s,String &p)->bool{
         s->send_P(200,PSTR("text/javascript"),GenDevCtrl_ModActionJavaScript);
         return(true);
      },m,50);
   }
   #endif

   //deep Sleep Anforderung
//   SysEvent e;
//   e.type=SYS_EVENT_REQDEEPSLEEP;
//   e.deepsleep.timer=30;
//   e.deepsleep.sleeptime=24*3600;
//   Controller->postSystemEvent(&e,PackName.c_str());
}

void GenDevCtrl::broadcastNewGPIOstate(long DevicePos,int newstate){
   SysEvent e;
   e.type=SYS_EVENT_DEVLOCCHANGE;
   e.dev.devicepos=DevicePos;
   e.dev.D.state=newstate;
   Controller->postSystemEvent(&e,PackName.c_str());
}


void GenDevCtrl::monGPIO(int pio){
   int newstate;
   int newtime=millis();

   if (digitalRead(pio)){
      newstate=1;
      gpioLst[pio].curLongState=false;
   }
   else{
      newstate=0;
   }
   if (newstate!=gpioLst[pio].oldShortListenState){
      CONS->printf("got local dev change old=%ld new=%ld\n",
                   gpioLst[pio].oldShortListenTime,newtime);
      if (newtime>gpioLst[pio].oldShortListenTime+50){  // 50ms entprellen!
         if (newstate){
            if (gpioLst[pio].curLogicalState){
               gpioLst[pio].curLogicalState=false;
            }
            else{
               gpioLst[pio].curLogicalState=true;
            }
         }
         broadcastNewGPIOstate(gpioLst[pio].DevicePos,
                               gpioLst[pio].curLogicalState); 
         // store new state
         gpioLst[pio].oldShortListenTime=newtime;
         gpioLst[pio].oldShortListenState=newstate;
      }
   }
   if (pio==3){ // handling for WPS logic
      if (newtime>4000){
         if (newstate==1){
            if (!gpioLst[pio].LongStateListen){
               CONS->printf("GPIO3 mon LongStateListen is now hot\n");
               gpioLst[pio].LongStateListen=true;
            }
         }
         else{
            if (gpioLst[pio].LongStateListen){
               if (newtime>gpioLst[pio].oldShortListenTime+4000){ 
                  if (!gpioLst[pio].curLongState){
                     CONS->printf("fifi trigger WPS\n");
                     SysEvent e;
                     e.type=SYS_EVENT_WPS_START;
                     gpioLst[pio].curLongState=true;
                     Controller->postSystemEvent(&e,PackName.c_str());
                  }
               }
            }
         }
      }
   }
}

void GenDevCtrl::loop(){
   for(int c=0;c<MAX_GPIO;c++){
      if (gpioLst[c].i!=NULL){
         if (gpioLst[c].i->loop()==0){
            delete(gpioLst[c].i);
            gpioLst[c].i=NULL;
         }
      }
   }
}

void GenDevCtrl::handleSystemEvent(SysEvent *e,const char *src){
   switch(e->type) {
      case SYS_EVENT_DEVREQCHANGE:
         CONS->printf("got request to change device %d from '%s'\n",
                      e->dev.devicepos,src);
         for(int gpio=0;gpio<MAX_GPIO;gpio++){
            if (gpioLst[gpio].DevicePos==e->dev.devicepos){
               if (e->dev.D.state==3){
                  if (digitalRead(gpio)){
                     e->dev.D.state=0;
                  }
                  else{
                     e->dev.D.state=1;
                  }
               }
               if (e->dev.D.state==1){
                  e->dev.cnt++;
                  digitalWrite(gpio,HIGH);
                  broadcastNewGPIOstate(gpioLst[gpio].DevicePos,1); 
                  CONS->printf("setting gpio %d to high\n",gpio);
               }
               if (e->dev.D.state==0){
                  e->dev.cnt++;
                  digitalWrite(gpio,LOW);
                  broadcastNewGPIOstate(gpioLst[gpio].DevicePos,0); 
                  CONS->printf("setting gpio %d to low\n",gpio);
               }
            }
         }
         break;
      case SYS_EVENT_DEVQUERY:
         CONS->printf("got query for  %d from '%s'\n",
                      e->dev.devicepos,src);
         for(int gpio=0;gpio<MAX_GPIO;gpio++){
            if (gpioLst[gpio].DevicePos==e->dev.devicepos){
            e->dev.cnt++;
            if (digitalRead(gpio)){
               e->dev.D.state=true;
            }
            else{
               e->dev.D.state=false;
            }
         }
      }
   }
}


void GenDevCtrl::handleWiFiEvent(WiFiEvent_t e){
//   String ip;
//   switch(e) {
//      case WIFI_EVENT_STAMODE_GOT_IP:
//         ip=WiFi.localIP().toString();
//         CONS->printf("GenDevCtrl: got IP '%s'\n",ip.c_str());
//         Print *p;
//         break;
//      case WIFI_EVENT_STAMODE_DISCONNECTED:
//         CONS->printf("GenDevCtrl: disconnect\n");
//         break;
//   }
}


void GenDevCtrl::setGPIOblink(int gpio,long ti,long cnt){
   if (gpioLst[gpio].i!=NULL){
      delete(gpioLst[gpio].i);
      gpioLst[gpio].i=NULL;
   }
   gpioLst[gpio].i=new Interval(ti,[&,gpio](long cnt,int intervalFlag)->long{
      if (this->gpioLst[gpio].curBlinkState){
         digitalWrite(gpio,LOW);
         this->gpioLst[gpio].curBlinkState=false;
      }
      else{
         digitalWrite(gpio,HIGH);
         this->gpioLst[gpio].curBlinkState=true;
      }
      return(cnt);
   },cnt);
}

int GenDevCtrl::command(Session *session,Print *cli,char **args,int argn){
   if (!strcmp(args[0],"set") && argn==3 &&
        (!stricmp(args[1],"GPIO0") ||
         !stricmp(args[1],"GPIO2"))){
      int newmode=-1;
      if (!stricmp(args[2],"on") ||
          !stricmp(args[2],"1") ||
          !stricmp(args[2],"high")){
         newmode=1;
      }
      else if (!stricmp(args[2],"off") ||
               !stricmp(args[2],"0") ||
               !stricmp(args[2],"low")){
         newmode=0;
      }
      else if (!stricmp(args[2],"blink")){
         newmode=2;
      }
      else if (!stricmp(args[2],"hf")){
         newmode=3;
      }
      else{
         return(CMD_SYNTAX);
      }
      int gpio=-1;
      if (!stricmp(args[1],"GPIO0")){
         gpio=0;
      }
      if (!stricmp(args[1],"GPIO2")){
         gpio=2;
      }
      pinMode(gpio,OUTPUT);
      if (newmode==1){
         digitalWrite(gpio,HIGH);
      }
      else if (newmode==0){
         digitalWrite(gpio,LOW);
      }
      else if (newmode==2){
         setGPIOblink(gpio,1000,-1);
      }
      else if (newmode==3){
         setGPIOblink(gpio,1,-1);
      }
      return(CMD_OK);
   }
   else if (!strcmp(args[0],"analogset") && argn==3 &&
        (!stricmp(args[1],"GPIO0") ||
         !stricmp(args[1],"GPIO2"))){

      int newmode=atoi(args[2]);
      int gpio=-1;
      if (!stricmp(args[1],"GPIO0")){
         gpio=0;
      }
      if (!stricmp(args[1],"GPIO2")){
         gpio=2;
      }
      pinMode(gpio,OUTPUT);
      if (newmode>=0 && newmode<=PWMRANGE){
         analogWrite(gpio,newmode);
      }
      else{
         return(CMD_SYNTAX);
      }
      return(CMD_OK);
   }
   else if (!strcmp(args[0],"help") &&
       ( (argn==1) ||
         (argn==2 && !strcmp(args[1],PackName.c_str())))){
      cli->printf("\n%s:\n",PackName.c_str());
      cli->printf("set [GPIO0|GPIO2] [on|off|blink]\n");
      cli->printf("    Sets the given GPIO Port as output and switch\n");
      cli->printf("    to the requested state.\n");
      cli->printf("\n");
      cli->printf("analogset [GPIO0|GPIO2] [0-%d]\n",PWMRANGE);
      cli->printf("    Sets the given GPIO Port as output and switch\n");
      cli->printf("    to the requested analog (PWM) value.\n");
      cli->printf("\n");
      return(CMD_PART);
   }
   return(0);
};


