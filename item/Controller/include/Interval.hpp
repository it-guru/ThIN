#include <Arduino.h>
#include <time.h>
#include <functional>

#ifndef _INTERVAL_OBJECT__
#define _INTERVAL_OBJECT__
typedef std::function<void(long)> IntervalHandlerFunction;
class Interval{
   private:
   long interval;
   long countdown;
   long oldt;
   long cnt;
   //void (*callback)(long);
   IntervalHandlerFunction callback;   

   long t(){
      return((long) millis());
   };

   public:
   Interval(long i,IntervalHandlerFunction f){
      interval=i;
      countdown=interval;
      oldt=t();
      callback=f;
      cnt=-1;         // endless
   };
   Interval(long i,IntervalHandlerFunction f,long setcnt){
      interval=i;
      countdown=interval;
      oldt=t();
      callback=f;
      cnt=setcnt; 
   };

   long loop(){
      if (cnt!=0){
         long newt=t();
         if (newt!=oldt){
            countdown-=(newt-oldt);
            oldt=newt;
         }
         if (countdown<=0){
            callback(newt); 
            countdown=interval;
         }
      }
      if (cnt>0){
         cnt--;
      }
      return(cnt);   // if cnt==0 then parent should kill the timer
   };
};

class xUptimeController {
   public:
     void loop(){
        unsigned long now=millis();
        if (now-lastTick>1000){
           seconds+=(now-lastTick)/1000UL;
           lastTick=now;
        }
     };
     unsigned long getSeconds(){
        unsigned long now=millis();
        return(seconds);
     };

   private:
     unsigned long seconds=0;
     unsigned long lastTick=0;
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
