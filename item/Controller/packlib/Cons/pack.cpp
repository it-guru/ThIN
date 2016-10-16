#include "./pack.h"

Cons::Cons(int t){
//   pinMode(LED_BUILTIN, OUTPUT); 
//   digitalWrite(LED_BUILTIN, LOW);
//   delay(3000);
//   digitalWrite(LED_BUILTIN, HIGH); 
//   delay(2000);
//   delay(10);
//   delay(100);
//   Serial.print("\n");
//   Serial.print("Controller running ...\n");
   set(t);
}

void Cons::setup(){
//   i=new Interval(5000,[&](long t){this->keepAliveMessage();});
}

void Cons::loop(){
}

void Cons::set(int newMode){
   int oldMode;

   oldMode=curMode;

   // Switch Operations //////////////
   if (newMode&CONS_SERIAL){
      delay(100);
      Serial.begin(115200,SERIAL_8N1,SERIAL_TX_ONLY);
     // This serial config allows to use GPIO3 (RX) as input as descripted at
     //http://www.forward.com.au/pfod/ESP8266/GPIOpins/ESP8266_01_pin_magic.html
      Serial.print("\n");
      delay(100);
      Serial.print("\n");
   }



   ///////////////////////////////////
   curMode=newMode;
}

//void Cons::printf(const char *fmt, ...){
//   int l;
//   char buffer[255];
//
//   l=strlen(fmt);
//   va_list ap;
//   va_start(ap, fmt);
//   ets_vsnprintf(buffer,255, fmt, ap);
//   va_end(ap);
//   
//   if (curMode&CONS_SERIAL){
//      Serial.printf("%09lu: ",millis());
//      Serial.print(buffer);
//      Serial.print("\n");
//   }
//   //if (this->curType&CON_TCP){
//   //   if (ConTcp.connected()){
//   //      ConTcp.print(buffer);
//   //   }
//   //}
//}



size_t Cons::write(uint8_t c){
   if (curMode&CONS_SERIAL){
      if (eol){
         Serial.printf("%09lu: ",millis());
      }
      if (c=='\n'){
         eol=1;
      }
      else{
         eol=0;
      }
      Serial.write(c);
   }
   return(1);
}

int Cons::command(Session *session,Print *cli,char **args,int argn){
   if (!strcmp(args[0],"conslog")){
      String l="log(";
      l+=session->ipaddr;
      l+="):";
      for(int c=1;c<argn;c++){
         if (c>1){
            l+=" ";
         }
         l+=args[1];
      }
      printf("%s\n",l.c_str());
      return(CMD_OK);
   }
   else if (!strcmp(args[0],"set") && argn==3 &&
            !strcmp(args[1],PackName.c_str())){
      if (!strcmp(args[2],"disabled")){
         curMode=0;
         return(CMD_OK);
      }
      else if (!strcmp(args[2],"serial")){
         curMode|=CONS_SERIAL;
         return(CMD_OK);
      }
      else{
         return(CMD_SYNTAX);
      }
   }
   else if (!strcmp(args[0],"help") && 
       ( (argn==1) ||
         (argn==2 && !strcmp(args[1],"console")))){
      cli->printf("\n%s:\n",PackName.c_str());
      cli->printf("conslog [text] [...]\n");
      cli->printf("    Send a [text] to the current console. If console\n");
      cli->printf("    is disable, no message is displayed anywhere.\n");
      cli->printf("\n","");
      cli->printf("set %s [disabled|serial]\n",PackName.c_str());
      cli->printf("    Sets a new target for console messages. If it is\n");
      cli->printf("    set to \"disabled\", all output will be supressed.\n");
      cli->printf("\n","");
      return(CMD_PART);
   }
   return(0);
};




