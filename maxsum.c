#include "maxsum.h"
//--------------------------------------------------------------------------------------------------
//----------------------------------- MAX SUM Algorithm --------------------------------------------
//--------------------------------------------------------------------------------------------------

void *maxsum_maxsuminit(void *arglist)
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

void *maxsum_MSsetupproblem(void *arglist)
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
	probdist2[1] = 0.3;
	probdist2[2] = 0.1;
	probdist2[3] = 0.2;
	probdist2[4] = 0.5;
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
	((hfactor *)fa->ndata)->columndiscretevalues = disval;
	((hfactor *)fa->ndata)->rowdiscretevalues = discretevalue;
	((hfactor *)fa->ndata)->columnlabel = str_hash("x2");
	((hfactor *)fa->ndata)->rowlabel = str_hash("x1");
	((hfactor *)fa->ndata)->ncol = 3;
	((hfactor *)fa->ndata)->nrow = 2;


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
	//((hfactor *)x3->ndata)->observed = 't';
  //((hfactor *)x3->ndata)->observedvariable = 1.0;

//Create ln probdist
//	double *lnprobdistfa = (double *)imalloc(E,4*sizeof(double));
//	double *lnprobdistfb = (double *)imalloc(E,4*sizeof(double));
//	double *lnprobdistfc = (double *)imalloc(E,4*sizeof(double));

//	lnprobdistfa[0] = 1;
//	lnprobdistfa[1] = 4;
//	lnprobdistfa[2] = 3;
//	lnprobdistfa[3] = 5;

//	lnprobdistfb[0] = 6;
//	lnprobdistfb[1] = 5;
//	lnprobdistfb[2] = 7;
//	lnprobdistfb[3] = 9;

//	lnprobdistfc[0] = 9;
//	lnprobdistfc[1] = 3;
//	lnprobdistfc[2] = 2;
//	lnprobdistfc[3] = 3;

//	((hfactor *)fa->ndata)->lnprobdist = lnprobdistfa;
//	((hfactor *)fb->ndata)->lnprobdist = lnprobdistfb;
//	((hfactor *)fc->ndata)->lnprobdist = lnprobdistfc;

	//Output to arglist
	arglist=NULL;
	dynamic_putarg("graph.hgph","hgph",(void *)graph,SZ,&arglist);

	return arglist;
}

void *maxsum_MaxSumAlgorithm(void *arglist)
{
	void *argptr, *argptr2, *xptr;
	hgph *graph;
	nodes *root;
	char *userinputroot;
	int n;
	hfactor *hfac;

	//Get previous hgph struct
	if(dynamic_getarg(arglist,"hgph",&argptr)=='f') return NULL;
	if(!invalidptr(E,argptr)) graph=(hgph *) argptr;

	//Get user input on root node
	if(dynamic_getarg(arglist,"root",&argptr2)=='f') return NULL;
	if(!invalidptr(E,argptr)) userinputroot=(char *) argptr2;

	//Get Xptr
	if(dynamic_getarg(arglist,"xclient",&xptr)=='f') return NULL;

	//Choose the root node
	root = find_node(str_hash(userinputroot),graph->nnodes,graph->nodelist);

	//Ensures lnprobdist is set up

	for(n=0;n<graph->nnodes;n++) //for every node
	{
		hfac = (hfactor *)graph->nodelist[n]->ndata;
		if(hfac->type == 'f')
		{
			if(hfac->lnprobdist==NULL)
			{
				CreateLnDist(graph);
				break;
			}
			else break;
		}
	}

	//Call the recursion step
	MSleaftoroot(root,root,graph);
	MSbacktrack(root,root,graph);

	//Test
	nodes *x1 = find_node(str_hash("x1"),graph->nnodes,graph->nodelist);
	nodes *x2 = find_node(str_hash("x2"),graph->nnodes,graph->nodelist);
	nodes *x3 = find_node(str_hash("x3"),graph->nnodes,graph->nodelist);
	nodes *x4 = find_node(str_hash("x4"),graph->nnodes,graph->nodelist);
	nodes *fa = find_node(str_hash("fa"),graph->nnodes,graph->nodelist);
	nodes *fb = find_node(str_hash("fb"),graph->nnodes,graph->nodelist);
	nodes *fc = find_node(str_hash("fc"),graph->nnodes,graph->nodelist);

	hfactor *hx1 = (hfactor *)x1->ndata;
	hfactor *hx2 = (hfactor *)x2->ndata;
	hfactor *hx3 = (hfactor *)x3->ndata;
	hfactor *hx4 = (hfactor *)x4->ndata;
	hfactor *hfa = (hfactor *)fa->ndata;
	hfactor *hfb = (hfactor *)fb->ndata;
	hfactor *hfc = (hfactor *)fc->ndata;

	printtoclient("Max Sum Successful", xptr);
	MSwriteresultstofile(graph, xptr);

	//Return arglist
	arglist=NULL;
	dynamic_putarg("graph.hgph","hgph",(void *)graph,SZ,&arglist);

	return arglist;
}

void MSleaftoroot(nodes *currentnode,nodes *callingnode, hgph *graph)
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
		mvec *message = MSleafoutput(currentnode, callingnode);
		MSaddmessagetonode(message, callingnode);
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
			MSleaftoroot(nextnode,currentnode,graph);
		}
	//RETURNING FROM RECURSION

	//What happens on this level before going up
	//Results are appended to the calling node
		if(currentnode==callingnode) return; //If this is the root node, forward traverse ends

		if(type == 'v') //If it is variable -> sum
		{
			mvec *message = MSVartoFac(callingnode, currentnode);
			MSaddmessagetonode(message, callingnode);
		}
		else // If it is factor node -> max **Remember Fnode connected to 2 Vnode
		{
			//Check if observed
			if(((hfactor*)nextnode->ndata)->observed == 't') //If it is observed
			{
				mvec **pkg = MSFactorToVarObserved(currentnode, nextnode);
				MSaddmessagetonode(pkg[0], callingnode);
				MSaddstoretonode(pkg[1], nextnode);
				//((hfactor *)nextnode->ndata)->MSstore = pkg[1];
				//((hfactor *)nextnode->ndata)->nMSstore++;
				pkg = ifree(E,pkg);
			}
			else //If it is unobserved
			{
				mvec **pkg = MSFactorToVarUnobserved(currentnode, nextnode);
				MSaddmessagetonode(pkg[0], callingnode);
				MSaddstoretonode(pkg[1], nextnode);
				//((hfactor *)nextnode->ndata)->MSstore = pkg[1];
				//((hfactor *)nextnode->ndata)->nMSstore++;
				pkg = ifree(E,pkg);
			}

		}

	}
	return;
}


void MSbacktrack(nodes *currentnode,nodes *callingnode, hgph *graph)
{
	//--------------------------Go down and update nodes with decision

		int nbranches = currentnode->nedges;

		//When further down the tree, every edge except the calling edge is forward
		int nforwardbranches = nbranches - 1;

		//If root node, every edge is a forward edge
		if(currentnode==callingnode) nforwardbranches = nbranches;

		char type = ((hfactor*)currentnode->ndata)->type;

		nodes *nextnode;
		double *msg;
		int i,k;

	//-----------------------------Find forward branches--------------------------
		//Already computed in forward traverse

			unsigned *forwardedges = ((hfactor*)currentnode->ndata)->fedges;

	//-----------------------------Calculate & send message first-----------------
		if (currentnode==callingnode) //If this is the root node - take max
		{

				nextnode = find_node(forwardedges[0],graph->nnodes,graph->nodelist);
				msg = MSfindmax(currentnode, nextnode);
				((hfactor*)currentnode->ndata)->MostLikelyState=*msg;
				msg = ifree(E,msg);
		}
		else if (type == 'v') //If this is a variable node - Determine best
		{
			msg = MSfindbestconfig(currentnode, callingnode);
			((hfactor*)currentnode->ndata)->MostLikelyState=*msg;
			msg = ifree(E,msg);
		}
		else //If this is a factor node - Find previousnode MLS value and store
		{
			((hfactor*)currentnode->ndata)->MostLikelyState=
				((hfactor*)callingnode->ndata)->MostLikelyState;

		}

	//---------------------------Recurse down---------------------------------

	for(k=0;k<nforwardbranches;k++) // For each nextnode
	{
		nextnode = find_node(forwardedges[k],graph->nnodes,graph->nodelist);
		MSbacktrack(nextnode,currentnode,graph);
	}
	return;
}

//--------------------------------------------------------------------------------------------------
//----------------------------------- Supplementary Max Sum Functions ------------------------------
//--------------------------------------------------------------------------------------------------

double *convertPDtoln(hfactor *hfac)
{
//-----------Converts a probability distribution in Hfac to be in natural logarithmic form

	int nrow = hfac->nrow;
	int ncol = hfac->ncol;
	int nelements = nrow *ncol;
	int i;

	double *probdist = hfac->probdist;
	double *lnprobdist = (double *)imalloc(E,nelements*sizeof(double));

	for(i=0;i<nelements;i++) //for each element
	{
		if(probdist[i]==0) //If the element has 0 probability, taking the ln of that will give an error. Hence instead use a very small value
		{
			lnprobdist[i] = log(0.000000001);
		}
		else
		{
			lnprobdist[i] = log(probdist[i]);
		}
	}

	return lnprobdist;
}

void CreateLnDist(hgph *graph)
{
//--------------------Trawls through the graph and updates all factor nodes with log probability distributions

	int nrow;
	int ncol;
	int nelements;
	double *probdist, *lnprobdist;
	int n, i;
	hfactor *hfac;
	int nnodes = graph->nnodes;

	for(n=0;n<nnodes;n++) //For every node
	{
		hfac = (hfactor *)graph->nodelist[n]->ndata;
		nrow = hfac->nrow;
		ncol = hfac->ncol;
		nelements = nrow *ncol;

		probdist = hfac->probdist;
		lnprobdist = (double *)imalloc(E,nelements*sizeof(double));

		for(i=0;i<nelements;i++) //for each element
		{
			if(probdist[i]==0) //If the element has 0 probability, taking the ln of that will give an error. Hence instead use a very small value
			{
				lnprobdist[i] = log(0.000000001);
			}
			else
			{
				lnprobdist[i] = log(probdist[i]);
			}
		}

		hfac->lnprobdist = lnprobdist; //Append to the node
	}
	return;

}

void MSaddmessagetonode(mvec *message, nodes *targetnode)
{
//-------------------Adds message to variable nodes -> MSmsgin

	//Finds current list & number of messages
	hfactor* tnodehfac = (hfactor *)targetnode->ndata;
	mvec **list = tnodehfac->MSmsgin;
	int nmsg = tnodehfac->nMSmsgin;

	//Copy to new list
	mvec **newlist = (mvec **)imalloc(E,(nmsg+1)*sizeof(mvec*));

	if(nmsg ==0) //If no messages, just add to the list
	{
		newlist[nmsg] = message;
	}
	else
	{
		int i = 0;
		for(i; i<nmsg; i++)
		{
			newlist[i] = list[i];
		}

		newlist[nmsg] = message;
	}

	//Free previous list
	tnodehfac->MSmsgin=ifree(E,tnodehfac->MSmsgin);

	//Append newlist to targetnode
	tnodehfac->MSmsgin=newlist;
	tnodehfac->nMSmsgin++;

	return;
}

void MSwriteresultstofile(hgph *graph, void *xptr)
{
//--------------------Write results to file for debugging purposes---------------------------------
	int nnodes = graph->nnodes;
	FILE *fpointer;
	char filename[MAXLEN] = "MaxProductResults.txt";
	fpointer = fopen(filename,"w");

	nodes *node;
	hfactor *hfac;
	mvec *vec,*mag;
	char resultstring[MAXLEN];

	int i,j,k;
	for(i=0;i<nnodes;i++) //For each node
	{
		node = graph->nodelist[i];
		hfac = (hfactor*)node->ndata;

		fprintf(fpointer, "***Node: %s\n", hfac->name);
		fprintf(fpointer, "**Type: %c\n", hfac->type);
		fprintf(fpointer, "*nMSmsgin: %d\n\n", hfac->nMSmsgin);
		fprintf(fpointer, "Forward:\n");
		for(j=0;j<hfac->nMSmsgin;j++) //For each message in
		{
			vec = hfac->MSmsgin[j];
			fprintf(fpointer, "Message #%d \t from %d\n", j, vec->sender);

			for(k=0;k<(vec->length);k++) //For each vector element
			{
				fprintf(fpointer, "%f \t", vec->vector[k]);
			}
			fprintf(fpointer, "\n");
		}

		fprintf(fpointer,"\n");
		fprintf(fpointer, "Backward:\n");
		for(j=0;j<hfac->nMSstore;j++) //For each stored config
		{
			vec = hfac->MSstore;
			fprintf(fpointer, "Stored Config: ");

			for(k=0;k<vec->length;k++)
			{
				fprintf(fpointer, "%f \t", vec->vector[k]);
			}

			fprintf(fpointer,"\n\n");
		}

		if(hfac->type =='v')
		{
			fprintf(fpointer, "$$Best State: %f\n", hfac->MostLikelyState);
		}
		else
		{
			fprintf(fpointer, "Back Message: %f\n", hfac->MostLikelyState);
		}
		fprintf(fpointer, "---------------------------------------------------------------------\n");
	}

	sprintf(resultstring, "File Output: %s", filename);
	printtoclient(resultstring, xptr);
	fclose(fpointer);

	return;
}


void MSaddstoretonode(mvec *store, nodes *targetnode)
{
//-------------------Adds stored discrete variables to -> MSstore

	hfactor* tnodehfac = (hfactor *)targetnode->ndata;
	tnodehfac->MSstore = store;
	tnodehfac->nMSstore++;

	return;
}

//--------------------------------------------------------------------------------------------------
//-----------------------------------  Max Sum Message Creators-------------------------------------
//--------------------------------------------------------------------------------------------------

mvec **MSFactorToVarUnobserved(nodes *factornode, nodes *previousnode)
{
//Creates message from Factor node next node (Previous node unobserved)
//Does the MAX over previous node which is the "nextnode"
//Needs to also determine

	int length;
	double *vec,*storevec;
	mvec *message, *store;
	mvec **msgout = (mvec **)imalloc(E,2*sizeof(mvec*));
	int i,j;
	double value;

	hfactor *fnhfac = (hfactor *)factornode->ndata;
	int nrow = fnhfac->nrow;
	int ncol = fnhfac->ncol;
	double *matrix = fnhfac->lnprobdist;
	mvec **messagein = fnhfac->MSmsgin;

//Allocate
	message = (mvec *)imalloc(E,1*sizeof(mvec)); //Allocate memory for message
	store = (mvec *)imalloc(E,1*sizeof(mvec)); //Allocate memory for message

//Determine length of vector to be produced
//Length will be number of states of the receiving node
//Need to find max & store which variable causes it
	if(fnhfac->rowlabel == previousnode->nhash)
	{
		length = fnhfac->ncol;
		vec = (double *)imalloc(E,length*sizeof(double));
		storevec = (double *)imalloc(E,length*sizeof(double));

		for(j=0;j<ncol;j++) //Moving along column
		{
			vec[j]=-99.9; //Initialised value
			storevec[j] = 0;
			for(i=0;i<nrow;i++) //Taking max of rows
			{
				value = matrix[i*ncol+j]+messagein[0]->vector[i];
				if(value>vec[j])
				{
					vec[j]=value;
					storevec[j] = fnhfac->rowdiscretevalues[i];
				}
			}
		}
	}
	else //If the previousnode is represented in the rows
	{
		length = fnhfac->ncol;
		vec = (double *)imalloc(E,length*sizeof(double));
		storevec = (double *)imalloc(E,length*sizeof(double));
		for(i=0;i<nrow;i++) //Moving along rows
		{
			vec[i]=-99.9; //Initialised value
			storevec[i] = 0;
			for(j=0;j<ncol;j++) //Taking max of coumns
			{
				value = matrix[i*ncol+j]+messagein[0]->vector[j];
				if(value>vec[i])
				{
					vec[i]=value;
					storevec[i] = fnhfac->columndiscretevalues[j];
				}
			}
		}
	}

	message->vector = vec;
	message->length = length;
	message->sender = factornode->nhash;
	store->vector= storevec;
	store->length = length;
	store->sender = factornode->nhash;

	msgout[0] = message;
	msgout[1] = store;

	return msgout;

}

mvec **MSFactorToVarObserved(nodes *factornode, nodes *previousnode)
{
//-------Creates message from Factor node next node (Previous node observed)

	int length;
	double *vec,*storevec;
	mvec *message, *store;
	mvec **msgout=(mvec **)imalloc(E,2*sizeof(mvec*));
	int i,n;
	double value;

	hfactor *fnhfac = (hfactor *)factornode->ndata;
	hfactor *pnhfac = (hfactor *)previousnode->ndata;
	int nrow = fnhfac->nrow;
	int ncol = fnhfac->ncol;
	double *matrix = fnhfac->lnprobdist;
	mvec **messagein = fnhfac->MSmsgin; //will only be 1 message in because factor node

//Allocate
	message = (mvec *)imalloc(E,1*sizeof(mvec)); //Allocate memory for message
	store = (mvec *)imalloc(E,1*sizeof(mvec)); //Allocate memory for message

//Determine length of vector to be produced
//Length will be number of states of the receiving node
//Need to find max & store which variable causes it

	if(fnhfac->rowlabel == previousnode->nhash)
	{
		length = fnhfac->ncol;
		vec = (double *)imalloc(E,length*sizeof(double));
		storevec = (double *)imalloc(E,length*sizeof(double));

		//Which row to take?
		n=0;
		while(n<nrow)
		{
			if(fnhfac->rowdiscretevalues[n] == pnhfac->observedvariable) break;
			n++;
		}

		//Take said row
		for(i=0; i<length; i++)
		{
			vec[i] = fnhfac->lnprobdist[n*ncol+i]+messagein[0]->vector[n];
		}

		//Store
		for(i=0; i<length; i++)
		{
			storevec[i] = fnhfac->rowdiscretevalues[n];
		}
	}
	else //If the previousnode is represented in the rows
	{
		length = fnhfac->ncol;
		vec = (double *)imalloc(E,length*sizeof(double));
		storevec = (double *)imalloc(E,length*sizeof(double));

		//Which column to take?
		n=0;
		while(n<ncol)
		{
			if(fnhfac->columndiscretevalues[n] == pnhfac->observedvariable) break;
			n++;
		}

		//Take said column
		for(i=0; i<length; i++)
		{
			vec[i] = fnhfac->lnprobdist[i*ncol+n]+messagein[0]->vector[n];
		}

		//Store
		for(i=0; i<length; i++)
		{
			storevec[i] = fnhfac->columndiscretevalues[n];
		}
	}

	message->vector = vec;
	message->length = length;
	message->sender = factornode->nhash;
	store->vector = storevec;
	store->length = length;
	store->sender = factornode->nhash;

	msgout[0] = message;
	msgout[1] = store;

	return msgout;

}

mvec *MSVartoFac(nodes *factornode, nodes *previousnode)
{
//-----------------Creates message from Variable node to factor node
//-----------Sum of messages

	int length;
	int i,j;
	double *vec;

	mvec *message = (mvec *)imalloc(E,1*sizeof(mvec));

	hfactor *fnhfac = (hfactor *)factornode->ndata;
	hfactor *pnhfac = (hfactor *)previousnode->ndata;
	int nmsg = pnhfac->nMSmsgin;
	mvec **msglist = pnhfac->MSmsgin;

	//Determine vector length
	length = msglist[0]->length;
	vec = (double *)imalloc(E,length*sizeof(double));

	//Sum vectors
	for(i=0;i<length;i++) //For Each element
	{
		vec[i] = 0;
		for(j=0;j<nmsg;j++) //For each message
		{
			vec[i]=vec[i]+msglist[j]->vector[i];
		}
	}

	message->vector = vec;
	message->sender = previousnode->nhash;
	message->length = length;

	return message;
}

mvec *MSleafoutput(nodes *leafnode, nodes *factornode)
{
//----------------Root message is vector of 0
	mvec *message = (mvec *)imalloc(E,1*sizeof(mvec)); //Allocate memory for message
	hfactor *hfac = (hfactor *)leafnode->ndata;
	hfactor *fnhfac = (hfactor *)factornode->ndata;

	//Determine length of message vector
	int msgveclength;
	if(leafnode->nhash == fnhfac->rowlabel) 	msgveclength = fnhfac->nrow;
	else msgveclength = fnhfac->ncol;

	//Allocate memory for vector and initialise value to 1 as per theory
	double *msgvec = (double *)imalloc(E,msgveclength*sizeof(double));
	int i = 0;
	for(i;i<msgveclength;i++)
	{
		msgvec[i] = 0;
	}
	message->vector = msgvec;
	message->sender = leafnode->nhash;
	message->length = msgveclength;

	return message;
}

double *MSfindmax(nodes *rootnode, nodes *factornode)
{
//---------------------------Compute Max Config at rootnode
//-----------Should return state rather than probability

		hfactor *rnhfac = (hfactor *)rootnode->ndata;
		hfactor *fnhfac = (hfactor *)factornode->ndata;
		int nmsg = rnhfac->nMSmsgin;
		mvec **list = rnhfac->MSmsgin;
		int k,i;
		double bestprob;

		double *message = (double *)imalloc(E,1*sizeof(double));

		//Sum messages
		int length = rnhfac->MSmsgin[0]->length;
		double vec[length];

		for(k=0;k<length;k++) //For each element
		{
			vec[k] = 0;
			for(i=0;i<nmsg;i++) // For each message
			{
				vec[k]=vec[k]+list[i]->vector[k];
			}
		}

		//From the vector, find the maximum probability
		i=0;
		bestprob = -99.9;
		k=0;
		while(k<length)
		{
			if(vec[k]>bestprob)
			{
					bestprob = vec[k];
					i=k;
			}
			k++;
		}

		//Hence, determine the corresponding state
		//Is it in row or column?
		if(fnhfac->columnlabel == rootnode->nhash) // In Column
		{
			message[0] = fnhfac->columndiscretevalues[i];
		}
		else //In Row
		{
			message[0] = fnhfac->rowdiscretevalues[i];
		}

		return message;
}

double *MSfindbestconfig(nodes *variablenode, nodes *previousfactornode)
{
//-------------------------Find best config at variable node
	hfactor *pfnhfac = (hfactor *)previousfactornode->ndata;
	hfactor *vnhfac = (hfactor *)variablenode->ndata;
	int n;
	double *MLS = (double *)imalloc(E,1*sizeof(double));
	//Look into the previous factor node to find back message
	double previousmsg = pfnhfac->MostLikelyState;
	mvec *store = vnhfac->MSstore;

	//Which element in the vector is it
	n=0;
	if(variablenode->nhash == pfnhfac->columnlabel) //If current node in col
	{
		while(n<store->length)
		{
			if(previousmsg == pfnhfac->rowdiscretevalues[n]) break;
			n++;
		}
		*MLS = store->vector[n];
	}
	else //If current node in row
	{
		while(n<store->length)
		{
			if(previousmsg == pfnhfac->columndiscretevalues[n]) break;
			n++;
		}
		*MLS = store->vector[n];
	}

	return MLS;
}
