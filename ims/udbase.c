/*
** cs154-2015 Project 5 ("p5ims") Instant Message Server
** udbase.c: for reading and writing the user database
*/

#include "ims.h"

/* your in-memory representation of user database can be pointed to by some
   global variables, declared in ims.h, defined here, and initialized by
   udbaseRead below.  When server is running with multiple threads, access to
   these globals should be guarded by a mutex. */

int udbaseRead(impEm *iem) {
  static const char me[]="udbaseRead";
  if (!filer) {
    impEmAdd(iem, "%s: couldn't open \"%s\" for reading: %s",
             me, udbaseFilename, strerror(errno));
    return 1;
  }

  /* ... YOUR CODE HERE to read database file into memory. Assuming
     that this is called before the server starts taking connections
     from clients, this does not need to be thread-safe. */
  udbase_init();
  return 0;
}

/* you can pass a NULL iem to this if you aren't interested in saving the
   error messages; impEmAdd will have no effect with a NULL iem */
int udbaseWrite(impEm *iem) {
  static const char me[]="udbaseWrite";

    filew = fopen(udbaseFilename, "w");
  /* ... make sure that user database is being written at the same
     that a client thread is modifying it, either with code here,
     or with limits on how udbaseWrite() is called */
  if (!filew) {
    impEmAdd(iem, "%s: couldn't open \"%s\" for writing: %s",
             me, udbaseFilename, strerror(errno));
    return 1;
  }

  /* ... YOUR CODE HERE to write the in-memory database to file */
  udbase_print();
  fclose(filew);
  return 0;
}
