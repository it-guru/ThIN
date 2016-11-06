#!/bin/bash

if [ ! -d ./packlib ]; then
   echo "recreateProgMem.sh needs to be called from ThIN root dir" >&2
   exit 1
fi

for dir in packlib/*/progCont; do
(
cd $dir
cd ..
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
   echo "    [&](Session &session,ESP8266WebServer *s,String &p)->bool{" >>${CDIR}.cpp

   if [[ $f =~ \.css$ ]]; then
      echo "now css"
      java -jar ../../contrib/yui/yuicompressor-2.4.2.jar  \
           --type css $f > ${f}.tmp1
   elif [[ $f =~ \.js$ ]]; then
      java -jar ../../contrib/yui/yuicompressor-2.4.2.jar  \
           --type js  $f > ${f}.tmp1
   else 
      cat $f > ${f}.tmp1
   fi
   xxd -i  ${f}.tmp1  > ${CDIR}.tmp
   rm ${f}.tmp1
  
   sed -i -e "s/_tmp1\[\]//" \
       ${CDIR}.tmp
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

)
done



