#include "./pack.h"
#include "./progCont.h"
#include <stdlib.h>




void LEDdimmer1::setup(){
   
   CONS->installStandardSysControl(3);

   int pio=2;

   pinMode(pio,OUTPUT);
   digitalWrite(pio,LOW);

   char *soldLevel=this->getLocalVar("level");
   if (soldLevel==NULL){
      soldLevel="0.1";
   }
   float oldLevel=atof(soldLevel);

   this->setLEDLevel(pio,oldLevel);

   gpioLst[pio].DevicePos=Controller->registerDevice(AOT,"LED");

   #ifdef packlib_WebSrv
   WebSrv *w=(WebSrv *) Controller->findPack("websrv");
   if (w!=NULL){
      const char *m[] = {"Main","Overview", NULL };
      w->regMod("MainOverview",[&]
                (Session &session,ESP8266WebServer *s,String &p)->bool{
         s->send_P(200,PSTR("text/javascript"),
                   PSTR_LEDDIMMER1__JSACT_MAINOVERVIEW_JS_DATA(),
                   LEDDIMMER1__JSACT_MAINOVERVIEW_JS_LENGTH);
         return(true);
      },m,50);
   }
   #endif

}

void LEDdimmer1::broadcastNewGPIOstate(long DevicePos,int newstate){
   SysEvent e;
   e.type=SYS_EVENT_DEVLOCCHANGE;
   e.dev.devicepos=DevicePos;
   e.dev.D.state=newstate;
   Controller->postSystemEvent(&e,PackName.c_str());
}

bool LEDdimmer1::validateVariable(String &var,char *ov,String &val,String &msg){
   if (var=="level"){
      return(true);
   }
   msg="ERROR: invalid variable '"+var+"'";
   return(false);
}




void LEDdimmer1::loop(){
}

void LEDdimmer1::setLEDLevel(int pio,float f){
   int ival=int(f*PWMRANGE);
   CONS->printf("set new analog %d\n",ival);
   analogWrite(pio,ival);
   //broadcastNewGPIOstate(gpioLst[gpio].DevicePos,0); 
}

void LEDdimmer1::handleSystemEvent(SysEvent *e,const char *src){
   long LEDpos=Controller->getDevicePos("LED");
   int pio=2;
   switch(e->type) {
      case SYS_EVENT_DEVREQCHANGE:
         CONS->printf("got request to analog change device %d from '%s'\n",
                      e->dev.devicepos,src);
         if (LEDpos==e->dev.devicepos){

            String msg;
            String val(e->dev.A.state);
            if (!this->setLocalVar("level",(char *) val.c_str(),msg)){
               CONS->printf("error %s\n",msg.c_str());
            }
            else{
               CONS->printf("OK stored in level %s\n",val.c_str());
            }
            e->dev.cnt++;
            this->setLEDLevel(pio,e->dev.A.state);
         }
         break;
      case SYS_EVENT_DEVQUERY:
         CONS->printf("got query for  %d from '%s'\n",
                      e->dev.devicepos,src);
         if (e->dev.devicepos==LEDpos){
            e->dev.cnt++;
            int ival=analogRead(pio);
            e->dev.D.state=((float)ival)/PWMRANGE;
         }
         break;
   }
}


int LEDdimmer1::command(Session *session,Print *cli,char **args,int argn){
   if (!strcmp(args[0],"help") &&
       ( (argn==1) ||
         (argn==2 && !strcmp(args[1],PackName.c_str())))){
      cli->printf("\n%s:\n",PackName.c_str());
      cli->printf("    nix\n");
      cli->printf("\n");
      return(CMD_PART);
   }
   return(0);
};


