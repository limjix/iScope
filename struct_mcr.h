typedef struct mcr
{
  //FULL IMAGE
  double *image;
  int size_row; //Corresponds to height
  int size_col; //Corresponds to width

  //For Graph
  int nrow; //Number of Frames Going Down - Corresponds to height
  int ncol; //Number of Frames Going Across - Corresponds to width

  //Discretised Image size
  int n_pixel_row; //Discretised size (Going Down) Corresponds to height
  int n_pixel_col; //Discretised size (Going Across) Corresponds to width

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
  int curr_row; //Corresponds to height
  int curr_col; //Corresponds to width

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
