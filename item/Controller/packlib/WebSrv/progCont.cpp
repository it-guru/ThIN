#include "./pack.h"
#include "./progCont.hpp"

void WebSrv::progCont(){
this->regNS("/html/index.html",
            [&](Session &session,ESP8266WebServer *s,String &p)->bool{
   srv->send_P(200,PSTR("text/html"),HTML_INDEX_HTML_DATA,HTML_INDEX_HTML_LENGTH);
  return(true);
});
this->regNS("/img/ThIN.gif",
            [&](Session &session,ESP8266WebServer *s,String &p)->bool{
   srv->send_P(200,PSTR("image/gif"),IMG_THIN_GIF_DATA,IMG_THIN_GIF_LENGTH);
  return(true);
});
this->regNS("/img/ThIN-mini.gif",
            [&](Session &session,ESP8266WebServer *s,String &p)->bool{
   srv->send_P(200,PSTR("image/gif"),IMG_THIN_MINI_GIF_DATA,IMG_THIN_MINI_GIF_LENGTH);
  return(true);
});
this->regNS("/img/background.svg",
            [&](Session &session,ESP8266WebServer *s,String &p)->bool{
   srv->send_P(200,PSTR("image/svg+xml"),IMG_BACKGROUND_SVG_DATA,IMG_BACKGROUND_SVG_LENGTH);
  return(true);
});
this->regNS("/js/require.js",
            [&](Session &session,ESP8266WebServer *s,String &p)->bool{
   srv->send_P(200,PSTR("text/javascript"),JS_REQUIRE_JS_DATA,JS_REQUIRE_JS_LENGTH);
  return(true);
});
this->regNS("/js/spin.js",
            [&](Session &session,ESP8266WebServer *s,String &p)->bool{
   srv->send_P(200,PSTR("text/javascript"),JS_SPIN_JS_DATA,JS_SPIN_JS_LENGTH);
  return(true);
});
this->regNS("/js/jquery.min.js",
            [&](Session &session,ESP8266WebServer *s,String &p)->bool{
   srv->send_P(200,PSTR("text/javascript"),JS_JQUERY_MIN_JS_DATA,JS_JQUERY_MIN_JS_LENGTH);
  return(true);
});

}
