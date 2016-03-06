#include "loopySP.h"

void *loopySP_algorithm(void *arglist)
{
	void *argptr, *argptr2, *xptr;
	hgph *graph;
	nodes *root;
	char *userinputroot;
  int i,k;
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

  //Scan through list to see if sender is present - Wouldn't run if no msg
  for(i=0;i<nmessages;i++)
  {
    if(list[i]->sender == sender) //Overwrite - nmessages no change
    {
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

//-------------------------------------------------------------------------
//----------------------- Message Creators --------------------------------
//-------------------------------------------------------------------------

void *initialburstvariable(nodes *varnode , hgph *graph)
{
  //---------------Creates unity vector
  //------ Length of unity vector corresponds to own number of states
  mvec *msg;
  hfactor *hfac = (hfactor *)varnode->ndata;
  int i;
  int length;
  double *vector;
  nodes *receivingnode;
  //Get all list of edges
  int nedges = varnode->nedges;
  unsigned *edge = varnode->edge;

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

  //Append to nodes
  for (i=0;i<nedges;i++) // For every outgoing edge
  {
    receivingnode = find_node(edge[i],graph->nnodes,graph->nodelist);
    LSPaddmessagetonode(msg,receivingnode);
  }

  return NULL;
}

//--------------------------------------------------------------------------

mvec *VariableNodeOutput(nodes *varnode, nodes *outnode)
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

  //Create list of incoming edges
  char outedgefound = 'f';
  for(i=0; i<nedges; j++)
  {
    if(outedgefound =='f')
    {
      if(edges[j]==outnode->nhash)
        { outedgefound = 't'; continue; }
      else incomingedges[j] = edges[j];
    }
    else incomingedges[j-1] = edges[j];
  }

  //Compare incoming edges and messages out
  mvec **listofmessagestoproduct= (mvec **)imalloc(E,(nedges-1)*sizeof(mvec *));
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

  //------Product of incoming messages
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

  //------- Create full message
  msg->length = length;
  msg->sender = varnode->nhash;
  msg->vector = vector;

  //---------- Normalise
  NormaliseMVEC(msg);

  //----Free
  listofmessagestoproduct = ifree(E,listofmessagestoproduct);
  return msg;
}

mvec *FactorNodeOutput(nodes *facnode, nodes *outnode)
{
  //--------------Sum of rows / cols --------------------------------------
  hfactor *hfacFN = (hfactor *)facnode->ndata;
  hfactor *hfacON = (hfactor *)outnode->ndata;

  double *probdist = hfacFN->probdist;
  int i;
  
 	int nedges = facnode->nedges;
	unsigned *edges = facnode->edge;

//-----Check to see if all messages present
	 //Create list of incoming edges
	 unsigned incomingedges[nedges-1];
  char outedgefound = 'f';
  for(i=0; i<nedges; j++)
  {
    if(outedgefound =='f')
    {
      if(edges[j]==outnode->nhash)
        { outedgefound = 't'; continue; }
      else incomingedges[j] = edges[j];
    }
    else incomingedges[j-1] = edges[j];
  }

  //Compare incoming edges and messages out
  mvec **listofmessagestoproduct= (mvec **)imalloc(E,(nedges-1)*sizeof(mvec *));
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

}

//-------------------------------------------------------------------------
//----------------------- Marginal Calculators ----------------------------
//-------------------------------------------------------------------------

void *LSPCalcMarginals(hgph *graph)
{
//--------- Creates final marginal


}
