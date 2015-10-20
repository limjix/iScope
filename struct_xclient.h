typedef struct xclient
{
  //window attributes
  char window[MAXLEN];
  int npanels;
  panels **pan;

  //keyboard command list
  keys *keymap;
  
  //interpreter object and function lists
  void *objlist;
  void *funclist; 

} xclient;

