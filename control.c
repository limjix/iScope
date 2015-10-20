#include "control.h"

//tar -cvf ../iScope93_john.tar *.c *.h *.cfg *.script 

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
  dynamic_putarg("std.void","xclient",NULL,SZ,&arglist);
  dynamic_putarg("std.char","script",(void *) initcfg,SZ,&arglist);
  dynamic_call("interpret","interpret_script",'s',arglist,&func);
  dynamic_wait(func,NULL);
  dynamic_closeargs(arglist);
}

