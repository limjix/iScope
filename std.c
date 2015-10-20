#include "std.h"

void *std_void_destroy(void *arglist)
{
  // ********************************** //
  // ** DESTRUCTOR FOR VOID POINTER ** //
  // ********************************** //

  //do absolutely nothing...

}

void *std_int_destroy(void *arglist)
{
  // ********************************** //
  // ** DESTRUCTOR FOR INTEGER ARRAY ** //
  // ********************************** //

}

void *std_uint8_destroy(void *arglist)
{
  // ********************************** //
  // ** DESTRUCTOR FOR BYTE ARRAY ** //
  // ********************************** //

}

void *std_unsigned_destroy(void *arglist)
{
  // ********************************** //
  // ** DESTRUCTOR FOR UNSIGNED ARRAY ** //
  // ********************************** //

}

void *std_double_destroy(void *arglist)
{
  // ********************************* //
  // ** DESTRUCTOR FOR DOUBLE ARRAY ** //
  // ********************************* //

}

void *std_char_destroy(void *arglist)
{
  // ************************************ //
  // ** DESTRUCTOR FOR CHARACTER ARRAY ** //
  // ************************************ //


}

void *std_sleep(void *arglist)
{
  void *argptr;
  uint64_t duration;
  struct timespec snooze;
  
  // ******************************** //
  // ** SLEEP FOR SPECIFIED PERIOD ** //
  // ******************************** //
  
  if(dynamic_getarg(arglist,"millisecs",&argptr)=='t')
  {
    if(invalidptr(E,argptr)) return NULL;
    duration= *((int *) argptr);
    duration*=1000000L;

    snooze.tv_sec=0;
    snooze.tv_nsec=duration;
    nanosleep(&snooze,NULL);
  }

  if(dynamic_getarg(arglist,"secs",&argptr)=='t')
  {
    if(invalidptr(E,argptr)) return NULL;
    duration=*((int *) argptr);

    printf("std: sleep(%u)\n",(unsigned) duration); 
    
    snooze.tv_sec=duration;
    snooze.tv_nsec=0L;
    nanosleep(&snooze,NULL);
  }
  
  return NULL;
}

void *std_for(void *arglist)
{
  // ******************************************* //
  // ** FUNCTIONAL IMPLEMENTATION OF FOR LOOP ** //
  // ******************************************* //
  


}

void *std_while(void *arglist)
{
  // ********************************************* //
  // ** FUNCTIONAL IMPLEMENTATION OF WHILE LOOP ** //
  // ********************************************* //


 

}

void *std_if(void *arglist)
{
  // *********************************************** //
  // ** FUNCTIONAL IMPLEMENTATION OF IF-THEN-ELSE ** //
  // *********************************************** //


}
