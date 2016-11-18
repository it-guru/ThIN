#include "./pack.h"

Cons::Cons(int t){
   progMode=digitalRead(0) ? true:false;

   if (t&CONS_SERIAL_AUTODISABLE){
      if (progMode){
         t=t&(~CONS_SERIAL);
      }
      else{
         t=t|CONS_SERIAL;
      }
   }
   if ((t&CONS_LED) && (t&CONS_SERIAL)){
      t=t&(~CONS_LED);  // disable nativ LED Mode, if Serial Cons is active
   }
   set(t);
}


class BtnCtrl{
   PackMaster *Controller;
   Interval   *pInt=NULL;
   long *pressLevels;
   int  mode;  // RISING pressed=high ; FALLING pressed=low
   long pressTime=0;
   bool pressedTarget=true;
   int  curPressLevel=-2;
   int gpio;
   public:
   BtnCtrl(PackMaster *Controller,int gpio,int mode,long *pressLevels){
       this->Controller=Controller;
       this->pressLevels=pressLevels;
       this->gpio=gpio;
       this->mode=mode;
       this->mode=RISING;
       this->initBtn();
   };
   BtnCtrl(PackMaster *Controller,int gpio){
       this->Controller=Controller;
       static long pressLevels[]={20,    // entprellen
                                3000,  // fast blink
                                3100,  // hot for WPS
                                10000, // unhot for WPS
                                30000, // hot deep reset
                                0};
       this->gpio=gpio;
       this->pressLevels=(long *)pressLevels;
       this->mode=FALLING;
       this->initBtn();
   };
   ~BtnCtrl(){
      if (this->pInt!=NULL){
         delete(this->pInt);
         this->pInt=NULL;
      }
   }
   void pressLevelHandler(int oldLevel,int newLevel){
       if (newLevel==0){
          CONS->printf("SYSLED: on\n");
          CONS->setSysLED(true);
       } 
       if (newLevel==1){
          CONS->printf("SYSLED: short blink\n");
          CONS->setSysLED(new ledCtrl(Controller,700));
       }
       if (newLevel==3){
          CONS->printf("SYSLED: short sinblink\n");
          CONS->setSysLED(new ledCtrl(Controller,4000,-50));
       }
       if (newLevel<0){
          CONS->printf("SYSLED: off\n");
          CONS->setSysLED(false);
       }
       CONS->printf("pressLevelHandler: oldLevel=%d newLevel=%d\n",
                    oldLevel,newLevel);
   };
   void initBtn(){
      if (this->mode==FALLING){
         pressedTarget=false;
      }
      pInt=Controller->addInterval(new Interval(10,
                               [&](long cnt,int iFlag)->long{
         long now=millis();
         if (digitalRead(gpio)==pressedTarget){
            if (curPressLevel==-2){
               pressTime=now;
               curPressLevel++;
            }
            if (curPressLevel>-2){
               if (now-pressTime>pressLevels[curPressLevel+1]){
                  int oldPressLevel=curPressLevel;
                  curPressLevel++;
                  pressLevelHandler(oldPressLevel,curPressLevel);      
               }
            }
         }
         else{
            if (curPressLevel>-2){
               pressLevelHandler(curPressLevel,-1);
               curPressLevel=-2; 
            }
         }
         return(cnt);
      }));
   };
};



bool Cons::installStandardSysControl(int pio){
   pinMode(pio,INPUT); 
   CONS->printf("Cons::installStandardSysControl on gpio=%d\n",pio);
   BtnCtrl *b=new BtnCtrl(Controller,pio);
   return(true);
}

void Cons::setSysLED(int level,bool rst){
   if (rst){
      if (sysLedCtrl!=NULL){
         delete(sysLedCtrl);
         sysLedCtrl=NULL;
      }
   }
   if (curMode&CONS_LED){
      if (level){
         analogWrite(packlib_Cons_SysLED,0);
         digitalWrite(packlib_Cons_SysLED,LOW);
      }
      else{
         analogWrite(packlib_Cons_SysLED,PWMRANGE);
         digitalWrite(packlib_Cons_SysLED,HIGH);
      }
   }
}

void Cons::setSysLED(float level,bool rst){
   if (rst){
      if (sysLedCtrl!=NULL){
         delete(sysLedCtrl);
         sysLedCtrl=NULL;
      }
   }
   if (curMode&CONS_LED){
      if (level>1.0) level=1.0;
      if (level<0.0) level=0.0;
      int l=(int) (level*PWMRANGE);
      analogWrite(packlib_Cons_SysLED,PWMRANGE-l);
   }
}

void Cons::setSysLED(ledCtrl *p){
   if (sysLedCtrl!=NULL){
      delete(sysLedCtrl);
      sysLedCtrl=NULL;
   }
   sysLedCtrl=p;
}



void Cons::setup(){
   this->installStandardSysControl(3);
}

void Cons::loop(){
}

void Cons::set(int newMode){
   int oldMode;

   oldMode=curMode;

   // Switch Operations //////////////
   if (newMode&CONS_SERIAL){
      delay(100);
      Serial.begin(74880,SERIAL_8N1,SERIAL_TX_ONLY); // this is the standard!
      //Serial.setDebugOutput(true);
     // This serial config allows to use GPIO3 (RX) as input as descripted at
     //http://www.forward.com.au/pfod/ESP8266/GPIOpins/ESP8266_01_pin_magic.html
      Serial.print("\n");
      delay(100);
      Serial.print("\n");
   }
   if (newMode&CONS_LED){
      pinMode(packlib_Cons_SysLED,OUTPUT);
      digitalWrite(packlib_Cons_SysLED,HIGH);
   }

   ///////////////////////////////////
   curMode=newMode;
}

//void Cons::printf(const char *fmt, ...){
//   int l;
//   char buffer[255];
//
//   l=strlen(fmt);
//   va_list ap;
//   va_start(ap, fmt);
//   ets_vsnprintf(buffer,255, fmt, ap);
//   va_end(ap);
//   
//   if (curMode&CONS_SERIAL){
//      Serial.printf("%09lu: ",millis());
//      Serial.print(buffer);
//      Serial.print("\n");
//   }
//   //if (this->curType&CON_TCP){
//   //   if (ConTcp.connected()){
//   //      ConTcp.print(buffer);
//   //   }
//   //}
//}



size_t Cons::write(uint8_t c){
   if (curMode&CONS_SERIAL){
      if (eol){
         Serial.printf("%09lu: ",millis());
      }
      if (c=='\n'){
         eol=1;
      }
      else{
         eol=0;
      }
      Serial.write(c);
   }
   return(1);
}

int Cons::command(Session *session,Print *cli,char **args,int argn){
   if (!strcmp(args[0],"conslog")){
      String l="log(";
      l+=session->ipaddr;
      l+="):";
      for(int c=1;c<argn;c++){
         if (c>1){
            l+=" ";
         }
         l+=args[1];
      }
      printf("%s\n",l.c_str());
      return(CMD_OK);
   }
   else if (!strcmp(args[0],"set") && argn==3 &&
            !strcmp(args[1],PackName.c_str())){
      if (!strcmp(args[2],"disabled")){
         curMode=0;
         return(CMD_OK);
      }
      else if (!strcmp(args[2],"serial")){
         curMode|=CONS_SERIAL;
         return(CMD_OK);
      }
      else{
         return(CMD_SYNTAX);
      }
   }
   else if (!strcmp(args[0],"help") && 
       ( (argn==1) ||
         (argn==2 && !strcmp(args[1],"console")))){
      cli->printf("\n%s:\n",PackName.c_str());
      cli->printf("conslog [text] [...]\n");
      cli->printf("    Send a [text] to the current console. If console\n");
      cli->printf("    is disable, no message is displayed anywhere.\n");
      cli->printf("\n","");
      cli->printf("set %s [disabled|serial]\n",PackName.c_str());
      cli->printf("    Sets a new target for console messages. If it is\n");
      cli->printf("    set to \"disabled\", all output will be supressed.\n");
      cli->printf("\n","");
      return(CMD_PART);
   }
   return(0);
};




