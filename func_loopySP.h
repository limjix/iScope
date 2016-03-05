void *loopySP_algorithm(void *arglist);

//Utilities
void LSPaddmessagetonode(mvec *msg, nodes *receivingnode);

//Message Creators
void *initialburstvariable(nodes *varnode , hgph *graph);
mvec *VariableNodeOutput(nodes *varnode, nodes *outnode);
mvec *FactorNodeOutput(nodes *facnode, nodes *outnode);

//Calc marginals
void *LSPCalcMarginals(hgph *graph);
