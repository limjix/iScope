#include "loopySP.h"

void *loopySP_algorithm(void *arglist)
{
	void *argptr, *argptr2, *xptr;
	hgph *graph;
	nodes *root;
	char *userinputroot;
  int i,k, iteration;
  hfactor *hfac;
  int nvar, nfac;
  nodes *node;

	//Get previous hgph struct
	if(dynamic_getarg(arglist,"hgph",&argptr)=='f') return NULL;
	if(!invalidptr(E,argptr)) graph=(hgph *) argptr;

	//Get Xptr
	if(dynamic_getarg(arglist,"xclient",&xptr)=='f') return NULL;

  //Get some stuff from graph
  int nnodes = graph->nnodes;
  nodes **nodelist = graph->nodelist;

	//-------Create serial schedule list
  nodes **varlist = (nodes **)imalloc(E,1*sizeof(nodes *));
  nodes **faclist = (nodes **)imalloc(E,1*sizeof(nodes *));

  nodes **newvarlist;
  nodes **newfaclist;

  nvar = 0;
  nfac = 0;

  //--------Create lists of variables and nodes
  for(i=0;i<nnodes;i++) // for every node, determine variable or node
  {
    hfac = nodelist[i]->ndata;

    if(hfac->type == 'v') //if variable
    {
      newvarlist = (nodes **)imalloc(E,(nvar+1)*sizeof(nodes *));

      //Copy Old list
      for(k=0;k<nvar;k++)
      {
        newvarlist[k]=varlist[k];
      }

      newvarlist[nvar]= nodelist[i]; //Append to list
      varlist = ifree(E,varlist); //Free old varlist
      varlist = newvarlist;
      nvar++;
    }
    else //if factor
    {
      newfaclist = (nodes **)imalloc(E,(nfac+1)*sizeof(nodes *));

      //Copy Old list
      for(k=0;k<nfac;k++)
      {
        newfaclist[k]=faclist[k];
      }

      newfaclist[nfac]= nodelist[i]; //Append to list
      faclist = ifree(E,faclist); //Free old varlist
      faclist = newfaclist;
      nfac++;
    }
  }

  //Initial Burst - Variable
  for(i=0;i<nvar;i++)
  {
    initialburstvariable(varlist[i] ,graph);
  }


	iteration = 0;
	while(iteration<10)
	{
		//Update Factor
		for(i=0;i<nfac;i++)
		{
			UpdateFactorNode(faclist[i],graph);
		}

		//Update Variable
		for(i=0;i<nvar;i++)
		{
			UpdateVariableNode(varlist[i],graph);
		}

		LSPCalcMarginals(graph);
		iteration++;
	}
	//----------------- Test ------------------------------------------------
	nodes *x1 = find_node(str_hash("x1"),graph->nnodes,graph->nodelist);
	nodes *x2 = find_node(str_hash("x2"),graph->nnodes,graph->nodelist);
	nodes *x3 = find_node(str_hash("x3"),graph->nnodes,graph->nodelist);
	nodes *x4 = find_node(str_hash("x4"),graph->nnodes,graph->nodelist);
	nodes *f12 = find_node(str_hash("f12"),graph->nnodes,graph->nodelist);
	nodes *f23 = find_node(str_hash("f23"),graph->nnodes,graph->nodelist);
	nodes *f34 = find_node(str_hash("f34"),graph->nnodes,graph->nodelist);
	nodes *f41 = find_node(str_hash("f41"),graph->nnodes,graph->nodelist);

	hfactor *hx1 = (hfactor *)x1->ndata;
	hfactor *hx2 = (hfactor *)x2->ndata;
	hfactor *hx3 = (hfactor *)x3->ndata;
	hfactor *hx4 = (hfactor *)x4->ndata;
	hfactor *hf12 = (hfactor *)f12->ndata;
	hfactor *hf23 = (hfactor *)f23->ndata;
	hfactor *hf34 = (hfactor *)f34->ndata;
	hfactor *hf41 = (hfactor *)f41->ndata;
	//------------------------------------------------------------------------

	LSPwritetofile(graph, xptr);

  //Loop to Converge by doing serial updates

  //Return arglist
	arglist=NULL;
	dynamic_putarg("graph.hgph","hgph",(void *)graph,SZ,&arglist);
	return arglist;
}

//-------------------------------------------------------------------------
//------------------------ Utilities --------------------------------------
//-------------------------------------------------------------------------

void LSPaddmessagetonode(mvec *msg, nodes *receivingnode)
{
//------------Adds message to nodes
//---------Overwrites if from same node
  hfactor *hfac = (hfactor *)receivingnode->ndata;
  int nmessages = hfac->nmessages;
  mvec **list = hfac->messagesin;
  int i;
  unsigned sender = msg->sender;
	mvec *deallocate;

  //Scan through list to see if sender is present - Wouldn't run if no msg
  for(i=0;i<nmessages;i++)
  {
    if(list[i]->sender == sender) //Overwrite - nmessages no change
    {
			list[i]->vector = ifree(E,list[i]->vector);
			list[i] = ifree(E,list[i]);
      list[i] = msg;
      return;
    }
  }

  //If not present
  mvec **newlist = (mvec **)imalloc(E,(nmessages+1)*sizeof(mvec*));
  for(i=0; i<nmessages; i++)
  {
    newlist[i] = list[i];
  }

  newlist[nmessages] = msg;

  //Free previous list
  hfac->messagesin=ifree(E,hfac->messagesin);

  //Append newlist to targetnode
  hfac->messagesin=newlist;
  hfac->nmessages++;

  return;
}

void deallocatemvec(mvec *msg)
{
	msg->vector = ifree(E,msg->vector);

	msg = ifree(E, msg);
	return;
}

void LSPaddmarginaltonode(mvec *marg, nodes *node)
{
	hfactor *hfac = (hfactor *)node->ndata;

	if(!(hfac->marginal)) //If marginal is present
	{
		hfac->marginal = marg;
	}
	else
	{
		hfac->marginal = ifree(E, hfac->marginal);
		hfac->marginal = marg;
	}

	return;
}

//-------------------------------------------------------------------------
//----------------------- Updators ------- --------------------------------
//-------------------------------------------------------------------------

void *initialburstvariable(nodes *varnode , hgph *graph)
{
  //---------------Creates unity vector
  //------ Length of unity vector corresponds to own number of states
  mvec *msg;
  hfactor *hfac = (hfactor *)varnode->ndata;
  int i,k;
  int length;
  double *vector;
  nodes *receivingnode;
  //Get all list of edges
  int nedges = varnode->nedges;
  unsigned *edge = varnode->edge;

	for (k=0;k<nedges;k++) // For every outgoing edge
  {
	  //Create msg out
	  msg = (mvec *)imalloc(E,1*sizeof(mvec));

	  //Determine number of states
	  //1. Find factornode
	  nodes *factorn = find_node(edge[0],graph->nnodes,graph->nodelist);
	  hfactor *hfactornode = (hfactor *)factorn->ndata;

	  if(hfactornode->columnlabel == varnode->nhash) //If column
	  {
	    length = hfactornode->ncol;
	  }
	  else
	  {
	    length = hfactornode->nrow;
	  }

	  vector = (double *)imalloc(E,length*sizeof(double));

	  for(i=0;i<length;i++) // For each element
	  {
	    vector[i] = 1;
	  }

	  msg->length = length;
	  msg->sender = varnode->nhash;
	  msg->vector = vector;

	  //Append to node
    receivingnode = find_node(edge[k],graph->nnodes,graph->nodelist);
    LSPaddmessagetonode(msg,receivingnode);
  }

  return NULL;
}

void UpdateVariableNode(nodes *VarNode, hgph *graph)
{
//------------ Cycle through all the edges to update each edge-------------

	hfactor *hfac = (hfactor *)VarNode->ndata;
	int nedges = VarNode->nedges;
	unsigned *edge = VarNode->edge;
	int i;
	nodes *outnode;
	mvec *msg;

	for(i=0;i<nedges;i++)
	{
		outnode = find_node(edge[i],graph->nnodes, graph->nodelist);
		hfactor *test = (hfactor *)outnode->ndata;
		msg = VariableNodeOutput(VarNode,outnode,graph);
		LSPaddmessagetonode(msg,outnode);
	}

	return;
}

void UpdateFactorNode(nodes *FacNode, hgph *graph)
{
	hfactor *hfac = (hfactor *)FacNode->ndata;
	int nedges = FacNode->nedges;
	unsigned *edge = FacNode->edge;
	int i;
	nodes *outnode;
	mvec *msg;

	for(i=0;i<nedges;i++)
	{
		outnode = find_node(edge[i],graph->nnodes, graph->nodelist);
		msg = FactorNodeOutput(FacNode,outnode,graph);
		LSPaddmessagetonode(msg,outnode);
	}

	return;
}

//--------------------------------------------------------------------------
//--------------------------- Message Creators -----------------------------
//--------------------------------------------------------------------------

mvec *VariableNodeOutput(nodes *varnode, nodes *outnode, hgph *graph)
{
  //---------------Product of messages
  hfactor *hfacvn = (hfactor *)varnode->ndata;
  hfactor *hfacon = (hfactor *)outnode->ndata;
  mvec *msg = (mvec *)imalloc(E,1*sizeof(mvec));
  int length;
  int i,j;
  int msgcheck = 0;
  double *vector;
  //-------Check to see if messages are present from all required nodes
  int nedges = varnode->nedges;
  unsigned *edges = varnode->edge;
  mvec **msgin = hfacvn->messagesin;
  int nmessages = hfacvn->nmessages;

  unsigned incomingedges[nedges-1];

//--------------------- IN CASE OF LEAF NODE --------------------------------
  if(nedges == 1) //Effectively a leaf --- Output Unity Vector
  { length = msgin[0]->length;
    vector = (double *)imalloc(E,length*sizeof(double));

    for(i=0;i<length;i++)
    {
      vector[i] = 1;
    }

    msg->length = length;
    msg->sender = varnode->nhash;
    msg->vector = vector;
    return msg;
  }
//-----------------------------------------------------------------------------
//------------------------Else ------------------------------------------------

  //Create list of incoming edges ----- EXCLUDE OUTGOING
  char outedgefound = 'f';
  for(i=0; i<nedges; i++)
  {
    if(outedgefound =='f')
    {
      if(edges[i]==outnode->nhash)
        { outedgefound = 't'; continue; }
      else incomingedges[i] = edges[i];
    }
    else incomingedges[i-1] = edges[i];
  }

  //Compare incoming edges and messages out
  mvec **listofmessagestoproduct= (mvec **)imalloc(E,(nedges-1)*sizeof(mvec*));
  for(i=0; i<nmessages ; i++) //Go down list of messages
  {
    for(j=0;j<(nedges-1);j++) //Go down list of incomingedges
    {
      if(incomingedges[j]==(msgin[i]->sender))
      {
        listofmessagestoproduct[j] = msgin[i];
        msgcheck++;
      }
    }
  }

  if(msgcheck != (nedges-1)) return NULL; // BREAKS IF NOT ALL MESSAGES PRESENT!

  //------Product of incoming messages-------------------------------------
  length = msgin[0]->length;
  vector = (double *)imalloc(E,length*sizeof(double));
  for(i=0;i<length;i++) // For each element
  {
    vector[i] = 1;
    for(j=0;j<(nedges-1);j++) //For each message
    {
      vector[i] = vector[i]*listofmessagestoproduct[j]->vector[i];
    }
  }

  //------- Create full message---------------------------------------------
  msg->length = length;
  msg->sender = varnode->nhash;
  msg->vector = vector;

  //---------- Normalise-----------------------------------------------------
  NormaliseMVEC(msg);

  //----Free
  listofmessagestoproduct = ifree(E,listofmessagestoproduct);
  return msg;
}

mvec *FactorNodeOutput(nodes *facnode, nodes *outnode, hgph *graph)
{
  //--------------Sum of rows / cols --------------------------------------
	//--------------REMEMBER FACTOR NODE ONLY HAS 2 Edges (In and Out)
  hfactor *hfacFN = (hfactor *)facnode->ndata;
  hfactor *hfacON = (hfactor *)outnode->ndata;
	hfactor *hfacIN;

	mvec *msg = (mvec *)imalloc(E,1*sizeof(mvec));
  double *probdist = hfacFN->probdist;
  int i,j,n;
	int length;
	double *vector;
	nodes *innode;

 	int nedges = facnode->nedges;
	unsigned *edges = facnode->edge;

//------------------------Find Incoming Edge---------------------------------
	unsigned incomingedge; //FINDS HASH OF INNODE # ONLY 1
  char incomingedgefound = 'f';
  for(i=0; i<nedges; i++)
  {
    if(incomingedgefound =='f')
    {
      if(edges[i]!=outnode->nhash)
			{
				incomingedge = edges[i];
				incomingedgefound = 't';
			}
  	}
	}

	innode = find_node(incomingedge,graph->nnodes,graph->nodelist);
	hfacIN = (hfactor *)innode->ndata;

//-----------------Find Incoming Message---------------------------------------
	mvec *incomingmsg;
	double *incomingvector;
	int nmessages = hfacFN->nmessages;
	mvec **messagesin = hfacFN->messagesin;

	for(i=0;i<nmessages;i++)
	{
			if(messagesin[i]->sender == innode->nhash)
			{
				incomingmsg = messagesin[i];
				incomingvector = incomingmsg->vector;
			}
	}

//-----------------------------Does Sum Product-----------------------------
//1. OutNode is observed
//2. Innode is observed but Outnode not observed
//3. Both not observed

	//============================= OutNode Observed =========================
	if(hfacON->observed == 't') //IF OUTNODE OBSERVED DOESN'T MATTER ABOUT ANYTHING ELSE
	{
		if(outnode->nhash == hfacFN->columnlabel) //If outnode is in column
		{
			length = hfacFN->ncol;
			vector = (double *)imalloc(E,length*sizeof(double));

			//Determine which state is observed
			for(i=0;i<length;i++)
			{
				vector[i] = 0;
				if(hfacON->observedvariable == hfacFN->columndiscretevalues[i]) vector[i] = 1;
			}

		}
		else
		{
			length = hfacFN->nrow;
			vector = (double *)imalloc(E,length*sizeof(double));

			//Determine which state is observed
			for(i=0;i<length;i++)
			{
				vector[i] = 0;
				if(hfacON->observedvariable == hfacFN->rowdiscretevalues[i]) vector[i] = 1;
			}
		}
	}
	//===================== Innode Observed ====================================
	else if(hfacIN->observed == 't') //IF IN NODE OBSERVED TAKE OBSERVED COLUMN/ROW
	{

		if(outnode->nhash == hfacFN->columnlabel) //If outnode is in column, innode in row
		{
			length = hfacFN->ncol;
			vector = (double *)imalloc(E,length*sizeof(double));

			n=0;
			while(n<(hfacFN->nrow))
			{
				if(hfacIN->observedvariable == hfacFN->rowdiscretevalues[n]) break;
				n++;
			}

			for(i=0;i<length;i++)
			{
				vector[i] = probdist[n*(hfacFN->ncol)+i];
			}

		}

		else //If outnode is in row, innode in column
		{
			length = hfacFN->nrow;
			vector = (double *)imalloc(E,length*sizeof(double));

			n=0;
			while(n<(hfacFN->ncol))
			{
				if(hfacIN->observedvariable == hfacFN->columndiscretevalues[n]) break;
				n++;
			}

			for(i=0;i<length;i++)
			{
				vector[i] = probdist[i*(hfacFN->ncol)+n];
			}

		}
	}
	//========================Innode not observed===============================
	else if(hfacIN->observed == 'f')
	{
		if(outnode->nhash == hfacFN->columnlabel) //If outnode is in column
		{
			length = hfacFN->ncol;
			vector = (double *)imalloc(E,length*sizeof(double));

			for(i=0;i<length;i++) //Go left to right
			{
				vector[i] = 0;
				for(j=0;j<(hfacFN->nrow);j++) //Go Down row
				{
					vector[i] = vector[i] + probdist[j*(hfacFN->ncol)+i]*incomingvector[j];
				}
			}

		}
		else //If outnode is in row
		{
			length = hfacFN->nrow;
			vector = (double *)imalloc(E,length*sizeof(double));

			for(j=0;j<length;j++) //Go left to right
			{
				vector[j] = 0;
				for(i=0;i<(hfacFN->ncol);i++) //Go down row
				{
					vector[j] = vector[j] + probdist[j*(hfacFN->ncol)+i]*incomingvector[i];
				}
			}
		}
	}
	//========================================================================

	//------- Create full message---------------------------------------------
  msg->length = length;
  msg->sender = facnode->nhash;
  msg->vector = vector;

  //---------- Normalise-----------------------------------------------------
  NormaliseMVEC(msg);

	return msg;
}

//-------------------------------------------------------------------------
//----------------------- Marginal Calculators ----------------------------
//-------------------------------------------------------------------------
void *LSPCalcMarginals(hgph *graph)
{
//--------- Creates final marginal
	nodes **nodelist = graph->nodelist;
	nodes *node;
	hfactor *hfac;
	int nnodes = graph->nnodes;
	int i,k,j;
	int nmessages;
	int length;
	mvec *marg, **msglist;
	double *vector;

	//MARGINAL == PRODUCT OF MESSAGES IN
	for(i=0;i<nnodes;i++) //For every node
	{
		node = nodelist[i];
		hfac = (hfactor *)node->ndata;

		if(hfac->type == 'v') // Only work if variable
		{
			nmessages = hfac->nmessages;
			msglist = hfac->messagesin;
			length = msglist[0]->length;
			marg = (mvec *)imalloc(E,1*sizeof(mvec));
			vector = (double *)imalloc(E,length*sizeof(double));
			msglist = hfac->messagesin;

			for(k=0;k<length;k++) //For each element
			{
				vector[k] = 1;
				for(j=0;j<nmessages;j++) //For each message
				{
					vector[k] = vector[k] * msglist[j]->vector[k];
				}
			}

			marg->length = length;
			marg->vector = vector;

			NormaliseMVEC(marg);

			LSPaddmarginaltonode(marg,node);
		}
	}
	return;
}

//--------------------------------------------------------------------------
//------------------------- Write to file ----------------------------------
//--------------------------------------------------------------------------

void LSPwritetofile(hgph *graph, void *xptr)
{
//--------------------Write results to file for debugging purposes---------------------------------
	int nnodes = graph->nnodes;
	FILE *fpointer;
	char filename[MAXLEN] = "LoopySumProductResults.txt";
	fpointer = fopen(filename,"w");

	nodes *node;
	hfactor *hfac;
	mvec *vec,*mag;
	char resultstring[MAXLEN];

	int i = 0;
	int j = 0;
	int k = 0;
	for(i;i<nnodes;i++) //For each node
	{
		node = graph->nodelist[i];
		hfac = (hfactor*)node->ndata;

	//Forward Traverse
		fprintf(fpointer, "***Node: %s\n", hfac->name);
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

	//Marginals
		if(hfac->type =='v')
		{
			fprintf(fpointer, "$$Marginals: \n");
			mag = hfac->marginal;
			for(j=0;j<(mag->length);j++)
			{
				fprintf(fpointer, "%f \t", mag->vector[j]);
			}
			fprintf(fpointer, "\n");
		}
		fprintf(fpointer, "------------------------------------------------------------------\n");

	}

	sprintf(resultstring, "File Output: %s", filename);
	printtoclient(resultstring, xptr);
	fclose(fpointer);

	return;
}
