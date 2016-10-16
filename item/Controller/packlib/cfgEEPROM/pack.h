#ifndef _lib_cfgEEPROM_h_
#define _lib_cfgEEPROM_h_

#include "../../kernel/PackMaster.h"
#include "../../config.h"

#include <EEPROM.h>

#define PMEMSIZE 1024
#define MAXUSER 3


typedef struct _persistentMemStore{
   byte  checksum;
   userEntry user[MAXUSER];
   int  cfgsize;
   char cfg[PMEMSIZE-sizeof(user)-sizeof(cfgsize)-sizeof(checksum)];
} persistentMemStore;




class cfgEEPROM : public Pack, public ConfigPack {
   protected:

   persistentMemStore m;
   unsigned long  memDirty=0;


   private:
   char *getVarEnt(const char *var){
      char *p=m.cfg;
      bool match=false;
      while((p-m.cfg)<m.cfgsize){
         if (!strcmp(p,var)) return(p);
         p=p+strlen(p)+1;
         p=p+strlen(p)+1;
      }
      return(NULL);
   }
   void removeEnt(const char *var){
      char *p=m.cfg;
      char *pstart;
      char *pend;
      int  newcfgsize;
      int  entSize=0;
      while((p-m.cfg)<m.cfgsize){
         if (!strcmp(p,var)){
            pstart=p;
            entSize=strlen(p)+1;
            p=p+strlen(p)+1;
            CONS->printf("removeEnt '%s'='%s' cfgsize=%d\n",var,p,m.cfgsize);
            pend=p+strlen(p)+1;
            entSize+=strlen(p)+1;
            newcfgsize=m.cfgsize-entSize;
            int mvSz=m.cfgsize-(pstart-m.cfg);
            memmove(pstart,pend,mvSz);
            m.cfgsize=newcfgsize;
            CONS->printf("removeEnt mvSz=%d m.cfgsize=%d\n",mvSz,m.cfgsize);
            return;
         }
         p=p+strlen(p)+1;
         p=p+strlen(p)+1;
      }
   }

   public:

   bool setUser(int8_t un,const char *user,const char *pass,int8_t authLevel){
      if (un>=MAXUSER) return(false);
      strncpy(m.user[un].username,user,sizeof(m.user[un].username));
      strncpy(m.user[un].password,pass,sizeof(m.user[un].password));
      m.user[un].authLevel=authLevel;
      return(true);
   }

   void init(){
      memset(&m,0,sizeof(m));
      setUser(0,"admin","admin",100);
      setUser(1,"user","user",50);
   }
   userEntry *getUser(int8_t un){
      if (un>=MAXUSER || un<0) return(NULL);
      if (m.user[un].username[0]==0) return(NULL);
      return(&m.user[un]);
   }
   virtual char *setVar(const char *var,const char *val){
      char *curEnt,*bk=NULL;
      if ((curEnt=getVarEnt(var))!=NULL || val==NULL){
         removeEnt(var);
         if (val==NULL) return(NULL);
      }
      if (m.cfgsize+strlen(var)+strlen(val)+3>sizeof(m.cfg)){
         onOutOfMemory(var);
         return(NULL);
      }
      strcpy(&m.cfg[m.cfgsize],var);
      m.cfgsize+=strlen(var)+1;
      strcpy(&m.cfg[m.cfgsize],val);
      bk=&m.cfg[m.cfgsize];
      m.cfgsize+=strlen(val)+1;
      CONS->printf("setVar %-30s = '%s' after cfgsize=%d\n",var,val,m.cfgsize);
      memDirty=millis();
      return(bk);
   }
   virtual char *getVar(const char *var){
      char *p=m.cfg;
      bool match=false;
      while((p-m.cfg)<m.cfgsize){
         if (!strcmp(p,var)) match=true;
         p=p+strlen(p)+1;
         if (match) return(p);
         p=p+strlen(p)+1;
      }
      return(NULL);
   }

   void list(){
      char *p=m.cfg;
      char *v;
      CONS->printf("list (cfgsize=%d):\n",m.cfgsize);
      while((p-m.cfg)<m.cfgsize){
         v=p;
         p=p+strlen(p)+1;
         CONS->printf("%-30s = '%s'\n",v,p);
         p=p+strlen(p)+1;
      }
   }
   bool setVar(const char *var,const char *val,String &msg){
      String svar=var;
      String sval=val;
      String pack=svar;
      if (pack.indexOf(".")>0){
         pack=pack.substring(0,pack.indexOf("."));
      }
      Pack *packObj=(Pack *) Controller->findPack(pack.c_str());
      if (packObj==NULL){
         msg="ERROR: unkonwn pack '"+pack+"'";
         return(false);
      }
      else{
         char *oval=getVar(svar.c_str());
         svar=svar.substring(svar.indexOf(".")+1);
         if (!packObj->validateVariable(svar,oval,sval,msg)){
            return(false);
         }
         if (oval==NULL || sval!=val){
            setVar(var,sval.c_str());
         }
      }
      return(true);
   }
   bool getEntry(unsigned long req, String &var,String &val){
      char *p=m.cfg;
      char *v;
      unsigned long c=0;
      while((p-m.cfg)<m.cfgsize){
         v=p;
         p=p+strlen(p)+1;
         if (req==c){
            var=v;
            val=p;
            return(true);
         }
         c++;
         p=p+strlen(p)+1;
      }
      return(false);
   }

   void verify(){
      char *p=m.cfg;
      char *v;
      while((p-m.cfg)<m.cfgsize){
         v=p;
         p=p+strlen(p)+1;
         printf("%-30s = '%s'\n",v,p);
         if (!verifyiVar(v,p)){
            removeEnt(v);
            p=v;
         }
         else{
            p=p+strlen(p)+1;
         }
      }
   }
   void makeChecksum(){
      byte *pmem=(byte *)&m;
      byte checksum=294;  // checksum base
      for(int adr=1;adr<PMEMSIZE;adr++){ // start calculation from 2nd byte
         checksum=checksum^pmem[adr];
      }
      m.checksum=checksum;
   }
   virtual void store();
   virtual void validateVariables();

   


   virtual bool verifyiVar(char *var,char *val){
      if (!strstr(var,"gmail")){
         return(false);
      }
      return(true);
   }
   virtual void onOutOfMemory(const char *var ){
      //printf("Out of memory while setting %s\n",var);
      //exit(1);
   }

   public:
   virtual void setup();
   virtual void loop();
   virtual void handleWiFiEvent(WiFiEvent_t e);
};

#endif

