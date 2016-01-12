void *john_testhelloworld(void *arglist);
void *john_tnodes(void *arglist);
void *john_kalmaninit(void *arglist);
void *john_kalmancreatenodes(void *arglist);
void *john_makeplotvector(void *arglist);
void *john_nodeplot(void *arglist);

void add_hfactortondata(nodes *curr,hfactor *hfac);
hfactor *create_hfactor();
void *john_testfunc(void *arglist);
nodes *createnode(int x, int y,char str[MAXLEN],hgph *graph);

//Sumproduct
void *john_sumproductinit(void *arglist);
void *john_setupproblem(void *arglist);
void *john_sumproductalgorithm(void *arglist);
void forwardtraverse(nodes *currentnode,nodes *callingnode, hgph *graph);
void backwardtraverse(nodes *currentnode,nodes *callingnode, hgph *graph);
void calculatemarginals();
void computefactormessage();
void computevariablemessage();
double *SumRowsOrCols(double *matrix,char specify, int nrow, int ncol);
