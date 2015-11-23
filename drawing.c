#include "drawing.h"

//gcc -g -fPIC -c xclient.c && gcc -g -fPIC -dynamiclib -o libxclient.dylib xclient.o keymap.o drawing.o callback.o -L. -lutilities -ldynamic -lexport -limport -lxserver

void drawing_raster(rgb *scr,rgb *dat,int xmn,int xmx,int ymn,int ymx,char colourmap)
{
  int ix,iy,datpt,scrpt;
  int xdat,ydat;
  double tx,ty,vx,vy;

  // ************************************************************** //
  // ** REDRAW USER DATA AS RASTER GRAPHICS AT SCREEN RESOLUTION ** //
  // ************************************************************** //

  if(invalidptr(E,dat)) return;
  if(invalidptr(E,scr)) return;
  
  if(scr->chmem!='t') return;
  if(invalidptr(E,scr->cred)) return;
  if(invalidptr(E,scr->cgreen)) return;
  if(invalidptr(E,scr->cblue)) return;
  if(invalidptr(E,scr->calpha)) return;
 
  if(dat->valmem!='t') return;
  if(invalidptr(E,dat->value)) return;

  if(dat->chmem!='t') return;
  if(invalidptr(E,dat->cred)) return;
  if(invalidptr(E,dat->cgreen)) return;
  if(invalidptr(E,dat->cblue)) return;
  if(invalidptr(E,dat->calpha)) return;
 
  //loop over the screen and decide what should be included
  for(iy=0;iy<scr->height;iy++)
    for(ix=0;ix<scr->width;ix++)
    {
      //translate between screen space and data window
      vx=(double) ix/scr->width;
      vy=(double) iy/scr->height;
      tx=(double) (xmn+vx*(xmx-xmn))/dat->width;
      ty=(double) (ymn+vy*(ymx-ymn))/dat->height;

      scrpt=ix+iy*scr->width;

      //check if this bit of the screen requires data
      if(tx>0&&tx<1&&ty>0&&ty<1)
      {
        xdat=(int) (tx*dat->width);
        ydat=(int) (ty*dat->height);
	datpt=xdat+ydat*dat->width;

        //map colour to screen (eg. val->RGBA: false colour or monochrome, or RGBA->RGBA: true or balance correction)
	scr->cred[scrpt]=get_colourmap(dat->value[datpt],
	                               dat->cred[datpt],
		  		       dat->cgreen[datpt],
				       dat->cblue[datpt],
				       dat->calpha[datpt],
				       'r',
				       colourmap);
	
	scr->cgreen[scrpt]=get_colourmap(dat->value[datpt],
	                                 dat->cred[datpt],
		  		  	 dat->cgreen[datpt],
					 dat->cblue[datpt],
					 dat->calpha[datpt],
					 'g',
					 colourmap);

	scr->cblue[scrpt]=get_colourmap(dat->value[datpt],
	                                dat->cred[datpt],
					dat->cgreen[datpt],
					dat->cblue[datpt],
					dat->calpha[datpt],
					'b',
					colourmap);

	scr->calpha[scrpt]=get_colourmap(dat->value[datpt],
	                                 dat->cred[datpt],
				  	 dat->cgreen[datpt],
					 dat->cblue[datpt],
					 dat->calpha[datpt],
					 'a',
					 colourmap);
      }
      else
      {
        //default is grey border
        scr->cred[scrpt]=(uint8_t) CHWHITE/2;
        scr->cgreen[scrpt]=(uint8_t) CHWHITE/2;
        scr->cblue[scrpt]=(uint8_t) CHWHITE/2;
        scr->calpha[scrpt]=(uint8_t) CHWHITE/2; //semi-opaque border round raster data region
      }
    }
}

void drawing_vector(svg *scr,hvec *vdat,int xmn,int xmx,int ymn,int ymx,char colourmap)
{
  int xscr,yscr;
  double vx,vy;
  double tx,ty;
  int ipt,npts;
  int xpt,ypt;

  // ********************************************* //
  // ** MAP VECTOR DATA TO SCREEN VECTOR BUFFER ** //
  // ********************************************* //

  //printf("unprotected vector redraw scr=%p vdat=%p vdat->posmem=%c vdat->dpt=%p\n",scr,vdat,vdat->posmem,vdat->dpt);

  if(invalidptr(E,scr)) return;
  if(invalidptr(E,vdat)) return;

  if(vdat->posmem!='t') return;
  if(invalidptr(E,vdat->dpt)) return;
  if(invalidptr(E,vdat->mn)) return;
  if(invalidptr(E,vdat->mx)) return;

  //loop runs at half speed to examine pairs of points
  npts=0;
  for(ipt=0;ipt<vdat->npts/2;ipt++)
  {
    //translate between screen space and data window
    vx=(double) vdat->dpt[2*ipt+0*vdat->npts];
    vy=(double) vdat->dpt[2*ipt+1*vdat->npts];
    tx=(double) (vx-xmn)/(xmx-xmn);
    ty=(double) (vy-ymn)/(ymx-ymn);

    //leave big safety margin (factor of 9 in area) so edges to 
    //off-screen points are rendered until clipped by the screen. 
    //examine points in pairs so that dashline is correctly clipped
    if(tx>-1&&tx<2&&ty>-1&&ty<2)
    //if(tx>0&&tx<1&&ty>0&&ty<1)
    {
      vx=(double) vdat->dpt[2*ipt+1+0*vdat->npts];
      vy=(double) vdat->dpt[2*ipt+1+1*vdat->npts];
      tx=(double) (vx-xmn)/(xmx-xmn);
      ty=(double) (vy-ymn)/(ymx-ymn);
      
      if(tx>-1&&tx<2&&ty>-1&&ty<2)
        npts+=2;
    }
  }

  printf("drawing: vector data (%d) rendered to screen\n",npts);

  //reallocate buffer memory to correct size
  if(scr->chmem=='t')
  {
    scr->ixpt=ifree(E,scr->ixpt);
    scr->iypt=ifree(E,scr->iypt);
    scr->isize=ifree(E,scr->isize);
    
    scr->cred=ifree(E,scr->cred);
    scr->cgreen=ifree(E,scr->cgreen);
    scr->cblue=ifree(E,scr->cblue);
    scr->calpha=ifree(E,scr->calpha);
    scr->chmem='f';
  }

  if(scr->chmem=='f')
  {
    scr->ixpt=(int *)imalloc(E,npts*sizeof(int));
    scr->iypt=(int *)imalloc(E,npts*sizeof(int));
    scr->isize=(int *)imalloc(E,npts*sizeof(int));
    scr->cred=(uint8_t *)imalloc(E,npts*sizeof(uint8_t));
    scr->cgreen=(uint8_t *)imalloc(E,npts*sizeof(uint8_t));
    scr->cblue=(uint8_t *)imalloc(E,npts*sizeof(uint8_t));
    scr->calpha=(uint8_t *)imalloc(E,npts*sizeof(uint8_t));
    if(invalidptr(E,scr->ixpt)) return;
    if(invalidptr(E,scr->iypt)) return;
    if(invalidptr(E,scr->isize)) return;
    if(invalidptr(E,scr->cred)) return;
    if(invalidptr(E,scr->cgreen)) return;
    if(invalidptr(E,scr->cblue)) return;
    if(invalidptr(E,scr->calpha)) return;
    memset(scr->ixpt,0,npts*sizeof(int));
    memset(scr->iypt,0,npts*sizeof(int));
    memset(scr->isize,0,npts*sizeof(int));
    memset(scr->cred,0,npts*sizeof(uint8_t));
    memset(scr->cgreen,0,npts*sizeof(uint8_t));
    memset(scr->cblue,0,npts*sizeof(uint8_t));
    memset(scr->calpha,0,npts*sizeof(uint8_t));
    scr->chmem='t';
  }

  //now populate the points
  npts=0;
  for(ipt=0;ipt<vdat->npts/2;ipt++)
  {
    //translate between screen space and data window
    vx=(double) vdat->dpt[2*ipt+0*vdat->npts];
    vy=(double) vdat->dpt[2*ipt+1*vdat->npts];
    tx=(double) (vx-xmn)/(xmx-xmn);
    ty=(double) (vy-ymn)/(ymx-ymn);

    //use same clipping margin as above
    if(tx>-1&&tx<2&&ty>-1&&ty<2)
    //if(tx>0&&tx<1&&ty>0&&ty<1)
    {
      //this bit of the screen requires data
      xscr=(int) (tx*scr->width);
      yscr=(int) (ty*scr->height);
      
      //test to ensure next point is also within clip zone
      vx=(double) vdat->dpt[2*ipt+1+0*vdat->npts];
      vy=(double) vdat->dpt[2*ipt+1+1*vdat->npts];
      tx=(double) (vx-xmn)/(xmx-xmn);
      ty=(double) (vy-ymn)/(ymx-ymn);
      
      if(tx>-1&&tx<2&&ty>-1&&ty<2)
      //if(tx>0&&tx<1&&ty>0&&ty<1)
      {
        //flip coordinates  
        scr->ixpt[npts]=xscr;
        scr->iypt[npts]=scr->height-yscr;
   
        //fixed colourmap for just now, can develop later as reqd
        scr->isize[npts]=2;
        scr->cred[npts]=255;
        scr->cgreen[npts]=0;
        scr->cblue[npts]=255; 
        scr->calpha[npts]=255; 

        //second point
        xscr=(int) (tx*scr->width);
        yscr=(int) (ty*scr->height);

        //flip coordinates  
        scr->ixpt[npts+1]=xscr;
        scr->iypt[npts+1]=scr->height-yscr;
   
        //fixed colourmap for just now, can develop later as reqd
        scr->isize[npts+1]=1;
        scr->cred[npts+1]=255;
        scr->cgreen[npts+1]=0;
        scr->cblue[npts+1]=255; 
        scr->calpha[npts+1]=255; 
      
        npts+=2;  //increment in pairs
      }
    }
  }

  scr->npts=npts;
}

void drawing_reposition(int scrnx,int scrny,int datnx,int datny,int *sxmn,int *sxmx,int *symn,int *symx,int panel,char command,char *text)
{
  int panstep,xzoom,yzoom;
  int xmn,xmx,ymn,ymx;
 
  // *********************************************** //
  // ** RESET BOUNDS TO MAP DATA BUFFER TO SCREEN ** //
  // *********************************************** //

  panstep=(*sxmx-*sxmn)/8; //pan in increments of screen width
  if(panstep>(*symx-*symn)/8) panstep=(*symx-*symn)/8;  //take min of x and y

  //printf("panstep=%d [%d:%d,%d:%d]\n",panstep,xmn,xmx,ymn,ymx);

  memset(text,0,sizeof(char)*MAXLEN);
 
  //copy current screen window to local vars
  xmn=*sxmn;
  xmx=*sxmx;
  ymn=*symn;
  ymx=*symx;

  //interpret instruction and adjust data patch accordingly
  if(command=='i')
  {
    //centre of data 1:1 screen to data resolution
    xmn=(datnx/2)-(scrnx/2);
    xmx=(datnx/2)+(scrnx/2);
    ymn=(datny/2)-(scrny/2);
    ymx=(datny/2)+(scrny/2);
    sprintf(text,"redraw panel %d : reset\n",panel);
  }
  else if(command=='r') //shift key changes implied direction
  {
    xmn-=panstep;
    xmx-=panstep;
    sprintf(text,"redraw panel %d : right\n",panel);
  }
  else if(command=='l')
  {
    xmn+=panstep; 
    xmx+=panstep;
    sprintf(text,"redraw panel %d : left\n",panel);
  }
  else if(command=='f')
  {
    ymn-=panstep;
    ymx-=panstep;
    sprintf(text,"redraw panel %d : forward\n",panel);
  }
  else if(command=='b')
  {
    ymn+=panstep;
    ymx+=panstep;
    sprintf(text,"redraw panel %d : back\n",panel);
  }
  else if(command=='+')
  {
    //isotropic zoom
    xzoom=(xmx-xmn)/4;
    yzoom=(ymx-ymn)/4;

    if(xmn+xzoom<xmx-xzoom&&ymn+yzoom<ymx-yzoom)
    {
      xmn+=xzoom;
      xmx-=xzoom;
      ymn+=yzoom;
      ymx-=yzoom;
    }
    sprintf(text,"redraw panel %d : in\n",panel);
  }
  else if(command=='-')
  {
    xzoom=(xmx-xmn)/4;
    yzoom=(ymx-ymn)/4;
    
    xmn-=xzoom;
    xmx+=xzoom;
    ymn-=yzoom;
    ymx+=yzoom;
    sprintf(text,"redraw panel %d : out\n",panel);
  }
  else if(command=='n')
  {
    //do nothing
  }
  else
  {
    sprintf(text,"unknown redraw\n");
  }

  printf("drawing: redraw_position: screen [%d,%d] ; user patch [%d:%d,%d:%d]\n",scrnx,scrny,xmn,xmx,ymn,ymx);

  //return modified values
  *sxmn=xmn;
  *sxmx=xmx;
  *symn=ymn;
  *symx=ymx;
}

uint8_t get_colourmap(double val,uint8_t red,uint8_t green,uint8_t blue,uint8_t alpha,char rgba,char mapping)
{
  // **************************************** //
  // ** FALSE COLOUR FROM GREYSCALE OR RGB ** //
  // **************************************** //

  if(mapping=='m') 
  {
    if(rgba=='a') return (uint8_t) (CHWHITE);
    else          return (uint8_t) (val*CHWHITE); //monochrome
  }
  else if(mapping=='b')
  {
    //bipolar
    if(val<-1.0)
    {
      if(rgba=='r') return (uint8_t) (0.0); 
      if(rgba=='g') return (uint8_t) (0.0); 
      if(rgba=='b') return (uint8_t) (CHWHITE);
      if(rgba=='a') return (uint8_t) (CHWHITE);   //fully opaque colour for the moment
    }
    else if(val<0.0)
    {
      if(rgba=='r') return (uint8_t) ((val+1.0)*CHWHITE); 
      if(rgba=='g') return (uint8_t) ((val+1.0)*CHWHITE); 
      if(rgba=='b') return (uint8_t) (CHWHITE); 
      if(rgba=='a') return (uint8_t) (CHWHITE);
    }
    else if(val<1.0)
    {
      if(rgba=='r') return (uint8_t) (CHWHITE); 
      if(rgba=='g') return (uint8_t) ((1.0-val)*CHWHITE); 
      if(rgba=='b') return (uint8_t) ((1.0-val)*CHWHITE); 
      if(rgba=='a') return (uint8_t) (CHWHITE);
    }
    else
    {
      if(rgba=='r') return (uint8_t) (val*CHWHITE);
      if(rgba=='g') return (uint8_t) (0.0); 
      if(rgba=='b') return (uint8_t) (0.0); 
      if(rgba=='a') return (uint8_t) (CHWHITE);
    }
  }
  else if(mapping=='t')
  {
    //rgba true colour
    if(rgba=='r') return red;
    if(rgba=='g') return green;
    if(rgba=='b') return blue;
    if(rgba=='a') return alpha;
  }
}

void *xclient_exporttoraster(void *arglist)
{
  void *filename;
  rgb *img;

  // ************************************ //
  // ** EXPORT EXISTING OBJECT TO FILE ** //
  // ************************************ //
  
  //obligatory arguments
  if(dynamic_getarg(arglist,"file",&filename)=='f') 
  {
    printf("xclient: missing file name\n");
    return NULL;
  }
  if(invalidptr(E,filename)) return NULL;

  if(dynamic_getarg(arglist,"img",&img)=='f') 
  {
    printf("xclient: missing object reference\n");
    return NULL;
  }
  if(invalidptr(E,img)) return NULL;
  
  if(str_match(filename,".eps"))
    export_eps((char *) filename,img->width,img->height,img->cred,img->cgreen,img->cblue);
  else if(str_match(filename,".png"))
    export_png((char *) filename,img->width,img->height,img->cred,img->cgreen,img->cblue);
}

void *xclient_importtoraster(void *arglist)
{
  int ix,iy,nx,ny,pc;
  double val;
  double sqrt3whitewhite;
  char filext[5];
  FILE *fptr;
  char *filename;
  int frame;
  void *argptr;
  rgb *img;

  // ***************************************** //
  // ** GENERATE RGB RASTER IMAGE FROM FILE ** //
  // ***************************************** //

  if(dynamic_getarg(arglist,"file",&argptr)=='f') 
  {
    printf("xclient: file not provided, cannot continue\n");
    return NULL;
  }
  else
  {
    filename=(char *) argptr;
    if(invalidptr(E,filename)) return NULL;
  }

  //optional argument for frame number
  frame=0;
  if(dynamic_getarg(arglist,"frame",&argptr)=='t')
  {
    if(invalidptr(E,argptr)) return NULL; 
    frame=*((int *) argptr);
    printf("xclient: frame=%d\n",frame);
  }

  //check that file can be opened
  fptr=NULL;
  if((fptr=fopen(filename,"r"))!=NULL)
  {
    fclose(fptr);

    //allocate fresh data space
    img=NULL;
    img=(rgb *)imalloc(E,sizeof(rgb));
    if(invalidptr(E,img)) return NULL;

    //read file extension
    for(ix=0;ix<4;ix++)
      filext[ix]=filename[str_length(filename)-4+ix];
    filext[4]=STRINGEND;

    if(str_hash(filext)==str_hash(".png"))
      import_png(filename,&(img->width),&(img->height),&(img->cred),&(img->cgreen),&(img->cblue));
    else if(str_hash(filext)==str_hash(".eps"))
      import_eps(filename,&(img->width),&(img->height),&(img->cred),&(img->cgreen),&(img->cblue));
    else if(str_hash(filext)==str_hash(".vid"))
      import_vid(filename,frame,&(img->width),&(img->height),&(img->cred),&(img->cgreen),&(img->cblue));

    printf("drawing: raster import %s [%d,%d]\n",filename,img->width,img->height);

    //make monochrome value from image and populate alpha channel
    img->value=NULL;
    img->value=(double *)imalloc(E,img->width*img->height*sizeof(double));
    if(invalidptr(E,img->value)) return NULL;
    img->calpha=NULL;
    img->calpha=(uint8_t *)imalloc(E,img->width*img->height*sizeof(uint8_t));
    if(invalidptr(E,img->calpha)) return NULL;

    sqrt3whitewhite=sqrt(3*CHWHITE*CHWHITE);

    if(invalidptr(E,img->cred)||
       invalidptr(E,img->cgreen)||
       invalidptr(E,img->cblue))
      return NULL;
      
    for(ix=0;ix<img->width;ix++)
      for(iy=0;iy<img->height;iy++)
      {
        pc=ix+iy*img->width;
        img->value[pc]=sqrt(img->cred[pc]*img->cred[pc]+
                            img->cgreen[pc]*img->cgreen[pc]+
                            img->cblue[pc]*img->cblue[pc])/sqrt3whitewhite;

        img->calpha[pc]=CHWHITE;
      }

    img->chmem='t';
    img->valmem='t';

    arglist=NULL;
    dynamic_putarg("xclient.rgb","rgb",img,SZ,&arglist);
    return arglist;
  }

  return NULL;
}

void *xclient_rastertoraster(void *arglist)
{
  void *argptr;
  rgb *src;
  rgb *dest;
  int pc,ix,iy,nx,ny;

  // ********************************************** //
  // ** COPY RASTER SOURCE BUFFER TO DESTINATION ** //
  // ********************************************** //
 
  if(dynamic_getarg(arglist,"src",&argptr)=='f')
  {
    printf("xclient: missing src raster to copy to dest\n");
    return NULL;
  } 

  src=(rgb *) argptr;
  if(invalidptr(E,src)) return;
  
  dest=NULL;
  dest=(rgb *)imalloc(E,sizeof(rgb));
  if(invalidptr(E,dest)) return;
  
  //set sizes based on source width
  nx=src->width;
  ny=src->height;

  //copy data
  if(src->chmem=='t')
  {
    dest->cred=NULL;
    dest->cgreen=NULL;
    dest->cblue=NULL;
    dest->calpha=NULL;
    dest->cred=(uint8_t *)imalloc(E,nx*ny*sizeof(uint8_t));
    dest->cgreen=(uint8_t *)imalloc(E,nx*ny*sizeof(uint8_t));
    dest->cblue=(uint8_t *)imalloc(E,nx*ny*sizeof(uint8_t));
    dest->calpha=(uint8_t *)imalloc(E,nx*ny*sizeof(uint8_t));
    if(invalidptr(E,dest->cred)) return;
    if(invalidptr(E,dest->cgreen)) return;
    if(invalidptr(E,dest->cblue)) return;
    if(invalidptr(E,dest->calpha)) return;
    dest->chmem='t';

    for(iy=0;iy<ny;iy++)
      for(ix=0;ix<nx;ix++)
      {
        pc=ix+iy*nx;
        dest->cred[pc]=src->cred[pc];
        dest->cgreen[pc]=src->cgreen[pc];
        dest->cblue[pc]=src->cblue[pc];
        dest->calpha[pc]=src->calpha[pc];
      }
  }

  if(src->valmem=='t')
  {
    dest->value=NULL;
    dest->value=(double *)imalloc(E,nx*ny*sizeof(double));
    if(invalidptr(E,dest->value)) return;
    dest->valmem='t';
    
    for(iy=0;iy<ny;iy++)
      for(ix=0;ix<nx;ix++)
      {
        pc=ix+iy*nx;
        dest->value[pc]=src->value[pc];
      }
  }

  dest->width=src->width;
  dest->height=src->height;

  arglist=NULL;
  dynamic_putarg("xclient.rgb","dest",(void *) dest,SZ,&arglist); 
  return arglist;
}

void *xclient_vectortovector(void *arglist)
{
  void *argptr;
  hvec *src,*dest;
  int ipt,idim,bufsize;

  // ************************************************* //
  // ** COPY VECTOR DATA FROM SOURCE TO DESTINATION ** //
  // ************************************************* //

  if(dynamic_getarg(arglist,"src",&argptr)=='f')
  {
    printf("xclient: missing src vector to copy to dest\n");
    return NULL;
  } 
  src=(hvec *) argptr;
  if(invalidptr(E,src)) return NULL;
  
  //allocate destination
  dest=NULL;
  dest=(hvec *)imalloc(E,sizeof(hvec));
  if(invalidptr(E,dest)) return NULL;

  dest->npts=src->npts;
  dest->nvals=src->nvals;
  dest->ndims=src->ndims;

  dest->mn=NULL;
  dest->mx=NULL;
  dest->mn=(int *)imalloc(E,src->ndims*sizeof(int));
  dest->mx=(int *)imalloc(E,src->ndims*sizeof(int));
  if(invalidptr(E,dest->mn)) return NULL;
  if(invalidptr(E,dest->mx)) return NULL;
  memset(dest->mn,0,src->ndims*sizeof(int));
  memset(dest->mx,0,src->ndims*sizeof(int));
  
  for(idim=0;idim<src->ndims;idim++)
  {
    dest->mn[idim]=src->mn[idim];
    dest->mx[idim]=src->mx[idim];
  }

  if(src->posmem=='t'&&src->ndims>0&&src->npts>0)
  {
    //point coordinates
    dest->dpt=NULL;
    dest->dpt=(double *)imalloc(E,src->npts*src->ndims*sizeof(double));
    if(invalidptr(E,dest->dpt)) return NULL;
    memset(dest->dpt,0,src->npts*src->ndims*sizeof(double));
  
    for(ipt=0;ipt<src->npts*src->ndims;ipt++)
      dest->dpt[ipt]=src->dpt[ipt];  
    
    if(src->valmem=='t')
    {
      //case of vector data of length npts
      dest->val=NULL;
      dest->val=(double *)imalloc(E,src->npts*src->nvals*sizeof(double));
      if(invalidptr(E,dest->val)) return NULL;
      memset(dest->val,0,src->npts*src->nvals*sizeof(double));
    
      for(ipt=0;ipt<src->npts*src->nvals;ipt++)
        dest->val[ipt]=src->val[ipt];  
    }
  }

  if(src->valmem=='t'&&src->posmem=='f'&&src->ndims>0&&src->npts>0)
  {
    //case of grid data
    bufsize=1;
    for(idim=0;idim<src->ndims;idim++)
      bufsize*=src->mx[idim]-src->mn[idim];
    bufsize*=src->nvals;

    dest->val=NULL;
    dest->val=(double *)imalloc(E,bufsize*sizeof(double));
    if(invalidptr(E,dest->val)) return NULL;
    memset(dest->val,0,bufsize*sizeof(double));
    
    for(ipt=0;ipt<bufsize;ipt++)
      dest->val[ipt]=src->val[ipt];  
  } 

  arglist=NULL;
  dynamic_putarg("graph.hvec","dest",(void *) dest,SZ,&arglist);
  return arglist;
}

void *xclient_graphtovector(void *arglist) 
{
  int idim;
  void *argptr;
  void *retargs;
  void *voronoi;
  unsigned plottype;
  hgph *src;
  hvec *dest;

  // ************************************************ //
  // ** COPY NETWORK DATA TO A VECTOR DATA SUB-SET ** //
  // ************************************************ //

  if(dynamic_getarg(arglist,"hgph",&argptr)=='f') 
  {
    printf("xclient: missing hgph to copy to vector dest\n");
    return NULL;
  } 
  src=(hgph *) argptr;
  if(invalidptr(E,src)) return NULL;
  
  plottype=str_hash("delaunay"); //default
  if(dynamic_getarg(arglist,"plot",&argptr)=='t')
  {
    if(invalidptr(E,argptr)) return NULL;
    plottype=str_hash((char *) argptr);
    printf("drawing: %s\n",(char *) argptr);
  }

  dest=NULL;
  dest=(hvec *)imalloc(E,sizeof(hvec));
  if(invalidptr(E,dest)) return NULL;

  //get voronoi triangulation
  arglist=NULL;
  dynamic_putarg("std.int","nnodes",(void *) &(src->nnodes),SZ,&arglist);     
  dynamic_putarg("graph.nodes","nodelist",(void *) src->nodelist,SZ,&arglist);   
  dynamic_putarg("std.unsigned","format",(void *) &plottype,SZ,&arglist);
  dynamic_call("voronoi","voronoi_init",'s',arglist,&voronoi);
  dynamic_wait(voronoi,&retargs);
  if(dynamic_getarg(retargs,"npts",&argptr)=='f') return;    if(!invalidptr(E,argptr)) dest->npts=*((int *) argptr);
  if(dynamic_getarg(retargs,"ptlist",&argptr)=='f') return;  if(!invalidptr(E,argptr)) dest->dpt=(double *) argptr;
  dynamic_closeargs(arglist);
  dynamic_closeargs(retargs);
  //voronoi_init(src->nnodes,src->nodelist,&(dest->npts),&(dest->dpt),plottype); 

  printf("drawing: dest->npts=%d\n",dest->npts);

  dest->ndims=src->ndims;
  dest->nvals=0;
  dest->valmem='f';
  dest->posmem='t';
  
  //this bounding information is crucial to set display
  if(src->ndims>0)
  {
    dest->mn=NULL;
    dest->mx=NULL;
    dest->mn=(int *)imalloc(E,src->ndims*sizeof(int));
    dest->mx=(int *)imalloc(E,src->ndims*sizeof(int));
    if(invalidptr(E,dest->mn)) return;
    if(invalidptr(E,dest->mx)) return;
    memset(dest->mn,0,src->ndims*sizeof(int));
    memset(dest->mx,0,src->ndims*sizeof(int));

    for(idim=0;idim<src->ndims;idim++)
    {
      dest->mn[idim]=src->mn[idim];
      dest->mx[idim]=src->mx[idim];
      printf("dest->mn[%d]=%d dest->mx[%d]=%d\n",idim,dest->mn[idim],idim,dest->mx[idim]);
    }

    dest->dimmem='t';
  }

  arglist=NULL;
  dynamic_putarg("graph.hvec","hvec",(void *) dest,SZ,&arglist);     
  return arglist;
}

