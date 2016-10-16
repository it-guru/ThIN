#ifndef _lib_WebSrv_h_
#define _lib_WebSrv_h_

#include "../../config.h"
#include "../../kernel/PackMaster.h"

#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <functional>
#include "../../include/DynHashTable.hpp"


typedef std::function<bool(ESP8266WebServer *s,String &p)> reqHandler;

typedef struct _reqHandlerRec{
   reqHandler f;
} reqHandlerRec;


class reqNode{
   protected:
   DynHashTable<reqHandlerRec *> nodeNS;
   reqNode                  *subNS=NULL;

   public:
   void on(char *p,reqHandler f);
   bool handle(ESP8266WebServer *s,String &p);
};

typedef struct _modObj{
   reqHandler action;
   const char**  mp;
} modObj;

typedef struct _modTreeRec{
   DynHashTable<struct _modTreeRec *>     rec;
   long modRecIndex=-1;
} modTreeRec;





class WebSrv : public Pack {
   protected:
   DynHashTable<modObj *>  modReg;
   modTreeRec              modTree;
   reqNode *ns;
   void addTreeLevel(String &js,int mLevel,modTreeRec *b);

   public:
   ESP8266WebServer *srv=NULL;
   WebSocketsServer *wss=NULL;
   WebSrv(){
      srv=new ESP8266WebServer(80);
      wss=new WebSocketsServer(81);
      ns=new reqNode();
   };
   virtual void setup();
   virtual void loop();
   virtual void handleWiFiEvent(WiFiEvent_t e);
   virtual void handleSystemEvent(SysEvent *e,const char *source);


   void pageInfo();
   bool redirectToIndex(ESP8266WebServer *s,String &p);
   bool json_WebSrvStatus(ESP8266WebServer *s,String &p);


   void onRequest();
   bool onRestRequest(ESP8266WebServer *s,String &p);
   void progCont();
   void regNS(char *p,reqHandler f);
   void regMod(char *p,reqHandler action=NULL,const char** mp=NULL );
   bool doFwdRequest(String url);   

   bool sendActionScript(ESP8266WebServer *s,String &p);
   bool sendMenuScript(ESP8266WebServer *s,String &p);
   bool logonHandler(ESP8266WebServer *s,String &p);
   bool logoutHandler(ESP8266WebServer *s,String &p);


   protected:
   String  TestBaseURL;

   public:
   void setTestBaseURL(String p){
      TestBaseURL=p;
   };
   void setTestNS(char *p){
      String fwd;
      fwd=TestBaseURL;
      fwd+=p;
      CONS->printf("setTestNS fwd=%s\n",fwd.c_str());
      this->regNS(p,[&,fwd](ESP8266WebServer *s,String &p)->bool{
         CONS->printf("in regNS fwd=%s\n",fwd.c_str());
         return(this->doFwdRequest(fwd));
      });
   };
   
};

#endif

