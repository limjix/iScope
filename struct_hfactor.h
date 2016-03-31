typedef struct hfactor
{
	char type; //identifies type as either variable (v) or factor(f)
	char *name; //Gives name to this node

//In the case it is a factornode - It needs to store a probability distribution
	double *probdist;
	double *columndiscretevalues;
	double *rowdiscretevalues;
	int nrow; //number of rows in the matrix
	int ncol; //number of columns in the matrix
	unsigned columnlabel; //Node str Hash
	unsigned rowlabel;    //Node str Hash

//In the case it is a variable node & is observed value
	char observed;
	double observedvariable;
	char seen; //For RPI decision to move

//---------------- Sum Product -------------------------------------------
//List of pointers to messages coming into node
	mvec **messagesin; //List of pointers, hence void **
	int nmessages; //How many messages
	mvec **bmessagesin; //Messages from backward propagation
	int nbmessages; //Number of messages from backward propagation
	unsigned *fedges; //List of forwardedges

//Marginals
	mvec *marginal;

//-------------------- Max Sum ---------------------------------------

	double *lnprobdist; //Logarithmic form of prob dist

	int nMSmsgin; //Number of messages in
	mvec **MSmsgin;

	int nMSstore; //Gives indication if it is present
	mvec *MSstore;

	double MostLikelyState;

} hfactor;
