#ifndef _lib_Cons_h_
#define _lib_Cons_h_

#include "../../kernel/PackMaster.h"

// Local default Defines (pre Config)

#include "../../config.h"

#define CONS_LED                1
#define CONS_SERIAL             2
#define CONS_TCP                4
#define CONS_SERIAL_AUTODISABLE 5  // use CONS_SERIAL in non flash mode GPIO0=0
#define CONS_NULL               0

int ets_vsprintf(char *str, const char *format, va_list arg);
int ets_vsnprintf(char *buffer, size_t sizeOfBuffer, size_t count, const char *format, va_list arg);



class Cons : public PackStdCons {
   protected:
   int curMode=0;
   int eol=1;
   boolean progMode=false;

   public:
   Cons(int t);
   virtual void setup();
   virtual void loop();
   virtual int  command(Session *session,Print *cli,char **args,int argn);


   // Interface for other Packages
   void set(int t);

   //virtual size_t write(const uint8_t *buffer, size_t size);
   virtual size_t write(uint8_t);

};

#endif

