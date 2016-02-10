//Sumproduct
void *sumproduct_sumproductinit(void *arglist);
void *sumproduct_setupproblem(void *arglist);
void *sumproduct_sumproductalgorithm(void *arglist);
void forwardtraverse(nodes *currentnode,nodes *callingnode, hgph *graph);
void backwardtraverse(nodes *currentnode,nodes *callingnode, hgph *graph);
void calculatemarginals(hgph *graph);

//Supplementary Sumproduct
void addmessagetonode(mvec *messageptr, nodes *targetnode);
void addmessagetonodeB(mvec *messageptr, nodes *targetnode);
void writeresultstofile(hgph *graph);

//Message Creators
mvec *SumRowsOrCols(nodes *factornode, nodes *previousnode, char specify);
mvec *sumobservednode(nodes *factornode, nodes *observednode);
mvec *productofvectors(mvec* vecA, mvec* vecB);
mvec *productofmessagesin(nodes *targetnode, char specify);
mvec *MsgToObservedNode(nodes *factornode, nodes *nextnode);
mvec *productofmessagesinB(nodes *targetnode, nodes *receivingnode);
