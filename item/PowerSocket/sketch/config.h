#ifndef CONFIG_H
#define CONFIG_H

typedef struct _Config {
   int  vers1;
   char syslog_server[80];
   char mq_server[80];
   char admin_password[10];
   int  vers2;
} ConfigStruct;

#define      ConfigBaseEEPROM 0
#define      EEPROMSize 513
#define Config_vers 0x431461

extern ConfigStruct Config;

void config_setup();
void config_save();

#endif
