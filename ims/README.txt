Instant Message Server

This directory should contain the following:

ims.h: Start of header file for the IM server, with declarations of the
  functions which are defined in the constituent source files.  Note how the
  source file is indicated (in a comment) above the declarations of the
  symbols defined in the source file.

globals.c: Some global variables.

udbase.c, user.c: Start of implementation of functions to read and write the user
  database; lots for you to fill in here.

op.c: server operations as it receives the corresponding protocol messages from 
clients

basic.c: Start of implementation of functions to start and stop server; lots
  for you to fill in here.

main.c: Start of code for main() function of the "ims" server, which you will
  complete for this project.  This code uses getopt() for handling the
  command-line options and setting some global variables based on these.

imp.tar, imp/: Header and library for protocol strings and protocol messages
   * imp/imp.h: the IM protocol header file
   * imp/libimp.so: a Linux shared library
  ****************************************************************************
  *** By default svn thinks that .so files can be ignored, so you may not  ***
  *** actually have imp/libimp.so. So, RIGHT NOW try: "make". If you get:  ***
  ***   ...                                                                ***
  ***   /usr/bin/ld: cannot find -limp                                     ***
  ***   collect2: ld returned 1 exit status                                ***
  ***   make: *** [ims] Error 1                                            ***
  *** Then you know that imp/libimp.so is missing.  So run:                ***
  *** "tar xvf imp.tar",  verify that you now have imp/libimp.so,          ***
  *** and try "make" again.                                                ***
  ****************************************************************************

Makefile.

rims: Reference IM server implementation. Example usage:
     cp db-example.txt db.txt
     ./rims -p 15400 -d db.txt
  and then in another shell:
     ./txtimc -s localhost -p 15400

db-empty.txt: A user database file with no registered users

db-example.txt: An example non-empty user database file

txtimc, txtimc.c: A very simple text-based IM client (Linux executable
txtimc) and its single source file txtimc.c.  Study this to see examples
of the imp library in use.

cndb, cndb.c: An executable (cndb) and its single source file (cndb.c).  This
utility has two purposes.  Supplied with one filename as its command-line
argument, this prints to stdout a canonical (sorted) ordering of users and
their friends.  Supplied with two filenames, it sorts and then compares them,
and reports any differences to stdout.  In both cases, the sorting is done
in-memory; the files on disk are untouched.

README.txt: This file
