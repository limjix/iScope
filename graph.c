#include "graph.h"

//gcc -g -fPIC -c graph.c && gcc -g -fPIC -dynamiclib -o libgraph.dylib graph.o -lutilties 

void add_diredge(nodes *inode,nodes *jnode)
{
  int iedge;
  unsigned *ilist;

  // ******************************************** //
  // ** CONNECT TWO NODES WITH A DIRECTED EDGE ** //
  // ******************************************** //

  if(invalidptr(E,inode)) return;
  if(invalidptr(E,jnode)) return;
  if(inode->nedges<0) return;
  if(jnode->nedges<0) return;

  //printf("graph: add_diredge: %d [%f,%f]->%d [%f,%f]\n",inode->nhash,inode->vecr[0],inode->vecr[1],jnode->nhash,jnode->vecr[0],jnode->vecr[1]);
  
  ilist=NULL;
  ilist=(unsigned *)imalloc(E,(inode->nedges+1)*sizeof(unsigned));
  
  if(!invalidptr(E,ilist))
  {
    //copy existing lists
    if(inode->nedges>0&&!invalidptr(E,inode->edge))
      for(iedge=0;iedge<inode->nedges;iedge++)
        ilist[iedge]=inode->edge[iedge];

    //store name of linked node
    ilist[inode->nedges]=jnode->nhash;

    //ifree existing list
    if(inode->nedges>0)
      inode->edge=ifree(E,inode->edge);

    //return updated lists
    inode->nedges++;
    inode->edge=ilist;
  }
}

void rem_diredge(nodes *inode,nodes *jnode)
{
  int iedge,idx,jdx;
  unsigned *ilist;

  // ******************************************** //
  // ** REMOVE DIRECTED EDGE BETWEEN TWO NODES ** //
  // ******************************************** //

  if(invalidptr(E,inode)) return;
  if(inode->nedges<1) return; //must be at least one edge in each node
  if(invalidptr(E,inode->edge)) return;
  
  //printf("graph: rem_diredge: %d [%f,%f]->%d [%f,%f]\n",inode->nhash,inode->vecr[0],inode->vecr[1],jnode->nhash,jnode->vecr[0],jnode->vecr[1]);
  
  //find node by name
  idx=-1;
  for(iedge=0;iedge<inode->nedges;iedge++)
    if(jnode->nhash==inode->edge[iedge])
      idx=iedge;

  if(idx>=0)
  {
    ilist=NULL;
    ilist=(unsigned *)imalloc(E,(inode->nedges-1)*sizeof(unsigned));
  
    if(!invalidptr(E,ilist))
    {
      //copy before idx,jdx
      for(iedge=0;iedge<idx;iedge++)
        ilist[iedge]=inode->edge[iedge];

      //copy after idx,jdx
      for(iedge=idx+1;iedge<inode->nedges;iedge++)
        ilist[iedge-1]=inode->edge[iedge];

      //ifree existing list
      inode->edge=ifree(E,inode->edge);

      //return updated lists
      inode->nedges--;
      inode->edge=ilist;
    }
  } 
}

void add_edge(nodes *inode,nodes *jnode)
{
  int iedge;
  unsigned *ilist,*jlist;

  // *********************************************** //
  // ** CONNECT TWO NODES WITH AN UNDIRECTED EDGE ** //
  // *********************************************** //

  if(invalidptr(E,inode)) return;
  if(invalidptr(E,jnode)) return;
  if(inode->nedges<0) return;
  if(jnode->nedges<0) return;

  //printf("graph: add_edge: %d [%f,%f]<->%d [%f,%f]\n",inode->nhash,inode->vecr[X],inode->vecr[Y],jnode->nhash,jnode->vecr[X],jnode->vecr[Y]);
  
  ilist=NULL;
  jlist=NULL;
  ilist=(unsigned *)imalloc(E,(inode->nedges+1)*sizeof(unsigned));
  jlist=(unsigned *)imalloc(E,(jnode->nedges+1)*sizeof(unsigned));
  
  if(!invalidptr(E,ilist)&&
     !invalidptr(E,jlist))
  {
    //copy existing lists
    if(inode->nedges>0&&!invalidptr(E,inode->edge))
      for(iedge=0;iedge<inode->nedges;iedge++)
        ilist[iedge]=inode->edge[iedge];
    if(jnode->nedges>0&&!invalidptr(E,jnode->edge))
      for(iedge=0;iedge<jnode->nedges;iedge++)
        jlist[iedge]=jnode->edge[iedge];

    //exchange names
    ilist[inode->nedges]=jnode->nhash;
    jlist[jnode->nedges]=inode->nhash;

    //ifree existing list
    inode->edge=ifree(E,inode->edge);
    jnode->edge=ifree(E,jnode->edge);

    //return updated lists
    inode->nedges++;
    jnode->nedges++;
    inode->edge=ilist;
    jnode->edge=jlist;
  }
}

void rem_edge(nodes *inode,nodes *jnode)
{
  int iedge,idx,jdx;
  unsigned *ilist,*jlist;

  // ********************************************** //
  // ** REMOVE UNDIRECTED EDGE BETWEEN TWO NODES ** //
  // ********************************************** //

  if(invalidptr(E,inode)) return;
  if(invalidptr(E,jnode)) return;
  if(inode->nedges<1) return; //must be at least one edge in each node
  if(jnode->nedges<1) return;
  if(invalidptr(E,inode->edge)) return;
  if(invalidptr(E,jnode->edge)) return;
  
  //printf("graph: rem_edge: %d [%f,%f]<->%d [%f,%f]\n",inode->nhash,inode->vecr[X],inode->vecr[Y],jnode->nhash,jnode->vecr[X],jnode->vecr[Y]);
  
  //find node by name
  idx=-1;
  for(iedge=0;iedge<inode->nedges;iedge++)
    if(jnode->nhash==inode->edge[iedge])
      idx=iedge;

  jdx=-1;
  for(iedge=0;iedge<jnode->nedges;iedge++)
    if(inode->nhash==jnode->edge[iedge])
      jdx=iedge;

  if(idx>=0&&jdx>=0)
  {
    ilist=NULL;
    jlist=NULL;
    ilist=(unsigned *)imalloc(E,(inode->nedges-1)*sizeof(unsigned));
    jlist=(unsigned *)imalloc(E,(jnode->nedges-1)*sizeof(unsigned));
  
    if(!invalidptr(E,ilist)&&
       !invalidptr(E,jlist))
    {
      //copy before idx,jdx
      for(iedge=0;iedge<idx;iedge++)
        ilist[iedge]=inode->edge[iedge];
      for(iedge=0;iedge<jdx;iedge++)
        jlist[iedge]=jnode->edge[iedge];

      //copy after idx,jdx
      for(iedge=idx+1;iedge<inode->nedges;iedge++)
        ilist[iedge-1]=inode->edge[iedge];
      for(iedge=jdx+1;iedge<jnode->nedges;iedge++)
        jlist[iedge-1]=jnode->edge[iedge];

      //free existing list
      inode->edge=ifree(E,inode->edge);
      jnode->edge=ifree(E,jnode->edge);

      //return updated lists
      inode->nedges--;
      jnode->nedges--;
      inode->edge=ilist;
      jnode->edge=jlist;
    }
  } 
}

void add_node(unsigned nhash,double *vecr,int *nnodes,nodes ***nodelist)
{
  int inode,id;
  nodes **newlist;

  // *************************** //
  // ** ADD NODE TO NODE LIST ** //
  // *************************** //

  if(*nnodes<0) return;
  if(invalidptr(E,vecr)) return;

  newlist=NULL;
  newlist=(nodes **)imalloc(E,(*nnodes+1)*sizeof(nodes *));

  if(!invalidptr(E,newlist))
  {
    //copy existing list
    if(*nnodes>0&&!invalidptr(E,*nodelist))
      for(inode=0;inode<*nnodes;inode++)
        newlist[inode]= (*nodelist)[inode];

    newlist[*nnodes]=NULL;
    newlist[*nnodes]=(nodes *)imalloc(E,sizeof(nodes));  
    if(!invalidptr(E,newlist[*nnodes]))
    {  
      //populate new node
      newlist[*nnodes]->nhash=nhash;         //name
      for(id=0;id<ND;id++)
        newlist[*nnodes]->vecr[id]=vecr[id]; //position
      newlist[*nnodes]->nedges=0;            //edge list
      newlist[*nnodes]->edge=NULL;           //edge list
      newlist[*nnodes]->ndata=NULL;          //payload pointer  
    
      //ifree existing list
      if(*nnodes>0)
        *nodelist=ifree(E,*nodelist);

      //return updated list
      (*nnodes)++;
      *nodelist=newlist;
      //printf("graph: new node %d: [%f,%f]\n",*nnodes,(*nodelist)[*nnodes-1]->vecr[X],(*nodelist)[*nnodes-1]->vecr[Y]);
    }
  }
}

nodes *find_node(unsigned nhash,int nnodes,nodes **nodelist)
{
  int inode;

  // ******************************************** //
  // ** FIND A NODE BY HASH FROM THE NODE LIST ** //
  // ******************************************** //

  //printf("graph: unprotected find_node() nhash=%d nnodes=%d\n",nhash,nnodes);

  if(nhash==0) return NULL;
  if(nnodes<=0) return NULL;
  if(invalidptr(E,nodelist)) return NULL;

  //search list for matching nhash
  for(inode=0;inode<nnodes;inode++)
    if(!invalidptr(E,nodelist[inode]))
      if(nodelist[inode]->nhash==nhash)
      {
        //printf("graph: find_node %d: [%f,%f]\n",nhash,nodelist[inode]->vecr[0],nodelist[inode]->vecr[1]);
        return nodelist[inode];
      }

  return NULL;
}

void rem_node(unsigned nhash,int *nnodes,nodes ***nodelist)
{
  int inode,iedge,id;
  int idx;
  nodes **newlist;

  // ******************************** //
  // ** REMOVE NODE FROM NODE LIST ** //
  // ******************************** //

  if(*nnodes<=0) return;
  if(invalidptr(E,*nodelist)) return;

  //find node by name
  idx=-1;
  for(inode=0;inode<*nnodes;inode++)
    if(!invalidptr(E,(*nodelist)[inode]))
      if(nhash==(*nodelist)[inode]->nhash)
        idx=inode;

  //if found, then remove from list
  if(idx>=0)
    if(!invalidptr(E,(*nodelist)[idx]))
    {
      newlist=NULL;
      newlist=(nodes **)imalloc(E,(*nnodes-1)*sizeof(nodes *));
  
      if(!invalidptr(E,newlist))
      {
        //copy before idx
        for(inode=0;inode<idx;inode++)
          newlist[inode]= *(nodelist)[inode];

        //free idx
	(*nodelist)[idx]->edge=ifree(E,(*nodelist)[idx]->edge);
        (*nodelist)[idx]=ifree(E,*(nodelist)[idx]);

        //copy after idx
        for(inode=idx+1;inode<*nnodes;inode++)
          newlist[inode-1]=(*nodelist)[inode];
      }
  
      //return updated list
      (*nnodes)--;
      *nodelist=newlist;
    }
}
