#include "ims.h"
#include "users.h"

int main(int argc, char *argv[]) {

  const char *me = argv[0];
    impEm* iem = impEmNew();
    udbaseFilename = strdup(argv[1]);
  if (udbaseRead(iem)) {
    impEmAdd(iem, "%s: failed to read database file \"%s\"",
             me, udbaseFilename);
    return 1;
  }
  printf("==============checking===============\n");
  /* immediately try writing database, so that any errors here can be
     reported as a failure of server start-up. Whether and how you do
     error handling for subsequent calls to udbaseWrite() is up to you */
     udbaseFilename = strdup(argv[2]);
  if (udbaseWrite(iem)) {
    impEmAdd(iem, "%s: failed to write database file \"%s\"",
             me, udbaseFilename);
    return 1;
  }
  
    return 0;
}