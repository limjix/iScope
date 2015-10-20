typedef struct hobj
{
  unsigned vhash;       //unique identifier
  char vlock;           //mutability control
  unsigned long vsize;  //byte-length of attached data
  void *vdata;          //pointer to data of any type
  char vtype[MAXLEN];   //location of destructors
} hobj;

