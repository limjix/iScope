//MaxSum
void *maxsum_maxsuminit(void *arglist);
void *maxsum_MSsetupproblem(void *arglist);
void *maxsum_MaxSumAlgorithm(void *arglist);
void MSleaftoroot(nodes *currentnode,nodes *callingnode, hgph *graph);
void MSbacktrack(nodes *currentnode,nodes *callingnode, hgph *graph);

//Supplementary Max Sum
double *convertPDtoln(hfactor *hfac);
void CreateLnDist(hgph *graph);
void MSaddmessagetonode(mvec *message, nodes *targetnode);
void MSwriteresultstofile(hgph *graph, void *xptr);
void MSaddstoretonode(mvec *store, nodes *targetnode);

//Message Creators for Max Sum
mvec **MSFactorToVarUnobserved(nodes *factornode, nodes *previousnode);
mvec **MSFactorToVarObserved(nodes *factornode, nodes *previousnode);
mvec *MSVartoFac(nodes *factornode, nodes *previousnode);
mvec *MSleafoutput(nodes *leafnode, nodes *factornode);
double *MSfindmax(nodes *rootnode, nodes *factornode);
double *MSfindbestconfig(nodes *variablenode, nodes *previousfactornode);
