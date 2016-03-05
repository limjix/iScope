#include "micrscan.h"

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
  MCRDat->n = n;
  MCRDat->d_size_x = width / n;
  MCRDat->d_size_y = height / n;
  MCRDat->image = image;

  //DISCRETISES FULL IMAGE
  imglist = (mIMG **) discretisefullslide(image, MCRDat);

  //Append
  MCRDat->FullList = imglist;
  MCRDat->nFL = 100;

  MCRDat->SeenList = (mIMG **)imalloc(E,1*sizeof(mIMG*));
  MCRDat->nSL = 0;

  //Output to arglist
	arglist=NULL;
	dynamic_putarg("MCRDat.mcr","mcr",(void*) MCRDat,SZ,&arglist);
  return arglist;
}


void *micrscan_controlloop(void *arglist)
{
//-------------- Runs the scanning Algorithm
  void *argptr;

  mcr *MCR;

  if(dynamic_getarg(arglist,"mcr",&argptr)=='f') return NULL;
  if(!invalidptr(E,argptr)) MCR=(mcr *) argptr;
//1. Where has the operator placed the starting position(Hopefully somewhere sensible)
  MCR->curr_x = 2;
  MCR->curr_y = 2;

//  movemicroscope(int nframes, int direction, mcr *MCR, int curr_row, int curr_col)
//2. Scan that start position

//3. Draw the tree

//4. Analyse....how far to go before next scan or move down?


}


void *discretisefullslide(double *fullimage, mcr *MCR)
{
  //--------------Takes full slide and breaks it down into chunks--------------
  int n = MCR->n;
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

void *movemicroscope(int nframes, int direction, mcr *MCR, int curr_row, int curr_col)
{
//--------- Mimics the raspberry Pi moving the stage
// Direction - 1 for x+ , 2 for x-, 3 for y+, 4 for y-

  mIMG *seenIMG;
  int n;
  int new_row;
  int new_col;


  if(direction == 1) //x+
  {
    for(n=0;n<nframes;n++)
    {
      new_row = curr_row;
      new_col = curr_col+n;
      seenIMG = acquireimage(MCR, new_row, new_col);
      addIMGtoSeenList(MCR,seenIMG);
    }
  }
  else if(direction == 2) //x-
  {
    for(n=0;n<nframes;n++)
    {
      new_row = curr_row;
      new_col = curr_col+n;
      seenIMG = acquireimage(MCR, new_row, new_col);
      addIMGtoSeenList(MCR,seenIMG);
    }
  }
  else if(direction ==3) //y+
  {
    for(n=0;n<nframes;n++)
    {
      new_row = curr_row;
      new_col = curr_col+n;
      seenIMG = acquireimage(MCR, new_row, new_col);
      addIMGtoSeenList(MCR,seenIMG);
    }
  }
  else if(direction == 4) //y-
  {
    for(n=0;n<nframes;n++)
    {
      new_row = curr_row;
      new_col = curr_col+n;
      seenIMG = acquireimage(MCR, new_row, new_col);
      addIMGtoSeenList(MCR,seenIMG);
    }
  }
  else //Broken
  {
    printf("NOT VALID");
  }

	return;
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

mIMG *acquireimage(mcr *MCR, int row, int col)
{
  //-----------------Finds which image to give it-------------------------
  mIMG *returnimg;
  int n = 10;

 	returnimg = MCR->FullList[row*n+col];

  return returnimg;
}

double HeuristicOnTissue(mIMG *image, mcr *MCR)
{
//-------- Determins percentage of tissue in sample
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

 //Take an average and find percentage
	hValue = sum/(size_x*size_y) ;

	return hValue;
}
