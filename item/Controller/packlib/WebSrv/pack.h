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

#define SESSIONVARNAME "THINSESSIONID"

typedef std::function<bool(Session &session,ESP8266WebServer *srv,String &p)> 
reqHandler;

typedef struct _reqHandlerRec{
   reqHandler f;
} reqHandlerRec;


class reqNode{
   protected:
   DynHashTable<reqHandlerRec *> nodeNS;
   reqNode                  *subNS=NULL;

   public:
   void on(char *p,reqHandler f);
   bool handle(Session &session,ESP8266WebServer *s,String &p);
};

typedef struct _modObj{
   reqHandler action;
   const char**  mp;
   int8_t        minAuthLevel;
} modObj;

typedef struct _modTreeRec{
   DynHashTable<struct _modTreeRec *>     rec;
   long modRecIndex=-1;
   int8_t        minAuthLevel;
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
   #ifdef packlib_WebSrv
   void progCont();
   #endif

   public:
   ESP8266WebServer *srv=NULL;
   WebSocketsServer *wss=NULL;
   WebSrv(){
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
   bool redirectToIndex(Session &session,ESP8266WebServer *s,String &p);
   bool json_WebSrvStatus(Session &session,ESP8266WebServer *s,String &p);
   String getCurrentSessionID();


   void onRequest();
   bool onRestRequest(Session &session,ESP8266WebServer *s,String &p);
   void regNS(char *p,reqHandler f);
   void regMod(char *p,reqHandler action=NULL,const char** mp=NULL ,
               int8_t minAuthLevel=0);
   bool doFwdRequest(String url);   

   bool sendActionScript(Session &session,ESP8266WebServer *s,String &p);
   bool sendMenuScript(Session &session,ESP8266WebServer *s,String &p);
   bool logonHandler(Session &session,ESP8266WebServer *s,String &p);
   bool logoffHandler(Session &session,ESP8266WebServer *s,String &p);


   protected:

   public:
};

#endif

