#include "./pack.h"

extern "C" {
#include<user_interface.h>
}


void WebSrv::pageInfo(){
   String p="";
   int c;
  
   p+="<html>";
   p+="<body>";
   p+="<h1>OK, you are connected - first step is done</h1>";
   p+="<b>Stage1:</b><br>";
   p+="Header:<br>";
   for(c=0;c<srv->headers();c++){
      p+=srv->headerName(c);
      p+=":<br>";
      p+=srv->header(c);
      p+="<br><hr>";
   }
  
   p+="Time:";
   p+=millis();
   p+="<br><hr>";
  
   p+="URI:";
   p+=srv->uri();
   p+="<br><hr>";
  
  
   p+="HostHeader:";
   p+=srv->hostHeader();
   p+="<br><hr>";
  
   p+="</body>";
   p+="</html>";
   srv->send(200,"text/html",p);
}


bool WebSrv::redirectToIndex(Session &session,ESP8266WebServer *s,String &p){
   String d="";
   int c;
  
   d+="<html>";
   d+="<head>";
   d+="<meta http-equiv=\"refresh\" content=\"0; URL=/html/index.html\">";
   d+="</head>";
   d+="<body>";
   d+="</body>";
   d+="</html>";
   s->send(200,"text/html",d);
   return(true);
}


bool WebSrv::json_WebSrvStatus(Session &session,ESP8266WebServer *s,String &p){
  DynamicJsonBuffer jsonBuffer;
  JsonObject& r = jsonBuffer.createObject();
  r["exitcode"]="0";
  r["exitmsg"]="OK";
  r["time"]=millis();
  String callback=s->arg("callback");

  JsonObject& data = r.createNestedObject("data");

  if (session.authLevel>=100){
     data["SketchSize"]=String(ESP.getSketchSize(),DEC);
    
     data["FreeSpace"]=String(ESP.getFreeSketchSpace(),DEC);
    
     data["ChipId"]=String(ESP.getChipId());
    
     data["FlashChipId"]=String(ESP.getFlashChipId());
    
     data["FreeHeap"]=String(ESP.getFreeHeap());
  }

  data["UpTime"]=String(Controller->Uptime.getSeconds(),DEC);

  data["Load"]=String((long) Controller->load(),DEC);

  data["Hostname"]=String(Controller->hostname());

  data["sessionUser"]=curWebSession->session.user;

  data["sessionAuthLevel"]=curWebSession->session.authLevel;

//  if (WiFiIsConnected){
     data["WiFi"]=String("connected");
     data["SSID"]=String(WiFi.SSID());
     data["IPAddress"]=String(IPAddress(WiFi.localIP()).toString());
//  }
////  else{
//     data["WiFi"]=String("disconnected");
//  }

  String dstr;
  r.prettyPrintTo(dstr);
  s->send(200, "application/javascript",callback+"("+dstr+");");
  return(true);
}

String WebSrv::getCurrentSessionID(){
   String sessionid="";
   if (srv->hasHeader("Cookie")){   
      int p;
      String cookie = srv->header("Cookie");
      //CONS->printf("Found cookie: '%s'\n",cookie.c_str());
      String sessionvarname(SESSIONVARNAME);
      sessionvarname+="=";
      if ((p=cookie.indexOf(sessionvarname)) != -1) { 
         p+=sessionvarname.length();
         sessionid=cookie.substring(p);
      }
   }
   return(sessionid);
}


void WebSrv::onRequest(){
   String path = srv->uri();
   bool   isHandled=false;

   String sessionid=this->getCurrentSessionID();
   CONS->printf("WebSrv: request: '%s' session='%s'\n",
                path.c_str(),sessionid.c_str());

   WebSession *wsession=pWebSessions;
   do{
      if (wsession->key==sessionid){
         break;
      }
      wsession=wsession->pNext;
   }while(wsession!=NULL);

   if (sessionid!="" && (wsession==NULL || wsession==pWebSessions)){
      CONS->printf("invalid session key '%s' found\n",sessionid.c_str());
      String header = "HTTP/1.1 302 OK\r\n";
      header+="Location: "+path+"\r\n";
      header+="Cache-Control: no-cache\r\n";
      header+="Set-Cookie: ";
      header+=SESSIONVARNAME;
      header+="=;path=/; expires=Thu, 01 Jan 1970 00:00:00 GMT\r\n";
      header+="Content-type: text/html\r\n";
      header+="\r\n";
      srv->sendContent(header);
      return;
   }
   if (wsession==NULL){
      //CONS->printf("WebSrv: curWebSession=anonymous\n");
      curWebSession=pWebSessions;
   }
   else{
      curWebSession=wsession;
      //CONS->printf("WebSrv: curWebSession key='%s'\n",wsession->key.c_str());
      curWebSession->lastUseTime=Controller->Uptime.getSeconds();
   }

   isHandled=ns->handle(curWebSession->session,srv,path);
   if (!isHandled){
      String message = "File Not Found\n\n";
      message += "URI: ";
      message += srv->uri();
      message += "\nMethod: ";
      message += (srv->method() == HTTP_GET)?"GET":"POST";
      message += "\nArguments: ";
      message += srv->args();
      message += "\n";
      for (uint8_t i=0; i<srv->args(); i++){
        message += " " + srv->argName(i) + ": " + srv->arg(i) + "\n";
      }
      srv->send(404,"text/plain",message);
   }
}

bool WebSrv::logonHandler(Session &session,ESP8266WebServer *s,String &p){
   String header = "HTTP/1.1 200 OK\r\n";
   //header+="Location: /js/sys/logon\r\n";
   header+="Cache-Control: no-cache\r\n";

   String d="";

   String username=srv->arg("username");
   String password=srv->arg("password");

   long   uid=-1;
   int8_t authLevel=-1;

   WebSession *wsession=NULL;
   String  sessionkey("");
   bool foundUser=false;

   if (Controller->auth!=NULL){
      foundUser=Controller->auth->authUser(username,password,&uid,&authLevel);
   }
   if (foundUser){
      wsession=pWebSessions;
      WebSession *pPre;
      while(wsession->pNext!=NULL){
         pPre=wsession;
         wsession=wsession->pNext;
         if (wsession!=NULL){
            if (wsession->lastUseTime<Controller->Uptime.getSeconds()-60){
               WebSession *pNext=wsession->pNext;
               delete(wsession);
               pPre->pNext=pNext;
               wsession=pPre;
            }
         }
      }
      wsession->pNext=new WebSession();
      wsession=wsession->pNext;
      sessionkey=username;
      sessionkey+="+";
      sessionkey+=srv->client().remoteIP().toString();
      sessionkey+="+";
      sessionkey+=String(millis(),DEC);
      sessionkey=md5sum(sessionkey);

      wsession->session.user=username;
      wsession->session.proto="web";
      wsession->session.uid=uid;
      wsession->key=sessionkey;
      wsession->lastUseTime=Controller->Uptime.getSeconds();
      wsession->session.authLevel=authLevel;
      wsession->session.ipaddr=srv->client().remoteIP().toString();
      header+="Set-Cookie: ";
      header+=SESSIONVARNAME;
      header+="="+sessionkey+";Path=/\r\n";
   }

   if (srv->method() == HTTP_POST){  // logon via ajax
      header+="Content-type: application/json\r\n";
      header+="\r\n";
      srv->sendContent(header);

      DynamicJsonBuffer json;
      JsonObject& root = json.createObject();      
      if (wsession!=NULL){
         root["exitcode"]=0;
         root["exitmsg"]="Logon OK";
      }
      else{
         root["exitcode"]=1;
         root["exitmsg"]="ERROR: invalid user or password";
      }

      String dstr;
      root.prettyPrintTo(dstr);
      srv->sendContent(dstr); 
   }
   else{
      header+="Content-type: text/html\r\n";
      header+="\r\n";
      srv->sendContent(header);
      d+="<html>";
      d+="<body>";
      d+="<h1>Logon</h1>";
      d+="</body>";
      d+="</html>";
      srv->sendContent(d);
   }
   return(true);
}


bool WebSrv::logoffHandler(Session &session,ESP8266WebServer *s,String &p){
   String header = "HTTP/1.1 200 OK\r\n";
   header+="Cache-Control: no-cache\r\n";

   String d="";
   String sessionid=this->getCurrentSessionID();

   if (sessionid!=""){
      header+="Set-Cookie: ";
      header+=SESSIONVARNAME;
      header+="=;path=/; expires=Thu, 01 Jan 1970 00:00:00 GMT\r\n";
      WebSession *wsession=pWebSessions;
      WebSession *pPre;
      while(wsession->pNext!=NULL){
         pPre=wsession;
         wsession=wsession->pNext;
         if (wsession!=NULL){
            if (wsession->key==sessionid){
               CONS->printf("logout sesion '%s'\n",sessionid.c_str());
               WebSession *pNext=wsession->pNext;
               delete(wsession);
               pPre->pNext=pNext;
               wsession=pPre;
            }
         }
      }
   }

   if (srv->method() == HTTP_POST){  // logoff via ajax
      header+="Content-type: application/json\r\n";
      header+="\r\n";
      srv->sendContent(header);

      DynamicJsonBuffer json;
      JsonObject& root = json.createObject();      
      root["exitcode"]=0;
      root["exitmsg"]="Logoff OK";

      String dstr;
      root.prettyPrintTo(dstr);
      srv->sendContent(dstr); 
   }
   else{
      header+="Content-type: text/html\r\n";
      header+="\r\n";
      srv->sendContent(header);
      d+="<html>";
      d+="<body>";
      d+="<h1>Logoff</h1>";
      d+="</body>";
      d+="</html>";
      srv->sendContent(d);
   }
   return(true);

}


bool WebSrv::onRestRequest(Session &session,ESP8266WebServer *s,String &p){
   CONS->printf("WebSrv: got rest request:'%s'\n",p.c_str());
   String line=p;
   if (line.startsWith("set/")){
      String subdev=line.substring(4);
      line=subdev.substring(subdev.indexOf("/"));
      subdev=subdev.substring(0,subdev.indexOf("/"));
      CONS->printf("WebSrv: set on subdev:'%s'\n",subdev.c_str());
      String param=line.substring(line.indexOf("/")+1);

      DynamicJsonBuffer json;
      JsonObject& root = json.createObject();      
      root["exitcode"]=1;
      root["exitmsg"]="unknown error";


      long devpos=Controller->getDevicePos(subdev.c_str());
      if (devpos!=-1){
         const char *devName=Controller->getDeviceName(devpos);
         SysDeviceType devType=Controller->getDeviceType(devpos);
         SysEvent e;
         e.type=SYS_EVENT_DEVREQCHANGE;
         if (devType==DOT || devType==DIO){   // io or output 
            e.dev.devicepos=devpos;
            e.dev.D.state=0;
      CONS->printf("WebSrv: set on param:'%s'\n",param.c_str());
            if (param=="3"){       // trigger toggle operation
               e.dev.D.state=3;
            }
            else if (param!="0"){  // all not 0 = 1
               e.dev.D.state=1;
            }
            CONS->printf("Rest: postSystemEvent Digital new ='%d'\n",
                         e.dev.D.state);
            root["exitcode"]=101;
            root["exitmsg"]="request posted but no response";
            Controller->postSystemEvent(&e,PackName.c_str());
            if (e.dev.cnt>0){
               root["exitcode"]=0;
               root["exitmsg"]="OK";
            }
         }
         else{
            root["exitcode"]=10;
            root["exitmsg"]="device definition error";
         }
      }
      else{
         root["exitcode"]=100;
         root["exitmsg"]="request on unknown device";
      }




      String res;
      root.prettyPrintTo(res); // todo - hier muss JSON Code geschrieben werden
      s->send(200,"text/javascript",res);
   }
   if (line.startsWith("query/")){
   }
   if (line.startsWith("save")){
   }
   return(true);
}


void WebSrv::regNS(char *p,reqHandler f)
{
   ns->on(p,f);
}

void WebSrv::regMod(char *p,reqHandler action,const char** mp,
                    int8_t minAuthLevel)
{
   modObj *newModuleHandler;
   newModuleHandler=new modObj();
   newModuleHandler->action=action;
   newModuleHandler->mp=mp;
   newModuleHandler->minAuthLevel=minAuthLevel;

   long modPos=modReg.set(p,newModuleHandler);
   if (newModuleHandler->mp!=NULL){
      modTreeRec *base=&modTree;
      for(int t=0;*(newModuleHandler->mp+t)!=NULL;t++){
         CONS->printf("regMod p=%s\n",*(newModuleHandler->mp+t));
         modTreeRec *pp=base->rec.get(*(newModuleHandler->mp+t));
         if (pp==NULL){
            modTreeRec *pnext=new modTreeRec();
            pnext->minAuthLevel=minAuthLevel;
            CONS->printf("regMod add indirect=%s in level t=%d minAuthLevel=%d\n",*(newModuleHandler->mp+t),t,pnext->minAuthLevel);
            base->rec.set(*(newModuleHandler->mp+t),pnext); 
            base=pnext;
         }
         else{
            base=pp;
         }
      }
      base->modRecIndex=modPos;
   }
}

void WebSrv::addTreeLevel(String &js,int mLevel,modTreeRec *b){
   for(long c=0;c<b->rec.length();c++){
      String o="";
      js+="if (App.session.authLevel>=";
      js+=String(b->minAuthLevel,DEC);
      js+="){";
      long modRecIndex=b->rec[c]->modRecIndex;
      if (modRecIndex!=-1){
         o=modReg.key(modRecIndex);
      }
      if (mLevel==0){
         js+="d+=App.addHeader('";
         js+=o.c_str();
         js+="','";
         js+=b->rec.key(c);
         js+="');\n";
      }
      if (mLevel==1){
         js+="d+=App.addEntry('";
         js+=o.c_str();
         js+="','";
         js+=b->rec.key(c);
         js+="');\n";
      }
      js+="}";
      Serial.printf("level=%d = %s \n",mLevel,b->rec.key(c));
      modTreeRec *r=b->rec[c];
      if (r!=NULL){
         addTreeLevel(js,mLevel+1,r); 
      } 
   }
}


bool WebSrv::sendMenuScript(Session &session,ESP8266WebServer *s,String &p){
   char buf[128];
   String js;
   strcpy_P(buf,PSTR("define([\"MenuTab\"],function(MenuTab){"));      js+=buf;
   strcpy_P(buf,PSTR("return({"));                                     js+=buf;
   strcpy_P(buf,PSTR("recreate:function(){"));                         js+=buf;
   strcpy_P(buf,PSTR("var d='';\n"));                                  js+=buf; 
   int mLevel=0;
   addTreeLevel(js,mLevel,&modTree);
   strcpy_P(buf,PSTR("return(d);\n"));                                 js+=buf;
   strcpy_P(buf,PSTR("}"));                                            js+=buf;
   strcpy_P(buf,PSTR("});"));                                          js+=buf; 
   strcpy_P(buf,PSTR("});"));                                          js+=buf;
   s->send(200,"text/javascript",js);                                 

   return(true);
}

bool WebSrv::sendActionScript(Session &session,ESP8266WebServer *s,String &p){

   String mod=p;
   if (mod.endsWith(".js")){
      mod=mod.substring(0,mod.length()-3);   
      long modPos=modReg.getIndex((char *)mod.c_str());
      CONS->printf("sendActionScript for mod '%s' p=%ld\n",mod.c_str(),modPos);
      if (modPos!=-1){
         modObj *modObj=modReg[modPos];
         if (modObj->action!=NULL){
            reqHandler f;
            f=modObj->action;
            return(f(session,s,p));
         } 
      }
   }
   return(false);
}



void WebSrv::setup(){
   if (wss!=NULL){
      wss->onEvent([&](uint8_t num,WStype_t type,uint8_t * payload,size_t sz){
         switch(type){
            case WStype_TEXT:
               CONS->printf("msg get:%s\n",payload);
               String line((char *)payload);
               if (line.startsWith("query:")){
                  String subdev=line.substring(6);
                  CONS->printf("query subdev='%s'\n",subdev.c_str());
                  long qdevpos=-1;
                  long devpos=-1;
                  if (subdev!="ALL"){
                     qdevpos=Controller->getDevicePos(subdev.c_str());
                  }
                  for(devpos=0;devpos<Controller->getDeviceCount();devpos++){
                     if (qdevpos==-1 || qdevpos==devpos){
                        const char *devName=Controller->getDeviceName(devpos);
                        SysDeviceType devType=Controller->getDeviceType(devpos);
                        SysEvent e;
                        e.type=SYS_EVENT_DEVQUERY;
                        e.dev.devicepos=devpos;
                        Controller->postSystemEvent(&e,PackName.c_str());
                        if (e.dev.cnt>0){
                           if (devType==DIO || devType==DOT || devType==DIN){
                              String msg="state:C:";
                              msg+=devName;
                              msg+=":";
                              msg+=String(e.dev.D.state, DEC);
                              msg+="\n";
                              wss->sendTXT(num,msg);
                           }
                        }
                     }
                  }
               }

            break; 
         }
      });
   }
   if (srv!=NULL){
      this->regNS("/info.html",[&]
                  (Session &session,ESP8266WebServer *s,String &p)->bool{
         this->pageInfo();
         return(true);
      });
      this->regNS("/index.html",[&]
                  (Session &session,ESP8266WebServer *s,String &p)->bool{
         return(this->redirectToIndex(session,s,p));
      });
      this->regNS("/js/REST/",[&]
                  (Session &session,ESP8266WebServer *s,String &p)->bool{
         return(this->onRestRequest(session,s,p));
      });
      this->regNS("/js/rest/",[&]
                  (Session &session,ESP8266WebServer *s,String &p)->bool{
         return(this->onRestRequest(session,s,p));
      });
      this->regNS("/js/json/WebSrvStatus",[&]
                  (Session &session,ESP8266WebServer *s,String &p)->bool{
         return(this->json_WebSrvStatus(session,s,p));
      });
      this->regNS("/js/MenuTab.js",[&]
                  (Session &session,ESP8266WebServer *s,String &p)->bool{
         return(this->sendMenuScript(session,s,p));
      });
      this->regNS("/jssys/logon",[&]
                  (Session &session,ESP8266WebServer *s,String &p)->bool{
         return(this->logonHandler(session,s,p));
      });
      this->regNS("/jssys/logoff",[&]
                  (Session &session,ESP8266WebServer *s,String &p)->bool{
         return(this->logoffHandler(session,s,p));
      });
      this->regNS("/jsact/handler/",[&]
                  (Session &session,ESP8266WebServer *s,String &p)->bool{
         return(this->sendActionScript(session,s,p));
      });
      progCont();
      this->regNS("/",[&]
                  (Session &session,ESP8266WebServer *s,String &p)->bool{
         return(this->redirectToIndex(session,s,p));
      });

      srv->onNotFound([&](){ this->onRequest(); });
      const char * headerkeys[] = {"User-Agent","Cookie"} ;
      size_t headerkeyssize = sizeof(headerkeys)/sizeof(char*);
      srv->collectHeaders(headerkeys, headerkeyssize );
   }
}

void WebSrv::loop(){
   if (srv!=NULL){
      srv->handleClient();
   } 
   if (wss!=NULL){
      wss->loop();
   }
}

void WebSrv::handleWiFiEvent(WiFiEvent_t e){
   switch(e) {
      case WIFI_EVENT_STAMODE_GOT_IP:
         CONS->printf("start WebServer\n");
         srv->begin();
         wss->begin();
         break;
      case WIFI_EVENT_STAMODE_DISCONNECTED:
         CONS->printf("disconnect WebServer\n");
         srv->stop();
         wss->disconnect();
         break;
   }
}

void WebSrv::handleSystemEvent(SysEvent *e,const char *source){
   char *src="unkonwn";
   if (src!=NULL){
      src=(char *)source;
   }
   switch(e->type) {
      case SYS_EVENT_DEVLOCCHANGE:
         if (wss!=NULL){
            long devpos=e->dev.devicepos;


            const char *devName=Controller->getDeviceName(devpos);
            SysDeviceType devType=Controller->getDeviceType(devpos);
            CONS->printf("handleSystemEvent:devpos=%ld devname=%s devtype=%d\n",
                         devpos,devName,devType);
            if (devType==DIO || devType==DIN || devType==DOT){
               String msg="state:C:";
               msg+=devName;
               msg+=":";
               msg+=String(e->dev.D.state, DEC);
               CONS->printf("WebSocket Server broadcast '%s'\n",msg.c_str());
               msg+="\n";
               wss->broadcastTXT(msg);
            }
         }
         break;
   }
}


void reqNode::on(char *p,reqHandler f){
   reqHandlerRec *r=new reqHandlerRec();
   r->f=f;
   nodeNS.set(p,r);
}

bool reqNode::handle(Session &session,ESP8266WebServer *s,String &p){
   for(long c=0;c<nodeNS.length();c++){
      char *ns=nodeNS.key(c);
      if (strlen(ns)>1 && ns[strlen(ns)-1]=='/'){
         if (p.startsWith(ns)){
            p=p.substring(strlen(ns));
            return(nodeNS[c]->f(session,s,p));
         }
      }
      else{
         if (!strcmp(p.c_str(),ns)){
            return(nodeNS[c]->f(session,s,p));
         }
      }
   }
   return(false);
}

