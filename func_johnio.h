void *john_testhelloworld(void *arglist);
void *john_tnodes(void *arglist);
void *john_kalmaninit(void *arglist);
void *john_kalmancreatenodes(void *arglist);
void *john_makeplotvector(void *arglist);
void *john_nodeplot(void *arglist);

void add_hfactortondata(nodes *curr,hfactor *hfac);
hfactor *create_hfactor();
void *john_testfunc(void *arglist);
nodes *createnode(int x, int y,char str[MAXLEN],hgph *graph, char type);

//Sumproduct
void *john_sumproductinit(void *arglist);
void *john_setupproblem(void *arglist);
void *john_sumproductalgorithm(void *arglist);
void forwardtraverse(nodes *currentnode,nodes *callingnode, hgph *graph);
void backwardtraverse(nodes *currentnode,nodes *callingnode, hgph *graph);
void calculatemarginals(hgph *graph);

//Supplementary Sumproduct
void addmessagetonode(mvec *messageptr, nodes *targetnode);
void addmessagetonodeB(mvec *messageptr, nodes *targetnode);
void writeresultstofile(hgph *graph);

//Linear Algebra Functions
mvec *SumRowsOrCols(nodes *factornode, nodes *previousnode, char specify);
mvec *sumobservednode(nodes *factornode, nodes *observednode);
mvec *productofvectors(mvec* vecA, mvec* vecB);
mvec *productofmessagesin(nodes *targetnode, char specify);
mvec *makeoutgoingfmsg(mvec *factorsum, nodes *factornode, nodes *previousnode);
