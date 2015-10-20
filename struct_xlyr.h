typedef struct xlyr
{
  unsigned hash;
  char type;
  char visible;
  int vieworder;
  char viewlogic;

  char lyrmem;
  Pixmap rgb;
  Pixmap alpha;
  Pixmap mask;
  GC rgbgc;
  GC alphagc;
  GC maskgc;
} xlyr;

