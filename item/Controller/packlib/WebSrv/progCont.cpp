#include "./pack.h"
#include "./progCont.hpp"

#ifdef packlib_WebSrv

void WebSrv::progCont(){
this->regNS("/html/index.html",
    [&](Session &session,ESP8266WebServer *s,String &p)->bool{
s->send_P(200,PSTR("text/html"),WEBSRV__HTML_INDEX_HTML_DATA,WEBSRV__HTML_INDEX_HTML_LENGTH);
  return(true);
});
this->regNS("/img/ThIN.gif",
    [&](Session &session,ESP8266WebServer *s,String &p)->bool{
s->send_P(200,PSTR("image/gif"),WEBSRV__IMG_THIN_GIF_DATA,WEBSRV__IMG_THIN_GIF_LENGTH);
  return(true);
});
this->regNS("/img/ThIN-mini.gif",
    [&](Session &session,ESP8266WebServer *s,String &p)->bool{
s->send_P(200,PSTR("image/gif"),WEBSRV__IMG_THIN_MINI_GIF_DATA,WEBSRV__IMG_THIN_MINI_GIF_LENGTH);
  return(true);
});
this->regNS("/img/background.gif",
    [&](Session &session,ESP8266WebServer *s,String &p)->bool{
s->send_P(200,PSTR("image/gif"),WEBSRV__IMG_BACKGROUND_GIF_DATA,WEBSRV__IMG_BACKGROUND_GIF_LENGTH);
  return(true);
});
this->regNS("/js/require.js",
    [&](Session &session,ESP8266WebServer *s,String &p)->bool{
s->send_P(200,PSTR("text/javascript"),WEBSRV__JS_REQUIRE_JS_DATA,WEBSRV__JS_REQUIRE_JS_LENGTH);
  return(true);
});
this->regNS("/js/spin.js",
    [&](Session &session,ESP8266WebServer *s,String &p)->bool{
s->send_P(200,PSTR("text/javascript"),WEBSRV__JS_SPIN_JS_DATA,WEBSRV__JS_SPIN_JS_LENGTH);
  return(true);
});
this->regNS("/js/jquery.min.js",
    [&](Session &session,ESP8266WebServer *s,String &p)->bool{
s->send_P(200,PSTR("text/javascript"),WEBSRV__JS_JQUERY_MIN_JS_DATA,WEBSRV__JS_JQUERY_MIN_JS_LENGTH);
  return(true);
});
}
#endif
