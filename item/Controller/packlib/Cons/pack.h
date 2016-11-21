#ifndef _lib_Cons_h_
#define _lib_Cons_h_

#include "../../kernel/PackMaster.h"

// Local default Defines (pre Config)

#include "../../config.h"

#define CONS_LED                1
#define CONS_SERIAL             2
#define CONS_TCP                4
#define CONS_SERIAL_AUTODISABLE 5  // use CONS_SERIAL in non flash mode GPIO0=0
#define CONS_NULL               0

int ets_vsprintf(char *str, const char *format, va_list arg);
int ets_vsnprintf(char *buffer, size_t sizeOfBuffer, size_t count, const char *format, va_list arg);

class ledCtrl{
   private:
   Interval *ctrlLed;
   long     t0;
   float      curLevel=0;
   int        wave=30;
   public:
   long     repeat;
   ledCtrl(long r) : ledCtrl(r,20){};
   ledCtrl(long r,int w){   // dig 30:70 sind -30:-70
      this->repeat=r;
      this->wave=w;
      t0=millis();
      ctrlLed=Controller->addInterval(new Interval(10,
                                       [&](long cnt,int intervalFlag)->long{
         long now=millis();
         long zyclePos=now-t0;
         float newLevel;
         if (zyclePos>repeat){
            zyclePos=0;
            t0=now;
         }
         if (wave>0 && wave<100){
            long flippos=(long)((float)repeat*wave/100.0);
            if (zyclePos<flippos){
               newLevel=1.0;
            }
            else{
               newLevel=0.0;
            }
         }
         else if (wave<0 && wave>-100){
            float f=(3.14)/100.0*(wave*-1);
            newLevel=(sin(3.14/repeat*zyclePos)*f-f+1.0);
            if (newLevel<0) newLevel=0.0;
            //String fs(newLevel,2);
            //CONS->printf("newLevel=%s zyclePos=%ld repeat=%ld\n",
            //             fs.c_str(),zyclePos,repeat);
         }
         //CONS->printf("zyclePos=%ld\n",zyclePos);
         if (newLevel!=curLevel){
            setLED(newLevel);
            curLevel=newLevel;
         }
         return(cnt);
      }));
      //CONS->printf("intervall %x added\n",ctrlLed);
   };
   ~ledCtrl(){
      //CONS->printf("intervall delete %x\n",ctrlLed);
      ctrlLed=Controller->delInterval(ctrlLed);
   }
   void setLED(float level){
      String lstr(level,2);
      CONS->printf("setLED=%s\n",lstr.c_str());
      Controller->Console()->setSysLED(level,false);
   }
};



class BtnCtrl{
   Interval   *pInt=NULL;
   long       *pressLevels;
   int        mode;  // RISING pressed=high ; FALLING pressed=low
   long       pressTime=0;
   bool       pressedTarget=true;
   bool       curSwiState=false;
   int        curPressLevel=-2;
   int        gpio;
   long       devBtnPos;
   long       devSwiPos;
   bool       devSwiState=false;
   const char *bname;
   public:
   BtnCtrl(int gpio,const char *n,int mode,long *pressLevels){
       this->pressLevels=pressLevels;
       this->gpio=gpio;
       this->mode=mode;
       this->bname=n;
       this->mode=RISING;
       this->initBtn();
   };
   BtnCtrl(int gpio,const char *n){
       static long pressLevels[]={20,    // entprellen
                                3000,  // fast blink
                                3100,  // hot for WPS
                                10000, // unhot for WPS
                                30000, // hot deep reset
                                0};
       this->gpio=gpio;
       this->bname=n;
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
   void notifyDevState(long DevicePos,int newstate){
      SysEvent e;
      e.type=SYS_EVENT_DEVLOCCHANGE;
      e.dev.devicepos=DevicePos;
      e.dev.D.state=newstate;
      Controller->postSystemEvent(&e,"BtnCtrl");
   }

   void pressLevelHandler(int oldLevel,int newLevel){
       if (newLevel==0){
          CONS->printf("SYSLED: on\n");
          CONS->setSysLED(true);
          this->notifyDevState(devBtnPos,1);
          if (devSwiState){
             devSwiState=false;
             this->notifyDevState(devSwiPos,0);
          }
          else{
             devSwiState=true;
             this->notifyDevState(devSwiPos,1);
          }
       } 
       if (newLevel==1){
          CONS->printf("SYSLED: short blink\n");
          CONS->setSysLED(new ledCtrl(700));
       }
       if (newLevel==3){
          CONS->printf("SYSLED: short sinblink\n");
          CONS->setSysLED(new ledCtrl(4000,-50));
       }
       if (newLevel<0){
          this->notifyDevState(devBtnPos,0);
       }
       if (newLevel<0 && (oldLevel==1 || oldLevel==2)){
          CONS->printf("Trigger: WPS Start\n");
          SysEvent e;
          e.type=SYS_EVENT_WPS_START;
          Controller->postSystemEvent(&e,NULL);
       }
       else if (newLevel<0){
          CONS->printf("SYSLED: off\n");
          CONS->setSysLED(false);
       }
       CONS->printf("pressLevelHandler: oldLevel=%d newLevel=%d\n",
                    oldLevel,newLevel);
   };
   void handleSystemEvent(SysEvent *e,const char *src){
      switch(e->type) {
         case SYS_EVENT_DEVQUERY:
            CONS->printf("got query for  %d from '%s'\n",
                         e->dev.devicepos,src);
            if (devBtnPos==e->dev.devicepos){
               if (digitalRead(gpio)){
                  e->dev.D.state=true;
               }
               else{
                  e->dev.D.state=false;
               }
               e->dev.cnt++;
            }
            if (devSwiPos==e->dev.devicepos){
               if (devSwiState){
                  e->dev.D.state=true;
               }
               else{
                  e->dev.D.state=false;
               }
               e->dev.cnt++;
            }
         break;
         case SYS_EVENT_WPS_END:
            CONS->setSysLED(false);
         break;
      }
   };
   void initBtn(){
      if (this->mode==FALLING){
         pressedTarget=false;
      }
      devBtnPos=Controller->registerDevice(DIN,"SYS_BTN");
      devSwiPos=Controller->registerDevice(DIN,"SYS_SWI");

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









typedef struct btnCtrl_struct{   
   int  mode;  // RISING pressed=high ; FALLING pressed=low
   long pressTime;
   int  pressLevel=0;  // 0=nix ; 1=3sec  ; 2=10sec ; 3=30sec
   ledCtrl *pLedCtrl;
} btnCtrlStruct;


class Cons : public PackStdCons {
   protected:
   int curMode=0;
   int eol=1;
   boolean progMode=false;
   ledCtrl *sysLedCtrl=NULL;
   BtnCtrl *sysBtnCtrl=NULL;


   public:
   Cons(int t);
   virtual void setup();
   virtual void setSysLED(int level,bool rst=true);
   virtual void setSysLED(float level,bool rst=true);
   virtual void setSysLED(ledCtrl *p);
   virtual bool installStandardSysControl(int pio);
   virtual void loop();
   virtual int  command(Session *session,Print *cli,char **args,int argn);
   virtual void handleSystemEvent(SysEvent *e,const char *source);


   // Interface for other Packages
   void set(int t);

   //virtual size_t write(const uint8_t *buffer, size_t size);
   virtual size_t write(uint8_t);

};

#endif

