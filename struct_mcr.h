typedef struct mcr
{
  int size_x; //Pixels in x
  int size_y; //Pixels in y

  int n;

  int d_size_x; //Discretised size x
  int d_size_y; //Discretised size_y

  mIMG **FullList;
  int nFL;

  mIMG **SeenList;
  int nSL;

  double *image;

  int nrep; //Number of repetitions of images

  int curr_x;
  int curr_y;
} mcr;
