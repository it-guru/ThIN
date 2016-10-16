#include "../config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <mem.h>

#ifndef _DYNHASHTABLE_H_
#define _DYNHASHTABLE_H_


template<typename Val>
class DynHashTable {
   public:
   DynHashTable(){};
   DynHashTable(int blk){
      DynHashTabBlk=blk;
   };

//   HashType(hash code,map value):hashCode(code),mappedValue(value){}

   long getIndex(char *nkey){
      for(long c=0;c<MapLength;c++){
         if (!strcmp(k[c],nkey)){
            return(c);
         }
      }
      return(-1);
   };
   long set(const char *nkey,Val nval){
      long p=getIndex((char *)nkey);
      Serial.printf("DynHashTabel: set %s to %ld\n",nkey,p);
      if (p==-1){
         if (k==NULL || v==NULL){
            k=(char **) malloc(sizeof(char *)*(alloced+DynHashTabBlk));
            v=(Val *)   malloc(sizeof(Val)*(alloced+DynHashTabBlk));
            alloced+=DynHashTabBlk;
         }
         else{
            if (MapLength+1>alloced){
               Serial.printf("DynHashTabel: start realloc MapLength=%ld alloced=%ld v=%ld k=%ld\n",
                             MapLength,alloced,v,k);
               char **tmp1;
               tmp1=(char **)realloc(k,sizeof(char **)*(alloced+DynHashTabBlk));
               k=tmp1;
               Val *tmp2;
               tmp2=(Val *)realloc(v,sizeof(Val *)*(alloced+DynHashTabBlk));
               v=tmp2;
               alloced+=DynHashTabBlk;
               Serial.printf("DynHashTabel: end realloc MapLength=%ld alloced=%ld v=%ld k=%ld\n",
                             MapLength,alloced,v,k);
            }
         }
         if (k!=NULL && v!=NULL){
            p=MapLength;
            MapLength++;
         }
      }
      if (p!=-1){
         Serial.printf("DynHashTabel: assign value k %s to %ld\n",nkey,p);
         k[p]=(char *)nkey;
         Serial.printf("DynHashTabel: assign value v %s to %ld adr=%ld\n",nkey,p,&v[p]);
         v[p]=nval;
         Serial.printf("DynHashTabel: assign value end\n");
      }
      else{
         Serial.printf("DynHashTabel: assign error p=-1\n");
      }
      return(p);
   };
   Val get(char *nkey){
      long p=getIndex(nkey);
      if (p==-1){
         return(NULL);
      }
      return(v[p]);
   };
   Val operator[](char *p){
      return(get(p));
   };
   Val operator[](const char *p){
      return(get((char *)p));
   };
   Val operator[](long i){
      if (i<0 || i>=length()){
         return(NULL);
      }
      return(v[i]);
   };
   char *key(long i){
      if (i<0 || i>=length()){
         return(NULL);
      }
      return(k[i]);
   };
   char *beginKey(){
      iter=0;
      return(k[iter]);
   };
   char *nextKey(){
      if (iter<MapLength){
         iter++;
      }
      return(k[iter]);
   };
   char *endKey(){
      return(k[MapLength]);
   };
   long length(){
      return(MapLength);
   };
private:
   long iter=0;
   long MapLength=0;
   long alloced=0;
   char **k=NULL;
   Val  *v=NULL;
   int  DynHashTabBlk=3;
};

#endif
