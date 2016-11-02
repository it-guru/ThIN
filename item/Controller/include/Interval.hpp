#include <Arduino.h>
#include <time.h>
#include <functional>

#ifndef _INTERVAL_OBJECT__
#define _INTERVAL_OBJECT__
typedef std::function<long(long,int)> IntervalHandlerFunction;
class Interval;

class Interval{
   protected:
   long interval;
   long countdown;
   long oldt;
   long cnt;
   int  intervalFlag=0;
   IntervalHandlerFunction callback;   

   long t();

   public:
   Interval *pNext=NULL;
   Interval(){};
   Interval(long i,IntervalHandlerFunction f);
   Interval(long i,IntervalHandlerFunction f,long setcnt);

   virtual long loop();
};


typedef enum IntervalBlinkType
{
   DIG_BLK    = 10,
   SIN_BLK,
} IntervalBlink_t;


class IntervalBlink : public Interval{
   
   IntervalBlink(long i,IntervalHandlerFunction f);
   IntervalBlink(long i,IntervalHandlerFunction f,long setcnt);
   long loop();

};

class UptimeController {
   public:
     void loop(){
        unsigned long now=millis();
        if (now<lastNow){  // rollover
           lastNow=0;
           lastNow=~lastNow; // get max of UL
           lastBase+=(lastNow)/1000UL;
        }
        lastNow=now;
     };
     unsigned long getSeconds(){
        unsigned long now=millis();
        unsigned long seconds=(now)/1000UL;
        return(seconds+lastBase);
     };

   private:
     unsigned long lastBase=0;
     unsigned long lastNow=0;
};












#endif
