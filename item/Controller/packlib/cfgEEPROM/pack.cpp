#include "./pack.h"


void cfgEEPROM::setup(){
   CONS->printf("cfgEEPROM: cfgEEPROMworkController setup() start\n");
   EEPROM.begin(PMEMSIZE);

   //
   // read current eeprom and check consistence
   //
   byte *pmem=(byte *)&m;
   CONS->printf("cfgEEPROM: reading EEPROM\n");
   for(int adr=0;adr<PMEMSIZE;adr++){
      pmem[adr]=EEPROM.read(adr);
   }
   CONS->printf("cfgEEPROM:  readchecksum=%d\n",pmem[0]);
   byte readchecksum=m.checksum;
   CONS->printf("cfgEEPROM:  m.checksum=%d\n",m.checksum);
   makeChecksum();
   CONS->printf("cfgEEPROM:  new checked=%d\n",m.checksum);
   userEntry *adm=getUser(0);
   if (readchecksum!=m.checksum || adm==NULL || strcmp(adm->username,"admin")){
      CONS->printf("cfgEEPROM:  checksum error\n");
      init();
      store();
   }
   Controller->cfg=this;
}


void cfgEEPROM::validateVariables(){
   char *p=m.cfg;
   char *v;
   CONS->printf("cfgEEPROM:  start validateVariables()\n");
   while((p-m.cfg)<m.cfgsize){
      v=p;
      p=p+strlen(p)+1;
      String pack=v;
      if (pack.indexOf(".")>0){
         pack=pack.substring(0,pack.indexOf("."));
      }
      Pack *packObj=(Pack *) Controller->findPack(pack.c_str());
      if (packObj==NULL){
         removeEnt(v);
         p=v;
      }
      else{
         String msg;
         String var=v;
         String val=p;
         var=var.substring(var.indexOf(".")+1);
         if (!packObj->validateVariable(var,NULL,val,msg)){
            removeEnt(v);
            p=v;
         }
         else if (strcmp(p,val.c_str())){
            setVar(v,val.c_str());
            p=v;
         }
         else{
            p=p+strlen(p)+1;
         }
      }
   }
}



void cfgEEPROM::store(){
   validateVariables();
   makeChecksum();
   byte *pmem=(byte *)&m;
   byte oldbyte;
   bool flashDirty=false;
   for(int adr=0;adr<PMEMSIZE;adr++){ // start calculation from 2nd byte
      oldbyte=EEPROM.read(adr);
      if (oldbyte!=pmem[adr]){
         flashDirty=true;
         EEPROM.write(adr,pmem[adr]);
      }
   }
   if (flashDirty){
      CONS->printf("cfgEEPROM:  ----+++++ commit eeprom  +++++-----\n");
      EEPROM.commit();
      memDirty=0;
      delay(100);  // give him time
   }
}

void cfgEEPROM::loop(){
   if (memDirty>0){
      if (memDirty<millis()-5000){
         CONS->printf("\ncfgEEPROM:  pre list\n");
         list();
         CONS->printf("\n\ncfgEEPROM:  autosave EEPROM start\n");
         store();
         CONS->printf("\ncfgEEPROM:  post list\n");
         list();
         CONS->printf("cfgEEPROM:  autosave EEPROM end\n");
         memDirty=0;
      }
   }
}

void cfgEEPROM::handleWiFiEvent(WiFiEvent_t e){
}




