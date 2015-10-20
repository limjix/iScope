typedef struct rgb
{
  //container for raster image data at screen resolution

  int width;       //sizes for raster type
  int height;
  
  char chmem;      //screen pixel colour information
  uint8_t *cred;
  uint8_t *cblue;
  uint8_t *cgreen;
  uint8_t *calpha;

  char valmem;     //data array
  double *value;
} rgb;
