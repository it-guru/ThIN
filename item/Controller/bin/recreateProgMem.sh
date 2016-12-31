#!/bin/bash

if [ ! -d ./packlib ]; then
   echo "recreateProgMem.sh needs to be called from ThIN root dir" >&2
   exit 1
fi

for dir in packlib/*; do
(
echo "Prozessing dir='$dir'"
cd $dir
if [ -d progCont ]; then
   PACK=$(basename `pwd`)
   CDIR="progCont"
echo "PACK=$PACK CDIR=$CDIR"

   cat <<EOF > ${CDIR}.cpp
#include "./pack.h"
#include "./${CDIR}.hpp"

#ifdef packlib_WebSrv

void ${PACK}::${CDIR}(){
EOF
   echo "" > ${CDIR}.hpp
   echo "" > ${CDIR}.h



   find ${CDIR} -name \*.js -o -name \*.html -o -name \*.css -o \
                -name \*.gif -o -name \*.svg | while read f; do
      F=$(echo $f | sed -e "s/^${CDIR}//");
      D=$(echo "${PACK}_${F}" | tr '[a-z]' '[A-Z'] | \
          sed -e 's/^\///' | sed -e 's/[-\/\.]/_/g')
      DD=$(basename $(dirname $f))
      

      echo "f=$f  F=$F D=$D dirname=$DD"
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
      echo "PGM_P PSTR_${D}_DATA(){return(${D}_DATA);}" >> ${CDIR}.hpp
      echo "PGM_P PSTR_${D}_DATA();" >> ${CDIR}.h
      echo "extern unsigned int ${D}_LENGTH;" >> ${CDIR}.h
      rm ${CDIR}.tmp
      if [ "$DD" != "jsact" ]; then
         echo "this->regNS(\"$F\"," >> ${CDIR}.cpp
         echo "    [&](Session &session,ESP8266WebServer *s,String &p)->bool{" >>${CDIR}.cpp

         if [[ $f =~ \.css$ ]]; then
            echo "   s->send_P(200,PSTR(\"text/css\"),${D}_DATA,${D}_LENGTH);" \
                 >>${CDIR}.cpp
         fi
         if [[ $f =~ \.html$ ]]; then
            echo "s->send_P(200,PSTR(\"text/html\"),${D}_DATA,${D}_LENGTH);" \
                 >>${CDIR}.cpp
         fi
         if [[ $f =~ \.js$ ]]; then
            echo "s->send_P(200,PSTR(\"text/javascript\"),${D}_DATA,${D}_LENGTH);" \
                 >>${CDIR}.cpp
         fi
         if [[ $f =~ \.gif$ ]]; then
            echo "s->send_P(200,PSTR(\"image/gif\"),${D}_DATA,${D}_LENGTH);" \
                 >>${CDIR}.cpp
         fi
         if [[ $f =~ \.svg$ ]]; then
            echo "s->send_P(200,PSTR(\"image/svg+xml\"),${D}_DATA,${D}_LENGTH);" \
                 >>${CDIR}.cpp
         fi

         echo "  return(true);" >>${CDIR}.cpp
         echo "});" >>${CDIR}.cpp
      fi
   done


   cat <<EOF >> ${CDIR}.cpp
}
#endif
EOF
fi

)
done



