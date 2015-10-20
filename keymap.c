#include "keymap.h"

//gcc -g -fPIC -c keymap.c && gcc -g -fPIC -dynamiclib -o libkeymap.dylib keymap.o -L. -lutilities 

void keymap_init(keys **retkey,char *kybcfg)
{
  FILE *fptr;
  char fline[MAXLEN];
  int ichar;
  int iopt;
  int iline;
  int noptions;
  int nlyrkeys;
  int nlines;
  int varlen[NKYBLINES];
  keys *keymap;

  // ************************************************ //
  // ** READ CONFIGURATION FOR USER KEYS FROM FILE ** //
  // ************************************************ //

  if(invalidptr(E,retkey)) return;
  if(invalidptr(E,kybcfg)) return;

  keymap=NULL;
  keymap=(keys *)imalloc(E,sizeof(keys));
  if(invalidptr(E,keymap)) return;

  fptr=NULL;
  if((fptr=fopen(kybcfg,"r"))!=NULL)
  {
    printf("keymap: mapping %s\n",kybcfg);

    //header for allocation
    nlines=NKYBLINES;
    noptions=0;
    nlyrkeys=0;
    for(iline=0;iline<nlines;iline++)
    {
      for(ichar=0;ichar<MAXLEN;ichar++)
        fline[ichar]=STRINGEND;
      fgets(fline,MAXLEN,fptr);
      
      if(iline==0) varlen[iline]=str_length("zoomin");
      if(iline==1) varlen[iline]=str_length("zoomout");
      if(iline==2) varlen[iline]=str_length("moveleft");
      if(iline==3) varlen[iline]=str_length("moveright");
      if(iline==4) varlen[iline]=str_length("moveback");
      if(iline==5) varlen[iline]=str_length("moveforward");
      if(iline==6) varlen[iline]=str_length("movedown");
      if(iline==7) varlen[iline]=str_length("moveup");
      if(iline==8) varlen[iline]=str_length("reset");
      if(iline==9) varlen[iline]=str_length("init");
      if(iline==10) varlen[iline]=str_length("grab");
      if(iline==11) varlen[iline]=str_length("pause");
      if(iline==12) varlen[iline]=str_length("playback");
      if(iline==13) varlen[iline]=str_length("start");
      if(iline==14) varlen[iline]=str_length("finish");
      if(iline==15) varlen[iline]=str_length("prevframe");
      if(iline==16) varlen[iline]=str_length("nextframe");
      if(iline==17) varlen[iline]=str_length("layeron");
      if(iline==18) varlen[iline]=str_length("layeroff");

      iopt=0;
      ichar=varlen[iline]+2; //zoomin='%c...
      while(fline[ichar]!=STRINGEND)
      {
        iopt++;
        ichar+=4;
      }

      if(iopt>noptions&&iline<8) //lines 8 and 9 are long layeron layeroff lists
        noptions=iopt;

      if(iopt>nlyrkeys&&iline>=8)
        nlyrkeys=iopt;
    }

    keymap->noptions=noptions;
    keymap->nlyrkeys=nlyrkeys;
   
    keymap->zoomin=NULL;
    keymap->zoomout=NULL;
    keymap->moveleft=NULL;
    keymap->moveright=NULL;
    keymap->moveback=NULL;
    keymap->moveforward=NULL;
    keymap->movedown=NULL;
    keymap->moveup=NULL;
    keymap->reset=NULL;
    keymap->init=NULL;
    keymap->grab=NULL;
    keymap->pause=NULL;
    keymap->playback=NULL;
    keymap->start=NULL;
    keymap->finish=NULL;
    keymap->prevframe=NULL;
    keymap->nextframe=NULL;
    keymap->layeron=NULL;
    keymap->layeroff=NULL;
    keymap->layername=NULL;

    keymap->zoomin=(char *)imalloc(E,noptions*sizeof(char));
    keymap->zoomout=(char *)imalloc(E,noptions*sizeof(char));
    keymap->moveleft=(char *)imalloc(E,noptions*sizeof(char));
    keymap->moveright=(char *)imalloc(E,noptions*sizeof(char));
    keymap->moveback=(char *)imalloc(E,noptions*sizeof(char));
    keymap->moveforward=(char *)imalloc(E,noptions*sizeof(char));
    keymap->movedown=(char *)imalloc(E,noptions*sizeof(char));
    keymap->moveup=(char *)imalloc(E,noptions*sizeof(char));
    keymap->reset=(char *)imalloc(E,noptions*sizeof(char));
    keymap->init=(char *)imalloc(E,noptions*sizeof(char));
    keymap->grab=(char *)imalloc(E,noptions*sizeof(char));
    keymap->pause=(char *)imalloc(E,noptions*sizeof(char));
    keymap->playback=(char *)imalloc(E,noptions*sizeof(char));
    keymap->start=(char *)imalloc(E,noptions*sizeof(char));
    keymap->finish=(char *)imalloc(E,noptions*sizeof(char));
    keymap->prevframe=(char *)imalloc(E,noptions*sizeof(char));
    keymap->nextframe=(char *)imalloc(E,noptions*sizeof(char));
    keymap->layeron=(char *)imalloc(E,nlyrkeys*sizeof(char));
    keymap->layeroff=(char *)imalloc(E,nlyrkeys*sizeof(char));
    keymap->layername=(unsigned *)imalloc(E,nlyrkeys*sizeof(unsigned));

    if(invalidptr(E,keymap->zoomin)||
       invalidptr(E,keymap->zoomout)||
       invalidptr(E,keymap->moveleft)||
       invalidptr(E,keymap->moveright)||
       invalidptr(E,keymap->moveback)||
       invalidptr(E,keymap->moveforward)||
       invalidptr(E,keymap->movedown)||
       invalidptr(E,keymap->moveup)||
       invalidptr(E,keymap->reset)||
       invalidptr(E,keymap->init)||
       invalidptr(E,keymap->grab)||
       invalidptr(E,keymap->pause)||
       invalidptr(E,keymap->playback)||
       invalidptr(E,keymap->start)||
       invalidptr(E,keymap->finish)||
       invalidptr(E,keymap->prevframe)||
       invalidptr(E,keymap->nextframe)||
       invalidptr(E,keymap->layeron)||
       invalidptr(E,keymap->layeroff)||
       invalidptr(E,keymap->layername))
      return;

    fseek(fptr,0,SEEK_SET); //go to top of file again

    //data capture
    for(iline=0;iline<nlines;iline++)
    {
      for(ichar=0;ichar<MAXLEN;ichar++)
        fline[ichar]=STRINGEND;
      fgets(fline,MAXLEN,fptr);
      
      iopt=0;
      ichar=varlen[iline]+2; //zoomin='%c...
      while(fline[ichar]!=STRINGEND)
      {
        if(iline==0) keymap->zoomin[iopt]=fline[ichar];
        if(iline==1) keymap->zoomout[iopt]=fline[ichar];
        if(iline==2) keymap->moveleft[iopt]=fline[ichar];
        if(iline==3) keymap->moveright[iopt]=fline[ichar];
        if(iline==4) keymap->moveback[iopt]=fline[ichar];
        if(iline==5) keymap->moveforward[iopt]=fline[ichar];
        if(iline==6) keymap->movedown[iopt]=fline[ichar];
        if(iline==7) keymap->moveup[iopt]=fline[ichar];
        if(iline==8) keymap->reset[iopt]=fline[ichar];
        if(iline==9) keymap->init[iopt]=fline[ichar];
        if(iline==10) keymap->grab[iopt]=fline[ichar];
        if(iline==11) keymap->pause[iopt]=fline[ichar];
        if(iline==12) keymap->playback[iopt]=fline[ichar];
        if(iline==13) keymap->start[iopt]=fline[ichar];
        if(iline==14) keymap->finish[iopt]=fline[ichar];
        if(iline==15) keymap->prevframe[iopt]=fline[ichar];
        if(iline==16) keymap->nextframe[iopt]=fline[ichar];
        if(iline==17) keymap->layeron[iopt]=fline[ichar];
        if(iline==18) keymap->layeroff[iopt]=fline[ichar];
	
	keymap->layername[iopt]=0;   //initialise name recognisably

	ichar+=4;
	iopt++;
      }
    }

    fclose(fptr);
  }
  else
  {
    nlines=NKYBLINES;
    noptions=NOPTIONS;
    nlyrkeys=NLYRKEYS;

    keymap->noptions=noptions;
    keymap->nlyrkeys=nlyrkeys;
    
    keymap->zoomin=NULL;
    keymap->zoomout=NULL;
    keymap->moveleft=NULL;
    keymap->moveright=NULL;
    keymap->moveback=NULL;
    keymap->moveforward=NULL;
    keymap->movedown=NULL;
    keymap->moveup=NULL;
    keymap->reset=NULL;
    keymap->init=NULL;
    keymap->grab=NULL;
    keymap->pause=NULL;
    keymap->playback=NULL;
    keymap->start=NULL;
    keymap->finish=NULL;
    keymap->prevframe=NULL;
    keymap->nextframe=NULL;
    keymap->layeron=NULL;
    keymap->layeroff=NULL;
    keymap->layername=NULL;
      
    keymap->zoomin=(char *)imalloc(E,noptions*sizeof(char));
    keymap->zoomout=(char *)imalloc(E,noptions*sizeof(char));
    keymap->moveleft=(char *)imalloc(E,noptions*sizeof(char));
    keymap->moveright=(char *)imalloc(E,noptions*sizeof(char));
    keymap->moveback=(char *)imalloc(E,noptions*sizeof(char));
    keymap->moveforward=(char *)imalloc(E,noptions*sizeof(char));
    keymap->movedown=(char *)imalloc(E,noptions*sizeof(char));
    keymap->moveup=(char *)imalloc(E,noptions*sizeof(char));
    keymap->reset=(char *)imalloc(E,noptions*sizeof(char));
    keymap->init=(char *)imalloc(E,noptions*sizeof(char));
    keymap->grab=(char *)imalloc(E,noptions*sizeof(char));
    keymap->pause=(char *)imalloc(E,noptions*sizeof(char));
    keymap->playback=(char *)imalloc(E,noptions*sizeof(char));
    keymap->start=(char *)imalloc(E,noptions*sizeof(char));
    keymap->finish=(char *)imalloc(E,noptions*sizeof(char));
    keymap->prevframe=(char *)imalloc(E,noptions*sizeof(char));
    keymap->nextframe=(char *)imalloc(E,noptions*sizeof(char));
    
    keymap->layeron=(char *)imalloc(E,nlyrkeys*sizeof(char));
    keymap->layeroff=(char *)imalloc(E,nlyrkeys*sizeof(char));
    keymap->layername=(unsigned *)imalloc(E,nlyrkeys*sizeof(unsigned));

    if(invalidptr(E,keymap->zoomin)||
       invalidptr(E,keymap->zoomout)||
       invalidptr(E,keymap->moveleft)||
       invalidptr(E,keymap->moveright)||
       invalidptr(E,keymap->moveback)||
       invalidptr(E,keymap->moveforward)||
       invalidptr(E,keymap->movedown)||
       invalidptr(E,keymap->moveup)||
       invalidptr(E,keymap->reset)||
       invalidptr(E,keymap->init)||
       invalidptr(E,keymap->grab)||
       invalidptr(E,keymap->pause)||
       invalidptr(E,keymap->playback)||
       invalidptr(E,keymap->start)||
       invalidptr(E,keymap->finish)||
       invalidptr(E,keymap->prevframe)||
       invalidptr(E,keymap->nextframe)||
       invalidptr(E,keymap->layeron)||
       invalidptr(E,keymap->layeroff)||
       invalidptr(E,keymap->layername))
      return;

    keymap->zoomin[0]='=';      keymap->zoomin[1]='_';
    keymap->zoomout[0]='-';     keymap->zoomout[1]='+';
    keymap->moveleft[0]=',';    keymap->moveleft[1]='>';
    keymap->moveright[0]='.';   keymap->moveright[1]='<';
    keymap->moveback[0]='z';    keymap->moveback[1]='A';
    keymap->moveforward[0]='a'; keymap->moveforward[1]='Z';
    keymap->movedown[0]='[';    keymap->movedown[1]='}';
    keymap->moveup[0]=']';      keymap->moveup[1]='{';
      
    keymap->reset[0]='r';
    keymap->init[0]='i';
    keymap->grab[0]='g';
    keymap->pause[0]='p';
    keymap->playback[0]='P';
    keymap->start[0]='s';
    keymap->finish[0]='f';
    keymap->prevframe[0]='p';
    keymap->nextframe[0]='n';

    keymap->layeron[0]='0'; keymap->layeroff[0]=')';
    keymap->layeron[1]='1'; keymap->layeroff[1]='!';
    keymap->layeron[2]='2'; keymap->layeroff[2]='@';
    keymap->layeron[3]='3'; keymap->layeroff[3]='#';
    keymap->layeron[4]='4'; keymap->layeroff[4]='$';
    keymap->layeron[5]='5'; keymap->layeroff[5]='%';
    keymap->layeron[6]='6'; keymap->layeroff[6]='^';
    keymap->layeron[7]='7'; keymap->layeroff[7]='&';
    keymap->layeron[8]='8'; keymap->layeroff[8]='*';
    keymap->layeron[9]='9'; keymap->layeroff[9]='(';

    for(iopt=0;iopt<NLYRKEYS;iopt++)
      keymap->layername[iopt]=0;   //initialise name/order recognisably
  }

  *retkey=keymap;
}

void keymap_close(keys *keymap)
{
  if(invalidptr(E,keymap)) return;

  keymap->zoomin=ifree(E,keymap->zoomin);
  keymap->zoomout=ifree(E,keymap->zoomout);
  keymap->moveleft=ifree(E,keymap->moveleft);
  keymap->moveright=ifree(E,keymap->moveright);
  keymap->moveback=ifree(E,keymap->moveback);
  keymap->moveforward=ifree(E,keymap->moveforward);
  keymap->movedown=ifree(E,keymap->movedown);
  keymap->moveup=ifree(E,keymap->moveup);
  keymap->reset=ifree(E,keymap->reset);
  keymap->init=ifree(E,keymap->init);
  keymap->grab=ifree(E,keymap->grab);
  keymap->pause=ifree(E,keymap->pause);
  keymap->playback=ifree(E,keymap->playback);
  keymap->start=ifree(E,keymap->start);
  keymap->finish=ifree(E,keymap->finish);
  keymap->prevframe=ifree(E,keymap->prevframe);
  keymap->nextframe=ifree(E,keymap->nextframe);
  keymap->layeron=ifree(E,keymap->layeron);
  keymap->layeroff=ifree(E,keymap->layeroff);
  keymap->layername=ifree(E,keymap->layername);

  keymap=ifree(E,keymap);
}

int keymap_islayer(keys *keymap,unsigned lyrhash)
{
  int iopt;

  // ****************************** //
  // ** MATCH KEY PRESS TO LAYER ** //
  // ****************************** //

  if(invalidptr(E,keymap)) return -1;
  if(invalidptr(E,keymap->layername)) return -1;

  for(iopt=0;iopt<keymap->nlyrkeys;iopt++)
    if(lyrhash==keymap->layername[iopt])
      return iopt;

  return -1;
}

unsigned keymap_getlayer(keys *keymap,char ch)
{
  int iopt;
  unsigned lyrhash;
  
  // ************************************************* //
  // ** RETURN NAME OF LAYER REQUESTED BY KEY PRESS ** //
  // ************************************************* //

  if(invalidptr(E,keymap)) return str_hash("null");
  if(invalidptr(E,keymap->layeron)) return str_hash("null");
  if(invalidptr(E,keymap->layeroff)) return str_hash("null");
  if(invalidptr(E,keymap->layername)) return str_hash("null");

  printf("keymap: keymap getlayer ch=%c nlyrkeys=%d\n",ch,keymap->nlyrkeys);
  
  for(iopt=0;iopt<keymap->nlyrkeys;iopt++)
  {
    printf("keymap: ch=%c on=%c off=%c name=%d\n",ch,keymap->layeron[iopt],keymap->layeroff[iopt],keymap->layername[iopt]);
    
    if(ch==keymap->layeron[iopt]||
       ch==keymap->layeroff[iopt])
    {
      lyrhash=keymap->layername[iopt];
      printf("keymap: lyrhash=%d\n",lyrhash);
      return lyrhash;
    }
  }

  return str_hash("null"); //flag 'not a layer'
}

int keymap_isshowhide(keys *keymap,char ch)
{
  int iopt;

  // ******************************************************* //
  // ** DECIDE IF THIS KEY PRESS EXPOSES OR HIDES A LAYER ** //
  // ******************************************************* //

  if(invalidptr(E,keymap)) return 0;
  if(invalidptr(E,keymap->layeron)) 0;
  if(invalidptr(E,keymap->layeroff)) 0;

  //printf("showhide ch=%c nlyrkeys=%d\n",ch,keymap->nlyrkeys);

  for(iopt=0;iopt<keymap->nlyrkeys;iopt++)
  {
    if(ch==keymap->layeron[iopt]||
       ch==keymap->layeroff[iopt])
      return 1;
  }

  return 0;
}

unsigned keymap_getshowhide(keys *keymap,char ch,unsigned renderlogic)
{
  int iopt;
 
  // ********************************************************** //
  // ** DECIDE WHICH LAYER IS EXPOSED OR HIDDEN BY KEY PRESS ** //
  // ********************************************************** //

  if(invalidptr(E,keymap)) return str_hash("null");
  if(invalidptr(E,keymap->layeron)) return str_hash("null");
  if(invalidptr(E,keymap->layeroff)) return str_hash("null");

  printf("keymap: getshowhide: ch=%c nlyrkeys=%d\n",ch,keymap->nlyrkeys);

  for(iopt=0;iopt<keymap->nlyrkeys;iopt++)
    if(ch==keymap->layeron[iopt])
      return renderlogic;  //expose
  
  for(iopt=0;iopt<keymap->nlyrkeys;iopt++)
    if(ch==keymap->layeroff[iopt])
      return str_hash("conceal");  //conceal

  return str_hash("null");
}

int keymap_ismovezoom(keys *keymap,char ch)
{
  int iopt;

  // *************************************************** //
  // ** DECIDE IF THIS KEY PRESS MOVES OR ZOOMS IMAGE ** //
  // *************************************************** //

  if(invalidptr(E,keymap)) return 0;
  if(invalidptr(E,keymap->zoomin)) return 0;
  if(invalidptr(E,keymap->zoomout)) return 0;
  if(invalidptr(E,keymap->moveleft)) return 0;
  if(invalidptr(E,keymap->moveright)) return 0;
  if(invalidptr(E,keymap->moveback)) return 0;
  if(invalidptr(E,keymap->moveforward)) return 0;
  if(invalidptr(E,keymap->movedown)) return 0;
  if(invalidptr(E,keymap->moveup)) return 0;

  for(iopt=0;iopt<keymap->noptions;iopt++)
    if(ch==keymap->zoomin[iopt]||
       ch==keymap->zoomout[iopt]||
       ch==keymap->moveleft[iopt]||
       ch==keymap->moveright[iopt]||
       ch==keymap->moveback[iopt]||
       ch==keymap->moveforward[iopt]||
       ch==keymap->movedown[iopt]||
       ch==keymap->moveup[iopt])
      return 1;
  
  return 0;
}

char keymap_getmovezoom(keys *keymap,char ch)
{
  int iopt;

  // ************************************************************** //
  // ** DECIDE HOW IMAGE POSITION SHOULD BE CHANGED BY KEY PRESS ** //
  // ************************************************************** //

  if(invalidptr(E,keymap)) return STRINGEND;
  if(invalidptr(E,keymap->zoomin)) return STRINGEND;
  if(invalidptr(E,keymap->zoomout)) return STRINGEND;
  if(invalidptr(E,keymap->moveleft)) return STRINGEND;
  if(invalidptr(E,keymap->moveright)) return STRINGEND;
  if(invalidptr(E,keymap->moveback)) return STRINGEND;
  if(invalidptr(E,keymap->moveforward)) return STRINGEND;
  if(invalidptr(E,keymap->movedown)) return STRINGEND;
  if(invalidptr(E,keymap->moveup)) return STRINGEND;

  for(iopt=0;iopt<keymap->noptions;iopt++)
    if(ch==keymap->zoomin[iopt])
      return '+'; //bigger
  
  for(iopt=0;iopt<keymap->noptions;iopt++)
    if(ch==keymap->zoomout[iopt])
      return '-'; //smaller
  
  for(iopt=0;iopt<keymap->noptions;iopt++)
    if(ch==keymap->moveleft[iopt])
      return 'l'; //left
  
  for(iopt=0;iopt<keymap->noptions;iopt++)
    if(ch==keymap->moveright[iopt])
      return 'r'; //right
  
  for(iopt=0;iopt<keymap->noptions;iopt++)
    if(ch==keymap->moveback[iopt])
      return 'b'; //back

  for(iopt=0;iopt<keymap->noptions;iopt++)
    if(ch==keymap->moveforward[iopt])
      return 'f'; //forward
  
  for(iopt=0;iopt<keymap->noptions;iopt++)
    if(ch==keymap->moveup[iopt])
      return 'u'; //up
  
  for(iopt=0;iopt<keymap->noptions;iopt++)
    if(ch==keymap->movedown[iopt])
      return 'd'; //down

  return STRINGEND;
}

int keymap_isrefresh(keys *keymap,char ch)
{
  int iopt;

  // ************************************************** //
  // ** DECIDE IF KEY PRESS IS A REQUEST FOR REFRESH ** //
  // ************************************************** //

  if(invalidptr(E,keymap)) return 0;

  for(iopt=0;iopt<keymap->noptions;iopt++)
    if(ch==keymap->reset[iopt]||
       ch==keymap->init[iopt]||
       ch==keymap->grab[iopt])
      return 1;

  return 0;
}

char keymap_getrefresh(keys *keymap,char ch)
{
  int iopt;

  // *************************************************** //
  // ** DECIDE WHAT TYPE OF REFRESH KEY PRESS DEMANDS ** //
  // *************************************************** //

  if(invalidptr(E,keymap)) return STRINGEND;

  for(iopt=0;iopt<keymap->noptions;iopt++)
    if(ch==keymap->reset[iopt])
      return 'r'; //reset 

  for(iopt=0;iopt<keymap->noptions;iopt++)
    if(ch==keymap->init[iopt])
      return 'i'; //init
  
  for(iopt=0;iopt<keymap->noptions;iopt++)
    if(ch==keymap->grab[iopt])
      return 'g'; //grab
  
  return STRINGEND;
}

int keymap_isstream(keys *keymap,char ch)
{
  int iopt;

  // ********************************************************* //
  // ** DECIDE IF KEY PRESS IS A REQUEST FOR STREAM CONTROL ** //
  // ********************************************************* //

  if(invalidptr(E,keymap)) return 0;

  printf("keymap: ch=%c %c %c %c %c %c %c\n",ch,keymap->pause[0],
                                                keymap->playback[0],
                                                keymap->start[0],
                                                keymap->finish[0],
                                                keymap->prevframe[0],
                                                keymap->nextframe[0]);

  for(iopt=0;iopt<keymap->noptions;iopt++)
    if(ch==keymap->pause[iopt]||
       ch==keymap->playback[iopt]||
       ch==keymap->start[iopt]||
       ch==keymap->finish[iopt]||
       ch==keymap->prevframe[iopt]||
       ch==keymap->nextframe[iopt])
      return 1;

  return 0;
}

char keymap_getstream(keys *keymap,char ch)
{
  int iopt;

  // **************************************************** //
  // ** DECIDE WHAT TYPE OF STREAM CONTROL IS DEMANDED ** //
  // **************************************************** //

  if(invalidptr(E,keymap)) return STRINGEND;

  for(iopt=0;iopt<keymap->noptions;iopt++)
    if(ch==keymap->pause[iopt])
      return 'h'; //halt playback
  
  for(iopt=0;iopt<keymap->noptions;iopt++)
    if(ch==keymap->playback[iopt])
      return 'r'; //play continuously
  
  for(iopt=0;iopt<keymap->noptions;iopt++)
    if(ch==keymap->start[iopt])
      return 's'; //skip to beginning
  
  for(iopt=0;iopt<keymap->noptions;iopt++)
    if(ch==keymap->finish[iopt])
      return 'f'; //skip to end

  for(iopt=0;iopt<keymap->noptions;iopt++)
    if(ch==keymap->prevframe[iopt])
      return 'p'; //previous frame
  
  for(iopt=0;iopt<keymap->noptions;iopt++)
    if(ch==keymap->nextframe[iopt])
      return 'n'; //next frame

  return STRINGEND;
}
