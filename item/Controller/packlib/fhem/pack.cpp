#include "./pack.h"

void fhem::srvConnect(){
   //CONS->printf("Start: fhem::srvConnect fhemsrv=%x\n",fhemsrv);
   if (fhemsrv==NULL){
      char *fhemsrvhost=Controller->cfg->getVar(Name(),"srv");
      char *fhemsrvport=Controller->cfg->getVar(Name(),"port");
      if (fhemsrvhost!=NULL){
         if (fhemsrvport==NULL){
            fhemsrvport="4000";
         }
         String s=fhemsrvport;
         int fhemsrvportnum=s.toInt();
         CONS->printf("not connected - try to connect\n");

         fhemsrv=new WiFiClient();
         if (fhemsrv->connect(fhemsrvhost,fhemsrvportnum)){ 
            CONS->printf("connect done\n");
            this->srvInitSession();
         }
         else{
            CONS->printf("connect failed\n");
            fhemsrv->stop();
            delete(fhemsrv);
            fhemsrv=NULL;
         }
      }
   }
   else {
      if (fhemsrv->connected()){
         // CONS->printf("fhem CONNECTED\n");
      } 
      else{
         CONS->printf("fhem1 not CONNECTED fhemsrv=%x\n",fhemsrv);
         fhemsrv->stop();
         delete(fhemsrv);
         fhemsrv=NULL;
      }
   }
   CONS->printf("End:  fhem::srvConnect fhemsrv=%x\n",fhemsrv);
}


bool fhem::validateVariable(String &var,char *ov,String &val,String &msg){
   CONS->printf("fhem::validateVariable=%s\n",var.c_str());
   if (var=="port"){
      return(true);
   }
   if (var=="srv"){
      CONS->printf("fhem:: fhem.srv variable = Ok\n");
      return(true);
   }
   msg="ERROR: invalid variable '"+var+"'";
   return(false);
}




void fhem::srvInitSession(){
   long pos;
   const char *devName;
   const char *devType;
   if (fhemsrv!=NULL && fhemsrv->connected()){
      for(pos=0;pos<Controller->getDeviceCount();pos++){
         devName=Controller->getDeviceName(pos);  
         devType=SysDeviceName[Controller->getDeviceType(pos)];  
         String def="define";
         def+=" ";
         def+=devName;
         def+=" ";
         def+=devType;
         CONS->printf("send devinition:%s\n",def.c_str());
         fhemsrv->printf("%s\n",def.c_str());
         this->processSrvMessages();
      }
      for(pos=0;pos<Controller->getDeviceCount();pos++){
         devName=Controller->getDeviceName(pos);  
         devType=SysDeviceName[Controller->getDeviceType(pos)];  
         CONS->printf("query state of '%s' from server\n",devName);
         fhemsrv->printf("query %s\n",devName);
         this->processSrvMessages();
      }
   }
}

String fhem::readCommand(){
    String ret;
    int c = fhemsrv->read();
    while(c >= 0 && c != '\n' && c !='\r') {
        ret += (char) c;
        c = fhemsrv->read();
    }
    if (c=='\n' || c=='\r'){
       ret += (char) c;
    }
    return ret;
}



void fhem::processSrvMessages(){
   while(fhemsrv->available()){
     CONS->printf("data is available\n");
     //String line = fhemsrv->readStringUntil('\n');
     String inbuffer = readCommand();
     String line;
     while(inbuffer.length()>0){
        int s=inbuffer.indexOf('\r');
        if (s==-1){
           s=inbuffer.indexOf('\n');
        }
        if (s>0){
           line=inbuffer.substring(0,s);
           inbuffer=inbuffer.substring(s+1);
           CONS->printf("got '%s'\n",line.c_str());
           if (line.startsWith("STATE:S:")||
               line.startsWith("state:S:")){
              CONS->printf("server request device change\n");
              // handle device change request from server
              String param=line.substring(8);
              //CONS->printf("param1='%s'\n",param.c_str());
              String subdev=param.substring(0,param.indexOf(":"));
              //CONS->printf("subdev2='%s'\n",subdev.c_str());
              param=param.substring(subdev.length()+1);
              //CONS->printf("param2='%s'\n",param.c_str());
              long devpos=Controller->getDevicePos(subdev.c_str());
              if (devpos!=-1){
                 const char *devName=Controller->getDeviceName(devpos);  
                 SysDeviceType devType=Controller->getDeviceType(devpos);
                 CONS->printf("device found at ='%ld'\n",devpos);
                 SysEvent e;
                 e.type=SYS_EVENT_DEVREQCHANGE;
                 if (devType==DOT || devType==DIO){   // io or output 
                    e.dev.devicepos=devpos;
                    e.dev.D.state=0;
                    if (param=="3"){       // trigger toggle operation
                       e.dev.D.state=3;
                    }
                    else if (param!="0"){  // all not 0 = 1
                       e.dev.D.state=1;
                    }
                    CONS->printf("postSystemEvent Digital new ='%d'\n",
                                 e.dev.D.state);
                    Controller->postSystemEvent(&e,PackName.c_str());
                    if (e.dev.cnt>0){
                       if (devType==DIO || devType==DOT || devType==DIN){
                          fhemsrv->printf("state:C:%s:%d\n",devName,
                                                            e.dev.D.state);
                       }
                    }
                 }
              }
           }
           if (line.startsWith("query:")){
              String subdev=line.substring(6);
              CONS->printf("query subdev='%s'\n",subdev.c_str());
              long devpos=Controller->getDevicePos(subdev.c_str());
              if (devpos!=-1){
                 const char *devName=Controller->getDeviceName(devpos);  
                 SysDeviceType devType=Controller->getDeviceType(devpos);  
                 SysEvent e;
                 e.type=SYS_EVENT_DEVQUERY;
                 e.dev.devicepos=devpos;
                 Controller->postSystemEvent(&e,PackName.c_str());
                 if (e.dev.cnt>0){
                    if (devType==DIO || devType==DOT || devType==DIN){
                       fhemsrv->printf("state:C:%s:%d\n",devName,
                                                         e.dev.D.state);
                    }
                 }
              }
           }
        }
     }
   }
}

void fhem::srvSend(){
   long devpos;
   const char *devName;
   SysDeviceType devType;
   if (fhemsrv!=NULL && fhemsrv->connected()){
      for(devpos=0;devpos<Controller->getDeviceCount();devpos++){
         devName=Controller->getDeviceName(devpos);  
         devType=Controller->getDeviceType(devpos);  

         SysEvent e;
         e.type=SYS_EVENT_DEVQUERY;
         e.dev.devicepos=devpos;
         Controller->postSystemEvent(&e,PackName.c_str());
         if (e.dev.cnt>0){
            if (devType==DIO || devType==DOT || devType==DIN){
               fhemsrv->printf("STATE:C:%s:%d\n",devName,e.dev.D.state);
            }
         }
         this->processSrvMessages();
      }
   }
}



void fhem::setup(){
   i=new Interval(10000,[&](long cnt,int intervalFlag)->long{
      this->srvSend();
      return(cnt);
   });
   con=new Interval(15000,[&](long cnt,int intervalFlag)->long{
      this->srvConnect();
      return(cnt);
   });
}

void fhem::loop(){
   if (WiFiOnline){
      if (fastConnect>0){
         srvConnect();
         fastConnect--;
      }
      (con!=NULL) ? con->loop():0;
      (i!=NULL)   ? i->loop():0;
      if (fhemsrv!=NULL && fhemsrv->connected()){
         this->processSrvMessages();
      }
   }
}

void fhem::handleWiFiEvent(WiFiEvent_t e){
   switch(e) {
      case WIFI_EVENT_STAMODE_GOT_IP:
         fastConnect++;
         WiFiOnline=true;
         break;
      case WIFI_EVENT_STAMODE_DISCONNECTED:
         if (fhemsrv!=NULL){
            fhemsrv->stop();
            delete(fhemsrv);
            fhemsrv=NULL;
         }
         WiFiOnline=false;
         break;
   }
}

void fhem::handleSystemEvent(SysEvent *e,const char *src){
   switch(e->type) {
      case SYS_EVENT_DEVLOCCHANGE:
         CONS->printf("got local device change notification %d from '%s'\n",
                      e->dev.devicepos,src);
         if (fhemsrv!=NULL && fhemsrv->connected()){
            long devpos=e->dev.devicepos;
            const char *devName=Controller->getDeviceName(devpos);  
            SysDeviceType devType=Controller->getDeviceType(devpos);  
            if (devType==DIO || devType==DIN){
               fhemsrv->printf("state:C:%s:%d\n",devName,
                                                 e->dev.D.state);
            }
         }
         break;
   }
}





int fhem::command(Session *session,Print *cli,char **args,int argn){
   if (!strcmp(args[0],"fhem") && argn==3 &&
        (!stricmp(args[1],"GPIO0") ||
         !stricmp(args[1],"GPIO2"))){
      return(CMD_OK);
   }
   else if (!strcmp(args[0],"help") &&
       ( (argn==1) ||
         (argn==2 && !strcmp(args[1],PackName.c_str())))){
      cli->printf("\n%s:\n",PackName.c_str());
      cli->printf("fhem [GPIO0|GPIO2] [on|off|blink]\n",PackName.c_str());
      cli->printf("    Dummy fhem command (at now)\n");
      cli->printf("    nix nix nix.\n");
      cli->printf("\n","");
      return(CMD_PART);
   }
   return(0);
};


