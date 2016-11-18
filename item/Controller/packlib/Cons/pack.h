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
   PackMaster *Controller;
   int        wave=30;
   public:
   long     repeat;
   ledCtrl(PackMaster *p,long r) : ledCtrl(p,r,20){};
   ledCtrl(PackMaster *p,long r,int w){   // dig 30:70 sind -30:-70
      this->repeat=r;
      this->wave=w;
      this->Controller=p;
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


   public:
   Cons(int t);
   virtual void setup();
   virtual void setSysLED(int level,bool rst=true);
   virtual void setSysLED(float level,bool rst=true);
   virtual void setSysLED(ledCtrl *p);
   virtual bool installStandardSysControl(int pio);
   virtual void loop();
   virtual int  command(Session *session,Print *cli,char **args,int argn);


   // Interface for other Packages
   void set(int t);

   //virtual size_t write(const uint8_t *buffer, size_t size);
   virtual size_t write(uint8_t);

};

#endif

