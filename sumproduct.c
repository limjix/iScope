#include "sumproduct.h"
//--------------------------------------------------------------------------------------------------
//----------------------------------- Sum Product Algorithm ----------------------------------------
//--------------------------------------------------------------------------------------------------

void *sumproduct_sumproductinit(void *arglist)
{
//-----------------------------Initialises the problem---------------------------------------------
	hgph *graph;
	graph=(hgph *)imalloc(E,sizeof(hgph));
	if(invalidptr(E,graph)) return NULL;

	//Add in nnodes and nodelist
	graph->nnodes = 0;
	graph->nodelist = NULL;

	//Exports initialised values
	arglist=NULL;
	dynamic_putarg("graph.hgph","hgph",(void *)graph,SZ,&arglist);

	return arglist;
}

void *sumproduct_setupproblem(void *arglist)
{
//--------------------------- Creates nodes and sets up model---------------------------------------
	void *argptr;
	nodes *x1, *fa, *x2, *fb, *x3, *fc, *x4;
	hgph *graph;

	//Get previous hgph struct
	if(dynamic_getarg(arglist,"hgph",&argptr)=='f') return NULL;
	if(!invalidptr(E,argptr)) graph=(hgph *) argptr;

	//Sets up nodes in their places
	x1 = createnode(1,5,"x1",graph,'v');
	fa = createnode(3,5,"fa",graph,'f');
	x2 = createnode(5,5,"x2",graph,'v');
	fb = createnode(7,5,"fb",graph,'f');
	x3 = createnode(9,5,"x3",graph,'v');
	fc = createnode(5,3,"fc",graph,'f');
	x4 = createnode(5,1,"x4",graph,'v');

	//Adds directed edge between mu and x
	add_edge(x1,fa);
	add_edge(fa,x2);
	add_edge(x2,fb);
	add_edge(fb,x3);
	add_edge(x4,fc);
	add_edge(fc,x2);

	//Create probability distribution table
	/*Row Major Ordering
	EG:
	{11 12 13
	 21 22 23
	 31 32 33}
	Becomes
	{11 12 13 21 22 23 31 32 33}
	*/

	double *probdist1; //fc
	probdist1 =(double *)imalloc(E,6*sizeof(double));

/*	probdist1[0] = 1.0;
	probdist1[1] = 0.1;
	probdist1[2] = 0.1;
	probdist1[3] = 1.0; */

//	probdist1[0] = 0.3;
//	probdist1[1] = 0.4;
//	probdist1[2] = 0.3;
//	probdist1[3] = 0.0;

	probdist1[0] = 0.3;
	probdist1[1] = 0.4;
	probdist1[2] = 0.5;
	probdist1[3] = 0.3;
	probdist1[4] = 0.1;
	probdist1[5] = 0.6;

	double *probdist2; //fa
	probdist2 =(double *)imalloc(E,6*sizeof(double));

/*	probdist2[0] = 1.0;
	probdist2[1] = 0.9;
	probdist2[2] = 0.9;
	probdist2[3] = 1.0; */

//	probdist2[0] = 0.5;
//	probdist2[1] = 0.2;
//	probdist2[2] = 0.3;
//	probdist2[3] = 0.5;

	probdist2[0] = 0.5;
	probdist2[1] = 0.2;
	probdist2[2] = 0.3;
	probdist2[3] = 0.5;
	probdist2[4] = 0.1;
	probdist2[5] = 0.2;

	double *probdist3; //fb
	probdist3 =(double *)imalloc(E,6*sizeof(double));

/*	probdist3[0] = 0.1;
	probdist3[1] = 1.0;
	probdist3[2] = 1.0;
	probdist3[3] = 0.1; */

//	probdist3[0] = 0.7;
//	probdist3[1] = 0.2;
//	probdist3[2] = 0.1;
//	probdist3[3] = 0.3;

	probdist3[0] = 0.7;
	probdist3[1] = 0.2;
	probdist3[2] = 0.3;
	probdist3[3] = 0.1;
	probdist3[4] = 0.3;
	probdist3[5] = 0.5;

	//Populate factor payloads with probability distribution
	double *discretevalue = (double *)imalloc(E,2*sizeof(double));
	discretevalue[0] = 0.0;
	discretevalue[1] = 1.0;

	double *disval = (double *)imalloc(E,3*sizeof(double));
	disval[0] = 0.0;
	disval[1] = 1.0;
	disval[2] = 2.0;

	((hfactor *)fa->ndata)->probdist = probdist2;
	((hfactor *)fa->ndata)->columndiscretevalues = discretevalue;
	((hfactor *)fa->ndata)->rowdiscretevalues = disval;
	((hfactor *)fa->ndata)->columnlabel = str_hash("x1");
	((hfactor *)fa->ndata)->rowlabel = str_hash("x2");
	((hfactor *)fa->ndata)->ncol = 2;
	((hfactor *)fa->ndata)->nrow = 3;


	((hfactor *)fb->ndata)->probdist = probdist3;
	((hfactor *)fb->ndata)->columndiscretevalues = disval;
	((hfactor *)fb->ndata)->rowdiscretevalues = discretevalue;
	((hfactor *)fb->ndata)->columnlabel = str_hash("x2");
	((hfactor *)fb->ndata)->rowlabel = str_hash("x3");
	((hfactor *)fb->ndata)->ncol = 3;
	((hfactor *)fb->ndata)->nrow = 2;


	((hfactor *)fc->ndata)->probdist = probdist1;
	((hfactor *)fc->ndata)->columndiscretevalues = disval;
	((hfactor *)fc->ndata)->rowdiscretevalues = discretevalue;
	((hfactor *)fc->ndata)->columnlabel = str_hash("x2");
	((hfactor *)fc->ndata)->rowlabel = str_hash("x4");
	((hfactor *)fc->ndata)->ncol = 3;
	((hfactor *)fc->ndata)->nrow = 2;

	//Test observed node
	((hfactor *)x1->ndata)->observed = 't';
	((hfactor *)x1->ndata)->observedvariable = 1.0;

	//Output to arglist
	arglist=NULL;
	dynamic_putarg("graph.hgph","hgph",(void *)graph,SZ,&arglist);

	return arglist;
}

void *sumproduct_sumproductalgorithm(void *arglist)
{
	void *argptr, *argptr2;
	hgph *graph;
	nodes *root;
	char *userinputroot;
	//Get previous hgph struct
	if(dynamic_getarg(arglist,"hgph",&argptr)=='f') return NULL;
	if(!invalidptr(E,argptr)) graph=(hgph *) argptr;

	//Get user input on root node
	if(dynamic_getarg(arglist,"root",&argptr2)=='f') return NULL;
	if(!invalidptr(E,argptr)) userinputroot=(char *) argptr2;

	//Choose the root node
	root = find_node(str_hash(userinputroot),graph->nnodes,graph->nodelist);

	//Call the recursion step
	forwardtraverse(root,root,graph);
	backwardtraverse(root,root,graph);
	calculatemarginals(graph);

	writeresultstofile(graph);

	//Return arglist
	arglist=NULL;
	dynamic_putarg("graph.hgph","hgph",(void *)graph,SZ,&arglist);

	return arglist;
}

void forwardtraverse(nodes *currentnode,nodes *callingnode, hgph *graph)
{
//-----------------------Go down to leaf and come up

	int nbranches = currentnode->nedges;

	//When further down the tree, every edge except the calling edge is forward
	int nforwardbranches = nbranches - 1;

	//If root node, every edge is a forward edge
	if(currentnode==callingnode) nforwardbranches = nbranches;

	char type = ((hfactor*)currentnode->ndata)->type;

	nodes *nextnode;

	//----If at leaf go up-----
	//----Leaf is the terminal nodes of the tree, as far as can go.
	if(nforwardbranches==0)
	{
		mvec *message = (mvec *)imalloc(E,1*sizeof(mvec)); //Allocate memory for message

		//Determine length of message vector
		int msgveclength;

		if(((hfactor *)callingnode->ndata)->columnlabel == currentnode->nhash) //If prob dist column is for the current node,
		{
			msgveclength = ((hfactor *)callingnode->ndata)->ncol;
		}
		else
		{
			msgveclength = ((hfactor *)callingnode->ndata)->nrow;
		}

		//Allocate memory for vector and initialise value to 1 as per theory
		double *msgvec = (double *)imalloc(E,msgveclength*sizeof(double));
		int i = 0;
		for(i;i<msgveclength;i++)
		{
			msgvec[i] = 1;
		}
		message->vector = msgvec;
		message->sender = currentnode->nhash;
		message->length = msgveclength;
		addmessagetonode(message, callingnode); //Attach full message to callingnode
		return;
	}

	//----If not at leaf, go down & come up----------
	else
	{
	//Copy edge list & remove calling node from the list
		unsigned *forwardedges = (unsigned *)imalloc(E,nforwardbranches*sizeof(unsigned)); //list of edges to pursue
		int j = 0;
		char callingnodefound = 'f';
		for( j; j<nbranches; j++)
		{
			if(callingnodefound =='f')
			{
			  if(currentnode->edge[j]==callingnode->nhash)
			    { callingnodefound = 't'; continue; }
			  else forwardedges[j] = currentnode->edge[j];
			}
			else forwardedges[j-1] = currentnode->edge[j];
		}
		((hfactor *)currentnode->ndata)->fedges = forwardedges; //Saves the list to the structure

	//RECURSION DOWN
		int i=0;
		for(i; i<nforwardbranches; i++)
		{
			nextnode = find_node(forwardedges[i],graph->nnodes,graph->nodelist);
			forwardtraverse(nextnode,currentnode,graph);
		}
	//RETURNING FROM RECURSION

	//What happens on this level before going up
	//Results are appended to the calling node
		if(currentnode==callingnode) return; //If this is the root node, forward traverse ends

		if(type == 'v') //variable -- product of factor messages
		{
			mvec *message = productofmessagesin(currentnode,'f'); //Products all the factor messages
			addmessagetonode(message, callingnode);
			return;
		}

		else if(type == 'f') //factor --- sum marginals
		{
			//Previous node is next node and next node is calling node
			if(((hfactor*)nextnode->ndata)->observed == 'f') //If the previous node was unobserved
			{
				mvec *message;
				if(((hfactor*)callingnode->ndata)->observed == 't') //If the nextnode is observed
				{
					message = MsgToObservedNode(currentnode, callingnode);
				}
				else //Else if calling node not observed
				{
					message = SumRowsOrCols(currentnode, nextnode,'f');
				}
				addmessagetonode(message,callingnode);
			}
			else //If the previous node was observed - take only the associated row or column
			{
				mvec *message;
				if(((hfactor*)callingnode->ndata)->observed == 't') //If the next node is observed
				{
					message = MsgToObservedNode(currentnode, callingnode);
				}
				else
				{
					message = sumobservednode(currentnode, nextnode);
				}
				addmessagetonode(message,callingnode);
			}
			return;
		}
	}
}

void backwardtraverse(nodes *currentnode,nodes *callingnode, hgph *graph)
{
//--------------------------Go down and update nodes

	int nbranches = currentnode->nedges;

	//When further down the tree, every edge except the calling edge is forward
	int nforwardbranches = nbranches - 1;

	//If root node, every edge is a forward edge
	if(currentnode==callingnode) nforwardbranches = nbranches;

	char type = ((hfactor*)currentnode->ndata)->type;
	char callingobs = ((hfactor*)callingnode->ndata)->observed;
	nodes *nextnode;

//-----------------------------Find forward branches-------------------------------------------------
	//Already computed in forward traverse

		unsigned *forwardedges = ((hfactor*)currentnode->ndata)->fedges;

//-----------------------------Calculate & send message first----------------------------------------------------
	if (currentnode==callingnode) //If this is the root node --- Message is 1 if only 1 branch, product of messages if not
	{
		int msgveclength;
		mvec *message;
		int k = 0;
		if(nforwardbranches == 1) //If only has 1 outgoing connection
		{
			message = (mvec *)imalloc(E,1*sizeof(mvec)); //Allocate memory for message
			nextnode= find_node(forwardedges[0],graph->nnodes,graph->nodelist);

			//Determine length of message vector
			if(((hfactor *)nextnode->ndata)->columnlabel == currentnode->nhash) //If prob dist column is for the current node,
			{
				msgveclength = ((hfactor *)nextnode->ndata)->ncol;
			}
			else
			{
				msgveclength = ((hfactor *)nextnode->ndata)->nrow;
			}

			//Allocate memory for vector and initialise value to 1 as per theory
			double *msgvec = (double *)imalloc(E,msgveclength*sizeof(double));
			int i;
			for(i=0;i<msgveclength;i++)
			{
				msgvec[i] = 1;
			}
			message->vector = msgvec;
			message->sender = currentnode->nhash;
			message->length = msgveclength;

			addmessagetonodeB(message, nextnode); //Attach full message to callingnode

		}
		else //If it has multiple outgoing connections
		{
			for(k;k<nforwardbranches;k++) // For each nextnode
			{
				nextnode = find_node(forwardedges[k],graph->nnodes,graph->nodelist);
				message = productofmessagesinB(currentnode, nextnode);
				addmessagetonodeB(message, nextnode);
			}
		}
	}
	else if (type == 'v') //If this is a variable node --- Product of messages in
	{
		mvec *message;
		int k = 0;
		for(k;k<nforwardbranches;k++) // For each nextnode
		{
			nextnode = find_node(forwardedges[k],graph->nnodes,graph->nodelist);
			message = productofmessagesinB(currentnode, nextnode);
			addmessagetonodeB(message, nextnode);
		}
	}
	else //If this is a factor node --- Sum of row/column multiplied by previous messages --- Factor node only connected to 2 nodes
	{
		if(((hfactor*)callingnode->ndata)->observed == 'f') //If it is unobserved
		{
			nextnode = find_node(forwardedges[0],graph->nnodes,graph->nodelist);
			mvec *message;
			if(((hfactor*)nextnode->ndata)->observed == 't') //If the nextnode is observed
			{
				message = MsgToObservedNode(currentnode, nextnode);
			}
			else //Else if calling node not observed
			{
				message = SumRowsOrCols(currentnode, callingnode,'b');
			}

			addmessagetonodeB(message, nextnode);

		}
		else //If it is observed
		{
			mvec *message;
			nextnode = find_node(forwardedges[0],graph->nnodes,graph->nodelist);
			if(((hfactor*)nextnode->ndata)->observed == 't') //If the nextnode is observed
			{
				message = MsgToObservedNode(currentnode, nextnode);
			}
			else
			{
				message = sumobservednode(currentnode, callingnode);
			}
			addmessagetonodeB(message, nextnode);
		}
	}


//-------------------------------Recurse Down-------------------------------------------------------

	int i=0;
	for(i; i<nforwardbranches; i++)
	{
		nextnode = find_node(forwardedges[i],graph->nnodes,graph->nodelist);
		backwardtraverse(nextnode,currentnode,graph);
	}
	//Return from Recursion

	return;
}

void calculatemarginals(hgph *graph)
{
//-----------------------Calculate marginals at all the variable nodes
//------Done by calculating product of messages

	nodes **nodelist = graph->nodelist;
	int nnodes = graph->nnodes;
	nodes *currentnode;
	hfactor *currenthfac;
	mvec *margvec;
	mvec *currentmessage;
	double *vec;
	int length,nmessages,nbmessages;

	int i = 0;
	int j,k,m;
	for(i;i<nnodes;i++) //for every node
	{
		margvec = (mvec *)imalloc(E,1*sizeof(mvec));
		currentnode = nodelist[i];
		currenthfac = (hfactor *)currentnode->ndata;
		nmessages = currenthfac->nmessages;
		nbmessages = currenthfac->nbmessages;

		//Does a check if it is a variable node
		if(currenthfac->type == 'f') continue;

		//Length of messages will all be same, must have 1 message either in or out
		if(nmessages!=0) length = currenthfac->messagesin[0]->length;
		else if(nbmessages!= 0) length = currenthfac->bmessagesin[0]->length;

		//Allocate memory
		vec = (double *)imalloc(E,length*sizeof(double));
		margvec->length = length;

		for(k=0;k<length;k++)//For every element in the vector initialise to 1
		{
			vec[k] = 1;
		}

		if(nmessages!=0) //If there is forward message
		{
			j = 0;
			for(j;j<nmessages;j++) //For every messagein (Forward)
			{
				currentmessage = currenthfac->messagesin[j];
				for(k=0;k<length;k++)//For every element in the vector
				{
					vec[k] = vec[k]*currentmessage->vector[k];
				}
			}
		}

		if(nbmessages!=0) //If there is backward message
		{
			j = 0;
			for(j;j<nbmessages;j++) //For every messagein (Backward)
			{
				currentmessage = currenthfac->bmessagesin[j];
				for(k=0;k<length;k++)//For every element in the vector
				{
					vec[k] = vec[k]*currentmessage->vector[k];
				}
			}
		}

		//Normalise
		//1. Sum
		double sumofelements= 0.0;
		for(m=0;m<length;m++) //For every element - find sum
		{
			sumofelements = sumofelements + vec[m];
		}
		//2. Divide by sum
		for(m=0;m<length;m++) //For every element - find sum
		{
			vec[m] = vec[m]/sumofelements;
		}

		//Append
		margvec->vector = vec;
		currenthfac->marginal = margvec;
	}

	return;
}

//--------------------------------------------------------------------------------------------------
//------------------------------ Supplementary Sum Product Functions -------------------------------
//--------------------------------------------------------------------------------------------------

void addmessagetonode(mvec *messageptr, nodes *targetnode)
{
//-----------------------Adds message to targetnode->messagesin for forward traverse;

	//Finds current list & number of messages
	hfactor* tnodehfac = (hfactor *)targetnode->ndata;
	mvec **list = tnodehfac->messagesin;
	int nmessages = tnodehfac->nmessages;

	//Copy to new list
	mvec **newlist = (mvec **)imalloc(E,(nmessages+1)*sizeof(mvec*));

	if(nmessages ==0) //If no messages, just add to the list
	{
		newlist[nmessages] = messageptr;
	}
	else
	{
		int i = 0;
		for(i; i<nmessages; i++)
		{
			newlist[i] = list[i];
		}

		newlist[nmessages] = messageptr;
	}

	//Free previous list
	tnodehfac->messagesin=ifree(E,tnodehfac->messagesin);

	//Append newlist to targetnode
	tnodehfac->messagesin=newlist;
	tnodehfac->nmessages++;

	return;
}

void addmessagetonodeB(mvec *messageptr, nodes *targetnode)
{
//-----------------------Adds message to targetnode->bmessagesin for backward traverse;

	//Finds current list & number of messages
	hfactor* tnodehfac = (hfactor *)targetnode->ndata;
	mvec **list = tnodehfac->bmessagesin;
	int nmessages = tnodehfac->nbmessages;

	//Copy to new list
	mvec **newlist = (mvec **)imalloc(E,(nmessages+1)*sizeof(mvec*));

	if(nmessages ==0) //If no messages, just add to the list
	{
		newlist[nmessages] = messageptr;
	}
	else
	{
		int i = 0;
		for(i; i<nmessages; i++)
		{
			newlist[i] = list[i];
		}

		newlist[nmessages] = messageptr;
	}

	//Free previous list
	tnodehfac->bmessagesin=ifree(E,tnodehfac->bmessagesin);

	//Append newlist to targetnode
	tnodehfac->bmessagesin=newlist;
	tnodehfac->nbmessages++;

	return;
}

void writeresultstofile(hgph *graph)
{
//--------------------Write results to file for debugging purposes---------------------------------
	int nnodes = graph->nnodes;
	FILE *fpointer;
	fpointer = fopen("SumProductResults.txt","w");

	nodes *node;
	hfactor *hfac;
	mvec *vec,*mag;

	int i = 0;
	int j = 0;
	int k = 0;
	for(i;i<nnodes;i++) //For each node
	{
		node = graph->nodelist[i];
		hfac = (hfactor*)node->ndata;

	//Forward Traverse
		fprintf(fpointer, "***Node: %d\n", node->nhash);
		fprintf(fpointer, "*Nmessages: %d\n", hfac->nmessages);

		for(j=0;j<(hfac->nmessages);j++) //For each message
		{
			vec = hfac->messagesin[j];
			fprintf(fpointer, "Message #%d \t from %d\n", j, vec->sender);

			for(k=0;k<(vec->length);k++) //For each vector element
			{
				fprintf(fpointer, "%f \t", vec->vector[k]);
			}
			fprintf(fpointer, "\n");
		}
		fprintf(fpointer, "\n");

	//Backward Traverse
		fprintf(fpointer, "*Nbmessages: %d\n", hfac->nbmessages);

		for(j=0;j<(hfac->nbmessages);j++) //For each message
		{
			vec = hfac->bmessagesin[j];
			fprintf(fpointer, "Message #%d \t from %d\n", j, vec->sender);

			for(k=0;k<(vec->length);k++) //For each vector element
			{
				fprintf(fpointer, "%f \t", vec->vector[k]);
			}
			fprintf(fpointer, "\n");
		}

		fprintf(fpointer, "\n");

	//Marginals
		if(hfac->type =='v')
		{
			fprintf(fpointer, "*Marginals: \n");
			mag = hfac->marginal;
			for(j=0;j<(mag->length);j++)
			{
				fprintf(fpointer, "%f \t", mag->vector[j]);
			}
			fprintf(fpointer, "\n");
		}
		fprintf(fpointer, "------------------------------------------------------------------\n");

	}

	fclose(fpointer);

	return;
}
//--------------------------------------------------------------------------------------------------
//----------------------------------- Message Creators----------------------------------------------
//--------------------------------------------------------------------------------------------------
mvec *SumRowsOrCols(nodes *factornode, nodes *previousnode, char specify)
{ //Sums rows of columns of a matrix represented as a vector
  //Specify 'f' for forwardtraverse 'b' for backward traverse
  //Produces array
  //Vector is assumed to be Row Major

	hfactor* fnhfac = (hfactor *)factornode->ndata;
	hfactor* pnhfac = (hfactor *)previousnode->ndata;
	mvec *messagein;
	if(specify=='f')
	{
		messagein = fnhfac->messagesin[0];
	}
	else
	{
		messagein = fnhfac->bmessagesin[0];
	}
	mvec *message = (mvec *)imalloc(E,1*sizeof(mvec));
	int nrow = fnhfac->nrow;
	int ncol = fnhfac->ncol;
	double *matrix = fnhfac->probdist;
	double *result;

	int i,j;
	//Determine length of resulting vector
	if(fnhfac->columnlabel == previousnode->nhash) //If the previousnode is represented in column
	{
		//Then the length of vector = to number of rows
		result = (double *)imalloc(E,nrow*sizeof(double));
		message->length = nrow;

		//Multiply incoming message -- which should only be 1 message as its a factor node
		//Then sum
		i=0;
		for(i;i<nrow;i++)
		{
			result[i]=0;
			j=0;
			for(j;j<ncol;j++)
			{
				result[i]=result[i]+matrix[i*ncol+j]*(messagein->vector[j]);
			}
		}

	}
	else //If the previousnode is represented in the rows
	{
		//Then the length of vector = number of columns
		result = (double *)imalloc(E,ncol*sizeof(double));
		message->length = ncol;

		//Multiply incoming message -- which should only be 1 message as its a factor node
		//Then sum
		j=0;
		for(j;j<ncol;j++)
		{
			result[j]=0;
			i=0;
			for(i;i<nrow;i++)
			{
				result[j]=result[j]+matrix[i*ncol+j]*(messagein->vector[i]);
			}
		}

	}

	message->sender = factornode->nhash;
	message->vector = result;
	return message;

}

mvec *sumobservednode(nodes *factornode, nodes *observednode)
{
	hfactor* fnhfac = (hfactor *)factornode->ndata;
	hfactor* onhfac = (hfactor *)observednode->ndata;
	int length;
	double *vect;
	mvec *answer = 	(mvec *)imalloc(E,1*sizeof(mvec));
	int n=0;

	//Determine if the observed value is column or row
	if( fnhfac->columnlabel == observednode->nhash ) //if observed value is in column, vector is column but with length of nrow
	{
		length = fnhfac->nrow;

		//Which column to take?
		while( n < (fnhfac->ncol) )
		{
			if( fnhfac->columndiscretevalues[n] == onhfac->observedvariable ) break;
			n++;
		}

		vect =  (double *)imalloc(E,length*sizeof(double));

		//Take said column
		int i = 0;
		for(i; i<length; i++)
		{
			vect[i] = fnhfac->probdist[i*(fnhfac->ncol)+n];
		}
	}

	else //If observed value is in row, the vector is row but with length of ncol
	{
		length = fnhfac->ncol;

		//Which row to take?
		while( n < (fnhfac->ncol) )
		{
			if( fnhfac->rowdiscretevalues[n] == onhfac->observedvariable ) break;
			n++;
		}

		vect =  (double *)imalloc(E,length*sizeof(double));

		//Take said row
		int i = 0;
		for(i; i<length; i++)
		{
			vect[i] = fnhfac->probdist[n*(fnhfac->ncol)+i];
		}
	}

	answer->sender = factornode->nhash;
	answer->length = length;
	answer->vector = vect;
	return answer;
}

mvec *productofmessagesin(nodes *targetnode, char specify)
{
//------------------Returns the product of the messages which are in mvecs
//------------------Will be called at variable nodes
//------------------Specify is used to specify if it is forwardtraverse or backtraverse 'f' or 'b' __LEGACY, NOT NEEDED FOR BACKWARD
//------BACKWARD SUPERCEDED BY productofmessagesinB
	hfactor* tnodehfac = (hfactor *)targetnode->ndata;
	mvec** list;
	int nmessages;
	if(specify=='f')
	{
		list = tnodehfac->messagesin;
		nmessages = tnodehfac->nmessages;
	}
	else
	{
		list = tnodehfac->bmessagesin;
		nmessages = tnodehfac->nbmessages;
	}

	int length;
	mvec *product = (mvec *)imalloc(E,1*sizeof(mvec));

	if(nmessages == 0) //If there are no messages in that node, a scalar of 1 is produced
	{
		product->length = 1;
		product->vector = (double *)imalloc(E,1*sizeof(double));
		product->vector[0] = 1;
	}
	else if(nmessages == 1) //If there is 1 message, it is just the same vector
	{
		product->length = list[0]->length;
		product->vector = list[0]->vector;
	}
	else //If there are multiple messages, product one by one
	{
		length = list[0]->length;
		double *vect = (double *)imalloc(E,length*sizeof(double));
		int j;
		int i = 0; // Goes according to length of the vector
		for(i;i<length;i++)
		{
			j = 0;
			vect[i] = 1;
			for(j; j<nmessages; j++) //Goes according to how many vectors
			{
				vect[i] = vect[i]*list[j]->vector[i];
			}

		}
		product->sender = targetnode->nhash;
		product->length = length;
		product->vector = vect;

	}

	return product;
}

mvec *productofvectors(mvec *vecA, mvec *vecB)
{
//------------------Returns dot product of 2 vectors such as MATLAB ./ operator
//------------------Works using the mvec container

	//Check to ensure the length of both vectors is the same
	if(vecA->length != vecB->length) return NULL;

	int length = vecA->length;
	mvec *answer;
	double vec[length];
	answer->length = vecA->length;

	int i = 0;
	for(i; i<length; i++)
	{
		vec[i] = vecA->vector[i] * vecB->vector[i];
	}

	answer->vector = vec;
	return answer;
}

mvec *MsgToObservedNode(nodes *factornode, nodes *nextnode)
{
//------------------------- Creates message to observed node
//---------Vector should be 1 at observed value and 0 elsewhere
//---------Specify 'f' for forward traverse, 'b' for backward traverse

	hfactor* fnhfac = (hfactor *)factornode->ndata;
	hfactor* nxhfac = (hfactor *)nextnode->ndata;
	mvec *message = (mvec *)imalloc(E,1*sizeof(mvec));
	double *vec;

	double observedval = nxhfac->observedvariable;
	unsigned variablelabel = nextnode->nhash;
	int ncol = fnhfac->ncol;
	int nrow = fnhfac->nrow;

	int n = 0;
	int i;
	if(variablelabel == fnhfac->columnlabel) //If variable is in column
	{
		//Which column?
		while(n<ncol)
		{
			if(fnhfac->columndiscretevalues[n]==observedval) break;
			n++;
		}

		message->length = ncol;
		vec = (double *)imalloc(E,ncol*sizeof(double));
		for(i=0;i<ncol;i++)
		{
			vec[i]=0;
		}

		vec[n]=1;

	}
	else //If variable is in row
	{
		//Which row?
		while(n<nrow)
		{
			if(fnhfac->rowdiscretevalues[n]==observedval) break;
			n++;
		}

		message->length = nrow;
		vec = (double *)imalloc(E,nrow*sizeof(double));
		for(i=0;i<nrow;i++)
		{
			vec[i]=0;
		}

		vec[n]=1;
	}
	message->vector = vec;
	message->sender = factornode->nhash;
	return message;
}

mvec *productofmessagesinB(nodes *targetnode, nodes *receivingnode)
{
	hfactor *thfac = (hfactor *)targetnode->ndata;
	hfactor *rhfac = (hfactor *)targetnode->ndata;
	mvec *message= (mvec *)imalloc(E,1*sizeof(mvec));
	double *vec;

	int nmessages = thfac->nmessages;
	int nbmessages = thfac->nbmessages;

	//Remove the receiving node's message from messagesin
	mvec **messagein = thfac->messagesin;
	mvec **msglist= (mvec **)imalloc(E,nmessages*sizeof(mvec*)); //list of messages to product

	int j;
	char receivingnodefound = 'f';
	for( j=0; j<nmessages; j++)
	{
		if(receivingnodefound =='f')
		{
		  if(messagein[j]->sender==receivingnode->nhash)
		    { receivingnodefound = 't'; continue; }
		  else msglist[j] = messagein[j];
		}
		else msglist[j-1] = messagein[j];
	}
	nmessages--;
	//Add the message from the backward path to the list
	if(nbmessages!=0)
	{
		msglist[nmessages] = thfac->bmessagesin[0];
		nmessages++;
	}
	//Product the messages

	int i,k;
	int length = msglist[0]->length;
	vec = (double *)imalloc(E,length*sizeof(double));
	for(i=0;i<length;i++) //For each element
	{
		vec[i]=1;
		for(k=0; k<nmessages;k++) //For each message
		{
			vec[i]= vec[i]*(msglist[k]->vector[i]);
		}
	}

	//Free messagelist
	msglist=ifree(E,msglist);
	message->vector = vec;
	message->length = length;
	message->sender = targetnode->nhash;
	return message;

}




//---------------------------------------------------------------------------------------------------------
