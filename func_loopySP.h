void *loopySP_algorithm(void *arglist);

//Utilities
void LSPaddmessagetonode(mvec *msg, nodes *receivingnode);
void deallocatemvec(mvec *msg);
void LSPaddmarginaltonode(mvec *marg, nodes *node);

//Update Nodes
void *initialburstvariable(nodes *varnode , hgph *graph);
void UpdateVariableNode(nodes *VarNode, hgph *graph);
void UpdateFactorNode(nodes *FacNode, hgph *graph);

//Message Creators
mvec *VariableNodeOutput(nodes *varnode, nodes *outnode, hgph *graph);
mvec *FactorNodeOutput(nodes *facnode, nodes *outnode, hgph *graph);

//Marginal Calculator
void *LSPCalcMarginals(hgph *graph);

//Write To file
void LSPwritetofile(hgph *graph, void *xptr);
