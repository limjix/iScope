#include "john.h"

  // ************************************** //
  // ** John's Library of Test Functions ** //
  // ************************************** //

void *john_testhelloworld(void *arglist)
{
  // ***************************** //
  // ** Hello World Test Script	** //
  // ***************************** //

	void *xptr;
	void *callback;
	char string[MAXLEN];

	if(dynamic_getarg(arglist,"xclient",&xptr)=='f') return NULL;

	sprintf(string,"Hello World\n");

	arglist=NULL;
	dynamic_putarg("std.void","xclient",xptr,SZ,&arglist);
	dynamic_putarg("std.char","string",(void *) string,SZ,&arglist); 
	dynamic_call("xclient","xclient_callback_write",'s',arglist,&callback);
	dynamic_wait(callback,NULL);
	dynamic_closeargs(arglist);

   return NULL;
}

void *john_tnodes(void *arglist)
{
  // ****************************** //
  // ** Graph Library Test Script** //
  // ****************************** //

   int nnodes;
   nodes **nodelist;
   double pos[2];
   nodes *node1,*node2,*node3,*jnode,*inode;
   int i;
	arglist=NULL;
	nodelist=NULL;
	nnodes = 0;

   //#1 Create 3 Nodes
	pos[0]=10.0;
  	pos[1]=4.0;
	add_node(str_hash("node1"),pos,&nnodes,&nodelist);

	pos[0]=15.0;
  	pos[1]=7.0;
	add_node(str_hash("node2"),pos,&nnodes,&nodelist);

	pos[0]=22.0;
  	pos[1]=2.0;
	add_node(str_hash("node3"),pos,&nnodes,&nodelist);


   //#2 Find those poor little nodes
	node1=find_node(str_hash("node1"),nnodes,nodelist);
	node2=find_node(str_hash("node2"),nnodes,nodelist);
	node3=find_node(str_hash("node3"),nnodes,nodelist);

   //#3 Give them a directed edge
	add_diredge(node1,node2);
	add_diredge(node1,node3);

   //#4 Output to file
	FILE * pfile;
	pfile = fopen("tnode.dat","w");

	for(i=0;i<nnodes;i++)
	{
	  inode=nodelist[i];

          if(inode->nedges>0)
          {
	    jnode=find_node(inode->edge[0],nnodes,nodelist);

	    fprintf(pfile,"%f %f\n",inode->vecr[0],inode->vecr[1]);
	    fprintf(pfile,"%f %f\n",jnode->vecr[0],jnode->vecr[1]);
	    fprintf(pfile,"\n");
          }
	}

	fclose(pfile);

	dynamic_closeargs(arglist);
	return NULL;
}


/*   Kalman

*/

void *john_kalmaninit(void *arglist)
{	
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

void *john_kalmancreatenodes(void *arglist)
{
	void *argptr;
	nodes *x, *mu, *prevmu;
	hgph *graph;
	double *input, pos[2], prevmuval, xval, *average;
	double userinput;
	char strmu[MAXLEN], strx[MAXLEN], strprevmu[MAXLEN]; 

	//Get an argument of user input
	if(dynamic_getarg(arglist,"userinput",&argptr)=='f') return NULL;
	if(!invalidptr(E,argptr)) userinput=*((double *) argptr);

	//Get previous hgph struct
	if(dynamic_getarg(arglist,"hgph",&argptr)=='f') return NULL;
	if(!invalidptr(E,argptr)) graph=(hgph *) argptr;

	//Allocate memory for xndata and mundata
	input=(double *)imalloc(E,sizeof(double));
	if(invalidptr(E,input)) return NULL;
	average=(double *)imalloc(E,sizeof(double));
	if(invalidptr(E,input)) return NULL;
	*input = userinput;	
	
	//Current node iteration (Always starts with 1)
	int i = (graph->nnodes/2)+1;

   	//Creates node mu
	pos[0]=i*5;    //x
  	pos[1]=15;     //y
	sprintf(strmu, "mu%d", i);
	add_node(str_hash(strmu),pos,&(graph->nnodes),&(graph->nodelist));
	mu=find_node(str_hash(strmu),graph->nnodes,graph->nodelist);
	
	//Creates node x
	pos[0]=i*5;    //x
  	pos[1]=1;      //y
	sprintf(strx, "x%d", i);
	add_node(str_hash(strx),pos,&(graph->nnodes),&(graph->nodelist));
	x=find_node(str_hash(strx),graph->nnodes,graph->nodelist);

	//Adds directed edge between mu and x
	add_diredge(mu,x);
	
	prevmu = NULL;
	//If previous mu exists, links current mu to previous mu
	if(graph->nnodes>2)
	{	
		sprintf(strprevmu, "mu%d", i-1);
		prevmu = find_node(str_hash(strprevmu),graph->nnodes,graph->nodelist);
		add_diredge(prevmu,mu);
	}	


	//Populates x payload with uservalue
	x->ndata= input;

	//Average mu and save
	if(prevmu != NULL)
	{
	prevmuval = *((double*) prevmu->ndata);
	xval = *((double*) x->ndata);
	*average = ((i-1)*(prevmuval)+(xval))/i;
	mu->ndata= average;
	}
	else
	{
	mu->ndata=input;
	}

	//Output to arglist
	arglist=NULL;
	dynamic_putarg("graph.hgph","hgph",(void*) graph,SZ,&arglist);

	return arglist; 

}


void *john_makeplotvector(void *arglist)
{
	void *argptr;
	hgph *graph;
	hvec *vector;
	nodes *mu;
	char strmu[MAXLEN];
	double *muval;
	int index;

	//Get previous hgph struct
	if(dynamic_getarg(arglist,"hgph",&argptr)=='f') return NULL;
	if(!invalidptr(E,argptr)) graph=(hgph *) argptr;

	//Allocates memory for vector
	vector=(hvec *)imalloc(E,sizeof(hvec));
	if(invalidptr(E,vector)) return NULL;

	//Some init parameters (Needed in order to use xclient view - all must be filled)
	vector->mn=(int *)imalloc(E,2*sizeof(int));
	vector->mx=(int *)imalloc(E,2*sizeof(int));
	if(invalidptr(E,vector->mn)) return NULL;
	if(invalidptr(E,vector->mx)) return NULL;
	vector->mn[0] = 75;
	vector->mn[1] = 75;
	vector->mx[0] = 85;
	vector->mx[1] = 85;
	vector->ndims = 2;
        vector->nvals=0;
	vector->dimmem ='t';
	vector->valmem='f';
        vector->posmem='t';
	
	//Current node iteration
	int i = (graph->nnodes/2);
	vector->npts = i;

	//Allocate memory for vec->dpt
	vector->dpt=(double* )imalloc(E,vector->npts*2*sizeof(double));
	if(invalidptr(E,vector->dpt)) return NULL;
	
	//Finds nodes iteratively and collects plot data
	for(index = 1; index<=i; index++)
	{
		//Finds mu
		sprintf(strmu, "mu%d", index);		
		mu=find_node(str_hash(strmu),graph->nnodes,graph->nodelist);
		muval = (double*) mu->ndata;
		
		//Adds data to vector to plot
		vector->dpt[index] = index;
		vector->dpt[vector->npts+index] = *muval;
	} 

	//Output to arglist
	arglist=NULL;
	dynamic_putarg("vector.hvec","hvec",(void*)vector,SZ,&arglist);
	return arglist;

}

void *john_nodeplot(void *arglist)
{	
  int idim;
  hgph *gph;

  void *argptr;
  hvec *vec;

  // ******************************** //
  // ** RUN DELAUNAY TRIANGULATION ** //
  // ******************************** //


  //Gets hvec
  if(dynamic_getarg(arglist,"hvec",&argptr)=='f')
  {
    printf("delaunay: no points to triangulate\n");
    return NULL;
  }

  if(invalidptr(E,argptr)) return NULL;
  vec=(hvec *) argptr;

  //Gets hgph structure
  if(dynamic_getarg(arglist,"hgph",&argptr)=='f') return NULL;
  if(!invalidptr(E,argptr)) gph=(hgph *) argptr;

  printf("delaunay: triangulating\n");

  //allocate space for output
  //gph=NULL;
  //gph=(hgph *)imalloc(E,sizeof(hgph));
  //if(invalidptr(E,gph)) return NULL;

  if(invalidptr(E,vec->dpt)) return NULL;
  /* YOUR GRAPH INITIALISATION*/

  //copying this bounding information is crucial to set display
  gph->ndims=vec->ndims;
  if(gph->dimmem!='t'&&vec->ndims>0)
  {
    gph->mn=NULL;
    gph->mx=NULL;
    gph->mn=(int *)imalloc(E,vec->ndims*sizeof(int));
    gph->mx=(int *)imalloc(E,vec->ndims*sizeof(int));
    if(invalidptr(E,gph->mn)) return NULL;
    if(invalidptr(E,gph->mx)) return NULL;
    memset(gph->mn,0,vec->ndims*sizeof(int));
    memset(gph->mx,0,vec->ndims*sizeof(int));

    for(idim=0;idim<vec->ndims;idim++)
    {
      gph->mn[idim]=vec->mn[idim];
      gph->mx[idim]=vec->mx[idim];
    }
  }

  gph->nodemem='t';

  printf("delaunay: finished\n");

  arglist=NULL;
  dynamic_putarg("graph.hgph","hgph",(void *) gph,SZ,&arglist);
  return arglist;

}

//Whenever you need to store data into a structure, you need to allocate memory..
//If not the pointer points to some irrelevant box because the stack has been wiped.

//--------------------------------------------------------------------------------------------------
//----------------------------------- Some utilities------------------------------------------------
//--------------------------------------------------------------------------------------------------

void add_hfactortondata(nodes *curr,hfactor *hfac)
{  // ******************************** //
  // ** Appends hfactor to ndata ** //
  // ******************************** //
	if(invalidptr(E,curr)) return;
        if(invalidptr(E,hfac)) return;
	curr->ndata = hfac;

}

hfactor *create_hfactor()
{  // ********************************************* //
  // ** Create a Hfactor node , allocates memory ** //
  // ********************************************** //
	hfactor *newhfac;
	newhfac =(hfactor *)imalloc(E,sizeof(hfactor));
	if(invalidptr(E,newhfac)) return;

	return newhfac;
}

nodes *createnode(int x, int y,char str[MAXLEN],hgph *graph)
{	
//--------------- Easy way to create nodes -------------------------

	double pos[2];
	nodes *node;
	if(invalidptr(E,graph)) return NULL;

	pos[0]=x;     //x
  	pos[1]=y;     //y
	add_node(str_hash(str),pos,&(graph->nnodes),&(graph->nodelist));
	node=find_node(str_hash(str),graph->nnodes,graph->nodelist);
	
	return node;
}


void *john_testfunc(void *arglist)
{

}

//--------------------------------------------------------------------------------------------------
//----------------------------------- Sum Product Algorithm ----------------------------------------
//--------------------------------------------------------------------------------------------------

void *john_sumproductinit(void *arglist)
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

void *john_setupproblem(void *arglist)
{
//--------------------------- Creates nodes and sets up model---------------------------------------
	void *argptr;
	nodes *x1, *fa, *x2, *fb, *x3, *fc, *x4;
	hgph *graph;

	//Get previous hgph struct
	if(dynamic_getarg(arglist,"hgph",&argptr)=='f') return NULL;
	if(!invalidptr(E,argptr)) graph=(hgph *) argptr;

	//Sets up nodes in their places
	x1 = createnode(1,5,"x1",graph);
	fa = createnode(3,5,"fa",graph);
	x2 = createnode(5,5,"x2",graph);
	fb = createnode(7,5,"fb",graph);
	x3 = createnode(9,5,"x3",graph);
	fc = createnode(5,3,"fc",graph);
	x4 = createnode(5,1,"x4",graph);

	//Adds directed edge between mu and x
	add_edge(x1,fa);
	add_edge(fa,x2);
	add_edge(x2,fb);
	add_edge(fb,x3);
	add_edge(x4,fc);
	add_edge(fc,x2);

	//Adds hfactor to ndata
	add_hfactortondata(x1,create_hfactor());
	add_hfactortondata(fa,create_hfactor());
	add_hfactortondata(x2,create_hfactor());
	add_hfactortondata(fb,create_hfactor());
	add_hfactortondata(x3,create_hfactor());
	add_hfactortondata(fc,create_hfactor());
	add_hfactortondata(x4,create_hfactor());

	//Populate Variable Payloads & specify type
	((hfactor *)x1->ndata)->type = 'v';
	((hfactor *)x2->ndata)->type = 'v';
	((hfactor *)x3->ndata)->type = 'v';
	((hfactor *)x4->ndata)->type = 'v';

	((hfactor *)fa->ndata)->type = 'f';
	((hfactor *)fb->ndata)->type = 'f';
	((hfactor *)fc->ndata)->type = 'f';

	//Create probability distribution table
	/*Row Major Ordering
	EG:
	{11 12 13
	 21 22 23
	 31 32 33}
	Becomes
	{11 12 13 21 22 23 31 32 33}
	*/

	double *probdist1;
	probdist1 =(double *)imalloc(E,4*sizeof(double));

	probdist1[0] = 1.0;
	probdist1[1] = 0.1;
	probdist1[2] = 0.1;
	probdist1[3] = 1.0;

	double *probdist2;
	probdist2 =(double *)imalloc(E,4*sizeof(double));

	probdist2[0] = 1.0;
	probdist2[1] = 0.9;
	probdist2[2] = 0.9;
	probdist2[3] = 1.0;

	double *probdist3;
	probdist3 =(double *)imalloc(E,4*sizeof(double));

	probdist3[0] = 0.1; 
	probdist3[1] = 1.0;
	probdist3[2] = 1.0;
	probdist3[3] = 0.1;

/*	probdist3[0] = 1.0;	
	probdist3[1] = 0.1;
	probdist3[2] = 0.4;
	probdist3[3] = 0.5;
	probdist3[4] = 1.0;
	probdist3[5] = 0.1;
	probdist3[6] = 0.9;
	probdist3[7] = 0.3;
	probdist3[8] = 0.2;
	probdist3[9] = 0.1;
	probdist3[10] = 0.3;
	probdist3[11] = 0.1;
*/

	//Populate factor payloads with probability distribution
	double *discretevalue = (double *)imalloc(E,2*sizeof(double));
	discretevalue[0] = 0;
	discretevalue[1] = 1;

	((hfactor *)fa->ndata)->probdist = probdist2;
	((hfactor *)fa->ndata)->columndiscretevalues = discretevalue;
	((hfactor *)fa->ndata)->rowdiscretevalues = discretevalue;
	((hfactor *)fa->ndata)->columnlabel = str_hash("x1");
	((hfactor *)fa->ndata)->rowlabel = str_hash("x2");
	((hfactor *)fa->ndata)->nrow = 2;
	((hfactor *)fa->ndata)->ncol = 2;

	((hfactor *)fb->ndata)->probdist = probdist3;
	((hfactor *)fb->ndata)->columndiscretevalues = discretevalue;
	((hfactor *)fb->ndata)->rowdiscretevalues = discretevalue;
	((hfactor *)fb->ndata)->columnlabel = str_hash("x2");
	((hfactor *)fb->ndata)->rowlabel = str_hash("x3");
	((hfactor *)fb->ndata)->nrow = 2;
	((hfactor *)fb->ndata)->ncol = 2;

	((hfactor *)fc->ndata)->probdist = probdist1;
	((hfactor *)fc->ndata)->columndiscretevalues = discretevalue;
	((hfactor *)fc->ndata)->rowdiscretevalues = discretevalue;
	((hfactor *)fc->ndata)->columnlabel = str_hash("x2");
	((hfactor *)fc->ndata)->rowlabel = str_hash("x4");
	((hfactor *)fc->ndata)->nrow = 2;
	((hfactor *)fc->ndata)->ncol = 2;

	//Create initialised vector for input messages
	/*double *initmessage1 = (double *)imalloc(E,2*sizeof(double));
	initmessage1[0]=1.0;
	initmessage1[1]=2.0;

	double *initmessage2 = (double *)imalloc(E,2*sizeof(double));
	initmessage2[0]=3.0;
	initmessage2[1]=4.0;

	double *initmessage3 = (double *)imalloc(E,2*sizeof(double));
	initmessage3[0]=5.0;
	initmessage3[1]=6.0;
	
	double *initmessage4 = (double *)imalloc(E,2*sizeof(double));
	initmessage4[0]=10.0;
	initmessage4[1]=10.0;

	mvec *message1 = (mvec *)imalloc(E,1*sizeof(mvec));
	mvec *message2 = (mvec *)imalloc(E,1*sizeof(mvec));
	mvec *message3 = (mvec *)imalloc(E,1*sizeof(mvec));
	mvec *message4 = (mvec *)imalloc(E,1*sizeof(mvec));
	message1->length = 2;
	message1->vector = initmessage1;
	message2->length = 2;
	message2->vector = initmessage2;
	message3->length = 2;
	message3->vector = initmessage3;
	message4->length = 2;
	message4->vector = initmessage4;
	*/

	//Observed variable nodes
	((hfactor *)x1->ndata)->observed = 'f';
	((hfactor *)x2->ndata)->observed = 'f';
	((hfactor *)x3->ndata)->observed = 'f';
	((hfactor *)x4->ndata)->observed = 'f';

	//Initialise lists for messages in
	((hfactor *)x1->ndata)->messagesin = (mvec **)imalloc(E,1*sizeof(mvec*));
	((hfactor *)fa->ndata)->messagesin = (mvec **)imalloc(E,1*sizeof(mvec*));
	((hfactor *)x2->ndata)->messagesin = (mvec **)imalloc(E,1*sizeof(mvec*));
	((hfactor *)fb->ndata)->messagesin = (mvec **)imalloc(E,1*sizeof(mvec*));
	((hfactor *)x3->ndata)->messagesin = (mvec **)imalloc(E,1*sizeof(mvec*));
	((hfactor *)fc->ndata)->messagesin = (mvec **)imalloc(E,1*sizeof(mvec*));
	((hfactor *)x4->ndata)->messagesin = (mvec **)imalloc(E,1*sizeof(mvec*));
	
	((hfactor *)x1->ndata)->nmessages = 0;
	((hfactor *)fa->ndata)->nmessages = 0;
	((hfactor *)x2->ndata)->nmessages = 0;
	((hfactor *)fb->ndata)->nmessages = 0;
	((hfactor *)x3->ndata)->nmessages = 0;
	((hfactor *)fc->ndata)->nmessages = 0;
	((hfactor *)x4->ndata)->nmessages = 0;

	//Test
/*	double *testx2val = (double *)imalloc(E,4*sizeof(double));
	double *testx3val = (double *)imalloc(E,3*sizeof(double));
	testx2val[0] = 1;
	testx2val[1] = 2;
	testx2val[2] = 3;
	testx2val[3] = 4;

	testx3val[0] = 1;
	testx3val[1] = 2;
	testx3val[2] = 3;

	((hfactor *)fb->ndata)->columndiscretevalues = testx2val;
	((hfactor *)fb->ndata)->rowdiscretevalues = testx3val;	
	((hfactor *)x2->ndata)->observed = 't';
	((hfactor *)x2->ndata)->observedvariable = 2;

	mvec *test= sumobservednode(fb, x2);
*/
	//Output to arglist
	arglist=NULL;
	dynamic_putarg("graph.hgph","hgph",(void *)graph,SZ,&arglist);

	return arglist;
}

void *john_sumproductalgorithm(void *arglist)
{
	void *argptr;
	hgph *graph;
	nodes *root;
	//Get previous hgph struct
	if(dynamic_getarg(arglist,"hgph",&argptr)=='f') return NULL;
	if(!invalidptr(E,argptr)) graph=(hgph *) argptr;

	//Choose the root node
	root = find_node(str_hash("x2"),graph->nnodes,graph->nodelist);

	//Call the recursion step
	forwardtraverse(root,root,graph);

	//Test
	nodes *x1 = find_node(str_hash("x1"),graph->nnodes,graph->nodelist);
	nodes *x2 = find_node(str_hash("x2"),graph->nnodes,graph->nodelist);
	nodes *x3 = find_node(str_hash("x3"),graph->nnodes,graph->nodelist);
	nodes *x4 = find_node(str_hash("x4"),graph->nnodes,graph->nodelist);
	nodes *fa = find_node(str_hash("fa"),graph->nnodes,graph->nodelist);
	nodes *fb = find_node(str_hash("fb"),graph->nnodes,graph->nodelist);
	nodes *fc = find_node(str_hash("fc"),graph->nnodes,graph->nodelist);

	hfactor *hx1 = (hfactor*)x1->ndata;
	hfactor *hx2 = (hfactor*)x2->ndata;
	hfactor *hx3 = (hfactor*)x3->ndata;
	hfactor *hx4 = (hfactor*)x4->ndata;
	hfactor *hfa = (hfactor*)fa->ndata;
	hfactor *hfb = (hfactor*)fb->ndata;
	hfactor *hfc = (hfactor*)fc->ndata;

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

	//when 1st called, every edge is a forward edge
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
			msgveclength = ((hfactor *)callingnode->ndata)->nrow;
		}
		else
		{
			msgveclength = ((hfactor *)callingnode->ndata)->ncol;
		}

		//Allocate memory for vector and initialise value to 1 as per theory
		double *msgvec = (double *)imalloc(E,msgveclength*sizeof(double));
		int i = 0;
		for(i;i<msgveclength;i++)
		{
			msgvec[i] = 1;
		}		
		message->vector = msgvec;
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
			mvec *message = productofmessagesin(currentnode); //Products all the factor messages
			addmessagetonode(message, callingnode);
			return;			
		}

		else if(type == 'f') //factor --- sum marginals
		{

			if(((hfactor*)nextnode->ndata)->observed == 'f') //If the previous node was unobserved
			{
				mvec *message = SumRowsOrCols(currentnode, nextnode);		
				addmessagetonode(message,callingnode);
			}
			else //If the previous node was observed - take only the associated row or column
			{
				mvec *message = sumobservednode(currentnode, nextnode);
				addmessagetonode(message,callingnode);
			}
			return;
		}
	}
}

void backwardtraverse(nodes *currentnode,nodes *callingnode, hgph *graph)
{
//--------------------------Go down and update nodes


}

void calculatemarginals()
{
//-----------------------Calculate marginals at all the nodes

}

//--------------------------------------------------------------------------------------------------
//------------------------------ Supplementary Sum Product Functions -------------------------------
//--------------------------------------------------------------------------------------------------

void addmessagetonode(mvec *messageptr, nodes *targetnode)
{
//-----------------------Adds message to targetnode->messagesin;
	
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

void writeresultstofile(hgph *graph)
{
//--------------------Write results to file for debugging purposes---------------------------------
	int nnodes = graph->nnodes;	
	FILE *fpointer;
	fpointer = fopen("SumProductResults.txt","w");

	nodes *node;
	hfactor *hfac;
	mvec *vec;	

	int i = 0;
	int j = 0;
	int k = 0;
	for(i;i<nnodes;i++) //For each node
	{
		node = graph->nodelist[i];
		hfac = (hfactor*)node->ndata;
				

		fprintf(fpointer, "Node: %d\n", node->nhash); 
		fprintf(fpointer, "Nmessages: %d\n", hfac->nmessages);

		for(j=0;j<hfac->nmessages;j++) //For each message
		{		
			vec = hfac->messagesin[j];			
			fprintf(fpointer, "Message #%d \n", j);
			
			for(k=0;k<vec->length;k++) //For each vector element
			{
				fprintf(fpointer, "%f \t", vec->vector[k]);
			}
			fprintf(fpointer, "\n");
		}
		fprintf(fpointer, "\n");
	}

	fclose(fpointer);

	return;
}
//--------------------------------------------------------------------------------------------------
//----------------------------------- Linear Algebra Functions -------------------------------------
//--------------------------------------------------------------------------------------------------

mvec *SumRowsOrCols(nodes *factornode, nodes *previousnode)
{ //Sums rows of columns of a matrix represented as a vector
  //Specify 'r' for sum rows or 'c' for sum columns
  //Produces array
  //Vector is assumed to be Row Major
	
	int i , j; //i is for row , j is for column corresponding to Matrix[i,j]
	double* result;
	hfactor* fnhfac = (hfactor *)factornode->ndata;
	hfactor* pnhfac = (hfactor *)previousnode->ndata;
	mvec *message = (mvec *)imalloc(E,1*sizeof(mvec));
	int nrow = fnhfac->nrow;
	int ncol = fnhfac->ncol;
	double *matrix = fnhfac->probdist;

	//Determine if row or column needs to be summed out
	if( fnhfac->columnlabel == previousnode->nhash ) //if previous node is in column, vector is column but with length of nrow
	{
		result = (double *)imalloc(E,nrow*sizeof(double));

		i=0;
		for(i;i<nrow;i++)
		{		
			result[i]=0;			
			j=0;
			for(j;j<ncol;j++)
			{
				result[i]=result[i]+matrix[i*ncol+j];	
			}
		}
		message->length = nrow;
	}
	else  //if previous node is in row, the vector is row but with length of column
	{
		result = (double *)imalloc(E,ncol*sizeof(double));

		j=0;
		for(j;j<ncol;j++)
		{		
			result[j]=0;			
			i=0;
			for(i;i<nrow;i++)
			{
				result[j]=result[j]+matrix[i*ncol+j];	
			}
		}
		message->length = ncol;
	}
	
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

	answer->length = length;
	answer->vector = vect;
	return answer;
}

mvec *productofmessagesin(nodes *targetnode)
{
//------------------Returns the product of the messages which are in mvecs
//------------------Will be called at variable nodes
	hfactor* tnodehfac = (hfactor *)targetnode->ndata;
	mvec** list = tnodehfac->messagesin;
	int nmessages = tnodehfac->nmessages;
	int length;
	mvec *product = (mvec *)imalloc(E,1*sizeof(mvec));;

	if(nmessages == 0) //If there are no messages in that node, a scalar of 1 is produced
	{
		product->length = 1;
		product->vector = (double *)imalloc(E,1*sizeof(double));	
		product->vector[0] = 1;
	}
	else if(nmessages == 1) //If there is 1 message, it is just the same vector
	{
		product->length = tnodehfac->messagesin[0]->length;
		product->vector = tnodehfac->messagesin[0]->vector;
	}
	else //If there are multiple messages, product one by one
	{	
		length = list[0]->length;
		double vect[length];	
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
		product->length = length;
		product->vector = vect;
		
	/*	//Need to initialise a unity vector
		product->length = (tnodehfac->messagesin[0])->length;

		int i = 0;
		for(i;i<product->length;i++) //fill the unity vector with 1
		{
			product->vector[i] = 1;
		}

		//Do the dot product
		i = 0;
		for(i; i< nmessages-1; i++) //If I had n vectors, I need to product them n-1 times
		{
			product = productofvectors(list[i], product);
		}
	*/
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





