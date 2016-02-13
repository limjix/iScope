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

nodes *createnode(int x, int y,char str[MAXLEN],hgph *graph, char type)
{
//--------------- Easy way to create nodes -------------------------

	double pos[2];
	nodes *node;
	if(invalidptr(E,graph)) return NULL;

	//Create the node
	pos[0]=x;     //x
  	pos[1]=y;     //y
	add_node(str_hash(str),pos,&(graph->nnodes),&(graph->nodelist));
	node=find_node(str_hash(str),graph->nnodes,graph->nodelist);

	//Create hfactor & append
	hfactor *newhfac;
	newhfac =(hfactor *)imalloc(E,sizeof(hfactor));
	if(invalidptr(E,newhfac)) return;

	node->ndata = newhfac;

	//Specify type
	((hfactor *)node->ndata)->type = type;

	//Specify name
	int namelength = strlen(str);
	char *name = (char *)imalloc(E,namelength*sizeof(char));
	name = str;
	((hfactor *)node->ndata)->name = name;

	//Initialise to Null the matrices
	((hfactor *)node->ndata)->probdist = NULL;
	((hfactor *)node->ndata)->columndiscretevalues = NULL;
	((hfactor *)node->ndata)->rowdiscretevalues = NULL;
	((hfactor *)node->ndata)->lnprobdist = NULL;

	//Initialised Observed to false
	((hfactor *)node->ndata)->observed = 'f';

	//Initialise nmessages to zero
	((hfactor *)node->ndata)->nmessages = 0;
	((hfactor *)node->ndata)->nbmessages = 0;
	((hfactor *)node->ndata)->nMSmsgin = 0;

	//Allocate memory for mvec and append
	((hfactor *)node->ndata)->messagesin = (mvec **)imalloc(E,1*sizeof(mvec*));
	((hfactor *)node->ndata)->bmessagesin = (mvec **)imalloc(E,1*sizeof(mvec*));
	((hfactor *)node->ndata)->MSmsgin = (mvec **)imalloc(E,1*sizeof(mvec*));

	return node;
}


void *john_testfunc(void *arglist)
{

}

//------------------------------------------------------------------------
//------------------- Initialise Graph From File -------------------------
//------------------------------------------------------------------------

void *john_initfromfile(void *arglist)
{
	void *argptr;
	char *filename;
	FILE *fptr;
	hgph *graph=(hgph *)imalloc(E,sizeof(hgph));
	//Get user config file
	if(dynamic_getarg(arglist,"Filename",&argptr)=='f') return NULL;
	if(!invalidptr(E,argptr)) filename=(char *) argptr;

	fptr = fopen(filename, "r+");

	char indicator[MAXLEN];
	char nodename[MAXLEN];
	char test[MAXLEN];
	char node1[MAXLEN], node2[MAXLEN];
	char nodetype , terminate;
	char rowlabel[MAXLEN];
	char columnlabel[MAXLEN];
	int nrow,ncol;
	double store;
	double *rowvec, *colvec, *matrix;
	nodes *node, *nodea, *nodeb;
	hfactor *hfac;

	int i;

	while(!feof(fptr))
	{
		fscanf(fptr, "%s", indicator); //Scans to see if Startnode or Startlink

		if(strcmp(indicator,"--STARTNODE---------------------------------")==0) //If Node Creation
		{
			fscanf(fptr, "%*s %s", nodename); //Gets nodename
			fscanf(fptr, "%*s %c", &nodetype); //Gets type

			if(nodetype == 'f') //If it is factor need more data
			{
				//Create the Node
				if(feof(fptr)) break;
				node = createnode(1,1,nodename,graph,'f');
				hfac = (hfactor *)node->ndata;

				//Populate with data
				fscanf(fptr, "%*s %s", rowlabel); //Gets rowlabel
				hfac->rowlabel = str_hash(rowlabel);
				fscanf(fptr, "%*s %s", columnlabel); //Gets columnlabel
				hfac->columnlabel = str_hash(columnlabel);

				fscanf(fptr, "%*s %d", &nrow); //Gets nrow
				hfac->nrow = nrow;
				fscanf(fptr, "%*s %d", &ncol); //Gets ncol
				hfac->ncol = ncol;

				rowvec = (double *)imalloc(E,nrow*sizeof(double));
				colvec = (double *)imalloc(E,ncol*sizeof(double));
				matrix = (double *)imalloc(E,ncol*nrow*sizeof(double));

				fscanf(fptr, "%*s"); //Skips over "RowVal"
				for(i=0;i<nrow;i++) //Stores each RowVal value
				{
					fscanf(fptr, "%lf", &store);
					rowvec[i]=store;
				}
				hfac->rowdiscretevalues=rowvec;

				fscanf(fptr, "%*s"); //Skips over "ColVal"
				for(i=0;i<ncol;i++) //Stores each ColVal value
				{
					fscanf(fptr, "%lf", &store);
					colvec[i]=store;
				}
				hfac->columndiscretevalues=colvec;

				fscanf(fptr, "%*s"); //Skips over "Matrix"
				for(i=0;i<(nrow*ncol);i++) //Stores each Matrix value
				{
					fscanf(fptr, "%lf", &store);
					matrix[i] = store;
				}
				hfac->probdist = matrix;

				fscanf(fptr, "%*s"); //Skips over "--EndNode"

			}
			else //If variable we are done
			{
				if(feof(fptr)) break;
				createnode(1,1,nodename,graph,'v');

				fscanf(fptr, "%*s"); //Skip --ENDNODE

			}
		}
		else //If Link Creation
		{
			while(1)
			{
				fscanf(fptr, "%s %s %c", node1, node2, &terminate); // Scan for node names
				nodea = find_node(str_hash(node1),graph->nnodes,graph->nodelist);
				nodeb = find_node(str_hash(node2),graph->nnodes,graph->nodelist);
				add_edge(nodea,nodeb);
				if(terminate == ';') break;
			}

			fscanf(fptr, "%*s"); //Skip --ENDLINK //Assume end of file
			break;
		}
	}

	fclose(fptr);
	arglist=NULL;
	dynamic_putarg("graph.hgph","hgph",(void *)graph,SZ,&arglist);

	return arglist;
}
