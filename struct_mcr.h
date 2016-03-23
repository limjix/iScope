typedef struct mcr
{
  //FULL IMAGE
  double *image;
  int size_x; //Pixels in x
  int size_y; //Pixels in y

  //For Graph
  int nrow; //Number of frames in each direction
  int ncol;

  //Discretised Image size
  int d_size_x; //Discretised size x
  int d_size_y; //Discretised size_y

  //Full list of discretised image
  mIMG **FullList;
  int nFL;

  //What has been seen
  mIMG **SeenList;
  int nSL;
  int nrep;

  //Buffer to mimic microscope
  mIMG **buff;
  int nbuff;

  //Position of Micr
  int curr_x;
  int curr_y;

  //-----------------------HFACTOR STUFF-------------------------------------
  //FOR MRF
  double *MRFprobdist;
  double *MRFcolvals;
  double *MRFrowvals;
  int MRFnrow;
  int MRFncol;

  //FOR OBSERVATIONS
  double *OBSprobdist;
  double *OBScolvals;
  double *OBSrowvals;
  int OBSnrow;
  int OBSncol;
  //--------------------------------------------------------------------------

} mcr;
