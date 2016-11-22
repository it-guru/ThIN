#include "./PackMaster.h"

extern "C" {
#include "user_interface.h"   // for wifi_get_hostname

   static inline unsigned get_ccount(void)
   {
           unsigned r;
           asm volatile ("rsr %0, ccount" : "=r"(r));
           return r;
   }

}

void hardInterval(){
   Controller->IntervalLoop(false); 
}


PackMaster::PackMaster(){
   t=new Ticker();
   t->attach_ms(20,hardInterval);
}


void PackMaster::add(const char *name,Pack *p){
   Pkg.set(name,p);
   p->Name(name);
   if (!strcmp(name,"console")){       // console can not be set in setup of 
      this->console=(PackStdCons*) p;  // because it is used already in setup
   }                                   // and add phase of other Pack's
}

int PackMaster::registerDevice(SysDeviceType t,const char *name){
   Console()->printf("register device '%s' as type of '%s'\n",
                     name,SysDeviceName[t]);
   return(Dev.set(name,(int) t));
}

long PackMaster::getDeviceCount(){
   return(Dev.length());
}


const char *PackMaster::getDeviceName(long pos){
   return(Dev.key(pos));
}

SysDeviceType PackMaster::getDeviceType(long pos){
   return((SysDeviceType) Dev[pos]);
}

long PackMaster::getDevicePos(const char *name){
   return(Dev.getIndex((char *)name));
}




Pack *PackMaster::findPack(const char *name){
   Pack *p;
   p=Pkg[name];
   return(p);
}


char *PackMaster::hostname(){
   return(wifi_station_get_hostname());
}

void PackMaster::hostname(char *hostname){
   wifi_station_set_hostname(hostname);
   return;
}

void PackMaster::setup(){
   Uptime.loop();
   for(long c=0;c<Pkg.length();c++){
      Pkg[c]->setup();
      yield();
      Uptime.loop();
   }
}

void PackMaster::DeepsleepUpdate(){
   if (deepSleep>0){
      deepSleepDeadLine=Uptime.getSeconds()+deepSleep;
   }
}


void PackMaster::postSystemEvent(SysEvent *e,const char *source){
   char *src="unkonwn";
   if (src!=NULL){
      src=(char *)source;
   }
   if (e->type==SYS_EVENT_REQDEEPSLEEP){
      deepSleep=e->deepsleep.timer;
      deepSleepSleeptime=e->deepsleep.sleeptime;
      CONS->printf("setting deepSleeptimer: timer:%ld sleeptime=%ld\n",
                   deepSleep,deepSleepSleeptime);
      DeepsleepUpdate();
   }
   else{
      for(long c=0;c<Pkg.length();c++){
         Pkg[c]->handleSystemEvent(e,src);
      }
   }
}

void PackMaster::begin(){
   for(long c=0;c<Pkg.length();c++){
      Pkg[c]->begin();
      yield();
   }
}

long PackMaster::load(){
   long l;
   l=tickscount*12.5;  // one tick = 12.5 ns
   return(l);
}

void PackMaster::loop(){
   unsigned t1=get_ccount();
   for(long c=0;c<Pkg.length();c++){
      Pkg[c]->loop();
      yield();
   }
   Uptime.loop();
   unsigned t2=get_ccount();
   tickscount=(tickscount+(t2-t1))/2;

   if (deepSleepDeadLine>0 && Uptime.getSeconds()>deepSleepDeadLine){
      CONS->printf("Controller: deepsleep time reached\n");
      delay(500);
      ESP.deepSleep(deepSleepSleeptime*1000000);
   }
   this->IntervalLoop(true);
}

//  *********************************************************************
void PackMaster::IntervalLoop(bool isSoft){
   Interval *pI=pHardInterval;
   if (isSoft){
      pI=pSoftInterval;
   }
   while(pI!=NULL){
      Interval *pOp=pI;
      long cnt=pOp->loop();
      pOp=pOp->pNext;
      if (!cnt) delInterval(isSoft,pI);
      pI=pOp;
   }
}

Interval *PackMaster::addInterval(bool isSoft,Interval *iObj){
   Interval **pStart=&pHardInterval;
   if (isSoft){
      pStart=&pSoftInterval;
   }
   iObj->pNext=NULL;
   if (*pStart==NULL){
      *pStart=iObj;
   }
   else{
      Interval *pI=*pStart;
      while(pI->pNext!=NULL) pI=pI->pNext;
      pI->pNext=iObj;
   }
   return(iObj);
}

Interval *PackMaster::delInterval(bool isSoft,Interval *pIdel){
   Interval **pStart=&pHardInterval;
   if (isSoft){
      pStart=&pSoftInterval;
   }
   if (*pStart==pIdel){
      *pStart=((*pStart)->pNext==NULL) ? NULL : (*pStart)->pNext;
   }
   else{
      Interval *pI=*pStart;
      do{
        Interval *pOld=pI;
        pI=pI->pNext;
        if (pI==pIdel){
           pOld->pNext=pI->pNext==NULL ? NULL : pI->pNext;
        }
      }while(pI!=NULL);
   }
   delete(pIdel);
   return(NULL);
}
//  *********************************************************************


int PackMaster::command(Session *session,Print *cli,String &cmd){
   CONS->printf("PackMaster: Command '%s'\n",cmd.c_str());
   DeepsleepUpdate();

   /////////////////////////////////////////////////////
   int const MAXARGN=128;
   char *args;
   char *argp[MAXARGN];   // max argn
   int  argn=0;
   bool in_param=false;
   bool in_punch=false;
   int  in_quote=0;
   int  c,orglen;

   orglen=cmd.length();
   args=(char *) malloc((orglen*sizeof(char))+1);
   strcpy(args,cmd.c_str());

   for(c=0;c<orglen;c++){
      //CONS->printf("PackMaster: fifi loop c='%d'\n",c);
      if (!in_punch && args[c]=='\\'){
         in_punch=true; 
         continue;
      }
      if (in_punch){
         in_punch=false; 
         continue;
      }
      if (args[c]=='"'){
         if ((!in_quote && c==0) || 
             (in_quote && args[c+1]!=0 && args[c+1]!=' ')){
            free(args);
            return(CMD_SYNTAX);
         }
         if (in_quote){
            args[c]=0;
         }
         in_quote=!in_quote;
      }


      if (in_param){
         //CONS->printf("PackMaster: inparam c='%d' c='%c'\n",c,args[c]);
         if (!in_quote && args[c]==' '){
            args[c]=0;
            in_param=false;
         } 
      }
      else{
         if (args[c]!=' '){
            if (args[c]=='"'){
               if (args[c+1]==0){
                  free(args);
                  return(CMD_SYNTAX);
               }
               argp[argn]=&args[c+1];
            }
            else{
               argp[argn]=&args[c];      
            }
            in_param=true;
            //CONS->printf("PackMaster: Command '%s' (pos=%d)\n",args+c,argn);
            argn++;
         }
      }
   }

   int commandResult;
   int finalResult=CMD_FAIL;
   for(long c=0;c<Pkg.length();c++){
      commandResult=Pkg[c]->command(session,cli,argp,argn);
      if (commandResult==CMD_OK || commandResult==CMD_SYNTAX){
         finalResult=commandResult;
         break;
      }
      if (commandResult==CMD_PART){
         finalResult=CMD_OK;
      }
   }
   free(args);
   return(finalResult);
}

void PackMaster::handleWiFiEvent(WiFiEvent_t event){
   if (event==WIFI_EVENT_STAMODE_DISCONNECTED){
      SysEvent e;
      e.type=SYS_EVENT_NET_DOWN;
      Controller->postSystemEvent(&e,"PackMaster");
   }
   for(long c=0;c<Pkg.length();c++){
      Pkg[c]->handleWiFiEvent(event);
   }
   if (event==WIFI_EVENT_STAMODE_GOT_IP){
      SysEvent e;
      e.type=SYS_EVENT_NET_UP;
      Controller->postSystemEvent(&e,"PackMaster");
   }
}


// Interfaces for Packages
PackStdCons *PackMaster::Console(){
   if (console==NULL){  // Hard Error - Invalid Console use
      delay(100);
      Serial.begin(115200);
      Serial.print("\n");
      delay(100);
      Serial.print("\n");
      Serial.print("ERROR: use of CONS with undefine 'console'\n");
      console->printf("\n");
      ESP.restart();  // ensure reboot
   }
   return(console);
}

String md5sum(String text){
  byte bbuff[16];
  String hash = "";
      MD5Builder nonce_md5;  
      nonce_md5.begin();
      nonce_md5.add(text);  
      nonce_md5.calculate();  
      nonce_md5.getBytes(bbuff);
      for ( byte i = 0; i < 16; i++) hash += byte2hex(bbuff[i]);
      return hash;   
}

String byte2hex(byte Zahl){
  String Hstring = String(Zahl, HEX);
  if (Zahl < 16){Hstring = "0" + Hstring;}
  return Hstring;
}

