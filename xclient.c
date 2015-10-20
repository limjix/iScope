#include "xclient.h"

//gcc -g -fPIC -c xclient.c && gcc -g -fPIC -dynamiclib -o libxclient.dylib xclient.o keymap.o drawing.o callback.o -L. -lutilities -ldynamic -lexport -limport -lxserver

void *xclient_init(void *arglist)
{
  xclient *user;
  void *kybconfig;
  void *argptr;

  // ************************************************** //
  // ** INITIALISE MAIN CLIENT-SIDE CALLBACK STORAGE ** //
  // ************************************************** //

  //keyboard mapping is obligatory argument
  if(dynamic_getarg(arglist,"kybconfig",&kybconfig)=='f')
  {
    printf("xclient: kybconfig not found\n");
    return NULL;
  }

  if(invalidptr(E,kybconfig)) return NULL;
  printf("xclient: kybconfig: %s\n",(char *) kybconfig);
 
  //allocate client data space
  user=(xclient *)imalloc(E,sizeof(xclient));
  if(invalidptr(E,user)) return NULL;

  //set keyboard mapping
  keymap_init(&(user->keymap),(char *) kybconfig);

  //initialise interpreter environment to NULL
  user->funclist=NULL;
  user->objlist=NULL;

  //self-reference this local gui interpreter environment in the object list
  dynamic_putarg("std.void","objlist",(void *) &(user->objlist),SZ,&(user->objlist));
  dynamic_putarg("std.void","funclist",(void *) &(user->funclist),SZ,&(user->objlist));
  dynamic_putarg("std.void","xclient",(void *) user,SZ,&(user->objlist));

  //return xclient structure as an output argument
  arglist=NULL;
  dynamic_putarg("std.void","xclient",(void *) user,SZ,&arglist);

  return arglist;
}

void *xclient_close(void *arglist)
{
  int ipanel;
  xclient *user;
  void *argptr;

  // *********************************** //
  // ** RELEASE CLIENT-HELD RESOURCES ** //
  // *********************************** //
  
  if(dynamic_getarg(arglist,"xclient",&argptr)=='f') return NULL;
  user=(xclient *) argptr;
  if(invalidptr(E,user)) return NULL;

  keymap_close(user->keymap);
 
  user->funclist=ifree(S,user->funclist);
  user->objlist=ifree(S,user->objlist);

  for(ipanel=0;ipanel<user->npanels;ipanel++)
    close_panel_buffer(user->pan[ipanel]);

  return NULL;
}

void *xclient_view(void *arglist)
{
  xclient *user;
  void *argptr;
  int panel;
  panels *pan;
  vis *visual;
  vis **vnew;
  int ivis,ilyr;
  char *lyrname;
  rgb *img;
  vecs *vec;
  
  // ********************************************* //
  // ** ADD EXISTING OBJECT TO GRAPHICAL OUTPUT ** //
  // ********************************************* //

  //obligatory arguments
  if(dynamic_getarg(arglist,"xclient",&argptr)=='f') 
  {
    printf("xclient: missing xclient pointer\n");
    return NULL;
  }
  user=(xclient *) argptr;
  if(invalidptr(E,user)) return NULL; 
  printf("user=%p\n",user);

  if(dynamic_getarg(arglist,"objname",&argptr)=='f') 
  {
    printf("xclient: missing data identifier\n");
    return NULL;
  }
  if(invalidptr(E,argptr)) return NULL;
  lyrname=(char *) argptr;

  //create new visual with default settings
  visual=NULL;
  visual=(vis *)imalloc(E,sizeof(vis));
  if(invalidptr(E,visual)) return NULL;
  visual->namehash=str_hash(lyrname);
  visual->drawhash=str_hash("raster");
  visual->viewhash=str_hash("expose");

  //optional specifiers
  if(dynamic_getarg(arglist,"render",&argptr)=='f') 
  {
    printf("xclient: no render mode specified, using default 'expose'\n");
  }
  else
  {
    if(invalidptr(E,argptr)) return NULL;
    visual->viewhash=str_hash((char *) argptr);
  }

  if(dynamic_getarg(arglist,"draw",&argptr)=='f') 
  {
    printf("xclient: no draw type specified, using default 'raster'\n");
  }
  else
  {
    if(invalidptr(E,argptr)) return NULL;
    printf("xclient: draw type %s\n",(char *) argptr);
    visual->drawhash=str_hash((char *) argptr);
  }

  if(dynamic_getarg(arglist,"index",&argptr)=='f') 
  {
    printf("xclient: no layer index specified, using default 0\n");
    ilyr=0;
  }
  else
  {
    if(invalidptr(E,argptr)) return NULL;
    ilyr=*((int *) argptr);
    printf("xclient: index %d\n",ilyr);
  }

  if(dynamic_getarg(arglist,"panel",&argptr)=='f') 
  {
    printf("xclient: no panel, using default 0\n");
    panel=0;
  }
  else
  {
    if(invalidptr(E,argptr)) return NULL;
    panel=*((int *) argptr);
    printf("xclient: panel %d\n",panel);
  }

  if(panel>=0&&panel<user->npanels)
  {
    if(invalidptr(E,user->pan)) return NULL;
    pan=user->pan[panel];
    printf("user=%p user->pan=%p user->pan[%d]=%p\n",user,user->pan,panel,pan);
    if(invalidptr(E,pan)) return NULL;
  
    //expand list of visuals
    vnew=NULL;
    vnew=(vis **)imalloc(E,(pan->nvis+1)*sizeof(vis));
    if(invalidptr(E,vnew)) return NULL;
    vnew[pan->nvis]=visual;
      
    if(pan->nvis>0)
    {
      for(ivis=0;ivis<pan->nvis;ivis++)
        vnew[ivis]=pan->visual[ivis];
      
      pan->visual=ifree(E,pan->visual);
    }

    pan->nvis++;
    pan->visual=vnew;

    printf("xclient: view: lyrname=%s %u\n",lyrname,str_hash(lyrname));

    //initialise layer
    set_layer((void *) user,lyrname,panel,ilyr);
    screen_redraw((void *) user,panel,'i');
  }
  else
  {
    //if user arguments are wrong, ifree memory
    visual=ifree(E,visual);
  }

  return NULL;
}

void close_panel_buffer(panels *pan)
{
  int ivis;
  int line;
  void *arglist;

  // ************************************ //
  // ** CLOSE CLIENT-SIDE PANEL MEMORY ** //
  // ************************************ //

  if(invalidptr(E,pan)) return;
  
  //close_rgb(pan->rscreen);
  line=ILINE;
  arglist=NULL;
  dynamic_putarg("xclient.rgb","object",(void *) pan->rscreen,SZ,&arglist);     
  dynamic_putarg("std.int","line",(void *) &line,SZ,&arglist);     
  dynamic_putarg("std.char","file",(void *) IFILE,SZ,&arglist);     
  dynamic_putarg("std.char","func",(void *) IFUNC,SZ,&arglist);     
  xclient_rgb_destroy(arglist);
  
  //close_svg(pan->vscreen);
  line=ILINE;
  arglist=NULL;
  dynamic_putarg("xclient.svg","object",(void *) pan->vscreen,SZ,&arglist);     
  dynamic_putarg("std.int","line",(void *) &line,SZ,&arglist);     
  dynamic_putarg("std.char","file",(void *) IFILE,SZ,&arglist);     
  dynamic_putarg("std.char","func",(void *) IFUNC,SZ,&arglist);     
  xclient_svg_destroy(arglist);
 
  for(ivis=0;ivis<pan->nvis;ivis++)
    pan->visual[ivis]=ifree(E,pan->visual[ivis]);
  
  if(pan->nvis>0)
    pan->visual=ifree(E,pan->visual);
}

void screen_redraw(void *p,int panel,char command)
{
  char done;
  int ivis,iobj;
  char fill;
  unsigned drawtype;
  void *argptr;
  xclient *user;
  panels *pan;
  svg *scv;
  rgb *scr;
  rgb *dat;
  vecs *vec;
  char text[MAXLEN];

  // *************************** //
  // ** REQUEST SCREEN UPDATE ** //
  // *************************** //

  user=(xclient *) p;
  if(invalidptr(E,user)) return;
  if(invalidptr(E,user->pan)) return;
  if(panel<0||panel>=user->npanels) return;
  pan=user->pan[panel];
  if(invalidptr(E,pan)) return;
  scr=pan->rscreen;
  if(invalidptr(E,scr)) return;
  scv=pan->vscreen;
  if(invalidptr(E,scv)) return;
  if(pan->nvis<1) return;
  if(invalidptr(E,pan->visual)) return;
  
  request_event_screen(user->window,str_hash("clean"),panel);
 
  printf("xclient: screen_redraw pan->nvis=%d\n",pan->nvis);

  //scroll through the layer list to expose layers in the given order
  done='f';
  for(ivis=0;ivis<pan->nvis;ivis++)
  {
    printf("xclient: ivis=%d pan->visual[ivis]->drawhash=%u %u pan->visual[ivis]->namehash=%u %u\n",ivis,pan->visual[ivis]->drawhash,str_hash("raster"),pan->visual[ivis]->namehash,str_hash("img"));
   
    if(pan->visual[ivis]->drawhash==str_hash("raster"))
      if(dynamic_getarghash(user->objlist,pan->visual[ivis]->namehash,&argptr)=='t')
      {
        if(invalidptr(E,argptr)) return; 
        dat=(rgb *) argptr;

        printf("xclient: raster redraw\n");

        //only reposition data window once
        if(done=='f')
        {
	  drawing_reposition(scr->width,scr->height,dat->width,dat->height,
	                     &(pan->scrxmn),&(pan->scrxmx),&(pan->scrymn),&(pan->scrymx),panel,command,text); 
	  printf("xclient: screen[%d,%d] raster[%d,%d] window[%d:%d,%d:%d]\n",scr->width,scr->height,dat->width,dat->height,pan->scrxmn,pan->scrxmx,pan->scrymn,pan->scrymx);
          done='t';
        }
	  
        drawing_raster(scr,dat,pan->scrxmn,pan->scrxmx,pan->scrymn,pan->scrymx,pan->colourmap);
   
        request_event_layer(user->window,pan->visual[ivis]->namehash,panel,str_hash("wipe"));
        request_event_raster(user->window,pan->visual[ivis]->namehash,panel,scr->width,scr->height,
                             scr->cred,scr->cgreen,scr->cblue,scr->calpha);
        request_event_layer(user->window,pan->visual[ivis]->namehash,panel,pan->visual[ivis]->viewhash);
      }
  }

  for(ivis=0;ivis<pan->nvis;ivis++)
    if(pan->vecvisible=='t'&&
       (pan->visual[ivis]->drawhash==str_hash("fillcircle")||
	pan->visual[ivis]->drawhash==str_hash("linecircle")||
	pan->visual[ivis]->drawhash==str_hash("fillsquare")||
	pan->visual[ivis]->drawhash==str_hash("linesquare")||
	pan->visual[ivis]->drawhash==str_hash("fillpolygon")||
	pan->visual[ivis]->drawhash==str_hash("linepolygon")||
	pan->visual[ivis]->drawhash==str_hash("solidline")||
	pan->visual[ivis]->drawhash==str_hash("dashline")
       )
      )
    {
      fill='p';
      if(pan->visual[ivis]->drawhash==str_hash("fillsquare")||
         pan->visual[ivis]->drawhash==str_hash("fillcircle")||
         pan->visual[ivis]->drawhash==str_hash("fillpolygon")||
         pan->visual[ivis]->drawhash==str_hash("solidline"))
	fill='f';
         
      if(pan->visual[ivis]->drawhash==str_hash("solidline"))        drawtype=str_hash("drawline");
      else if(pan->visual[ivis]->drawhash==str_hash("dashline"))    drawtype=str_hash("drawline");
      else if(pan->visual[ivis]->drawhash==str_hash("fillcircle"))  drawtype=str_hash("drawcircle");
      else if(pan->visual[ivis]->drawhash==str_hash("linecircle"))  drawtype=str_hash("drawcircle");
      else if(pan->visual[ivis]->drawhash==str_hash("fillsquare"))  drawtype=str_hash("drawsquare");
      else if(pan->visual[ivis]->drawhash==str_hash("linesquare"))  drawtype=str_hash("drawsquare"); 
      else if(pan->visual[ivis]->drawhash==str_hash("fillpolygon")) drawtype=str_hash("drawpolygon");
      else if(pan->visual[ivis]->drawhash==str_hash("linepolygon")) drawtype=str_hash("drawpolygon");

      if(dynamic_getarghash(user->objlist,pan->visual[ivis]->namehash,&argptr)=='f') return;
      if(invalidptr(E,argptr)) return;
      vec=(vecs *) argptr;	  
      printf("xclient: vector redraw\n");
       
      if(done=='f')
      {
        drawing_reposition(scv->width,scv->height,vec->mx[0]-vec->mn[0],vec->mx[1]-vec->mn[1],
                        &(pan->scrxmn),&(pan->scrxmx),&(pan->scrymn),&(pan->scrymx),panel,command,text); 
	printf("xclient: screen[%d,%d] vector[%d,%d] window[%d:%d,%d:%d]\n",scr->width,scr->height,vec->mx[0]-vec->mn[0],vec->mx[1]-vec->mn[1],pan->scrxmn,pan->scrxmx,pan->scrymn,pan->scrymx);
        done='t';
      }
        
      drawing_vector(scv,vec,pan->scrxmn,pan->scrxmx,pan->scrymn,pan->scrymx,'n');
        
      request_event_layer(user->window,pan->visual[ivis]->namehash,panel,str_hash("wipe"));
      request_event_vector(user->window,pan->visual[ivis]->namehash,panel,drawtype,fill,scv->npts,scv->ixpt,scv->iypt,
                           scv->isize,scv->cred,scv->cgreen,scv->cblue,scv->calpha);
      request_event_layer(user->window,pan->visual[ivis]->namehash,panel,pan->visual[ivis]->viewhash);
  }

  request_event_screen(user->window,str_hash("update"),panel); 
  request_event_write(user->window,str_length(text),text); //echo final redraw position information
}

void set_layer(void *p,char *lyrname,int panel,int idx)
{
  xclient *user;

  // ******************************** //
  // ** INITIALISE LAYER ON SERVER ** //
  // ******************************** //

  user=(xclient *) p;
  if(invalidptr(E,user)) return;
  if(invalidptr(E,lyrname)) return;
  
  if(invalidptr(E,user->keymap)) return;
  if(invalidptr(E,user->keymap->layername)) return;

  if(idx>=0&&idx<user->keymap->nlyrkeys)
    user->keymap->layername[idx]=str_hash(lyrname);

  //set up log file in here to record draws and wipes

  request_event_layer(user->window,str_hash(lyrname),panel,str_hash("init"));
}

void *xclient_svg_destroy(void *arglist)
{
  int idim;
  svg *vec;
  int line;
  char *file;
  char *func;
  void *argptr;

  // *********************************** //
  // ** CLOSE SCREEN VECTOR RESOURCES ** //
  // *********************************** //

  //obligatory arguments
  if(dynamic_getarg(arglist,"object",&argptr)=='f') return NULL;
  if(invalidptr(S,argptr)) return NULL;
  vec=(svg *) argptr;
  
  //optional arguments
  if(dynamic_getarg(arglist,"line",&argptr)=='t'&&
     dynamic_getarg(arglist,"file",&argptr)=='t'&&
     dynamic_getarg(arglist,"func",&argptr)=='t')
  {
    dynamic_getarg(arglist,"file",&argptr);
    if(!invalidptr(E,argptr)) 
      line=*((int *) argptr);
    
    dynamic_getarg(arglist,"file",&argptr);
    if(!invalidptr(E,argptr)) 
      file=(char *) argptr;
  
    dynamic_getarg(arglist,"func",&argptr);
    if(!invalidptr(E,argptr)) 
      func=(char *) argptr;
  
    if(vec->chmem=='t')
    {
      vec->ixpt=ifree(line,file,func,'e',vec->ixpt);
      vec->iypt=ifree(line,file,func,'e',vec->iypt);
      vec->isize=ifree(line,file,func,'e',vec->isize);
    
      vec->cred=ifree(line,file,func,'e',vec->cred);
      vec->cgreen=ifree(line,file,func,'e',vec->cgreen);
      vec->cblue=ifree(line,file,func,'e',vec->cblue);
      vec->calpha=ifree(line,file,func,'e',vec->calpha);
      vec->chmem='f';
    }
  
    if(vec->chmem=='f')
      vec=ifree(line,file,func,'e',vec);
  }
  else
  {
    if(vec->chmem=='t')
    {
      vec->ixpt=ifree(E,vec->ixpt);
      vec->iypt=ifree(E,vec->iypt);
      vec->isize=ifree(E,vec->isize);
    
      vec->cred=ifree(E,vec->cred);
      vec->cgreen=ifree(E,vec->cgreen);
      vec->cblue=ifree(E,vec->cblue);
      vec->calpha=ifree(E,vec->calpha);
      vec->chmem='f';
    }
  
    if(vec->chmem=='f')
      vec=ifree(E,vec);
  }

  return NULL;
}

void *xclient_rgb_destroy(void *arglist)
{
  int line;
  char *file;
  char *func;
  void *argptr;
  rgb *img;

  // ******************************* //
  // ** CLOSE RGB IMAGE RESOURCES ** //
  // ******************************* //
  
  //obligatory arguments
  if(dynamic_getarg(arglist,"object",&argptr)=='f') return NULL;
  if(invalidptr(S,argptr)) return NULL;
  img=(rgb *) argptr;
  
  //optional arguments
  if(dynamic_getarg(arglist,"line",&argptr)=='t'&&
     dynamic_getarg(arglist,"file",&argptr)=='t'&&
     dynamic_getarg(arglist,"func",&argptr)=='t')
  {
    dynamic_getarg(arglist,"file",&argptr);
    if(!invalidptr(E,argptr)) 
      line=*((int *) argptr);
    
    dynamic_getarg(arglist,"file",&argptr);
    if(!invalidptr(E,argptr)) 
      file=(char *) argptr;
  
    dynamic_getarg(arglist,"func",&argptr);
    if(!invalidptr(E,argptr)) 
      func=(char *) argptr;
    
    if(img->chmem=='t')
    {
      img->cred=ifree(line,file,func,'e',img->cred);
      img->cgreen=ifree(line,file,func,'e',img->cgreen);
      img->cblue=ifree(line,file,func,'e',img->cblue);
      img->calpha=ifree(line,file,func,'e',img->calpha);
      img->chmem='f';
    }

    if(img->valmem=='t')
    {
      img->value=ifree(line,file,func,'e',img->value);
      img->valmem='f';
    }

    if(img->valmem=='f'&&img->chmem=='f')
      img=ifree(line,file,func,'e',img);
  }
  else
  {
    if(img->chmem=='t')
    {
      img->cred=ifree(E,img->cred);
      img->cgreen=ifree(E,img->cgreen);
      img->cblue=ifree(E,img->cblue);
      img->calpha=ifree(E,img->calpha);
      img->chmem='f';
    }

    if(img->valmem=='t')
    {
      img->value=ifree(E,img->value);
      img->valmem='f';
    }

    if(img->valmem=='f'&&img->chmem=='f')
      img=ifree(E,img);
  }

  return NULL;
}

