typedef struct vis
{
  unsigned namehash;  //identifier for each data set
  unsigned drawhash;  //draw format (raster,circle,square,polygon, whether to fill or not)
  unsigned viewhash;  //exposure format (expose,paint,in,out,atop)
} vis;
