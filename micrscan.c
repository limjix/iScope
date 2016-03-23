#include "micrscan.h"

//------------------------------------------------------------------------------
//--------------------------iSCOPE FUNCTIONS -----------------------------------
//------------------------------------------------------------------------------

void *micrscan_init(void *arglist)
{
  //-----------SETUP OF PROBLEM ------ SIMULATING A SLIDE
  void *argptr, *xptr, *argptr2, *argptr3;
  char *filename;
  FILE *fptr;
  int height,width;
  double store;
  mcr *MCRDat;
  int i,j,k,n;
  mIMG **imglist;

  //Get user .slide file
  if(dynamic_getarg(arglist,"Filename",&argptr)=='f') return NULL;
  if(!invalidptr(E,argptr)) filename=(char *) argptr;

  //Get Height and Width
  if(dynamic_getarg(arglist,"Heightpx",&argptr2)=='f') return NULL;
  if(!invalidptr(E,argptr)) height=*((int *) argptr2);

  if(dynamic_getarg(arglist,"Widthpx",&argptr3)=='f') return NULL;
  if(!invalidptr(E,argptr)) width=*((int *) argptr3);

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
  MCRDat->size_x = width; //Number of pixels in x
  MCRDat->size_y = height; //Number of pixels in y
  MCRDat->nrow = n;
  MCRDat->ncol = n;
  MCRDat->d_size_x = width / n;
  MCRDat->d_size_y = height / n;
  MCRDat->image = image;

  //DISCRETISES FULL IMAGE
  imglist = (mIMG **) discretisefullslide(image, MCRDat);

  //----------------Create Factor Node Details--------------------------------
  //FOR MRF
  double *MRFprobdist = (double *)imalloc(E,4*sizeof(double));
  probdist[0] = 0.4;
  probdist[1] = 0.1;
  probdist[2] = 0.1;
  probdist[3] = 0.4;

  double *MRFcolvals = (double *)imalloc(E,2*sizeof(double));
  colvals[0] = 0;
  colvals[1] = 1;

  double *MRFrowvals = (double *)imalloc(E,2*sizeof(double));
  rowvals[0] = 0;
  rowvals[1] = 1;

  int MRFnrow = 2;
  int MRFncol = 2;

  //FOR OBSERVATIONS
  double *OBSprobdist = (double *)imalloc(E,4*sizeof(double));
  probdist[0] = 0.4;
  probdist[1] = 0.1;
  probdist[2] = 0.1;
  probdist[3] = 0.4;

  double *OBScolvals = (double *)imalloc(E,2*sizeof(double));
  colvals[0] = 0;
  colvals[1] = 1;

  double *OBSrowvals = (double *)imalloc(E,2*sizeof(double));
  rowvals[0] = 0;
  rowvals[1] = 1;

  int OBSnrow = 2;
  int OBSncol = 2;

  //Append-------------------------------------------------------------------
  MCRDat->FullList = imglist;
  MCRDat->nFL = 100;

  MCRDat->SeenList = (mIMG **)imalloc(E,1*sizeof(mIMG*));
  MCRDat->nSL = 0;

  MCRDat->curr_x = 2;
  MCRDat->curr_y = 2;

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
  void *argptr, *mcrptr;
  int nframes, direction;
  mcr *MCR;
  mIMG *seenIMG;
  int i;
  //-------------------- GETS INSTRUCTIONS ----------------------------------
  if(dynamic_getarg(arglist,"nframes",&argptr)=='f') return NULL;
  if(!invalidptr(E,argptr)) nframes=*((int *) argptr);

  if(dynamic_getarg(arglist,"direction",&argptr)=='f') return NULL;
  if(!invalidptr(E,argptr)) direction=*((int *) argptr);

  if(dynamic_getarg(arglist,"MCR",&mcrptr)=='f') return NULL;
  if(!invalidptr(E,mcrptr)) MCR=(mcr *) mcrptr;

  //-------------------------Clear old buffer---------------------------------
  mIMG **buff = MCR->buff;
  MCR->buff = ifree(E,MCR->buff);
  MCR->buff = (mIMG **)imalloc(E,1*sizeof(mIMG*));

  //-----------------------Execute movement----------------------------------
  int n;
  int curr_row = MCR->curr_y;
  int curr_col = MCR->curr_x;

  int new_row;
  int new_col;

  if(direction == 1) //x+
  {
    for(n=0;n<nframes;n++)
    {
      new_row = curr_row;
      new_col = curr_col+n;
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
        MCR->curr_x = new_col;
        MCR->curr_y = new_row;
        printf("Moved %d",direction);
      }
    }
  }
  else if(direction == 2) //x-
  {
    for(n=0;n<nframes;n++)
    {
      new_row = curr_row;
      new_col = curr_col+n;
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
        MCR->curr_x = new_col;
        MCR->curr_y = new_row;
        printf("Moved %d",direction);
      }
    }
  }
  else if(direction ==3) //y+
  {
    for(n=0;n<nframes;n++)
    {
      new_row = curr_row;
      new_col = curr_col+n;
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
        MCR->curr_x = new_col;
        MCR->curr_y = new_row;
        printf("Moved %d",direction);
      }
    }
  }
  else if(direction == 4) //y-
  {
    for(n=0;n<nframes;n++)
    {
      new_row = curr_row;
      new_col = curr_col+n;
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
        MCR->curr_x = new_col;
        MCR->curr_y = new_row;
        printf("Moved %d",direction);
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
  hgph *graph;
  void *mcrptr;
  mcr *MCR;
  int nrow = MCR->nrow;
  int ncol = MCR->ncol;
  int i, j, nfac, nvar;
  char name[MAXLEN], str1[MAXLEN], str2[MAXLEN];
  nodes *vnode, *fnode, *v1, *v2;
  double *probdist, *colvals, *rowvals;
  hfactor *hfac;

  if(dynamic_getarg(arglist,"MCR",&mcrptr)=='f') return NULL;
  if(!invalidptr(E,mcrptr)) MCR=(mcr *) mcrptr;

  //--------------------------- Creates & Initialise Graph Struct -----------
  graph=(hgph *)imalloc(E,sizeof(hgph));
  graph->nnodes = 0;
  graph->nodelist = NULL;
  nfac = 0;
  nvar = 0;

  //--------------Create X-Y Variable Node Grid -----------------------------
  for(i=0;i<nrow;i++) //Going across row
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
  for(i=0; i<(nrow-1);i++) //Going down row (n-1) is because spaces in between variable
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
  for(j=0;j<(ncol-1);j++) //Going across col (n-1) is because spaces in between variable
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

  void *argptr;
  mcr *MCR;
  int i;
  nodes *fnode, *vnode, *gvnode;
  char name[MAXLEN];
  mIMG *image;
  hfactor *hfac;

  if(dynamic_getarg(arglist,"mcr",&argptr)=='f') return NULL;
  if(!invalidptr(E,argptr)) MCR = (mcr *)argptr;

  mIMG **buff = MCR->buff;
  int nbuff = MCR->nbuff;
  double result;

  for(i=0;i<nbuff;i++) //For every image
  {
    image = buff[i];
    result = HeuristicOnTissue(image,MCR);

    //Create Variable Node
    sprintf(name, "O%d%d",);
    vnode = createnode(i,j,name,graph,'v');
    //Create Factor Node
    sprintf(name, "FO%d%d",);
    fnode = createnode(i,j,name,graph,'f');

    //Populate Factor Node
    hfac = (hfactor*) fnode->ndata;
    hfac->probdist = MCR->OBSprobdist;
    hfac->columndiscretevalues = MCR->OBScolvals;
    hfac->rowdiscretevalues = MCR->OBSrowvals;
    hfac->nrow = MCR->OBSnrow;
    hfac->ncol = MCR->OBSncol;

    //Find relevant node on graph


    //Create Edge
    add_edge(vnode,fnode)

    //Mark Variable Node as Seen
    ((hfactor *)gvnode->ndata)->observed = 't';
    ((hfactor *)gvnode->ndata)->observedvariable = result;
  }

  arglist = NULL;
  return arglist;
}

void *micrscan_DecideNextMove(void *arglist)
{
  //------Looking at the previous scan line, is it worth going back?
  //------Looking at the next scan line, decides how far should it go
  void *argptr;
  hgph *graph;
  if(dynamic_getarg(arglist,"hgph",&argptr)=='f') return NULL;
  if(!invalidptr(E,argptr)) graph = (hgph *)argptr;



  arglist = NULL;
  //Needs to return ncmds and instructions for RPI
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

  int size_x = MCR->d_size_x;
  int size_y = MCR->d_size_y;
  int nxpx = MCR->size_x;

  int k,m,i,j;
  for(k=0;k<n;k++) //Move in y (row)
  {
      y = size_y*k;
    for(m=0;m<n;m++) //Move in x (column)
    {
      x = size_x*m;

      //Create memory space for it
      mIMG *currimg = (mIMG *)imalloc(E,1*sizeof(mIMG));

      //Take the data from the big slide
  	   for(j=0;j<size_y;j++) //Move in y (Row)
       {
         for(i=0;i<size_x;i++) //move in x (Column)
         {
           currimg->image[j*size_x+i] = fullimage[(j+y)*nxpx+(i+x)];
         }
       }

  	    currimg->left_x = x;
        currimg->right_x = x+size_x;
        currimg->top_y = y;
        currimg->bottom_y = y+size_y;

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

//-----------------------------------------------------------------------------
//--------------------------- Heuristic --------------------------------------
//-----------------------------------------------------------------------------

double HeuristicOnTissue(mIMG *image, mcr *MCR)
{
//-------- Determins average light intensity in photo
	double hValue;
  double *imagebox = image->image;

	int size_x = MCR->d_size_x; //Number of x pixels
	int size_y = MCR->d_size_y; //Number of y pixels

 //Sum out number of 1s in the image
	double sum = 0;
	int i;
	for(i = 0; i < (size_x*size_y) ; i++) // For every pixel
	{
		sum = sum + imagebox[i];
	}

 //Take an average
	hValue = sum/(size_x*size_y) ;

	return hValue;
}

//------------------------------------------------------------------------------
//-------------------------- Thresholding Functions ----------------------------
//------------------------------------------------------------------------------

void mimicthresholding()
{

}

//-----------------------------------------------------------------------------
//------------------------- STUFF FOR THE ACTUAL MICROSCOPE -------------------
//-----------------------------------------------------------------------------
void *micrscan_MeanImageHeuristic(void *arglist)
{
  //-----Tests image manipulation
  //-------WORKS TO GIVE MEAN OF IMAGE
  void *rgbptr;
  rgb *image;
  uint8_t *red, *green, *blue;
  double *BW, sum, mean;
  int width,height,i;

  if(dynamic_getarg(arglist,"rgb",&rgbptr)=='f') return NULL;
  if(!invalidptr(E,rgbptr)) image=(rgb *) rgbptr;

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
  double max = 0;
  for(i=0;i<width*height;i++)
  {
    if(BW[i]>max) max = BW[i];
  }

  return NULL;
}

void thresholding()
{

}
