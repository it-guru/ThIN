#include "./pack.h"
//#undef max
//#include <vector>


static const telnet_telopt_t my_telopts[] = {
    { TELNET_TELOPT_ECHO,      TELNET_WILL, TELNET_DONT },
    { TELNET_TELOPT_TTYPE,     TELNET_WILL, TELNET_DO   },
    { TELNET_TELOPT_COMPRESS2, TELNET_WONT, TELNET_DO   },
    { TELNET_TELOPT_ZMP,       TELNET_WONT, TELNET_DO   },
    { TELNET_TELOPT_MSSP,      TELNET_WONT, TELNET_DO   },
    { TELNET_TELOPT_BINARY,    TELNET_WILL, TELNET_DO   },
    { TELNET_TELOPT_NAWS,      TELNET_WILL, TELNET_DONT },
    { -1, 0, 0 }
};

char *strerror(int n){
   return("xx");
}

void tchandler(telnet_t *telnet, telnet_event_t *event, void *cliP){
   TelCli *cli;

   cli=(TelCli *) cliP;
   cli->handleTelnet(event); 
}

//#######################################################################
//#######################################################################
//#######################################################################

void TelCli::handleTelnet(telnet_event_t *ev){
   char *tmpBuffer;
   switch (ev->type) {
    case TELNET_EV_DATA:
       //process_user_input(user, event->data.buffer, event->data.size);
       if (ev->data.size<256){
          tmpBuffer=(char *) malloc(ev->data.size+1);
          tmpBuffer[ev->data.size]=0;
          memcpy(tmpBuffer,ev->data.buffer,ev->data.size);
          lineBuffer+=tmpBuffer;
          free(tmpBuffer);
       }
       else{
          CONS->printf("TELNET size error\n");
       }
       if (lineBuffer.endsWith("\n") && lineBuffer.length()==1){
          lineBuffer="";
       }
       if (lineBuffer.endsWith("\n") || lineBuffer.endsWith("\r")){
          lineBuffer.replace("\r"," ");
          lineBuffer.replace("\n"," ");
          lineBuffer.trim();
          if (sessionStep==ONLINE){
             if (lineBuffer.length()>0){
                if (lineBuffer.compareTo("exit")==0 ||
                    lineBuffer.compareTo("quit")==0){
                   inShutdown=true;
                }
                else{
                   runningCmd=lineBuffer;
                   int res=Controller->command(&session,
                                               (Print *)this,runningCmd); 
                   if (res==CMD_FAIL){
                      printf("'%s' command not found\n",runningCmd.c_str());
                   }
                   if (res==CMD_SYNTAX){
                      printf("'%s' syntax error\n",runningCmd.c_str());
                   }
                   if (res!=CMD_RUNS){
                      runningCmd="";
                   }
                }
                lineBuffer=""; 
             }
          }
          else if (sessionStep==ASKPASSWORD){
             if (lineBuffer.length()>0){
                if (Controller->auth!=NULL){
                   long   uid=-1;
                   int8_t authLevel=-1;
                   String password(lineBuffer);

                   bool foundUser=Controller->auth->authUser(
                                      session.user,password,&uid,&authLevel);
                   if (foundUser){
                      session.uid=uid;
                      sessionStep=ONLINE;
                   }
                   else{
                      sessionStep=ASKUSER;
                   }
                }
                else{
                   sessionStep=ASKUSER;
                }
                lineBuffer=""; 
                if (session.uid!=-1){
                   sessionStep=ONLINE;
                }
             }
          }
          else if (sessionStep==ASKUSER){
             if (lineBuffer.length()>0){
                if (lineBuffer=="anonymous"){
                   session.user=lineBuffer;
                   session.uid=-1;
                   sessionStep=ONLINE;
                }
                else{
                   if (Controller->cfg!=NULL){
                    //  int8_t uid=0;
                    //  userEntry *u=Controller->cfg->getUser(uid);
                    //  while(u!=NULL){
                    //      if (lineBuffer==u->username){
                    //         session.user=lineBuffer;
                    //         break;
                    //      }
                    //
                    //      u=Controller->cfg->getUser(++uid);
                    //  }
                      session.user=lineBuffer;
                      sessionStep=ASKPASSWORD;
                   }
                }
                lineBuffer=""; 
             }
          }
          sendPrompt();
       }
       break;
    case TELNET_EV_SEND:
       fd.write(ev->data.buffer, ev->data.size);
       break;
    case TELNET_EV_ERROR:
       CONS->printf("TELNET error: %s", ev->error.msg);
       break;
    case TELNET_EV_WILL:
    case TELNET_EV_DO:
    case TELNET_EV_IAC:
       char sb[1];
       sb[0] = TELNET_TTYPE_SEND;
       telnet_subnegotiation(tc,TELNET_TELOPT_TTYPE, sb, 1);

       break; 
    case TELNET_EV_TTYPE:
       term=ev->ttype.name;
       //CONS->printf("TTYPE %s %s\n", ev->ttype.cmd ? "SEND" : "IS",
       //      ev->ttype.name ? ev->ttype.name : "");
       if (ev->ttype.cmd==TELNET_TTYPE_IS){
         printf("Connected to ThIN device %s\n",Controller->hostname());
         isTTY=true;
         sendPrompt();
       }
       break;
    }

}

TelCli::TelCli(PackMaster *pController,WiFiClient newfd){
   fd=newfd;
   lineBuffer="";
   term="";
   inShutdown=false;
   isTTY=false;
   sessionStep=ASKUSER;
   session.user="anonymous";
   session.proto="telnet";
   session.ipaddr=newfd.remoteIP().toString();
   Controller=pController;
   tc=telnet_init(my_telopts,tchandler,0,this);
}

void TelCli::sendPrompt(){
   if (isTTY){
      if (sessionStep==ASKUSER){
         printf("Login: ");
      }
      if (sessionStep==ASKPASSWORD){
         printf("Password: ");
      }
      if (sessionStep==ONLINE){
         printf("anonymous@%s:$ ",Controller->hostname());
      }
   }
}




size_t TelCli::write(uint8_t c){
   char buf[1];

   buf[0]=c;
   telnet_send(tc,buf,1);
   return(1);
}


size_t TelCli::write(const uint8_t *buffer, size_t size) {
    telnet_send(tc,(const char *)buffer,size);
    return size;
}



bool TelCli::handleClient(){
   if (inShutdown){
   //   telnet_free(tc);
   //   fd.stop();
      return(false);
   }
   if (fd.available()){
      int c;
      c=fd.read();
      if (c==4){
         //telnet_free(tc);
         //fd.stop();
         return(false);
      }
      char b[2];            // send recived data to
      b[0]=(char) c;        // libtelnet library
      b[1]=0;               //
      telnet_recv(tc,b,1);  //
   }
   return(true);
}


TelCli::~TelCli(){
   CONS->printf("TelSrv: ~TelCli 1\n"); 
   telnet_free(tc);
   CONS->printf("TelSrv: ~TelCli 2\n"); 
   if (fd.available()){
      fd.stop();
   }
}

//#######################################################################
//#######################################################################
//#######################################################################

void TelSrv::setup(){
}

void TelSrv::begin(){

}

void TelSrv::loop(){
   uint8_t i;

   if (srv!=NULL){
      if (srv->hasClient()){
         CONS->printf("TelSrv: new Client\n"); 
         for(i = 0; i < TELSRV_MAX_CLIENTS; i++){
            if (serverClients[i]==NULL){
               CONS->printf("TelSrv: new Client stored at %d\n",i); 
               serverClients[i]=new TelCli(Controller,srv->available());
               break;
            }
         }
      }

     // WiFiClient serverClient = srv->available();
     // // send message "no connections avalilable"
     // serverClient.stop();

      for(i = 0; i < TELSRV_MAX_CLIENTS; i++){
         if (serverClients[i]!=NULL){
            if (!serverClients[i]->handleClient()){
               CONS->printf("TelSrv: delete Client at %d\n",i); 
               delete(serverClients[i]);
               serverClients[i]=NULL;
            }
         }
      }
   } 
}

void TelSrv::startListener(){
   memset(serverClients,'\0',sizeof(serverClients));
   if (srv==NULL){
      srv=new WiFiServer(23);
   }
   if (srv!=NULL){
      srv->begin();
   }
}

void TelSrv::stopListener(){
   CONS->printf("TelSrv: stopListener start\n");
   if (srv!=NULL){
      int i;
      for(i = 0; i < TELSRV_MAX_CLIENTS; i++){
         if (serverClients[i]!=NULL){
            delete(serverClients[i]);
            memset(serverClients,'\0',sizeof(serverClients));
         }
      }
      srv->stop();
      delete(srv);
      srv=NULL;
   }
   CONS->printf("TelSrv: stopListener end\n");
}
void TelSrv::handleWiFiEvent(WiFiEvent_t e){
   switch(e) {
      case WIFI_EVENT_STAMODE_GOT_IP:
         CONS->printf("TelSrv: begin\n");
         startListener();
         break;
      case WIFI_EVENT_STAMODE_DISCONNECTED:
         CONS->printf("TelSrv: disconnect\n");
         stopListener();
         break;
   }
}

int TelSrv::command(Session *session,Print *cli,char **args,int argn){
   if (!strcmp(args[0],"show") && argn==3 &&
            !strcmp(args[1],PackName.c_str())){
      if (!strcmp(args[2],"clients")){
         for(int i=0;i<TELSRV_MAX_CLIENTS;i++){
            if (serverClients[i]!=NULL){
               cli->printf("P%02d IP:%s\n",i,
                           serverClients[i]->session.ipaddr.c_str());
            }
         }
         return(CMD_OK);
      }
      else{
         return(CMD_SYNTAX);
      }
   }
   else if (!strcmp(args[0],"help") &&
       ( (argn==1) ||
         (argn==2 && !strcmp(args[1],PackName.c_str())))){
      cli->printf("\n%s:\n",PackName.c_str());
      cli->printf("quit\n");
      cli->printf("    Terminates the telnet session.\n");
      cli->printf("\n","");
      cli->printf("exit\n");
      cli->printf("    Terminates the telnet session.\n");
      cli->printf("\n","");
      cli->printf("show %s clients\n",PackName.c_str());
      cli->printf("    Lists the current conntected telnet clients on\n");
      cli->printf("    ThiN device.\n");
      cli->printf("\n","");
      return(CMD_PART);
   }
   return(0);
};

//#######################################################################
//#######################################################################
//#######################################################################


