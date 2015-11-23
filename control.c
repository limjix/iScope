#include "control.h"

//tar -cvf ../iScope107.tar *.cpp *.c *.h *.cfg *.script 

//WISH LIST
//@ garbage-collected memory (anti seg-fault reliability)
//@ maintain list of open libraries (speedup function calling)
//@ uncompressed mp4 support (video compatibility)
//
//MY TO DO LIST
//@ implement destructors for object types (avoid memory leaking)
//@ MPI-2 derived treatment of multiple address spaces in interpreter (parallel MOBILE)
//@ for/while/if/else standard library (Turing-completeness)
//@ log(n) octree node lookup, octree multigrid algorithm (unstructured MOBILE)
//
//JOHN'S TO DO LIST
//@ spectral graph
//@ hungarian algorithm
//@ graphical bayes
//@ stitching thread
//@ autofocus thread

int main(int argc,char *argv[])
{
  void *arglist;
  void *func;
  char initcfg[MAXLEN];
 
  if(argc<=1)
    sprintf(initcfg,"init.script");
  else
    sprintf(initcfg,"%s",argv[1]);

  //iheap(); //set heap to NULL

  // initialise xserver counter to zero
  dynamic_call("xserver","xserver_nullwindow",'s',NULL,&func);
  dynamic_wait(func,NULL);

  // run interpreter startup script
  arglist=NULL;
  dynamic_putarg("std.char","script",(void *) initcfg,SZ,&arglist);
  dynamic_call("interpret","interpret_code",'s',arglist,&func);
  dynamic_wait(func,NULL);
  dynamic_closeargs(arglist);
}

