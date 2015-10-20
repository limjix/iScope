typedef struct xreq
{
  unsigned cmd;

  //target attributes
  char ccontext;
  int panel;
  unsigned wintag;
  int *running;

  //layer name
  unsigned layer;
  char logic;

  //keystroke attributes
  char keystroke;

  //mouse attributes
  int xmouse;
  int ymouse;
  int bmouse;

  int ctr;
  
  //text attributes
  int ntext;
  char *text;

  //image attributes: copies of user pointers so no malloc/free issues
  char type; //vector or raster
  char fill;
  int nx;
  int ny;
  int npts;
  int *xpt;
  int *ypt;
  int *size;
  uint8_t *red;
  uint8_t *green;
  uint8_t *blue;
  uint8_t *alpha;
} xreq;

