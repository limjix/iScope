typedef struct svg
{
  //container for scaleable vector graphics at screen resolution

  int npts;
  int width;    //bounds of corresponding raster
  int height; 

  char chmem;
  int *ixpt;     //2D for screen rendering
  int *iypt;
  int *isize;
  uint8_t *cred;
  uint8_t *cgreen;
  uint8_t *cblue;
  uint8_t *calpha;
} svg;

