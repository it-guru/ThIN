#include "./config.h"
#include "./console.h"
#include <EEPROM.h>

ConfigStruct Config;

 

void config_setup(){
   EEPROM.begin(EEPROMSize);
   EEPROM.get( ConfigBaseEEPROM, Config );
   if (Config.vers1!=Config_vers || Config.vers2!=Config_vers){
      Config.vers1=Config_vers;
      strcpy(Config.syslog_server,"syslog.localnet");
      strcpy(Config.mq_server,"mqserver.localnet");
      strcpy(Config.admin_password,"");
      Config.vers2=Config_vers;
   }
}

void config_save(){
   
   EEPROM.put( ConfigBaseEEPROM, Config );
   con.mdelay(100);
   EEPROM.commit();
   con.mdelay(100);           
}



