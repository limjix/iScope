typedef struct hgph
{
  int ndims;
  int nnodes;

  char dimmem;
  int *mn;
  int *mx;

  char nodemem;
  nodes **nodelist;
} hgph;
