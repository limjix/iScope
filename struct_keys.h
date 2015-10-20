typedef struct keys
{
  //keymap attributes
  int noptions;
  int nlyrkeys;
  char *zoomin;
  char *zoomout;
  char *moveleft;
  char *moveright;
  char *moveback;
  char *moveforward;
  char *movedown;
  char *moveup;
  char *reset;
  char *init;
  char *grab;
  char *pause;
  char *playback;
  char *start;
  char *finish;
  char *prevframe;
  char *nextframe;
  char *layeron;
  char *layeroff;
  unsigned *layername;
} keys;
