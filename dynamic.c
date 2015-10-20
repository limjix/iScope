#include "dynamic.h"

//gcc -g -fPIC -c dynamic.c && gcc -g -fPIC -dynamiclib -o libdynamic.dylib dynamic.o -ldl -lpthread -L. -lutilities

void *dynamic_default(void *input)
{
  // ******************************************************** //
  // ** DEFAULT FUNCTION WHEN DYNAMIC LIBARARY DEACTIVATED ** //
  // ******************************************************** //

  printf("do nothing: #define DYLIB not set in head_const.h\n");

  return input;
}

void *dynamic_thread(void *args)
{
  hfunc *func;
  void *output;
 
  // *************************************************** //
  // ** WRAPPER FOR FUNCTIONS INVOKED THROUGH PTHREAD ** //
  // *************************************************** //

  func=(hfunc *) args;

  output=(*(func->funcptr))(func->input); //invoke library function

  //close thread
  #ifdef PTHREAD
    if(func->parallel=='p')
      pthread_exit(output);
  #endif
  
  return output;
}

void dynamic_openlib(char *name,void **library)
{
  void *lib;
 
  // ************************************* //
  // ** OPEN DYNAMIC LIBRARY AT RUNTIME ** //
  // ************************************* //

  #ifdef DYLIB
    lib=dlopen(name,RTLD_LAZY);
  
    if(!lib) 
    {
      printf("error in dynamic_open(): %s\n",dlerror());
      *library=NULL;
      return;
    }
  #endif
    
  *library=lib;
}

void dynamic_call(char *libname,char *function,char parallel,void *input,void **args)
{
  char *error;
  hfunc *func; 
  char library[MAXLEN];

  // ******************************************************************************** //
  // ** RETRIEVE FUNCTION POINTER FROM LIBRARY AT RUNTIME AND INVOKE IN NEW THREAD ** //
  // ******************************************************************************** //

  func=NULL;
  func=(hfunc *)imalloc(E,sizeof(hfunc));  //memory deallocated in dynamic_wait()
  if(invalidptr(E,func)) return;

  //design decision for simplicity to open library at each call
  //functions are thus accessed solely by strings, 
  //one for the library, one for the function name
  sprintf(library,LIBPRE "%s" LIBPOST,libname); //language trick to concantenate string literals
  dynamic_openlib(library,&(func->libptr)); 

  if(func->libptr!=NULL)
  {
    sprintf(func->libname,"%s",library);
    sprintf(func->funcname,"%s",function);
    printf("dynamic: running %s:%s\n",library,function);
    func->parallel=parallel;
    func->input=input;                    
    func->output=NULL;                     
    func->funcptr=&dynamic_default;        //default initialisation
  
    #ifdef DYLIB
      func->funcptr=dlsym(func->libptr,function);
  
      //check for errors
      error=NULL;
      error=dlerror();
      if(!invalidptr(S,error))  
      {
        printf("error in dynamic_function(): %s\n",error);
        func->funcptr=NULL;
        return;
      }
    #endif

    #ifdef PTHREAD
      if(parallel=='p')
        pthread_create(&(func->thread),NULL,&dynamic_thread,(void *) func);
      else
    #endif
        func->output=dynamic_thread((void *) func);
  
    *args=(void *) func;
  }
  else
  {
    *args=NULL;
  }
}

void dynamic_closelib(void *lib)
{
  // *************************** //
  // ** CLOSE DYNAMIC LIBRARY ** //
  // *************************** //

  #ifdef DYLIB
    dlclose(lib);
  #endif
}

void dynamic_wait(void *arg,void **output)
{
  hfunc *func;

  // **************************************************** //
  // ** WAIT FOR THREADED FUNCTION TO FINISH EXECUTION ** //
  // **************************************************** //

  func=(hfunc *) arg; //format convert from outside world
  if(invalidptr(S,func)) return;

  if(func->libptr!=NULL)
  {
    #ifdef PTHREAD
      if(func->parallel=='p')
        pthread_join(func->thread,output);
      else
    #endif 
        if(!invalidptr(S,output))
          *output=func->output;

    dynamic_closelib(func->libptr); //close library after each use

    //free resources for the function handle
    func=ifree(E,func);
  }
}

void dynamic_putarg(char *type,char *name,void *value,unsigned long size,void **args)
{
  int iarg;
  harg *argptr;
  hobj **newlist;

  // **************************************** //
  // ** ADD AN OBJECT TO THE ARGUMENT LIST ** //
  // **************************************** //

  if(invalidptr(E,args)) return;
  argptr=(harg *) *args;
  
  //expand list
  if(invalidptr(S,argptr))
  {
    //create arguments handle
    argptr=(harg *)imalloc(E,sizeof(harg));
    if(invalidptr(E,argptr)) return;

    //create new list of arguments
    newlist=NULL;
    newlist=(hobj **)imalloc(E,sizeof(hobj *));
    if(invalidptr(E,newlist)) return;
    argptr->nargs=0;  
  }
  else
  {
    newlist=NULL;
    newlist=(hobj **)imalloc(E,(argptr->nargs+1)*sizeof(hobj *));
    if(invalidptr(E,newlist)) return;
 
    //copy old list to new list and release old list
    if(!invalidptr(E,argptr->arglist))
    {
      for(iarg=0;iarg<argptr->nargs;iarg++)
        newlist[iarg]=argptr->arglist[iarg];
  
      argptr->arglist=ifree(E,argptr->arglist);
    }
  }

  //allocate new object
  newlist[argptr->nargs]=NULL;
  newlist[argptr->nargs]=(hobj *)imalloc(E,sizeof(hobj));
  if(invalidptr(E,newlist[argptr->nargs])) return;
  newlist[argptr->nargs]->vhash=str_hash(name);  //unique identifier
  newlist[argptr->nargs]->vlock='f';             //mutability control
  newlist[argptr->nargs]->vsize=size;            //size of data block
  newlist[argptr->nargs]->vdata=value;           //location of data
  sprintf(newlist[argptr->nargs]->vtype,"%s",type);  //store string to locate destructor

  //printf("dynamic: putarg %s: newlist[%d]->vdata=%p\n",name,argptr->nargs,newlist[argptr->nargs]->vdata);

  //update the argument list and pass it back out 
  argptr->arglist=newlist;
  argptr->nargs++;
  *args=(void *) argptr;
}

void dynamic_delarg(void *args,char *name,char mem)
{
  int iarg;
  int remarg;
  unsigned nhash;
  harg *argptr;
  hobj **newlist;
  void *funclist;
  
  char destructor[MAXLEN];
  void *objlist;
  void *interpreter;
  void *arglist;
  int nchars;
  char *text;

  // ********************************************* //
  // ** REMOVE AN OBJECT FROM THE ARGUMENT LIST ** //
  // ********************************************* //

  argptr=(harg *) args;
  if(invalidptr(E,argptr)) return;
  if(argptr->nargs==0) return;

  newlist=NULL;
  newlist=(hobj **)imalloc(E,(argptr->nargs-1)*sizeof(hobj *));
  if(invalidptr(E,newlist)) return;

  nhash=str_hash(name);
  remarg=0; //safe default

  for(iarg=0;iarg<argptr->nargs;iarg++) //scan all arguments
    if(!invalidptr(E,argptr->arglist))
      if(!invalidptr(E,argptr->arglist[iarg]))
        if(argptr->arglist[iarg]->vhash==nhash) //match object name
          remarg=iarg;

  //copy old list to new list and release old list
  if(!invalidptr(E,argptr->arglist))
  {
    for(iarg=0;iarg<remarg;iarg++)
      newlist[iarg]=argptr->arglist[iarg];
    
    for(iarg=remarg+1;iarg<argptr->nargs;iarg++)
      newlist[iarg-1]=argptr->arglist[iarg];

    //apply destructor
    if(mem=='d')
    {
      //construct text string to interpret, using type information
      sprintf(destructor,"<-%s_destroy<-object:%s;",argptr->arglist[remarg]->vtype,name);
      nchars=strlen(destructor);

      //set up local interpreter environment
      funclist=NULL;
      objlist=NULL;

      //add pointer to be destroyed as object to local environment
      dynamic_putarg("std.void",name,argptr->arglist[remarg]->vdata,SZ,&objlist);    

      arglist=NULL;
      dynamic_putarg("std.void","funclist",funclist,SZ,&arglist);     
      dynamic_putarg("std.void","objlist",objlist,SZ,&arglist);     
      dynamic_putarg("std.int","nchars",(void *) &nchars,SZ,&arglist);     
      dynamic_putarg("std.char","text",(void *) destructor,SZ,&arglist);     
      dynamic_call("interpret","interpret_command",'s',arglist,&interpreter);
      dynamic_wait(interpreter,NULL);
      dynamic_closeargs(arglist);
    }

    argptr->arglist[remarg]=ifree(E,argptr->arglist[remarg]); //free object but not the data tagged within it
    
    argptr->arglist=ifree(E,argptr->arglist); //free list

    //return modified list
    argptr->nargs--;
    argptr->arglist=newlist;
  }
}

char dynamic_getarg(void *args,char *name,void **value)
{
  int iarg;
  unsigned nhash;
  harg *argptr;

  // *********************************** //
  // ** OBSERVE AN ARGUMENT IN A LIST ** //
  // *********************************** //
 
  //printf("dynamic: unprotected\n");
  
  argptr=(harg *) args;
  if(invalidptr(S,argptr)) return;
  
  nhash=str_hash(name);

  for(iarg=0;iarg<argptr->nargs;iarg++) //scan all arguments
    if(!invalidptr(E,argptr->arglist))
      if(!invalidptr(E,argptr->arglist[iarg]))
        if(argptr->arglist[iarg]->vhash==nhash) //match object name
        {
          *value=(void *) argptr->arglist[iarg]->vdata; //retrieve variable data
	  //printf("dynamic: getarg: %s value=%p nargs=%d\n",name,*value,argptr->nargs);
          return 't';
        }

  return 'f';
}

char dynamic_getarghash(void *args,unsigned nhash,void **value)
{
  int iarg;
  harg *argptr;

  // *********************************** //
  // ** OBSERVE AN ARGUMENT IN A LIST ** //
  // *********************************** //
 
  //printf("dynamic: unprotected\n");
  
  argptr=(harg *) args;
  if(invalidptr(E,argptr)) return;
  
  for(iarg=0;iarg<argptr->nargs;iarg++) //scan all arguments
    if(!invalidptr(E,argptr->arglist))
      if(!invalidptr(E,argptr->arglist[iarg]))
      {
        //printf("dynamic: getarghash: nhash=%u, argptr->arglist[iarg]->vhash=%u\n",nhash,argptr->arglist[iarg]->vhash); 
        if(argptr->arglist[iarg]->vhash==nhash) //match object name
        {
          *value=(void *) argptr->arglist[iarg]->vdata; //retrieve variable data
	  //printf("dynamic: getarghash: %s value=%p nargs=%d\n",name,*value,argptr->nargs);
          return 't';
        } 
      }

  return 'f';
}

void dynamic_closeargs(void *args)
{
  int iarg;
  harg *argptr;

  // ************************************ //
  // ** CLEAN UP STORAGE FOR ARGUMENTS ** //
  // ************************************ //
  
  argptr=(harg *) args;
  if(invalidptr(S,argptr)) return;
  if(invalidptr(E,argptr->arglist)) return;

  //release memory for objects (but not their data)
  for(iarg=0;iarg<argptr->nargs;iarg++)
    argptr->arglist[iarg]=ifree(E,argptr->arglist[iarg]);

  //release memory for object list
  argptr->arglist=ifree(E,argptr->arglist);
}

void dynamic_funcargs(void *handle,char var,void **arg)
{
  hfunc *func;

  // *************************************** //
  // ** RETURN ELEMENT OF FUNCTION HANDLE ** //
  // *************************************** //
  
  func=(hfunc *) handle;
  if(invalidptr(E,func)) return;

  if(var=='i') *arg=func->input;
  if(var=='o') *arg=func->output;
  if(var=='p') *arg=(void *) &(func->parallel);
  if(var=='t') *arg=(void *) &(func->instance);
  if(var=='l') *arg=(void *) func->libname;
  if(var=='f') *arg=(void *) func->funcname;
}

void *dynamic_hfunc_destroy(void *arglist)
{
  hfunc *func;
  void *argptr;
 
  // ******************************************** //
  // ** RELEASE MEMORY FOR HFUNC DATA STRUCTURE ** //
  // ******************************************** //

  if(dynamic_getarg(arglist,"object",&argptr)=='f') return NULL;
  if(!invalidptr(E,argptr)) return NULL;
  func=(hfunc *) argptr;

  printf("dynamic: closing hfunc\n");

  func=ifree(E,func);

  return NULL;  
}
