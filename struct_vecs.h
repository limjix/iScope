typedef struct vecs
{
  //general-purpose container for vector or grid data 

  int npts;
  int nvals;
  int ndims;
  
  char dimmem;
  int *mn;       //for gridded data without need for posmem, or for storage of bounds of vector data
  int *mx;

  char posmem;
  double *dpt;   //positon
  
  char valmem;
  double *val;   //value at position
} vecs;

