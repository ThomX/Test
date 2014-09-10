#!/bin/tcsh

#RJW 25/7/07
#this is the make file used at present to create the native library used by the GDA
#
# This does not work using gcc v4.1.2, but does work with v3.4.6
#
set AUXDIR = ../auxlib
set OBJECTS = "detcon_misc.o detcon_heartbeat.o detcon_hw.o detcon_server.o detcon_det.o detcon_sio.o detcon_entry.o detcon_par.o detcon_glob.o detcon_exec.o dummy.o ADSCController.o trnlog.o mdcclock.o dskhd.o check_environ.o timer_unix.o"
#
gcc -c -DLINUX -D_REENTRANT -O -I/usr/lib/jvm/java/include -I/usr/lib/jvm/java/include/linux -I../incl ADSCController.c
#
gcc -c -DLINUX -D_REENTRANT -O detcon_misc.c detcon_heartbeat.c\
 detcon_hw.c detcon_server.c detcon_det.c detcon_sio.c\
 detcon_entry.c detcon_par.c detcon_glob.c detcon_exec.c dummy.c
#
gcc -c -DLINUX -D_REENTRANT -O $AUXDIR/trnlog.c $AUXDIR/mdcclock.c\
 $AUXDIR/dskhd.c $AUXDIR/check_environ.c $AUXDIR/timer_unix.c
#
# Create the library in this directory and then it can be copied to where it is needed
#
gcc -shared -o libdetcon.so $OBJECTS -lpthread /usr/lib/libc.so
gcc -shared -o libdetcon_dls.so $OBJECTS -lpthread /usr/lib/libc.so
#
#rm ${OBJECTS}

