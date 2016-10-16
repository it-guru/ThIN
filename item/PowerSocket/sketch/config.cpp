#include "./config.h"
#include "./console.h"
#include <EEPROM.h>


ConfigStruct Config;

 

void config_setup(){
   //EEPROM.begin(EEPROMSize);
   //EEPROM.get( ConfigBaseEEPROM, Config );
   if (Config.vers1!=Config_vers || Config.vers2!=Config_vers){
      Config.vers1=Config_vers;
      strcpy(Config.syslog_server,"syslog.localnet");
      strcpy(Config.mq_server,"mqserver.localnet");
      strcpy(Config.admin_password,"");

      String hostname="ThIN_";
      String s="PSOCK_";
      uint8_t mac[6];
      WiFi.macAddress(mac);
      for (int i = 3; i < 6; ++i) {
         s+=String(mac[i], 16);
      }
      s.toUpperCase();
      hostname+=s;
      hostname.toCharArray(Config.hostname,sizeof(Config.hostname));
      Config.vers2=Config_vers;
   }
}

void config_save(){
   
 //  EEPROM.put( ConfigBaseEEPROM, Config );
 //  con.mdelay(100);
 //  EEPROM.commit();
   con.mdelay(100);           
}



