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

struct _WebSession;

typedef struct _WebSession{
   struct _WebSession *pNext=NULL; 
   Session            session;
   String             key;
   unsigned long      lastUseTime;
} WebSession;



class WebSrv : public Pack {
   protected:
   DynHashTable<modObj *>  modReg;
   modTreeRec              modTree;
   reqNode *ns;
   WebSession *pWebSessions;
   WebSession *curWebSession;

   void addTreeLevel(String &js,int mLevel,modTreeRec *b);

   public:
   ESP8266WebServer *srv=NULL;
   WebSocketsServer *wss=NULL;
   WebSrv(){
      srv=new ESP8266WebServer(80);
      wss=new WebSocketsServer(81);
      ns=new reqNode();
      pWebSessions=new WebSession();
      pWebSessions->session.user="anonymous";
      pWebSessions->session.proto="web";
      pWebSessions->session.ipaddr="0.0.0.0";
      pWebSessions->key="";
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
   bool logoffHandler(ESP8266WebServer *s,String &p);


   protected:

   public:
};

#endif

