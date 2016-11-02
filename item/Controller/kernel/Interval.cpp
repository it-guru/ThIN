#include "../include/Interval.hpp"

Interval::Interval(long i,IntervalHandlerFunction f){
   interval=i;
   countdown=interval;
   oldt=t();
   callback=f;
   cnt=-1;         // endless
};

Interval::Interval(long i,IntervalHandlerFunction f,long setcnt){
   interval=i;
   countdown=interval;
   oldt=t();
   callback=f;
   cnt=setcnt;
};

long Interval::t(){
   return((long) millis());
};


long Interval::loop(){
   if (cnt<0 || cnt>0){
      long newt=t();
      if (newt!=oldt){
         countdown-=(newt-oldt);
         oldt=newt;
      }
      //Serial.printf("cnt=%ld countdown=%ld\n",cnt,countdown);
      if (countdown<=0){
         if (cnt>0){
            cnt--;
         }
         intervalFlag=intervalFlag ? 0 : 1;
         cnt=callback(cnt,intervalFlag);
         countdown=interval;
      }
   }
   return(cnt);   // if cnt==0 then parent should kill the timer
};

IntervalBlink::IntervalBlink(long i,IntervalHandlerFunction f){
   interval=i;
   countdown=interval;
   oldt=t();
   callback=f;
   cnt=-1;         // endless
};

IntervalBlink::IntervalBlink(long i,IntervalHandlerFunction f,long setcnt){
   interval=i;
   countdown=interval;
   oldt=t();
   callback=f;
   cnt=setcnt;
};


long IntervalBlink::loop(){
   if (cnt!=0){
      if (cnt>0){
         cnt--;
      }
      long newt=t();
      if (newt!=oldt){
         countdown-=(newt-oldt);
         oldt=newt;
      }
      if (countdown<=0){
         intervalFlag=intervalFlag ? 0 : 1;
         cnt=callback(cnt,intervalFlag);
         countdown=interval;
      }
   }
   return(cnt);   // if cnt==0 then parent should kill the timer
};

