#ifdef PLUSPLUS
  extern "C" void dynamic_wait(void *arg,void **output);
  extern "C" void dynamic_call(char *library,char *function,char parallel,void *input,void **args);
  extern "C" void dynamic_putarg(char *type,char *name,void *value,unsigned long size,void **args);
  extern "C" char dynamic_getarg(void *args,char *name,void **value);
  extern "C" char dynamic_getarghash(void *args,unsigned nhash,void **value);
  extern "C" void dynamic_closeargs(void *inputargs);
  extern "C" void dynamic_funcargs(void *handle,char var,void **arg);
  extern "C" void dynamic_delarg(void *args,char *name,char mem);
#else
  void dynamic_wait(void *arg,void **output);
  void dynamic_call(char *library,char *function,char parallel,void *input,void **args);
  void dynamic_putarg(char *type,char *name,void *value,unsigned long size,void **args);
  char dynamic_getarg(void *args,char *name,void **value);
  char dynamic_getarghash(void *args,unsigned nhash,void **value);
  void dynamic_closeargs(void *inputargs);
  void dynamic_funcargs(void *handle,char var,void **arg);
  void dynamic_delarg(void *args,char *name,char mem);
#endif
