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
   long set(char *nkey,Val nval){
      return(set((const char *)nkey,nval));
   };
   long set(const char *nkey,Val nval){
      long p=getIndex((char *)nkey);
      if (p==-1){
         if (k==NULL || v==NULL){
            k=(char **) malloc(sizeof(char *)*(alloced+DynHashTabBlk));
            v=(Val *)   malloc(sizeof(Val)*(alloced+DynHashTabBlk));
            alloced+=DynHashTabBlk;
         }
         else{
            if (MapLength>=alloced){
               char **tmp1;
               tmp1=(char **)realloc(k,sizeof(char **)*(alloced+DynHashTabBlk));
               k=tmp1;
               Val *tmp2;
               tmp2=(Val *)realloc(v,sizeof(Val *)*(alloced+DynHashTabBlk));
               v=tmp2;
               alloced+=DynHashTabBlk;
            }
         }
         if (k!=NULL && v!=NULL){
            p=MapLength;
            MapLength++;
         }
      }
      if (p!=-1){
         //k[p]=(char *)nkey;
         k[p]=(char *)malloc(strlen(nkey)+1);
         strcpy(k[p],nkey);
         v[p]=nval;
      }
      else{
         Serial.printf("DynHashTabel: assign error p=-1\n");
      }
      return(p);
   };
   Val get(const char *nkey){
      return(get((char *)nkey));
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
