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
	x4 = createnode(5,1,"y2",graph);


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
//	((hfactor *)x1->ndata)->test = 1;

	//Populate Variable Payloads & specify type
	((hfactor *)x1->ndata)->type = 'v';
	((hfactor *)x2->ndata)->type = 'v';
	((hfactor *)x3->ndata)->type = 'v';
	((hfactor *)x4->ndata)->type = 'v';

	((hfactor *)fa->ndata)->type = 'f';
	((hfactor *)fb->ndata)->type = 'f';
	((hfactor *)fc->ndata)->type = 'f';

	//Create probability distribution table
	double *probdist1;
	probdist1 =(double *)imalloc(E,12*sizeof(double));

	probdist1[0] = 1.0;
	probdist1[1] = 0.5;
	probdist1[2] = 0.9;
	probdist1[3] = 0.4;
	probdist1[4] = 0.2;
	probdist1[5] = 3.0;
	probdist1[6] = 2.3;
	probdist1[7] = 9.0;
	probdist1[8] = 0.7;
	probdist1[9] = 0.1;
	probdist1[10] = 1.2;
	probdist1[11] = 1.7;

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

	//Populate Factor Payloads
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
	((hfactor *)fa->ndata)->nrow = 2;
	((hfactor *)fa->ndata)->ncol = 2;

	((hfactor *)fc->ndata)->probdist = probdist1;
	((hfactor *)fc->ndata)->columndiscretevalues = discretevalue;
	((hfactor *)fc->ndata)->rowdiscretevalues = discretevalue;
	((hfactor *)fc->ndata)->columnlabel = str_hash("x2");
	((hfactor *)fc->ndata)->rowlabel = str_hash("x4");
	((hfactor *)fa->ndata)->nrow = 2;
	((hfactor *)fa->ndata)->ncol = 2;

	double* testingsum = SumRowsOrCols(probdist1,'c', 3, 4);

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
	root = find_node(str_hash("fa"),graph->nnodes,graph->nodelist);

	//Call the recursion step
//	forwardtraverse(root,root, graph);

	return;
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
	unsigned forwardedges[nforwardbranches]; //list of edges to pursue
	nodes *nextnode;
	
	//----If at leaf go up-----
	if(nforwardbranches==0)  
	{	
		*(((hfactor *)currentnode->ndata)->forwardtraverse)=1.0;		
		return;
	}
	
	//----If not at leaf, go down ----------
	else	              
	{	
	//Copy edge list & remove calling node from the list
		int j = 0;
		char callingnodefound = 'f';
		for( j; j<nbranches; j++)
		{
			if(callingnodefound ='f')
			{
			  if(currentnode->edge[j]==callingnode->nhash) 
			    { callingnodefound = 't'; continue; }
			  else forwardedges[j] = currentnode->edge[j];
			}
			else forwardedges[j-1] = currentnode->edge[j];
		}

	//RECURSION DOWN
		int i=0;
		for(i; i<nforwardbranches; i++)
		{	
			nextnode = find_node(forwardedges[i],graph->nnodes,graph->nodelist);
			forwardtraverse(nextnode,currentnode,graph);
		}
	//RETURNING FROM RECURSION

	//What happens on this level before going up
		if(type == 'v') //variable -- product of factor messages
		{
			
			return;			
		}

		else if(type == 'f') //factor --- sum marginals
		{
			
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

void computefactormessage()
{

}

void computevariablemessage()
{

}

//--------------------------------------------------------------------------------------------------
//----------------------------------- Linear Algebra Functions -------------------------------------
//--------------------------------------------------------------------------------------------------

double *SumRowsOrCols(double *matrix,char specify, int nrow, int ncol)
{ //Sums rows of columns of a matrix represented as a vector
  //Specify 'r' for sum rows or 'c' for sum columns
  //Produces array
  //Vector is assumed to be Row Major
	
	int i , j; //i is for row , j is for column corresponding to Matrix[i,j]
	double* result;

	//Allocate memory
	if(specify == 'r') result = (double *)imalloc(E,nrow*sizeof(double));
	else if(specify =='c') result = (double *)imalloc(E,ncol*sizeof(double));

	if(specify == 'r') //if sum rows
	{
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
	}
	
	else if(specify =='c') //if sum columns
	{
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
	}
	
	return result;
}









