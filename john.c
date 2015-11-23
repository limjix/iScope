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


/*   What Is the plan? 
    Function - I want the user to input a value
    Function - Once its done, it creates 2 new nodes (mu and x) and populates x
    Function - Links mu and x with directed edge
    Function - Does a running average of x and updates mu
    Function - Creates a vector of updated mu and x number
    Script (after this its smooth sailing)- It plots the vector to show updated mu as x changes

*/

void *john_kalmaninit(void *arglist)
{

	hgph *graph;
	graph=(hgph *)imalloc(E,sizeof(graph));
	if(invalidptr(E,graph)) return NULL;

	//Add in nnodes and nodelist
	graph->nnodes = 0;
	graph->nodelist = NULL;

	//Exports initialised values
	arglist=NULL;
	dynamic_putarg("std:hgph","hgph",graph,SZ,&arglist);

	return arglist;
}

void *john_kalmancreatenodes(void *arglist)
{
	void *argptr;
	int input;
	nodes *x, *mu, *prevmu;
	hgph *graph;
	double pos[2];
	char strmu[MAXLEN], strx[MAXLEN], strprevmu[MAXLEN]; 
	float average;

	//Get an argument of user input
	if(dynamic_getarg(arglist,"userinput",&argptr)=='f') return NULL;
	if(!invalidptr(E,argptr)) input=*((int *) argptr);

	//Get previous hgph struct
	if(dynamic_getarg(arglist,"hgph",&argptr)=='f') return NULL;
	if(!invalidptr(E,argptr)) graph=(hgph *) argptr;

	//Current node iteration
	int i = (graph->nnodes/2)+1;

   	//Creates node mu
	pos[0]=1;
  	pos[1]=2;
	sprintf(strmu, "mu%d", i);
	add_node(str_hash(strmu),pos,&(graph->nnodes),&(graph->nodelist));
	mu=find_node(str_hash(strmu),graph->nnodes,graph->nodelist);
	
	//Creates node x
	pos[0]=1;
  	pos[1]=1;
	sprintf(strx, "x%d", i);
	add_node(str_hash(strx),pos,&(graph->nnodes),&(graph->nodelist));
	x=find_node(str_hash(strx),graph->nnodes,graph->nodelist);

	//Adds directed edge between mu and x
	add_diredge(mu,x);

	//If previous mu exists, links current mu to previous mu
	if(graph->nnodes>2)
	{	
		sprintf(strprevmu, "mu%d", i-1);
		prevmu = find_node(str_hash(strprevmu),graph->nnodes,graph->nodelist);
		add_diredge(prevmu,mu);
	}	


	//Populates x payload with uservalue.
	x->ndata= &input;

	//Average mu and save
	average = ((i-1)*(prevmu->ndata)+(x->ndata))/i;
	mu->ndata=&average;

	//Output to arglist
	arglist=NULL;
	dynamic_putarg("std:hgph","hgph",graph,SZ,&arglist);

	return arglist; 
}




/*void *john_linknodes(void *arglist)
{
	int nnodes;
        nodes **nodelist;
	nodes *node1, *node2;

	//Get previous nodelist
	if(dynamic_getarg(arglist,"nodelist",&nodelist)=='f') return NULL;
	//Get previous nnodes
	if(dynamic_getarg(arglist,"nnodes",&nnodes)=='f') return NULL;
	
	//Identifies those nodes
	node1=find_node(str_hash("mu"),nnodes,nodelist);
	node2=find_node(str_hash("x"),nnodes,nodelist);

	//Adds a directed edge to them
	add_diredge(node1,node2);
	
	//Output to arglist
	dynamic_putarg("std:int","nnodes",nnodes,SZ,&arglist);
	dynamic_putarg("std:int","nodelist",nodelist,SZ,&arglist);
	return arglist;
}
*/


void *john_makeplotvector(void *arglist)
{
	void *argptr;
	hgph *graph;

	//Get previous hgph struct
	if(dynamic_getarg(arglist,"hgph",&argptr)=='f') return NULL;
	if(!invalidptr(E,argptr)) graph=(hgph *) argptr;

	
}


void *john_testfunc(void *arglist)
{	int i;
	char test[MAXLEN];

	sprintf(test,"mu%d",i);
	printf("test");

	return NULL;

}


