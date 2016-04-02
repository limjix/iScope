#include "micrscan.h"

//------------------------------------------------------------------------------
//--------------------------iSCOPE FUNCTIONS -----------------------------------
//------------------------------------------------------------------------------

void *micrscan_init(void *arglist)
{
  //-----------SETUP OF PROBLEM ------ SIMULATING A SLIDE
  void *argptr, *xptr, *argptr2, *argptr3, *argptr4, *argptr5;
  char *filename;
  FILE *fptr;
  int height,width;
  double store;
  mcr *MCRDat;
  int i,j,k,n;
  mIMG **imglist;
  int curr_row;
  int curr_col;

  //Get user .slide file
  if(dynamic_getarg(arglist,"Filename",&argptr)=='f') return NULL;
  if(!invalidptr(E,argptr)) filename=(char *) argptr;

  //Get Height and Width
  if(dynamic_getarg(arglist,"Heightpx",&argptr2)=='f') return NULL;
  if(!invalidptr(E,argptr)) height=*((int *) argptr2);

  if(dynamic_getarg(arglist,"Widthpx",&argptr3)=='f') return NULL;
  if(!invalidptr(E,argptr)) width=*((int *) argptr3);

  //Get current row & col
  if(dynamic_getarg(arglist,"Col",&argptr4)=='f') return NULL;
  if(!invalidptr(E,argptr4)) curr_col=*((int *) argptr4);

  if(dynamic_getarg(arglist,"Row",&argptr5)=='f') return NULL;
  if(!invalidptr(E,argptr5)) curr_row=*((int *) argptr5);

  //Get xclient
  if(dynamic_getarg(arglist,"xclient",&xptr)=='f') return NULL;

  double *image = (double *)imalloc(E,height*width*sizeof(double));

  //GETS THE FULL IMAGE
  fptr = fopen(filename, "r");

  k = 0;
  for(i=0;i<height;i++) //Go up to down
  {
    for(j=0;j<width;j++) //Go left to right
    {
      fscanf(fptr, "%lf", &store );
      image[k]=store;
      //printf("%lf , %d\n", image[k], k);
      k++;
    }
  }

  //Creates MCRDAT
  n = 10;
  MCRDat = (mcr *)imalloc(E,1*sizeof(mcr));
  MCRDat->size_col = width;
  MCRDat->size_row = height;
  MCRDat->nrow = n;
  MCRDat->ncol = n;
  MCRDat->n_pixel_col = width / n;
  MCRDat->n_pixel_row = height / n;
  MCRDat->image = image;

  //DISCRETISES FULL IMAGE
  imglist = (mIMG **) discretisefullslide(image, MCRDat);

  //----------------Create Factor Node Details--------------------------------
  //---- Use Ising Model

  double nu, beta;
  nu = 2.1;
  beta = 1.0;

  //FOR MRF-------------
  double *MRFcolvals = (double *)imalloc(E,2*sizeof(double));
  MRFcolvals[0] = -1;
  MRFcolvals[1] = 1;

  double *MRFrowvals = (double *)imalloc(E,2*sizeof(double));
  MRFrowvals[0] = -1;
  MRFrowvals[1] = 1;

  int MRFnrow = 2;
  int MRFncol = 2;

  double *MRFprobdist = (double *)imalloc(E,4*sizeof(double));
  MRFprobdist[0] = exp(beta*MRFcolvals[0]*MRFrowvals[0]);
  MRFprobdist[1] = exp(beta*MRFcolvals[1]*MRFrowvals[0]);
  MRFprobdist[2] = exp(beta*MRFcolvals[0]*MRFrowvals[1]);
  MRFprobdist[3] = exp(beta*MRFcolvals[1]*MRFrowvals[1]);

  //Normalise
  double sum = 0;
  for(i=0;i<MRFnrow*MRFncol;i++)
  {
    sum = sum + MRFprobdist[i];
  }

  for(i=0;i<MRFnrow*MRFncol;i++)
  {
    MRFprobdist[i] = MRFprobdist[i]/sum;
  }

  //FOR OBSERVATIONS--------
  double *OBScolvals = (double *)imalloc(E,2*sizeof(double));
  OBScolvals[0] = -1;
  OBScolvals[1] = 1;

  double *OBSrowvals = (double *)imalloc(E,2*sizeof(double));
  OBSrowvals[0] = -1;
  OBSrowvals[1] = 1;

  double *OBSprobdist = (double *)imalloc(E,4*sizeof(double));
  OBSprobdist[0] = exp(nu*MRFcolvals[0]*MRFrowvals[0]);
  OBSprobdist[1] = exp(nu*MRFcolvals[1]*MRFrowvals[0]);
  OBSprobdist[2] = exp(nu*MRFcolvals[0]*MRFrowvals[1]);
  OBSprobdist[3] = exp(nu*MRFcolvals[1]*MRFrowvals[1]);

  int OBSnrow = 2;
  int OBSncol = 2;

  //Normalise
  sum = 0;
  for(i=0;i<OBSnrow*OBSncol;i++)
  {
    sum = sum + OBSprobdist[i];
  }

  for(i=0;i<OBSnrow*OBSncol;i++)
  {
    OBSprobdist[i] = OBSprobdist[i]/sum;
  }

  //Append-------------------------------------------------------------------
  MCRDat->FullList = imglist;
  MCRDat->nFL = 100;

  MCRDat->SeenList = (mIMG **)imalloc(E,1*sizeof(mIMG*));
  MCRDat->nSL = 0;

  MCRDat->curr_row = curr_row;
  MCRDat->curr_col = curr_col;

  MCRDat->buff = (mIMG **)imalloc(E,1*sizeof(mIMG*));
  MCRDat->nbuff = 0;

  //FOR MRF
  MCRDat->MRFprobdist = MRFprobdist;
  MCRDat->MRFcolvals = MRFcolvals;
  MCRDat->MRFrowvals = MRFrowvals;
  MCRDat->MRFnrow = MRFnrow;
  MCRDat->MRFncol = MRFncol;

  //FOR OBSERVATIONS
  MCRDat->OBSprobdist = OBSprobdist;
  MCRDat->OBScolvals = OBScolvals;
  MCRDat->OBSrowvals = OBSrowvals;
  MCRDat->OBSnrow = OBSnrow;
  MCRDat->OBSncol = OBSncol;

  //Output to arglist -------------------------------------------------------
	arglist=NULL;
	dynamic_putarg("MCRDat.mcr","mcr",(void*) MCRDat,SZ,&arglist);
  return arglist;
}

void *micrscan_drivemicroscope(void *arglist)
{
//--------- Mimics the raspberry Pi moving the stage
// Direction - 1 for x+ , 2 for x-, 3 for y+, 4 for y-
//--Must Be Immediately coupled with AnalyseBuffer to translate those seen photos to observed variables
  void *argptr, *mcrptr, *xptr;
  int nframes, direction;
  mcr *MCR;
  mIMG *seenIMG;
  int i;
  char printstr[MAXLEN];
  //-------------------- GETS INSTRUCTIONS ----------------------------------
  if(dynamic_getarg(arglist,"nframes",&argptr)=='f') return NULL;
  if(!invalidptr(E,argptr)) nframes=*((int *) argptr);

  if(dynamic_getarg(arglist,"direction",&argptr)=='f') return NULL;
  if(!invalidptr(E,argptr)) direction=*((int *) argptr);

  if(dynamic_getarg(arglist,"mcr",&mcrptr)=='f') return NULL;
  if(!invalidptr(E,mcrptr)) MCR=(mcr *) mcrptr;

  if(dynamic_getarg(arglist,"xclient",&xptr)=='f') return NULL;

  //-------------------------Clear old buffer---------------------------------
  mIMG **buff = MCR->buff;
  MCR->buff = ifree(E,MCR->buff);
  MCR->buff = (mIMG **)imalloc(E,1*sizeof(mIMG*));
  MCR->nbuff = 0;

  //-----------------------Execute movement----------------------------------
  int n;
  int curr_row = MCR->curr_row;
  int curr_col = MCR->curr_col;

  int new_row;
  int new_col;

  if(direction == 1) //x+
  {
    for(n=0;n<nframes;n++)
    {
      new_row = curr_row;
      new_col = curr_col+(n+1);
      if(new_row <0) //Safety Catch
      {
        return;
      }
      else if(new_row > ((MCR->nrow)-1) )
      {
        return;
      }
      else if(new_col < 0)
      {
        return;
      }
      else if(new_col > ((MCR->ncol)-1))
      {
        return;
      }
      else
      {
        seenIMG = acquireimage(MCR, new_row, new_col);
        addIMGtoSeenList(MCR,seenIMG);
        addIMGtoBuffer(MCR,seenIMG);
        MCR->curr_col = new_col;
        MCR->curr_row = new_row;
        sprintf(printstr, "Moved %d\n",direction);
        printtoclient(printstr, xptr);
        sprintf(printstr, "Curr Row = %d, Curr Col = %d\n",MCR->curr_row ,MCR->curr_col);
        printtoclient(printstr, xptr);

      }
    }
  }
  else if(direction == 2) //x-
  {
    for(n=0;n<nframes;n++)
    {
      new_row = curr_row;
      new_col = curr_col-(n+1);
      if(new_row <0) //Safety Catch
      {
        return;
      }
      else if(new_row > ((MCR->nrow)-1) )
      {
        return;
      }
      else if(new_col < 0)
      {
        return;
      }
      else if(new_col > ((MCR->ncol)-1))
      {
        return;
      }
      else
      {
        seenIMG = acquireimage(MCR, new_row, new_col);
        addIMGtoSeenList(MCR,seenIMG);
        addIMGtoBuffer(MCR,seenIMG);
        MCR->curr_col = new_col;
        MCR->curr_row = new_row;
        sprintf(printstr, "Moved %d\n",direction);
        printtoclient(printstr, xptr);
        sprintf(printstr, "Curr Row = %d, Curr Col = %d\n",MCR->curr_row ,MCR->curr_col);
        printtoclient(printstr, xptr);
      }
    }
  }
  else if(direction ==3) //y+
  {
    for(n=0;n<nframes;n++)
    {
      new_row = curr_row+(n+1);
      new_col = curr_col;
      if(new_row <0) //Safety Catch
      {
        return;
      }
      else if(new_row > ((MCR->nrow)-1) )
      {
        return;
      }
      else if(new_col < 0)
      {
        return;
      }
      else if(new_col > ((MCR->ncol)-1))
      {
        return;
      }
      else
      {
        seenIMG = acquireimage(MCR, new_row, new_col);
        addIMGtoSeenList(MCR,seenIMG);
        addIMGtoBuffer(MCR,seenIMG);
        MCR->curr_col = new_col;
        MCR->curr_row = new_row;
        sprintf(printstr, "Moved %d\n",direction);
        printtoclient(printstr, xptr);
        sprintf(printstr, "Curr Row = %d, Curr Col = %d\n",MCR->curr_row ,MCR->curr_col);
        printtoclient(printstr, xptr);
      }
    }
  }
  else if(direction == 4) //y-
  {
    for(n=0;n<nframes;n++)
    {
      new_row = curr_row-(n+1);
      new_col = curr_col;
      if(new_row <0) //Safety Catch
      {
        return;
      }
      else if(new_row > ((MCR->nrow)-1) )
      {
        return;
      }
      else if(new_col < 0)
      {
        return;
      }
      else if(new_col > ((MCR->ncol)-1))
      {
        return;
      }
      else
      {
        seenIMG = acquireimage(MCR, new_row, new_col);
        addIMGtoSeenList(MCR,seenIMG);
        addIMGtoBuffer(MCR,seenIMG);
        MCR->curr_col = new_col;
        MCR->curr_row = new_row;
        sprintf(printstr, "Moved %d\n",direction);
        printtoclient(printstr, xptr);
        sprintf(printstr, "Curr Row = %d, Curr Col = %d\n",MCR->curr_row ,MCR->curr_col);
        printtoclient(printstr, xptr);
      }
    }
  }
  else //Broken
  {
    printf("NOT VALID");
  }

	return;
}

void *micrscan_CreateMicrGraph(void *arglist)
{
  //CREATES MRF graph based on NCOL & NROW

  hgph *graph;
  void *mcrptr;
  mcr *MCR;
  int i, j, nfac, nvar;
  char name[MAXLEN], str1[MAXLEN], str2[MAXLEN];
  nodes *vnode, *fnode, *v1, *v2;
  double *probdist, *colvals, *rowvals;
  hfactor *hfac;

  if(dynamic_getarg(arglist,"mcr",&mcrptr)=='f') return NULL;
  if(!invalidptr(E,mcrptr)) MCR=(mcr *) mcrptr;

  int nrow = MCR->nrow;
  int ncol = MCR->ncol;

  //--------------------------- Creates & Initialise Graph Struct -----------
  graph=(hgph *)imalloc(E,sizeof(hgph));
  graph->nnodes = 0;
  graph->nodelist = NULL;
  nfac = 0;
  nvar = 0;

  //--------------Create X-Y Variable Node Grid -----------------------------
  for(i=0;i<nrow;i++) //Going down row
  {
    for(j=0;j<ncol;j++) //Going across col
    {
        sprintf(name,"V%d%d",i,j);
        vnode = createnode(i,j,name,graph,'v');
        nvar++;
    }
  }

  //-----------------------Create Factor nodes and edges-----------------------
  //ROW MAJOR ORDERING USED HERE
  //1. Factor Nodes in horizontal direction
  for(i=0; i<(nrow);i++) //Going down row (n-1) is because spaces in between variable
  {
    for(j=0;j<(ncol-1);j++) //Going across col
    {
        sprintf(name,"F%d%d%d%d",i,j,i,j+1 );
        fnode = createnode(i,j,name,graph,'f'); //Creat Node

        hfac = (hfactor *)fnode->ndata;

        //Append all probability stuff
        hfac->probdist = MCR->MRFprobdist;
        hfac->columndiscretevalues = MCR->MRFcolvals;
        hfac->rowdiscretevalues = MCR->MRFrowvals;
        hfac->nrow = MCR->MRFnrow;
        hfac->ncol = MCR->MRFncol;

        sprintf(str1, "V%d%d",i,j);
        hfac->columnlabel = str_hash(str1);

        sprintf(str2, "V%d%d",i,j+1);
        hfac->rowlabel = str_hash(str2);

        v1 = find_node(hfac->columnlabel,graph->nnodes,graph->nodelist);
        v2 = find_node(hfac->rowlabel,graph->nnodes,graph->nodelist);

        add_edge(v1,fnode);
        add_edge(v2,fnode);

        nfac++;
    }
  }

  //2. Factor nodes in vertical direction
  for(j=0;j<(ncol);j++) //Going across col (n-1) is because spaces in between variable
  {
    for(i=0;i<(nrow-1);i++) //Going down row
    {
      sprintf(name,"F%d%d%d%d",i,j,i+1,j);
      fnode = createnode(i,j,name,graph,'f'); //Creat Node

      hfac = (hfactor *)fnode->ndata;

      //Append all probability stuff
      hfac->probdist = MCR->MRFprobdist;
      hfac->columndiscretevalues = MCR->MRFcolvals;
      hfac->rowdiscretevalues = MCR->MRFrowvals;
      hfac->nrow = MCR->MRFnrow;
      hfac->ncol = MCR->MRFncol;

      sprintf(str1, "V%d%d",i,j);
      hfac->columnlabel = str_hash(str1);

      sprintf(str2, "V%d%d",i+1,j);
      hfac->rowlabel = str_hash(str2);

      v1 = find_node(hfac->columnlabel,graph->nnodes,graph->nodelist);
      v2 = find_node(hfac->rowlabel,graph->nnodes,graph->nodelist);

      add_edge(v1,fnode);
      add_edge(v2,fnode);

      nfac++;
    }
  }

  //-----------------------------------------------------------------------
  arglist = NULL;
  dynamic_putarg("graph.hgph","hgph",(void *)graph,SZ,&arglist);
  return arglist;
}

void *micrscan_AnalyseBufferImages(void *arglist)
{
  //-------ANALYSE IMAGE IN BUFFER
  //-------TURNS EACH IMAGE SEEN INTO AN OBSERVATION
  //-------Creates a variable node and factor node for that observation
  //-------Finds relevant variable node and attaches the observation to it

  void *argptr, *argptr2;
  hgph *graph;
  mcr *MCR;
  int i;
  nodes *fnode, *vnode, *gvnode;
  char vname[MAXLEN], fname[MAXLEN];
  mIMG *image;
  hfactor *hfac, *gvhfac;
  int row;
  int col;
  char findname[MAXLEN];

  if(dynamic_getarg(arglist,"mcr",&argptr)=='f') return NULL;
  if(!invalidptr(E,argptr)) MCR = (mcr *)argptr;

  if(dynamic_getarg(arglist,"hgph",&argptr2)=='f') return NULL;
  if(!invalidptr(E,argptr)) graph = (hgph *)argptr2;

  mIMG **buff = MCR->buff;
  int nbuff = MCR->nbuff;
  double result;

  for(i=0;i<nbuff;i++) //For every image
  {
    image = buff[i];
    result = HeuristicOnTissue(image,MCR);

    //Convert from PIXEL COORDINATES TO GRAPH COORDINATES
    row = round(image->top_y / MCR->n_pixel_row);
    col = round(image->left_x / MCR->n_pixel_col);

    //Create Variable Node
    sprintf(vname, "O%d%d", row, col);
    vnode = createnode(col,row,vname,graph,'v');
    //Create Factor Node
    sprintf(fname, "FO%d%d", row, col);
    fnode = createnode(col,row,fname,graph,'f');

    //Populate Factor Node
    hfac = (hfactor*) fnode->ndata;
    hfac->probdist = MCR->OBSprobdist;
    hfac->columndiscretevalues = MCR->OBScolvals;
    hfac->rowdiscretevalues = MCR->OBSrowvals;
    hfac->nrow = MCR->OBSnrow;
    hfac->ncol = MCR->OBSncol;

    //Find relevant node on graph
    gvnode = (nodes *)QueryGraph(graph,row,col);

    //Create Edge
    add_edge(vnode,fnode);
    add_edge(fnode,gvnode);

    gvhfac = (hfactor*) gvnode->ndata;

    //Hfac labels
    hfac->columnlabel = str_hash(vname);
    sprintf(findname,"V%d%d",row,col);
    hfac->rowlabel = str_hash(findname);

    //Mark Variable Node as Seen
    ((hfactor *)vnode->ndata)->observed = 't';
    gvhfac->seen = 't';

    if(result>0.2)
    {
      result = 1.0;
    }
    else
    {
      result = -1.0;
    }

    ((hfactor *)vnode->ndata)->observedvariable = result;
  }

  arglist = NULL;
  return arglist;
}

void *micrscan_DecideNextMove(void *arglist)
{
  //------Looking at the previous scan line, is it worth going back?
  //------Looking at the next scan line, decides how far should it go
  void *argptr, *mcrptr;
  hgph *graph;
  int duration;
  int i;
  int new_row, previous_row;
  char nodename[MAXLEN];
  nodes *node;
  hfactor *hfac;
  char minfound = 'f';
  int mincol, maxcol, curr_col;
  mcr *MCR;

  //Get Graph
  if(dynamic_getarg(arglist,"hgph",&argptr)=='f') return NULL;
  if(!invalidptr(E,argptr)) graph = (hgph *)argptr;

  //Get MCR
  if(dynamic_getarg(arglist,"mcr",&mcrptr)=='f') return NULL;
  if(!invalidptr(E,mcrptr)) MCR=(mcr *) mcrptr;

  //1. Find forward lines
  new_row = MCR->curr_row+1;
  for(i=0;i<MCR->ncol; i++)
  {
    sprintf(nodename,"V%d%d", new_row, i);
    node = find_node(str_hash(nodename),graph->nnodes,graph->nodelist);
    hfac = (hfactor *)node->ndata;

    //Determine if mincol
    if(minfound =='f')
    {
      if(hfac->MostLikelyState==1.0)
      {
        mincol = i; //i+1 cause graph starts from 1
        minfound = 't';
      }
    }

    //Determine if maxcol
    if(hfac->MostLikelyState==1.0)
    {
      maxcol = i;
    }

  }

  //2. Determine what needs to happen
  int distancetomin, distancetomax;

  curr_col = MCR->curr_col;
  distancetomin = curr_col-mincol;
  distancetomax = maxcol - curr_col;

  if(distancetomin>0 && distancetomax>0) //Current position in centre of tissue
  {
      //1. Go shorter distance
      if(distancetomin<distancetomax) //If going to left quicker than right
      {

      }
      else //If going to right quicker than left
      {

      }
      //2. Backtrack to longer distance
  }
  else if(distancetomin<0 && distancetomax>0) //Current position to the left of tissue
  {
      //Just go to the right
  }
  else if(distancetomin>0 && distancetomax<0) //Current position to the right of issue
  {
      //Just go to the left
  }


  //Return
  arglist = NULL;

  return arglist;
}


//------------------------------------------------------------------------------
//-------------------------  Utilities ---------------------------------------
//------------------------------------------------------------------------------
void *discretisefullslide(double *fullimage, mcr *MCR)
{
  //--------------Takes full slide and breaks it down into chunks--------------
  int n = MCR->nrow;
  mIMG **imglist = (mIMG **)imalloc(E,n*n*sizeof(mIMG*));
  int x,y;

  int size_col = MCR->n_pixel_col;
  int size_row = MCR->n_pixel_row;
  int nxpx = MCR->size_col;

  int k,m,i,j;
  for(k=0;k<n;k++) //Move in y (row)
  {
    y = size_row*k;
    for(m=0;m<n;m++) //Move in x (column)
    {
      x = size_col*m;

      //Create memory space for it
      mIMG *currimg = (mIMG *)imalloc(E,1*sizeof(mIMG));

      //Take the data from the big slide
  	   for(j=0;j<size_row;j++) //Move in y (Row)
       {
         for(i=0;i<size_col;i++) //move in x (Column)
         {
           currimg->image[j*size_col+i] = fullimage[(j+y)*nxpx+(i+x)];
         }
       }

  	    currimg->left_x = x;
        currimg->right_x = x+size_col;
        currimg->top_y = y;
        currimg->bottom_y = y+size_row;

        imglist[k*n+m] = currimg;
    }
  }

  return (void *)imglist;
}


void addIMGtoSeenList(mcr *MCR, mIMG *seenIMG)
{
  //Adds IMG to Seen list
  int n = MCR->nSL;
  int i;
  mIMG **list = MCR->SeenList;


  for(i=0;i<n;i++) //For every image seen already
  {
    if(list[i] == seenIMG)
    {
      MCR->nrep++;
      return;
    }
  }

  mIMG **newlist = (mIMG **)imalloc(E,(n+1)*sizeof(mIMG*)) ;
  for(i=0;i<n;i++) //For every seen image copy
  {
      newlist[i] = list[i];
  }

  newlist[n] = seenIMG;
  MCR->nSL++;

  MCR->SeenList = ifree(E,MCR->SeenList);
  MCR->SeenList = newlist;

  return;
}

void addIMGtoBuffer(mcr *MCR, mIMG *seenIMG)
{
  //Adds IMG to Seen list
  int n = MCR->nbuff;
  int i;
  mIMG **list = MCR->buff;

  mIMG **newlist = (mIMG **)imalloc(E,(n+1)*sizeof(mIMG*)) ;
  for(i=0;i<n;i++) //For every seen image copy
  {
      newlist[i] = list[i];
  }

  newlist[n] = seenIMG;
  MCR->nbuff++;

  MCR->buff = ifree(E,MCR->buff);
  MCR->buff = newlist;
  return;
}

mIMG *acquireimage(mcr *MCR, int row, int col)
{
  //-----------------Finds which image to give it-------------------------
  mIMG *returnimg;
  int n = 10;

 	returnimg = MCR->FullList[row*n+col];

  return returnimg;
}

nodes *QueryGraph(hgph *graph, int row, int col)
{
  //Finds variable nodes within the graph structure & returns them
  //Type = v for variable f for factor

  nodes **nodelist = graph->nodelist;
  int nnodes = graph->nnodes;
  int i;
  hfactor *hfac;
  nodes *currnode, *node;
  char str[MAXLEN];

  sprintf(str, "V%d%d",row,col);
  node = find_node(str_hash(str),graph->nnodes, graph->nodelist);

  if(invalidptr(E,node)) return NULL;
  return node;
}





//----------------------------------------------------------------------------
//------------------------- Write to File ------------------------------------
//----------------------------------------------------------------------------
void *micrscan_GraphStructToFile(void *arglist)
{
  //--------------------Write results to file for debugging purposes---------------------------------
    hgph *graph;
  	FILE *fpointer;
  	char filename[MAXLEN] = "GRAPHSTRUCTURE.txt";
  	fpointer = fopen(filename,"w");
    void *xptr, *argptr;

  	nodes *node, *node2;
  	hfactor *hfac, *hfac2;
  	mvec *vec,*mag;
  	char resultstring[MAXLEN];

    if(dynamic_getarg(arglist,"hgph",&argptr)=='f') return NULL;
    if(!invalidptr(E,argptr)) graph = (hgph *)argptr;

    if(dynamic_getarg(arglist,"xclient",&xptr)=='f') return NULL;

    int nnodes = graph->nnodes;
  	int i = 0;
  	int j = 0;
  	int k = 0;

  	for(i;i<nnodes;i++) //For each node
  	{
  		node = graph->nodelist[i];
  		hfac = (hfactor*)node->ndata;

    	//Print Node name
  		fprintf(fpointer, "***Node: %s\n", hfac->name);
  		fprintf(fpointer, "*Connections: \n");

      //Print Connected Edges
      for(j = 0; j<node->nedges;j++) //For all edges
      {
        node2 = find_node(node->edge[j],graph->nnodes,graph->nodelist);
        hfac2 = (hfactor *)node2->ndata;

        fprintf(fpointer, "Edge: %s\n", hfac2->name);
      }

      //For hfac print probdist,
      if(hfac->type == 'f')
      {
        //Print probdist
        fprintf(fpointer, "Probdist:\n");
        for(j = 0; j<(hfac->nrow)*(hfac->ncol);j++)
        {
          fprintf(fpointer, "%lf", hfac->probdist[j]);
        }

        fprintf(fpointer, "\n");
        //Print columndiscretevalues
        fprintf(fpointer, "ColVal:\n");
        for(j = 0; j<hfac->ncol ;j++)
        {
          fprintf(fpointer, "%lf", hfac->columndiscretevalues[j]);
        }

        fprintf(fpointer, "\n");
        //Print rowdiscretevalues
        fprintf(fpointer, "RowVal:\n");
        for(j = 0; j<hfac->nrow ;j++)
        {
          fprintf(fpointer, "%lf", hfac->rowdiscretevalues[j]);
        }
        fprintf(fpointer, "\n");

        //print nrow
        fprintf(fpointer, "Nrow: %d\n", hfac->nrow);
        //print ncol
        fprintf(fpointer, "NCol: %d\n", hfac->ncol);

        //print columnlabel
        fprintf(fpointer, "Columnlabel: %d\n", hfac->columnlabel);
        //print rowlabel
        fprintf(fpointer, "Rowlabel: %d\n", hfac->rowlabel);

      }

  		fprintf(fpointer, "\n");
  		fprintf(fpointer, "------------------------------------------------------------------\n");

  	}

  	sprintf(resultstring, "File Output: %s", filename);
  	printtoclient(resultstring, xptr);
  	fclose(fpointer);

  	return;
}

//-----------------------------------------------------------------------------
//--------------------------- Heuristic --------------------------------------
//-----------------------------------------------------------------------------

double HeuristicOnTissue(mIMG *image, mcr *MCR)
{
//-------- Determins average light intensity in photo
	double hValue;
  double *imagebox = image->image;

	int size_col = MCR->n_pixel_col; //Number of x pixels
	int size_row = MCR->n_pixel_row; //Number of y pixels

 //Sum out number of 1s in the image
	double sum = 0;
	int i;
	for(i = 0; i < (size_col*size_row) ; i++) // For every pixel
	{
		sum = sum + imagebox[i];
	}

 //Take an average
	hValue = sum/(size_row*size_col) ;

	return hValue;
}


//-----------------------------------------------------------------------------
//------------------------- ACTUAL MICROSCOPE ---------------------------------
//-----------------------------------------------------------------------------
double MeanImageHeuristic(rgb *image)
{
  //TAKES IN RGB , GIVES OUT MEAN OF THE IMAGE
  void *rgbptr;
  uint8_t *red, *green, *blue;
  double *BW, sum, mean;
  int width,height,i;

//  if(dynamic_getarg(arglist,"rgb",&rgbptr)=='f') return NULL;
//  if(!invalidptr(E,rgbptr)) image=(rgb *) rgbptr;

  //-------------Find mean of the channel
  red = image->cred;
  green = image->cgreen;
  blue = image->cblue;

  width = image->width;
  height = image->height;

  BW = (double *)imalloc(E, width*height*sizeof(double));

  for(i=0;i<width*height;i++)
  {
    BW[i] = sqrt((double)red[i]*(double)red[i]+(double)green[i]*(double)green[i]+(double)blue[i]*(double)blue[i]);
  }

//------------------------------Find mean--------------------------------------
  sum = 0;
  for(i=0;i<width*height;i++)
  {
    sum = sum+BW[i];
  }

  mean = sum/(width*height);

//--------------------------------Find max------------------------------------
//  double max = 0;
//  for(i=0;i<width*height;i++)
//  {
//    if(BW[i]>max) max = BW[i];
//  }

  return mean;
}

void *micrscan_AnalyseBuf(void *arglist)
{
  //Analyses buffer to attach observations to the graph

    void *argptr, *argptr2, *argptr3;
    hvideo *vid;
    int i,j,k;
    hgph *graph;
    mcr *MCR;

    //Buffer stuff
    uint8_t *red;
    uint8_t *green;
    uint8_t *blue;
    int ix;
    int iy;
    int nx;
    int ny;
    uint64_t bufpt;
    uint64_t rgbpt;
    rgb *image = (rgb *)imalloc(E,sizeof(rgb));

    //Node search variables
    double mean;
    nodes *fnode, *vnode, *gvnode;
    char vname[MAXLEN], fname[MAXLEN], findname[MAXLEN];;
    hfactor *hfac, *gvhfac;
    int row;
    int col;

    //Gets hVideo structure
    if(dynamic_getarg(arglist,"hvideo",&argptr)=='f') return NULL;
    if(!invalidptr(E,argptr)) vid=(hvideo *) argptr;

    if(dynamic_getarg(arglist,"hgph",&argptr2)=='f') return NULL;
    if(!invalidptr(E,argptr2)) graph = (hgph *)argptr2;

    if(dynamic_getarg(arglist,"mcr",&argptr3)=='f') return NULL;
    if(!invalidptr(E,argptr3)) MCR = (mcr *)argptr3;

    //Get video info
    int nframes = vid->nframes;
    nx = vid->xres;
    ny = vid->yres;
    uint8_t *buf = vid->videobuf;
    image->width = nx;
    image->height = ny;

    red=(uint8_t *)imalloc(E,vid->xres*vid->yres*vid->ncolours*sizeof(uint8_t));
    green=(uint8_t *)imalloc(E,vid->xres*vid->yres*vid->ncolours*sizeof(uint8_t));
    blue=(uint8_t *)imalloc(E,vid->xres*vid->yres*vid->ncolours*sizeof(uint8_t));

    //For every frame stuck in the buffer
    for(i=0;i<nframes;i++)
    {

      //Form Image
      for(iy=0;iy<ny;iy++)
      {
        for(ix=0;ix<nx;ix++)
        {
          bufpt=3*ix+iy*3*nx;
          rgbpt=ix+iy*nx;
          red[rgbpt]=vid->videobuf[bufpt+0];
          green[rgbpt]=vid->videobuf[bufpt+1];
          blue[rgbpt]=vid->videobuf[bufpt+2];
        }
      }

      //Append
      image->cred = red;
      image->cgreen = green;
      image->cblue = blue;

      //Analyse
      mean = MeanImageHeuristic(image);

      //Find graph coordinates
      //Axis and PM were the actions just undertaken
      if(MCR->axis == 'x' && MCR->pm == '+')
      {
        row = MCR->curr_row;
        col = MCR->prev_col + ;
      }
      else if(MCR->axis == 'x' && MCR->pm == '-')
      {
        row = MCR->curr_row;
        col = MCR->prev_col - ;
      }
      else if(MCR->axis == 'y' && MCR->pm == '+')
      {
        row = MCR->curr_row;
        col = MCR->prev_col;
      }
      else if(MCR->axis == 'y' && MCR->pm == '-')
      {
        row = MCR->curr_row;
        col = MCR->prev_col;
      }
      else
      {
        //ERROR
      }

      //Create Variable node
      sprintf(vname, "O%d%d", row, col);
      vnode = createnode(col,row,vname,graph,'v');

      //Create Factor Node
      sprintf(fname, "FO%d%d", row, col);
      fnode = createnode(col,row,fname,graph,'f');

      //Populate Factor Node
      hfac = (hfactor*) fnode->ndata;
      hfac->probdist = MCR->OBSprobdist;
      hfac->columndiscretevalues = MCR->OBScolvals;
      hfac->rowdiscretevalues = MCR->OBSrowvals;
      hfac->nrow = MCR->OBSnrow;
      hfac->ncol = MCR->OBSncol;

      //Find relevant node on graph
      gvnode = (nodes *)QueryGraph(graph,row,col);

      //Create Edges
      add_edge(vnode,fnode);
      add_edge(fnode,gvnode);

      gvhfac = (hfactor*) gvnode->ndata;

      //Hfac Labels
      hfac->columnlabel = str_hash(vname);
      sprintf(findname,"V%d%d",row,col);
      hfac->rowlabel = str_hash(findname);

      //Mark Variable Node as Seen
      ((hfactor *)vnode->ndata)->observed = 't';
      gvhfac->seen = 't';

      //Threshold
      if(mean>0.2)
      {
        mean = 1.0;
      }
      else
      {
        mean = -1.0;
      }

      //Observedvariable
      ((hfactor *)vnode->ndata)->observedvariable = mean;
    }

    //Free
    red = ifree(E,red);
    green = ifree(E,green);
    blue = ifree(E,blue);

    arglist = NULL;
    return arglist;
}

void *UpdateLocation(void *arglist)
{
//Function to update microscope position in MCR

  void *mcrptr, *argptr1, *argptr2, *argptr3;
  mcr *MCR;
  int dur;
  char *pm, *axis;

  if(dynamic_getarg(arglist,"mcr",&mcrptr)=='f') return NULL;
  if(!invalidptr(E,mcrptr)) MCR = (mcr *)mcrptr;

  if(dynamic_getarg(arglist,"dur",&argptr1)=='f') return NULL;
  if(!invalidptr(E,argptr1)) dur = *((int *)argptr1);

  if(dynamic_getarg(arglist,"pm",&argptr2)=='f') return NULL;
  if(!invalidptr(E,argptr2)) pm = (char *)argptr2;

  if(dynamic_getarg(arglist,"axis",&argptr3)=='f') return NULL;
  if(!invalidptr(E,argptr3)) axis = (char *)argptr3;

  int prev_row = MCR->curr_row;
  int prev_col = MCR->curr_col;
  MCR->prev_row = prev_row;
  MCR->prev_col = prev_col;

  //Update
  MCR->pm = *pm;
  MCR->dur = dur;
  MCR->axis = *axis;

  if(*axis == 'x' && *pm =='+')
  {
    MCR->curr_row = prev_row;
    MCR->curr_col = prev_col + dur/50;
  }
  else if(*axis == 'x' && *pm == '-')
  {
    MCR->curr_row = prev_row;
    MCR->curr_col = prev_col - dur/50;
  }
  else if(*axis == 'y' && *pm == '+')
  {
    MCR->curr_row = prev_row + dur/50;
    MCR->curr_col = prev_col;
  }
  else if(*axis == 'y' && *pm == '-')
  {
    MCR->curr_row = prev_row - dur/50;
    MCR->curr_col = prev_col;
  }

  arglist = NULL;
  return arglist;
}

void *micrscan_scaninit(void *arglist)
{
  //-----------SETUP OF PROBLEM ------ ACTUAL MICROSCOPE
  void *xptr, *argptr2, *argptr3, *argptr4, *argptr5;
  char *filename;
  FILE *fptr;
  int height,width;
  double store;
  mcr *MCRDat;
  int i,j,k,n;
  mIMG **imglist;
  int curr_row;
  int curr_col;

  //Get Height and Width
  if(dynamic_getarg(arglist,"HeightDur",&argptr2)=='f') return NULL;
  if(!invalidptr(E,argptr)) height=*((int *) argptr2);

  if(dynamic_getarg(arglist,"WidthDur",&argptr3)=='f') return NULL;
  if(!invalidptr(E,argptr)) width=*((int *) argptr3);

  //Get current row & col
  if(dynamic_getarg(arglist,"CurrCol",&argptr4)=='f') return NULL;
  if(!invalidptr(E,argptr4)) curr_col=*((int *) argptr4);

  if(dynamic_getarg(arglist,"CurrRow",&argptr5)=='f') return NULL;
  if(!invalidptr(E,argptr5)) curr_row=*((int *) argptr5);

  //Get xclient
  if(dynamic_getarg(arglist,"xclient",&xptr)=='f') return NULL;

  double *image = (double *)imalloc(E,height*width*sizeof(double));

  //Creates MCRDAT
  MCRDat = (mcr *)imalloc(E,1*sizeof(mcr));
  MCRDat->rownodeduration = 50; //1 Node in the row direction represents 50 miliseconds
  MCRDat->colnodeduration = 50; //1 Node in the col direction represents 50 miliseconds

  MCRDat->nrow = height / (MCRDat->rownodeduration); //Graph has nrows
  MCRDat->ncol = width / (MCRDat->colnodeduration); //Graph has ncols

  //Default RPI stuff
  MCRDat->freq = 50;

  //DISCRETISES FULL IMAGE
  imglist = (mIMG **) discretisefullslide(image, MCRDat);

  //----------------Create Factor Node Details--------------------------------
  //---- Use Ising Model

  double nu, beta;
  nu = 2.1;
  beta = 1.0;

  //FOR MRF-------------
  double *MRFcolvals = (double *)imalloc(E,2*sizeof(double));
  MRFcolvals[0] = -1;
  MRFcolvals[1] = 1;

  double *MRFrowvals = (double *)imalloc(E,2*sizeof(double));
  MRFrowvals[0] = -1;
  MRFrowvals[1] = 1;

  int MRFnrow = 2;
  int MRFncol = 2;

  double *MRFprobdist = (double *)imalloc(E,4*sizeof(double));
  MRFprobdist[0] = exp(beta*MRFcolvals[0]*MRFrowvals[0]);
  MRFprobdist[1] = exp(beta*MRFcolvals[1]*MRFrowvals[0]);
  MRFprobdist[2] = exp(beta*MRFcolvals[0]*MRFrowvals[1]);
  MRFprobdist[3] = exp(beta*MRFcolvals[1]*MRFrowvals[1]);

  //Normalise
  double sum = 0;
  for(i=0;i<MRFnrow*MRFncol;i++)
  {
    sum = sum + MRFprobdist[i];
  }

  for(i=0;i<MRFnrow*MRFncol;i++)
  {
    MRFprobdist[i] = MRFprobdist[i]/sum;
  }

  //FOR OBSERVATIONS--------
  double *OBScolvals = (double *)imalloc(E,2*sizeof(double));
  OBScolvals[0] = -1;
  OBScolvals[1] = 1;

  double *OBSrowvals = (double *)imalloc(E,2*sizeof(double));
  OBSrowvals[0] = -1;
  OBSrowvals[1] = 1;

  double *OBSprobdist = (double *)imalloc(E,4*sizeof(double));
  OBSprobdist[0] = exp(nu*MRFcolvals[0]*MRFrowvals[0]);
  OBSprobdist[1] = exp(nu*MRFcolvals[1]*MRFrowvals[0]);
  OBSprobdist[2] = exp(nu*MRFcolvals[0]*MRFrowvals[1]);
  OBSprobdist[3] = exp(nu*MRFcolvals[1]*MRFrowvals[1]);

  int OBSnrow = 2;
  int OBSncol = 2;

  //Normalise
  sum = 0;
  for(i=0;i<OBSnrow*OBSncol;i++)
  {
    sum = sum + OBSprobdist[i];
  }

  for(i=0;i<OBSnrow*OBSncol;i++)
  {
    OBSprobdist[i] = OBSprobdist[i]/sum;
  }

  //Append-------------------------------------------------------------------
  MCRDat->curr_row = curr_row;
  MCRDat->curr_col = curr_col;

  //FOR MRF
  MCRDat->MRFprobdist = MRFprobdist;
  MCRDat->MRFcolvals = MRFcolvals;
  MCRDat->MRFrowvals = MRFrowvals;
  MCRDat->MRFnrow = MRFnrow;
  MCRDat->MRFncol = MRFncol;

  //FOR OBSERVATIONS
  MCRDat->OBSprobdist = OBSprobdist;
  MCRDat->OBScolvals = OBScolvals;
  MCRDat->OBSrowvals = OBSrowvals;
  MCRDat->OBSnrow = OBSnrow;
  MCRDat->OBSncol = OBSncol;

  //Output to arglist -------------------------------------------------------
	arglist=NULL;
	dynamic_putarg("MCRDat.mcr","mcr",(void*) MCRDat,SZ,&arglist);
  return arglist;
}
