#include "callback.h"

//gcc -g -fPIC -c callback.c && gcc -g -fPIC -dynamiclib -o libcallback.dylib callback.o -L. -lutilities -ldynamic

void *xclient_callback_command(void *arglist)
{
  void *interpreter;
  void *argptr;
  void *retargs;
  void *xptr;
  void *tokengraph;
  int panel;
  int nchars;
  char *text;
  char script[MAXLEN];
  xclient *user;

  // ********************************************************* //
  // ** XSERVER REQUESTS CLIENT TO PROCESS USER TEXT STRING ** //
  // ********************************************************* //

  if(dynamic_getarg(arglist,"xclient",&xptr)=='f') return NULL;
  if(dynamic_getarg(arglist,"panel",&argptr)=='f') return NULL;   if(!invalidptr(E,argptr)) panel=*((int *) argptr);
  if(dynamic_getarg(arglist,"nchars",&argptr)=='f') return NULL;  if(!invalidptr(E,argptr)) nchars=*((int *) argptr);
  if(dynamic_getarg(arglist,"text",&argptr)=='f') return NULL;    if(!invalidptr(E,argptr)) text=(char *) argptr;
  //printf("callback: p=%p panel=%d nchars=%d text=%s\n",p,panel,nchars,text);

  user=(xclient *) xptr;
  if(invalidptr(E,user)) return NULL;

  printf("callback: callback from text panel %d: %s",panel,text);
  request_event_write(user->window,str_length(text),text);    //copy user input text to screen history

  //populate object list with interpreter and GUI variables if they don't already exist
  if(dynamic_getarg(user->objlist,"funclist",&argptr)=='f')
    dynamic_putarg("std.void","funclist",(void *) user->funclist,SZ,&(user->objlist));

  if(dynamic_getarg(user->objlist,"xclient",&argptr)=='f')
    dynamic_putarg("std.void","xclient",xptr,SZ,&(user->objlist));

  if(dynamic_getarg(user->objlist,"window",&argptr)=='f')
    dynamic_putarg("std.char","window",(void *) user->window,SZ,&(user->objlist));

  if(dynamic_getarg(user->objlist,"objlist",&argptr)=='f')
    dynamic_putarg("std.void","objlist",(void *) user->objlist,SZ,&(user->objlist)); //self-reference last

  //prepare argument list according to button operation or carriage return
  if(user->interpret=='t')
  {
    //strip trailing \n character from command text
    sscanf(text,"%s\n",script);

    //button-operated execution: include all the default GUI variables and pass a script (without 'quotes')
    arglist=NULL;
    dynamic_putarg("std.void","funclist",(void *) user->funclist,SZ,&arglist);
    dynamic_putarg("std.void","xclient",xptr,SZ,&arglist);
    dynamic_putarg("std.char","window",(void *) user->window,SZ,&arglist);
    dynamic_putarg("std.void","objlist",(void *) user->objlist,SZ,&arglist);
    dynamic_putarg("std.char","script",(void *) script,SZ,&arglist);
    user->interpret='f';
  }
  else
  {
    arglist=NULL;
    dynamic_putarg("std.void","objlist",(void *) user->objlist,SZ,&arglist); //pass memory space to runtime
    dynamic_putarg("std.char","segment",(void *) text,SZ,&arglist);
  }

  //execute user command
  dynamic_call("interpret","interpret_code",'s',arglist,&interpreter);
  dynamic_wait(interpreter,&retargs);
  dynamic_closeargs(arglist);
  dynamic_getarg(retargs,"objlist",&(user->objlist));  //retrieve updated object list
  dynamic_closeargs(retargs);

  return NULL;
}

void *xclient_callback_button(void *arglist)
{
  xclient *user;
  panels *pan;
  char text[MAXLEN];
  char cmdlist[MAXLEN];
  char control;

  void *vidstream;
  void *argptr;
  void *xptr;
  int panel;
  char context;
  int button;
  int nchars;
  char *buttonname;

  // **************************************************** //
  // ** SERVER REQUESTS CLIENT TO PROCESS BUTTON CLICK ** //
  // **************************************************** //

  //printf("callback: callback_xserver_button()\n");
  if(dynamic_getarg(arglist,"xclient",&xptr)=='f') return NULL;
  if(dynamic_getarg(arglist,"panel",&argptr)=='f') return NULL;      if(!invalidptr(E,argptr)) panel=*((int *) argptr);
  if(dynamic_getarg(arglist,"context",&argptr)=='f') return NULL;    if(!invalidptr(E,argptr)) context=*((char *) argptr);
  if(dynamic_getarg(arglist,"button",&argptr)=='f') return NULL;     if(!invalidptr(E,argptr)) button=*((int *) argptr);
  if(dynamic_getarg(arglist,"nchars",&argptr)=='f') return NULL;     if(!invalidptr(E,argptr)) nchars=*((int *) argptr);
  if(dynamic_getarg(arglist,"buttonname",&argptr)=='f') return NULL; if(!invalidptr(E,argptr)) buttonname=(char *) argptr;
  //printf("callback: p=%p panel=%d context=%c button=%d nchars=%d buttonname=%s\n",p,panel,context,button,nchars,buttonname);

  user=(xclient *) xptr;
  if(invalidptr(E,user)) return NULL;
  if(panel<0||panel>=user->npanels) return NULL;
  if(invalidptr(E,user->pan)) return NULL;
  pan=user->pan[panel];
  if(invalidptr(E,pan)) return NULL;

  printf("callback: button '%s' clicked in context %c wrt panel %d\n",buttonname,context,panel);

  if(context=='i'&&str_hash(buttonname)==str_hash("colourmap"))
  {
    //cycle colour scheme and update image
    if(pan->colourmap=='m')      pan->colourmap='b';
    else if(pan->colourmap=='b') pan->colourmap='t';
    else if(pan->colourmap=='t') pan->colourmap='m';

    screen_redraw(xptr,panel,'n'); //update screen
  }
  else if(context=='i'&&str_hash(buttonname)==str_hash("nuclei"))
  {
    //toggle vector graphics
    if(pan->vecvisible=='t') pan->vecvisible='f';
    else                     pan->vecvisible='t';

    screen_redraw(xptr,panel,'n');
  }
  else if(context=='i'&&str_hash(buttonname)==str_hash("stream"))
  {
    printf("callback: stream control... pan->livestream=%c\n",pan->livestream);

    memset(text,0,MAXLEN*sizeof(char));

    //toggle streaming (play/pause)
    if(pan->livestream=='c')
    {
      pan->livestream='d'; //preserve info on type of streaming when toggling
      sprintf(text,"stream off\n");
    }
    else if(pan->livestream=='v')
    {
      control='h';
      arglist=NULL;
      dynamic_putarg("std.char","control",(void *) &control,SZ,&arglist);
      dynamic_call("vidstream","vidstream_control",'s',arglist,&vidstream);
      dynamic_wait(vidstream,NULL);
      dynamic_closeargs(arglist);
      //vidstream_control('h');
      pan->livestream='w';
      sprintf(text,"stream off\n");
    }
    else if(pan->livestream=='d')
    {
      pan->livestream='c';
      sprintf(text,"camera streaming\n");
    }
    else if(pan->livestream=='w')
    {
      control='r';
      arglist=NULL;
      dynamic_putarg("std.char","control",(void *) &control,SZ,&arglist);
      dynamic_call("vidstream","vidstream_control",'s',arglist,&vidstream);
      dynamic_wait(vidstream,NULL);
      dynamic_closeargs(arglist);
      //vidstream_control('r');
      pan->livestream='v';
      sprintf(text,"video streaming\n");
    }

    request_event_write(user->window,str_length(text),text);
  }
  else if(context=='c'&&str_hash(buttonname)==str_hash("help"))
  {
    //list needs to be broken into small chunks to prevent undiagnosable seg fault
    memset(cmdlist,0,MAXLEN*sizeof(char));

    sprintf(cmdlist,"reserved file extensions:\n .vid .eps .png\n");
    sprintf(cmdlist,"%scommand format:\n",cmdlist);
    sprintf(cmdlist,"%soutputs<=function<=inputs\n",cmdlist);
    sprintf(cmdlist,"%s\n",cmdlist);

    request_event_write(user->window,str_length(cmdlist),cmdlist);
    memset(cmdlist,0,MAXLEN*sizeof(char));

    sprintf(cmdlist,"argument format:\n");
    sprintf(cmdlist,"%s outputobj:outputarg\n",cmdlist);
    sprintf(cmdlist,"%s inputarg:inputobj\n",cmdlist);
    sprintf(cmdlist,"%s\n",cmdlist);

    request_event_write(user->window,str_length(cmdlist),cmdlist);
    memset(cmdlist,0,MAXLEN*sizeof(char));

    sprintf(cmdlist,"function call format:\n");
    sprintf(cmdlist,"%slib.func.tag calls\n",cmdlist);
    sprintf(cmdlist,"%svoid *lib_func(void *arglist)\n",cmdlist);
    sprintf(cmdlist,"%sin " LIBPRE "lib" LIBPOST "\n",cmdlist);

    request_event_write(user->window,str_length(cmdlist),cmdlist);
    memset(cmdlist,0,MAXLEN*sizeof(char));

    sprintf(cmdlist,"core libraries:\n");
    sprintf(cmdlist,"%s xclient\n",cmdlist);
    sprintf(cmdlist,"%s xserver\n",cmdlist);
    sprintf(cmdlist,"%s interpret\n",cmdlist);
    sprintf(cmdlist,"%s dynamic\n",cmdlist);
    sprintf(cmdlist,"%s graph\n",cmdlist);
    sprintf(cmdlist,"%s export\n",cmdlist);
    sprintf(cmdlist,"%s import\n",cmdlist);
    sprintf(cmdlist,"%s utilities\n",cmdlist);

    request_event_write(user->window,str_length(cmdlist),cmdlist);
    memset(cmdlist,0,MAXLEN*sizeof(char));

    sprintf(cmdlist,"auxiliary libraries:\n");
    sprintf(cmdlist,"%s rpiclient\n",cmdlist);
    sprintf(cmdlist,"%s vidstream\n",cmdlist);
    sprintf(cmdlist,"%s camera\n",cmdlist);
    sprintf(cmdlist,"%s voronoi\n",cmdlist);
    sprintf(cmdlist,"%s delaunay\n",cmdlist);
    sprintf(cmdlist,"%s cluster\n",cmdlist);
    sprintf(cmdlist,"%s godunov\n",cmdlist);

    request_event_write(user->window,str_length(cmdlist),cmdlist);
    memset(cmdlist,0,MAXLEN*sizeof(char));
  }
  else if(context=='c'&&str_hash(buttonname)==str_hash("script"))
  {
    //shortcut to correctly execute a script without typing:
    // <-interpret.code<-xclient:xclient,window:window,objlist:objlist,
    //
    // script:'###'; or segment:{ }; must follow to execute properly

    //pass command to xclient_callback_command with a flag
    user->interpret='t';
    request_event_keystroke(user->window,CRETURN,0,&panel); //any old integer will do, use panel
  }
  else
  {
    printf("callback: unknown button '%s' clicked in context %c wrt panel %d\n",buttonname,context,panel);
    sleep(1);
  }

  return NULL;
}

void *xclient_callback_keystroke(void *arglist)
{
  unsigned lyrhash;
  unsigned showhide;
  int mvzm;
  char stream;
  char refresh;
  char text[MAXLEN];
  xclient *user;
  panels *pan;
  keys *keymap;
  rgb *img;

  void *vidstream;
  void *pwmdrive;
  char axis;
  char pm;
  int dur;

  void *argptr;
  void *xptr;
  int panel;
  int scrnx;
  int scrny;
  char command;

  // **************************************************** //
  // ** REDRAWING SCREEN ACCORDING TO USER KEY COMMAND ** //
  // **************************************************** //

  //printf("callback: callback_xserver_keystroke()\n");
  if(dynamic_getarg(arglist,"xclient",&xptr)=='f') return NULL;
  if(dynamic_getarg(arglist,"panel",&argptr)=='f') return NULL;   if(!invalidptr(E,argptr)) panel=*((int *) argptr);
  if(dynamic_getarg(arglist,"scrnx",&argptr)=='f') return NULL;   if(!invalidptr(E,argptr)) scrnx=*((int *) argptr);
  if(dynamic_getarg(arglist,"scrny",&argptr)=='f') return NULL;   if(!invalidptr(E,argptr)) scrny=*((int *) argptr);
  if(dynamic_getarg(arglist,"command",&argptr)=='f') return NULL; if(!invalidptr(E,argptr)) command=*((char *) argptr);
  //printf("callback: p=%p panel=%d scrnx=%d scrny=%d command=%c\n",p,panel,scrnx,scrny,command);

  user=(xclient *) xptr;
  if(invalidptr(E,user)) return;
  if(panel<0||panel>=user->npanels||invalidptr(E,user->pan)) return;
  keymap=user->keymap;
  pan=user->pan[panel];
  if(invalidptr(E,pan)) return;
  if(invalidptr(E,keymap)) return;

  printf("callback: image key %c for panel %d\n",command,panel);
  printf("callback: panel stream flag: %c\n",pan->livestream);

  if(pan->livestream=='c')
  {
    // ************************** //
    // ** VIEW LIVE IMAGE DATA ** //
    // ************************** //

    if(keymap_ismovezoom(keymap,command))
    {
      mvzm=keymap_getmovezoom(keymap,command);

      //move microscope directly (using duration previously set by command prompt)
      if(mvzm=='l') { axis='x'; pm='+'; dur=-1; }
      if(mvzm=='r') { axis='x'; pm='-'; dur=-1; }
      if(mvzm=='b') { axis='y'; pm='+'; dur=-1; }
      if(mvzm=='f') { axis='y'; pm='-'; dur=-1; }
      if(mvzm=='d') { axis='z'; pm='+'; dur=-1; }
      if(mvzm=='u') { axis='z'; pm='-'; dur=-1; }

      arglist=NULL;
      dynamic_putarg("std.char","axis",(void *) &axis,SZ,&arglist);
      dynamic_putarg("std.char","pm",(void *) &pm,SZ,&arglist);
      dynamic_putarg("std.int","dur",(void *) &dur,SZ,&arglist);
      dynamic_call("rpiclient","rpiclient_pwmdrive",'s',arglist,&pwmdrive);
      dynamic_wait(pwmdrive,NULL);
      dynamic_closeargs(arglist);

      //zoom live image out
      if(mvzm=='-')
        pan->livezoom*=2;

      //zoom live image in
      if(mvzm=='+')
        if(pan->livezoom>1)
	  pan->livezoom/=2;
    }
    else if(keymap_isrefresh(keymap,command))
    {
      refresh=keymap_getrefresh(keymap,command);

      if(refresh=='g')
        request_event_screen(user->window,str_hash("grab"),panel); //trigger screen grab to callback screen grab
    }
    else
    {
      sprintf(text,"command %c: unknown live instruction",command);
      request_event_write(user->window,str_length(text),text);
    }
  }
  else if(pan->livestream=='v'||pan->livestream=='w')
  {
    // **************************** //
    // ** VIEW VIDEO STREAM DATA ** //
    // **************************** //

    //order of search is important: here '[' and ']' are duplicated commands in kyb.cfg
    //and priority is to use them for frame-stepping, not for out-of plane movement, so
    //keymap_isstream() will find a match first, only if this fails will movezoom be active
    if(keymap_isstream(keymap,command))
    {
      stream=keymap_getstream(keymap,command);

      printf("callback: callback_image_key %c: stream control %c\n",command,stream);

      arglist=NULL;
      dynamic_putarg("std.char","control",(void *) &stream,SZ,&arglist);
      dynamic_call("vidstream","vidstream_control",'s',arglist,&vidstream);
      dynamic_wait(vidstream,NULL);
      dynamic_closeargs(arglist);
      //vidstream_control(stream);
    }
    else if(keymap_ismovezoom(keymap,command))
    {
      printf("callback: callback_image_key %c: movezoom\n",command);
      mvzm=keymap_getmovezoom(keymap,command);

      screen_redraw(xptr,panel,mvzm);

      request_event_write(user->window,str_length(text),text);
    }
    else if(keymap_isrefresh(keymap,command))
    {
      refresh=keymap_getrefresh(keymap,command);

      printf("callback: callback_image_key %c: refresh %c\n",command,refresh);

      if(refresh=='i')
      {
	pan->colourmap='m';
        pan->vecvisible='t';

        screen_redraw(xptr,panel,'i');  //initialise

        sprintf(text,"(re)initialise");
        request_event_write(user->window,str_length(text),text);
      }
      else if(refresh=='r') //if data has not changed, (re-)expose layer
      {
        screen_redraw(xptr,panel,'n'); //do nothing to position when refreshing
      }
      else if(refresh=='g')
      {
        request_event_screen(user->window,str_hash("update"),panel); //update screen from double buffer
        request_event_screen(user->window,str_hash("grab"),panel); //trigger screen grab to callback screen grab
      }
    }
    else
    {
      sprintf(text,"command %c: unknown stream request",command);
      request_event_write(user->window,str_length(text),text);
    }
  }
  else
  {
    // **************************** //
    // ** VIEW STORED IMAGE DATA ** //
    // **************************** //

    if(keymap_isshowhide(keymap,command))
    {
      lyrhash=str_hash("null");
      lyrhash=keymap_getlayer(keymap,command);
      showhide=keymap_getshowhide(keymap,command,str_hash("paint"));

      printf("callback: callback_image_key %c: showhide %d layer %d \n",command,showhide,lyrhash);

      request_event_layer(user->window,lyrhash,panel,showhide);
      request_event_screen(user->window,str_hash("update"),panel);
    }
    else if(keymap_ismovezoom(keymap,command))
    {
      printf("callback: callback_image_key %c: movezoom\n",command);
      mvzm=keymap_getmovezoom(keymap,command);

      screen_redraw(xptr,panel,mvzm);

      request_event_write(user->window,str_length(text),text);
    }
    else if(keymap_isrefresh(keymap,command))
    {
      refresh=keymap_getrefresh(keymap,command);

      printf("callback: callback_image_key %c: refresh %c panel %d\n",command,refresh,panel);

      if(refresh=='i')
      {
        //initialise all the layers
	pan->colourmap='m';
        pan->vecvisible='t';

        screen_redraw(xptr,panel,'i');

        sprintf(text,"(re)initialise\n");
        request_event_write(user->window,str_length(text),text);
      }
      else if(refresh=='r') //if data has not changed, (re-)expose layer
      {
        screen_redraw(xptr,panel,'n'); //do not move the position
      }
      else if(refresh=='g')
      {
        request_event_screen(user->window,str_hash("update"),panel); //update screen from double buffer
        request_event_screen(user->window,str_hash("grab"),panel); //trigger screen grab to callback screen grab
      }
    }
    else
    {
      sprintf(text,"command %c: unknown redraw\n",command);
      request_event_write(user->window,str_length(text),text);
    }
  }

  return NULL;
}

void *xclient_callback_pixelclick(void *arglist)
{
  int ivis,iobj;
  int ic;
  double val;
  double tx,ty;
  int xuser,yuser,userpt;
  int camnx,camny,xcen,ycen;
  uint8_t red,green,blue,alpha;
  char text[MAXLEN];
  FILE *fptr;
  xclient *user;
  panels *pan;
  rgb *img;

  void *argptr;
  void *xptr;
  int panel;
  int scrnx;
  int scrny;
  int ix;
  int iy;

  // ******************************************************* //
  // ** SERVER REQUESTS DATA ON A PIXEL AT GIVEN LOCATION ** //
  // ******************************************************* //

  //printf("callback: callback_xserver_pixelclick()\n");
  if(dynamic_getarg(arglist,"xclient",&xptr)=='f') return NULL;
  if(dynamic_getarg(arglist,"panel",&argptr)=='f') return NULL;  if(!invalidptr(E,argptr)) panel=*((int *) argptr);
  if(dynamic_getarg(arglist,"scrnx",&argptr)=='f') return NULL;  if(!invalidptr(E,argptr)) scrnx=*((int *) argptr);
  if(dynamic_getarg(arglist,"scrny",&argptr)=='f') return NULL;  if(!invalidptr(E,argptr)) scrny=*((int *) argptr);
  if(dynamic_getarg(arglist,"ix",&argptr)=='f') return NULL;     if(!invalidptr(E,argptr)) ix=*((int *) argptr);
  if(dynamic_getarg(arglist,"iy",&argptr)=='f') return NULL;     if(!invalidptr(E,argptr)) iy=*((int *) argptr);
  //printf("callback: p=%p panel=%d scrnx=%d scrny=%c ix=%d iy=%d\n",p,panel,scrnx,scrny,ix,iy);

  user=(xclient *) xptr;
  if(invalidptr(E,user)) return NULL;
  if(panel<0||panel>=user->npanels) return NULL;
  if(invalidptr(E,user->pan)) return NULL;
  pan=user->pan[panel];
  if(invalidptr(E,pan)) return NULL;

  if(pan->livestream=='c')
  {
    //in live-stream mode, pick data only from what is rendered to screen
    img=pan->rscreen;
    if(invalidptr(E,img)) return NULL;
    if(invalidptr(E,img->cred)) return NULL;
    if(invalidptr(E,img->cgreen)) return NULL;
    if(invalidptr(E,img->cblue)) return NULL;

    memset(text,0,sizeof(char)*MAXLEN);

    if(ix>=0&&ix<img->width&&iy>=0&&iy<img->height)
    {
      userpt=ix+iy*img->width;

      //get colour
      red=img->cred[userpt];
      green=img->cgreen[userpt];
      blue=img->cblue[userpt];

      //this shouldn't be hardwired
      if((fptr=fopen("cam.cfg","r"))!=NULL)
      {
        //get camera resolution
        fscanf(fptr,"xres=%d\n",&camnx);
        fscanf(fptr,"yres=%d\n",&camny);
        fclose(fptr);

        //get position as a function of camera resolution
        xcen=(camnx/2)-(img->width*pan->livezoom/2);
        ycen=(camny/2)-(img->height*pan->livezoom/2);

        xuser=xcen+ix*pan->livezoom;
        yuser=ycen+iy*pan->livezoom;
      }
      else
      {
        //safe default
	xuser=ix;
	yuser=iy;
      }

      sprintf(text,"pixel[%d,%d]:\nred=%d\ngreen=%d\nblue=%d\n",xuser,yuser,red,green,blue);
    }
    else
    {
      sprintf(text,"pixel[%d,%d]:\noutwith data region\n",ix,iy);
    }

    request_event_write(user->window,str_length(text),text);
  }
  else
  {
    //in off-line mode, use full data-set
    img=NULL;
    for(ivis=0;ivis<pan->nvis;ivis++)
      if(pan->visual[ivis]->drawhash==str_hash("raster"))
      {
	//take last raster object on the visuals list for this panel
        if(dynamic_getarghash(user->objlist,pan->visual[ivis]->namehash,&argptr)=='f') return NULL;
	img=(rgb *) argptr;
      }

    if(invalidptr(S,img)) return NULL;
    if(invalidptr(E,pan->rscreen)) return NULL;

    printf("xclient: raster_pixel offline\n");

    //real-valued position of click in screen
    tx=(double) ix/pan->rscreen->width;
    ty=(double) iy/pan->rscreen->height;

    //integer-valued coordinate in image space
    xuser=pan->scrxmn+tx*(pan->scrxmx-pan->scrxmn);
    yuser=pan->scrymn+ty*(pan->scrymx-pan->scrymn);

    memset(text,0,sizeof(char)*MAXLEN);

    if(xuser>=0&&xuser<img->width&&yuser>=0&&yuser<img->height)
    {
      userpt=xuser+yuser*img->width;

      if(img->valmem!='t') return NULL;
      if(img->chmem!='t') return NULL;
      if(invalidptr(E,img->value)) return NULL;
      if(invalidptr(E,img->cred)) return NULL;
      if(invalidptr(E,img->cgreen)) return NULL;
      if(invalidptr(E,img->cblue)) return NULL;
      if(invalidptr(E,img->calpha)) return NULL;

      val=img->value[userpt];
      red=img->cred[userpt];
      green=img->cgreen[userpt];
      blue=img->cblue[userpt];
      alpha=img->calpha[userpt];

      sprintf(text,"pixel[%d,%d]:\nvalue=%e\nred=%d\ngreen=%d\nblue=%d\nalpha=%d\n",xuser,yuser,val,red,green,blue,alpha);
    }
    else
    {
      sprintf(text,"pixel[%d,%d]:\noutwith data region\n",xuser,yuser);
    }

    request_event_write(user->window,str_length(text),text);
  }

  return NULL;
}

void *xclient_callback_layergrab(void *arglist)
{
  int ic;
  char text[MAXLEN];
  char filename[MAXLEN];
  xclient *user;
  keys *keymap;

  void *argptr;
  void *xptr;
  int panel;
  unsigned lyrhash;
  int scrnx;
  int scrny;
  uint8_t *red;
  uint8_t *green;
  uint8_t *blue;
  uint8_t *alpha;

  // ****************************************************** //
  // ** SERVER SENDS LAYER RGB DATA FOR CLIENT AMUSEMENT ** //
  // ****************************************************** //

  //printf("callback: callback_xserver_layergrab()\n");
  if(dynamic_getarg(arglist,"xclient",&xptr)=='f') return NULL;
  if(dynamic_getarg(arglist,"panel",&argptr)=='f') return NULL;    if(!invalidptr(E,argptr)) panel=*((int *) argptr);
  if(dynamic_getarg(arglist,"lyrhash",&argptr)=='f') return NULL;  if(!invalidptr(E,argptr)) lyrhash=*((unsigned *) argptr);
  if(dynamic_getarg(arglist,"scrnx",&argptr)=='f') return NULL;    if(!invalidptr(E,argptr)) scrnx=*((int *) argptr);
  if(dynamic_getarg(arglist,"scrny",&argptr)=='f') return NULL;    if(!invalidptr(E,argptr)) scrny=*((int *) argptr);
  if(dynamic_getarg(arglist,"red",&argptr)=='f') return NULL;      if(!invalidptr(E,argptr)) red=(uint8_t *) argptr;
  if(dynamic_getarg(arglist,"green",&argptr)=='f') return NULL;    if(!invalidptr(E,argptr)) green=(uint8_t *) argptr;
  if(dynamic_getarg(arglist,"blue",&argptr)=='f') return NULL;     if(!invalidptr(E,argptr)) blue=(uint8_t *) argptr;
  if(dynamic_getarg(arglist,"alpha",&argptr)=='f') return NULL;    if(!invalidptr(E,argptr)) alpha=(uint8_t *) argptr;
  //printf("callback: p=%p panel=%d lyrhash=%d scrnx=%c scrny=%d red=%p green=%p blue=%p alpha=%p\n",p,panel,lyrhash,scrnx,scrny,red,green,blue,alpha);

  user=(xclient *) xptr;
  if(invalidptr(E,user)) return NULL;

  keymap=user->keymap;
  if(invalidptr(E,user->keymap)) return NULL;

  memset(text,0,sizeof(char)*MAXLEN);

  if(keymap_islayer(keymap,lyrhash)>=0)
  {
    sprintf(filename,"layergrab%01d.png",panel);
    export_png(filename,scrnx,scrny,red,green,blue);

    sprintf(filename,"layer grab%01d.eps",panel);
    export_eps(filename,scrnx,scrny,red,green,blue);

    sprintf(text,"grab layer %d [%d,%d]\n",keymap_islayer(keymap,lyrhash),scrnx,scrny);
    request_event_write(user->window,str_length(text),text);
  }
  else
  {
    sprintf(filename,"screengrab%01d.png",panel);
    export_png(filename,scrnx,scrny,red,green,blue);

    sprintf(filename,"screengrab%01d.eps",panel);
    export_eps(filename,scrnx,scrny,red,green,blue);

    sprintf(text,"grab panel %d [%d,%d]\n",panel,scrnx,scrny);
    request_event_write(user->window,str_length(text),text);
  }

  red=ifree(E,red);
  green=ifree(E,green);
  blue=ifree(E,blue);
  alpha=ifree(E,alpha);

  return NULL;
}

void *xclient_callback_shutdown(void *arglist)
{
  void *stream;
  char control;

  // *************************************** //
  // ** CLIENT-SIDE GUI SHUTDOWN SEQUENCE ** //
  // *************************************** //

  printf("callback: xclient_callback_shutdown()\n");

  arglist=NULL;
  dynamic_call("camera","camera_kill",'s',arglist,&stream);
  dynamic_wait(stream,NULL);
  dynamic_call("rpiclient","rpiclient_close",'s',arglist,&stream);
  dynamic_wait(stream,NULL);

  control='k'; //kill command
  arglist=NULL;
  dynamic_putarg("std.char","control",(void *) &control,SZ,&arglist);
  dynamic_call("vidstream","vidstream_control",'s',arglist,&stream);
  dynamic_wait(stream,NULL);
  dynamic_closeargs(arglist);

  return NULL;
}

void *xclient_callback_buffer(void *arglist)
{
  int ip;
  xclient *user;
  panels *pan;
  rgb *img;
  svg *vec;

  void *destroy;
  void *argptr;
  void *xptr;
  int ipanel;
  int npanels;
  char ccontext;
  int xmn;
  int xmx;
  int ymn;
  int ymx;
  int line;

  // *********************************************************** //
  // ** SET UP CLIENT-SIDE SCREEN-BUFFER FOR EACH IMAGE PANEL ** //
  // *********************************************************** //

  if(dynamic_getarg(arglist,"xclient",&xptr)=='f') return NULL;
  if(dynamic_getarg(arglist,"ipanel",&argptr)=='f') return NULL;   if(!invalidptr(E,argptr)) ipanel=*((int *) argptr);
  if(dynamic_getarg(arglist,"npanels",&argptr)=='f') return NULL;  if(!invalidptr(E,argptr)) npanels=*((int *) argptr);
  if(dynamic_getarg(arglist,"ccontext",&argptr)=='f') return NULL; if(!invalidptr(E,argptr)) ccontext=*((char *) argptr);
  if(dynamic_getarg(arglist,"xmn",&argptr)=='f') return NULL;      if(!invalidptr(E,argptr)) xmn=*((int *) argptr);
  if(dynamic_getarg(arglist,"xmx",&argptr)=='f') return NULL;      if(!invalidptr(E,argptr)) xmx=*((int *) argptr);
  if(dynamic_getarg(arglist,"ymn",&argptr)=='f') return NULL;      if(!invalidptr(E,argptr)) ymn=*((int *) argptr);
  if(dynamic_getarg(arglist,"ymx",&argptr)=='f') return NULL;      if(!invalidptr(E,argptr)) ymx=*((int *) argptr);
  //printf("callback: p=%p ipanel=%d npanels=%d ccontext=%c xmn=%d xmx=%d ymn=%d ymx=%d\n",p,ipanel,npanels,ccontext,xmn,xmx,ymn,ymx);

  user=(xclient *) xptr;
  if(invalidptr(E,user)) return NULL;

  //allocate memory to xclient user-exposed structure if not already done
  if(invalidptr(S,user->pan))
  {
    user->pan=(panels **)imalloc(E,npanels*sizeof(panels *));
    if(invalidptr(E,user->pan)) return NULL;

    for(ip=0;ip<npanels;ip++)
    {
      user->pan[ip]=(panels *)imalloc(E,sizeof(panels));
      if(invalidptr(E,user->pan[ip])) return NULL;
      pan=user->pan[ip];

      //initialise constants
      pan->panmem='f';
      pan->livezoom=1;
      pan->livestream='f';
      pan->colourmap='m';
      pan->vecvisible='t';
      pan->nvis=0;
      pan->rscreen=NULL;
      pan->vscreen=NULL;
      pan->visual=NULL;
    }

    user->npanels=npanels;
  }

  //allocate screen buffers to image structures
  if(ipanel>=0&&ipanel<npanels&&ccontext=='i')
  {
    if(invalidptr(E,user->pan)) return NULL;
    pan=user->pan[ipanel];
    if(invalidptr(E,pan)) return NULL;

    //free existing buffer for screen resize
    if(pan->panmem!='f')
    {
      //close_rgb(pan->rscreen);
      line=ILINE;
      arglist=NULL;
      dynamic_putarg("xclient.rgb","object",(void *) pan->rscreen,SZ,&arglist);
      dynamic_putarg("std.int","line",(void *) &line,SZ,&arglist);
      dynamic_putarg("std.char","file",(void *) IFILE,SZ,&arglist);
      dynamic_putarg("std.char","func",(void *) IFUNC,SZ,&arglist);
      dynamic_call("xclient","xclient_rgb_destroy",'s',arglist,&destroy);
      dynamic_wait(destroy,NULL);
      dynamic_closeargs(arglist);

      //close_svg(pan->vscreen);
      line=ILINE;
      arglist=NULL;
      dynamic_putarg("xclient.svg","object",(void *) pan->vscreen,SZ,&arglist);
      dynamic_putarg("std.int","line",(void *) &line,SZ,&arglist);
      dynamic_putarg("std.char","file",(void *) IFILE,SZ,&arglist);
      dynamic_putarg("std.char","func",(void *) IFUNC,SZ,&arglist);
      dynamic_call("xclient","xclient_svg_destroy",'s',arglist,&destroy);
      dynamic_wait(destroy,NULL);
      dynamic_closeargs(arglist);
    }

    printf("callback: initialising screen buffers [%d,%d]\n",xmx-xmn,ymx-ymn);

    pan->panmem='t';

    //initialise screen raster buffer
    pan->rscreen=(rgb *)imalloc(E,sizeof(rgb));
    img=pan->rscreen;

    //printf("debug: pan[%d]->rscreen=%p\n",ipanel,img);

    if(invalidptr(E,img)) return NULL;
    img->valmem='f';
    img->chmem='f';
    img->width=0;
    img->height=0;

    img->cred=(uint8_t *)imalloc(E,(xmx-xmn)*(ymx-ymn)*sizeof(uint8_t));
    img->cgreen=(uint8_t *)imalloc(E,(xmx-xmn)*(ymx-ymn)*sizeof(uint8_t));
    img->cblue=(uint8_t *)imalloc(E,(xmx-xmn)*(ymx-ymn)*sizeof(uint8_t));
    img->calpha=(uint8_t *)imalloc(E,(xmx-xmn)*(ymx-ymn)*sizeof(uint8_t));
    memset(img->cred,3*CHWHITE/4,(xmx-xmn)*(ymx-ymn)*sizeof(uint8_t));
    memset(img->cgreen,3*CHWHITE/4,(xmx-xmn)*(ymx-ymn)*sizeof(uint8_t));
    memset(img->cblue,3*CHWHITE/4,(xmx-xmn)*(ymx-ymn)*sizeof(uint8_t));
    memset(img->calpha,3*CHWHITE/4,(xmx-xmn)*(ymx-ymn)*sizeof(uint8_t));

    //tempted to put a startup logo in here!

    img->chmem='t';
    img->width=xmx-xmn;
    img->height=ymx-ymn;

    //initialise screen vector buffer
    pan->vscreen=(svg *)imalloc(E,sizeof(svg));

    //minimal population of values because pan->vscreen->npts is reset
    //during redraw according to visible portion of underlying data set
    vec=pan->vscreen;

    if(invalidptr(E,vec)) return NULL;
    vec->width=xmx-xmn;
    vec->height=ymx-ymn;
    vec->chmem='f';
  }

  return NULL;
}

void *xclient_callback_imgtoscreen(void *arglist)
{
  int ix,iy;  int scrpt,campt;
  int skip,xcen,ycen;
  int xpt,ypt,xmax,ymax;
  xclient *user;
  panels *pan;
  rgb *img;

  void *argptr;
  void *xptr;
  int panel;
  int nx;
  int ny;
  uint8_t *red;
  uint8_t *green;
  uint8_t *blue;

  // ***************************************************** //
  // ** REQUEST SCREEN UPDATE WITH CURRENT CAMERA IMAGE ** //
  // ***************************************************** //

  //printf("callback: callback_camera_toscreen()\n");
  if(dynamic_getarg(arglist,"xclient",&xptr)=='f') return NULL;
  if(dynamic_getarg(arglist,"panel",&argptr)=='f') return NULL; if(!invalidptr(E,argptr)) panel=*((int *) argptr);
  if(dynamic_getarg(arglist,"nx",&argptr)=='f') return NULL;    if(!invalidptr(E,argptr)) nx=*((int *) argptr);
  if(dynamic_getarg(arglist,"ny",&argptr)=='f') return NULL;    if(!invalidptr(E,argptr)) ny=*((int *) argptr);

//begin debug camera
  //if(dynamic_getarg(arglist,"red",&argptr)=='f') return NULL;   if(!invalidptr(E,argptr))
red=(uint8_t *) argptr;
  //if(dynamic_getarg(arglist,"green",&argptr)=='f') return NULL; if(!invalidptr(E,argptr))
green=(uint8_t *) argptr;
  //if(dynamic_getarg(arglist,"blue",&argptr)=='f') return NULL;  if(!invalidptr(E,argptr))
blue=(uint8_t *) argptr;
  //printf("callback: panel=%d nx=%d ny=%d red=%p green=%p blue=%p\n",panel,nx,ny,red,green,blue);
  red=(uint8_t *)malloc(nx*ny*sizeof(uint8_t));
  green=(uint8_t *)malloc(nx*ny*sizeof(uint8_t));
  blue=(uint8_t *)malloc(nx*ny*sizeof(uint8_t));
  memset(red,0,nx*ny*sizeof(uint8_t));
  memset(green,0,nx*ny*sizeof(uint8_t));
  memset(blue,255,nx*ny*sizeof(uint8_t));
//end debug camera

  user=(xclient *) xptr;
  if(invalidptr(E,user)) return NULL;
  if(panel<0||panel>=user->npanels) return NULL;
  if(invalidptr(E,user->pan)) return NULL;
  pan=user->pan[panel];
  if(invalidptr(E,pan)) return NULL;
  img=pan->rscreen;
  if(invalidptr(E,img)) return NULL;

  if(invalidptr(E,img->cred)) return NULL;
  if(invalidptr(E,img->cgreen)) return NULL;
  if(invalidptr(E,img->cblue)) return NULL;
  if(invalidptr(E,img->calpha)) return NULL;

//begin debug camera
pan->livestream='c';
//end debug camera

  //if first use, set live stream to camera, but off
  if(pan->livestream=='f')
    pan->livestream='d';

  if(pan->livestream=='c')
  {
    printf("callback: loading camera live view to panel %d\n",panel);

    //set to zero so that any unfilled edges are black
    memset(img->cred,0,sizeof(uint8_t)*img->width*img->height);
    memset(img->cgreen,0,sizeof(uint8_t)*img->width*img->height);
    memset(img->cblue,0,sizeof(uint8_t)*img->width*img->height);

    //keep live image in centre of camera field
    //of view for best optical performance
    skip=pan->livezoom;
    xcen=(nx/2)-(img->width*skip/2);
    ycen=(ny/2)-(img->height*skip/2);
    xmax=(img->width-1)*skip+xcen;
    ymax=(img->height-1)*skip+ycen;

    //directly populate screen buffer
    for(iy=0;iy<img->height;iy++)
      for(ix=0;ix<img->width;ix++)
      {
        //get camera-space location
        xpt=ix*skip+xcen;
        ypt=iy*skip+ycen;

        campt=xpt+ypt*nx;
        scrpt=ix+iy*img->width;

        //if within bounds then retrieve from camera buffer
        if(xpt>0&&xpt<=nx&&ypt>0&&ypt<=ny)
        {
          img->cred[scrpt]=red[campt];
          img->cgreen[scrpt]=green[campt];
          img->cblue[scrpt]=blue[campt];
        }
      }

    set_layer(xptr,"live",panel,0);
    request_event_raster(user->window,str_hash("live"),panel,img->width,img->height,img->cred,img->cgreen,img->cblue,img->calpha);
    request_event_screen(user->window,str_hash("clean"),panel);
    request_event_layer(user->window,str_hash("live"),panel,str_hash("expose"));
    request_event_screen(user->window,str_hash("update"),panel);
  }

  return NULL;
}

void *xclient_callback_screenredraw(void *arglist)
{
  char text[MAXLEN];

  void *argptr;
  void *xptr;
  int panel;
  char *filename;
  int iframe;
  xclient *user;

  // ******************************** //
  // ** SEND VIDEO IMAGE TO SCREEN ** //
  // ******************************** //

  //printf("callback: callback_vidstream_toscreen()\n");
  if(dynamic_getarg(arglist,"xclient",&xptr)=='f') return NULL;
  if(dynamic_getarg(arglist,"panel",&argptr)=='f') return NULL;    if(!invalidptr(E,argptr)) panel=*((int *) argptr);
  if(dynamic_getarg(arglist,"filename",&argptr)=='f') return NULL; if(!invalidptr(E,argptr)) filename=(char *) argptr;
  if(dynamic_getarg(arglist,"iframe",&argptr)=='f') return NULL;   if(!invalidptr(E,argptr)) iframe=*((int *) argptr);
  //printf("callback: p=%p panel=%d filename=%s iframe=%d\n",p,panel,filename,iframe);

  user=(xclient *) xptr;
  if(invalidptr(E,user)) return NULL;

  printf("callback: %s frame %d\n",filename,iframe);

  if(iframe==0) //initialisation routine
  {
    if(!invalidptr(E,user->pan))
      if(panel>=0&&panel<user->npanels)
        if(!invalidptr(E,user->pan[panel]))
	  user->pan[panel]->livestream='w'; //video, but streaming off

    screen_redraw(xptr,panel,'i');  //plot image to screen
  }
  else
  {
    screen_redraw(xptr,panel,'n');
  }

  //don't write frame number if continuously updating
  if(iframe>0)
  {
    sprintf(text,"%s frame %d\n",filename,iframe);
    request_event_write(user->window,str_length(text),text);
  }

  return NULL;
}

void *xclient_callback_write(void *arglist)
{
  void *xptr;
  void *argptr;
  int nchars;
  char *string;
  char xstring[MAXLEN];
  xclient *user;

  // ************************************************* //
  // ** PASS MESSAGE TO XSERVER FOR SCREEN PRINTING ** //
  // ************************************************* //

  //obligatory argument
  if(dynamic_getarg(arglist,"string",&argptr)=='f') return NULL;
  if(!invalidptr(E,argptr)) string=(char *) argptr;

  //optional argument nchars
  nchars=str_length(string);
  if(dynamic_getarg(arglist,"nchars",&argptr)=='t')
    if(!invalidptr(E,argptr))
      nchars=*((int *) argptr);

  //optional arguments
  if(dynamic_getarg(arglist,"xclient",&xptr)=='t')
  {
    if(!invalidptr(E,xptr)) user=(xclient *) xptr;

    //ensure that string has a \n finish to print correctly
    if(string[nchars]!='\n') sprintf(xstring,"%s\n",string);
    else sprintf(xstring,"%s",string);

    request_event_write(user->window,str_length(xstring),xstring);
  }
  else
  {
    printf("\nxclient: xclient_callback_write(%s)\n\n",string);
  }

  return NULL;
}

void *xclient_callback_windowtag(void *arglist)
{
  void *xptr;
  void *argptr;
  xclient *user;

  // ********************************************************** //
  // ** STORE X SERVER INSTANCE AS A STRING AND AS AN OBJECT ** //
  // ********************************************************** //

  if(dynamic_getarg(arglist,"xclient",&xptr)=='f') return NULL;
  if(dynamic_getarg(arglist,"instance",&argptr)=='f') return NULL;

  user=(xclient *) xptr;
  if(invalidptr(E,user)) return NULL;
  if(invalidptr(E,argptr)) return NULL;

  //store tag for the xserver as a string in xclient structure
  sprintf(user->window,"%s",(char *) argptr);

  //store tag for the xserver as an object in the gui object list
  dynamic_putarg("str.void","instance",(void *) user->window,SZ,&(user->objlist));

  return NULL;
}

void *xclient_callback_object(void *arglist)
{
  void *argptr;
  xclient *user;
  char *objname;

  // *********************************** //
  // ** STORE OBJECT IN USER->OBJLIST ** //
  // *********************************** //

  if(dynamic_getarg(arglist,"xclient",&argptr)=='f')
    return NULL;
  if(!invalidptr(E,argptr))
    user=(xclient *) argptr;

  if(dynamic_getarg(arglist,"objname",&argptr)=='f')
    return NULL;
  if(!invalidptr(E,argptr))
    objname=(char *) argptr;

  if(dynamic_getarg(arglist,"object",&argptr)=='f')
    return NULL;
  if(!invalidptr(E,argptr))
    dynamic_putarg("std.void",objname,argptr,SZ,&(user->objlist));

  return NULL;
}
