#!/bin/bash

PACK=$(basename `pwd`)
CDIR="progCont"


cat <<EOF > ${CDIR}.cpp
#include "./pack.h"
#include "./${CDIR}.hpp"

void ${PACK}::${CDIR}(){
EOF
echo "" > ${CDIR}.hpp



find ${CDIR} -name \*.js -o -name \*.html -o -name \*.css -o \
             -name \*.gif -o -name \*.svg | while read f; do
   F=$(echo $f | sed -e "s/^${CDIR}//");
   D=$(echo $F | tr '[a-z]' '[A-Z'] | sed -e 's/^\///' | sed -e 's/[-\/\.]/_/g')
   echo "f=$f  F=$F D=$D"
   echo "this->regNS(\"$F\"," >> ${CDIR}.cpp
   echo "            [&](Session &session,ESP8266WebServer *s,String &p)->bool{" >>${CDIR}.cpp
   xxd -i  $f  > ${CDIR}.tmp
   sed -i -e "s/unsigned int .*len =/unsigned int ${D}_LENGTH =/" \
       ${CDIR}.tmp
   sed -i -e "s/unsigned char .*=/const char ${D}_DATA\[\] PROGMEM = /g" \
       ${CDIR}.tmp
   cat ${CDIR}.tmp >> ${CDIR}.hpp
   rm ${CDIR}.tmp
   if [[ $f =~ \.css$ ]]; then
      echo "   srv->send_P(200,PSTR(\"text/css\"),${D}_DATA,${D}_LENGTH);" \
           >>${CDIR}.cpp
   fi
   if [[ $f =~ \.html$ ]]; then
      echo "   srv->send_P(200,PSTR(\"text/html\"),${D}_DATA,${D}_LENGTH);" \
           >>${CDIR}.cpp
   fi
   if [[ $f =~ \.js$ ]]; then
      echo "   srv->send_P(200,PSTR(\"text/javascript\"),${D}_DATA,${D}_LENGTH);" \
           >>${CDIR}.cpp
   fi
   if [[ $f =~ \.gif$ ]]; then
      echo "   srv->send_P(200,PSTR(\"image/gif\"),${D}_DATA,${D}_LENGTH);" \
           >>${CDIR}.cpp
   fi
   if [[ $f =~ \.svg$ ]]; then
      echo "   srv->send_P(200,PSTR(\"image/svg+xml\"),${D}_DATA,${D}_LENGTH);" \
           >>${CDIR}.cpp
   fi

   echo "  return(true);" >>${CDIR}.cpp
   echo "});" >>${CDIR}.cpp
done


cat <<EOF >> ${CDIR}.cpp

}
EOF



