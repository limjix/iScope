typedef void *pfunc(void *);

typedef struct hfunc
{
  unsigned instance;     //index the instance of an unresolved function call
  char parallel;         //flag 'p' if new thread 's' if same thread
  char libname[MAXLEN];  //string name of library
  char funcname[MAXLEN]; //string name of function
  void *libptr;          //pointer to library
  void *input;           //input argument list
  void *output;          //output argument list  
  pfunc *funcptr;        //pointer to function we need to execute
  #ifdef PTHREAD
    pthread_t thread;    //thread ID
  #endif
} hfunc;
