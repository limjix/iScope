typedef struct xsrv
{
  Display *dis;
  int screen;
  Window win;
  Visual *vis;
  Window root;
  GC rgbgc;
  GC alphagc;
  GC maskgc;
  Pixmap drgb;
  Pixmap dalpha;
  Pixmap dmask;
  Atom deletewindow;
  
  int depth;
  int width;
  int height;
  int xpos;
  int ypos;
  unsigned long red_mask;
  unsigned long green_mask;
  unsigned long blue_mask;

  int ncmd;
  unsigned *cmd;
  func_t *func;

  int bitdepth;
  int bquantum;
  void *callback;
  char guicfg[MAXLEN];
  char instance[MAXLEN];
  xbox *wbox; //full box
} xsrv;

