typedef struct nodes
{
  unsigned nhash;     //unique identifier
  double vecr[ND];    //nodal position vector
  int nedges;         //length of edge array allocations
  unsigned *edge;     //connectivity list
  void *ndata;        //payload of your choice
} nodes;

