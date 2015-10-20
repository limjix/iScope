#include "interpret.h"

//gcc -g -fPIC -c interpret.c && gcc -g -fPIC -dynamiclib -o libinterpret.dylib interpret.o -L. -lutilities -ldynamic -lgraph 

void *interpret_script(void *arglist)
{
  void *argptr;
  void *script;
  void *funclist;
  void *objlist;

  // ******************************************** //
  // ** SET UP A CLEAN INTERPRETER ENVIRONMENT ** //
  // ******************************************** //

  //obligatory input argument
  if(dynamic_getarg(arglist,"script",&script)=='f') return NULL;
  
  //optional argument
  if(dynamic_getarg(arglist,"objlist",&argptr)=='t')
  {
    if(invalidptr(E,argptr)) return NULL;
    objlist=*((void **) argptr); //argptr==&(user->objlist)
    funclist=NULL; //all parallel function calls in this script should resolve before script returns: local scope

    if(dynamic_getarg(arglist,"funclist",&argptr)=='t')
    {
      if(invalidptr(E,argptr)) return NULL;
      funclist=*((void **) argptr); //argptr==&(user->funclist) if you **really** want parallel functions at global scope.
    }
  }
  else
  {
    //start interpreter with a null object list, a clean start unless otherwise specified 
    objlist=NULL;
    funclist=NULL;
  }
  
  //read compile execute script
  interpret_runscript(&objlist,&funclist,(char *) script);

  //pass back modified interpreter environment
  arglist=NULL;
  dynamic_putarg("std.void","funclist",funclist,SZ,&arglist);
  dynamic_putarg("std.void","objlist",objlist,SZ,&arglist);
  return arglist;
}

void *interpret_tokens(void *arglist)
{
  // ********************* //
  // ** TOKENISE SCRIPT ** //
  // ********************* //


}

void *interpret_command(void *arglist)
{
  void *xptr;
  void *argptr;
  void *inputargs;
  void *outputargs;
  void *funclist;
  void *objlist;
  void *window;
  void *func;
  int nchars;
  char *text;
  int ntokens;
  nodes **tokens;

  // ****************************************************************** //
  // ** INTERPRET GUI COMMAND WITHIN ITS OWN INTERPRETER ENVIRONMENT ** //
  // ****************************************************************** //

  if(dynamic_getarg(arglist,"funclist",&funclist)=='f') return NULL;
  if(dynamic_getarg(arglist,"objlist",&objlist)=='f') return NULL;
  if(dynamic_getarg(arglist,"nchars",&argptr)=='f') return NULL;  if(!invalidptr(E,argptr)) nchars=*((int *) argptr);
  if(dynamic_getarg(arglist,"text",&argptr)=='f') return NULL;    if(!invalidptr(E,argptr)) text=(char *) argptr;
  
  //if this window's xclient structure is supplied but not yet added to the object list, then add it
  if(dynamic_getarg(arglist,"xclient",&xptr)=='t')
    if(dynamic_getarg(arglist,"window",&window)=='t')
      if(dynamic_getarg(objlist,"xclient",&argptr)=='f')
      {
        dynamic_putarg("xclient.xclient",(char *) window,xptr,SZ,&objlist);
        printf("interpret: window=%s\n",(char *) window);
      }

  printf("interpret: nchars=%d text=%s\n",nchars,text);
 
  //compile and execute line
  interpret_lexer(nchars+1,text,&ntokens,&tokens); //strlen(text) needs incrementing for some reason
  interpret_string(&ntokens,&tokens);
  interpret_constant(&ntokens,&tokens);
  interpret_function(&ntokens,&tokens);
  interpret_operator(&ntokens,&tokens);
  interpret_chainflip(&ntokens,&tokens);
  interpret_binding(&ntokens,&tokens);
  interpret_line(ntokens,tokens,&objlist,&funclist,0,1);

  //return modified interpreter environment
  arglist=NULL;
  dynamic_putarg("std.void","funclist",funclist,SZ,&arglist);
  dynamic_putarg("std.void","objlist",objlist,SZ,&arglist); 
  return arglist;
}

void interpret_runscript(void **objlist,void **funclist,char *filename)
{
  FILE *fptr;
  char line[MAXLEN];
  int iline,nlines;
  int *ntokens;
  nodes ***tokens;

  // ************************************************************* //
  // ** READ AND EXECUTE RUDIMENTARY NON-TURING-COMPLETE SCRIPT ** //
  // ************************************************************* //

  printf("interpret: %s\n",filename);

  //implement a rudimentary line-reading interpreter
  fptr=NULL;
  if((fptr=fopen(filename,"r"))!=NULL)
  {
    //get number of lines
    nlines=0;
    while(!feof(fptr))
    {
      fgets(line,MAXLEN,fptr);
      nlines++;
    }
    nlines--;
    fclose(fptr);

    ntokens=NULL;
    tokens=NULL;
    ntokens=(int *)imalloc(E,nlines*sizeof(int));
    tokens=(nodes ***)imalloc(E,nlines*sizeof(nodes **));
    if(invalidptr(E,ntokens)) return;
    if(invalidptr(E,tokens)) return;

    fptr=NULL;
    if((fptr=fopen(filename,"r"))!=NULL)
    {
      printf("interpret: nlines=%d\n",nlines);

      for(iline=0;iline<nlines;iline++)
      {
        //read script line from file
        fgets(line,MAXLEN,fptr);
        printf("%s",line);

	//tokenise line
        interpret_lexer(str_length(line),line,&(ntokens[iline]),&(tokens[iline]));
        
	//compile identifiers parenthesised by ' ' delimiters 
        interpret_string(&(ntokens[iline]),&(tokens[iline]));    
        
	//compile integers and delimiters associated with real-valued constants
	interpret_constant(&(ntokens[iline]),&(tokens[iline]));  

        //compile function call instructions
	interpret_function(&(ntokens[iline]),&(tokens[iline]));

	//compile remaining delimiters into operators
	interpret_operator(&(ntokens[iline]),&(tokens[iline]));

        //reverse direction of chain for execution
	interpret_chainflip(&(ntokens[iline]),&(tokens[iline]));

        //bind operators to their arguments
	interpret_binding(&(ntokens[iline]),&(tokens[iline]));
      }
      fclose(fptr);
    }
  
    //follow program flow
    iline=0;
    while(iline>=0)
      iline=interpret_line(ntokens[iline],tokens[iline],objlist,funclist,iline,nlines); 
  }
}

int interpret_line(int ntokens,nodes **tokens,void **objlist,void **funclist,int iline,int nlines)
{
  // ************************************************ //
  // ** INTERPRET TOKEN CHAIN AND INVOKE FUNCTIONS ** //
  // ************************************************ //
 
  printf("interpret: line %d\n",iline); 
  interpret_runtime(ntokens,tokens,objlist,funclist);

  iline++;
  if(iline>=nlines)
    return -1;

  return iline;
}

void interpret_binding(int *ntkns,nodes ***tknchain)
{

}

void interpret_runtime(int ntokens,nodes **tokens,void **objlist,void **funclist)
{
  void *func;
  void *outputargs;
  void *inputargs;
  void *argptr;
  void *dummy;
  char funcflag;
  char argsflag;
  unsigned identifier,operator,function,constant,string,start;
  unsigned copyobj,copyarg,passarglist,integer,real;
  nodes *next,*curr,*prev;
  tkns *ntkn,*ctkn,*ptkn;

  // ************************************** //
  // ** RUNTIME EXECUTION OF TOKEN CHAIN ** //
  // ************************************** //

  integer=str_hash("integer");
  real=str_hash("real");
  identifier=str_hash("identifier");
  operator=str_hash("operator");
  function=str_hash("function");
  copyobj=str_hash("copyobj");
  copyarg=str_hash("copyarg");
  passarglist=str_hash("passarglist");
  start=str_hash("start");
  constant=str_hash("constant");
  string=str_hash("string");

  //run in execution order
  curr=find_node(str_hash("end"),ntokens,tokens);
  if(invalidptr(E,curr)) return;
  
  ntkn=curr->ndata;
  ctkn=curr->ndata;
  ptkn=curr->ndata;
  funcflag='f';
  argsflag='f';
 
  inputargs=NULL;
  outputargs=NULL;

  while(curr->nedges>0&&curr->nhash!=start)
  {
    if(invalidptr(E,curr->edge)) return;

    //shunt saved tokens backwards
    ntkn=ctkn;
    ctkn=ptkn;
    ptkn=(tkns *) curr->ndata;
    if(!invalidptr(E,ptkn)&&!invalidptr(E,ctkn)&&!invalidptr(E,ntkn))
    {
      //printf("interpret: test triad: %s %s %s\n",ptkn->name,ctkn->name,ntkn->name);

      // arg:obj  input argument
      if(funcflag=='f'&&
         ntkn->type==identifier&&
         ctkn->type==operator&&ctkn->tag==copyarg&&
         ptkn->type==identifier)
      {
        printf("interpret: arg:obj %s:%s\n",ptkn->name,ntkn->name);
        if(dynamic_getarg(*objlist,ntkn->name,&argptr)=='t')
	  dynamic_putarg("std.void",ptkn->name,argptr,SZ,&inputargs);
      }

      // arg:string input argument
      if(funcflag=='f'&&
         ntkn->type==string&&
         ctkn->type==operator&&ctkn->tag==copyarg&&
         ptkn->type==identifier)
      {
        printf("interpret: arg:string %s:%s\n",ptkn->name,ntkn->name);
        dynamic_putarg("std.void",ptkn->name,(void *) ntkn->name,SZ,&inputargs);
      }

      // arg:const input argument
      if(funcflag=='f'&&
         ntkn->type==constant&&
         ctkn->type==operator&&ctkn->tag==copyarg&&
         ptkn->type==identifier)
      {
        printf("interpret: arg:const %s:%s\n",ptkn->name,ntkn->name);
	printf("interpret: ntkn->cst=%f %p ntkn->idx=%d %p\n",ntkn->cst,&ntkn->cst,ntkn->idx,&ntkn->idx);
	if(ntkn->tag==real) dynamic_putarg("std.double",ptkn->name,(void *) &(ntkn->cst),SZ,&inputargs);
	if(ntkn->tag==integer) dynamic_putarg("std.int",ptkn->name,(void *) &(ntkn->idx),SZ,&inputargs);
      }

      // obj:arg  output argument
      if(funcflag=='t'&&
         ntkn->type==identifier&&
         ctkn->type==operator&&ctkn->tag==copyarg&&
         ptkn->type==identifier)
      {
        printf("interpret: obj:arg %s:%s\n",ptkn->name,ntkn->name);
        if(dynamic_getarg(outputargs,ntkn->name,&argptr)=='t')
        {
          //overwrite if already exists
          if(dynamic_getarg(*objlist,ptkn->name,&dummy)=='t')
	    dynamic_delarg(*objlist,ptkn->name,'d');
          
	  dynamic_putarg("std.void",ptkn->name,argptr,SZ,objlist);
        }
      }

      // obj:=obj copy
      if(ntkn->type==identifier&&
         ctkn->type==operator&&ctkn->tag==copyobj&&
         ptkn->type==identifier)
      {
        printf("interpret: obj:=obj %s:=%s\n",ptkn->name,ntkn->name);
        if(dynamic_getarg(*objlist,ntkn->name,&argptr)=='t')
        {
          //overwrite if already exists
          if(dynamic_getarg(*objlist,ptkn->name,&dummy)=='t')
	    dynamic_delarg(*objlist,ptkn->name,'d');
          
	  dynamic_putarg("std.void",ptkn->name,argptr,SZ,objlist);
	  //function call needed to create a completely new instance, not just pointer copying
        }
      }

      // obj:=const assignment
      if(ntkn->type==constant&&
         ctkn->type==operator&&ctkn->tag==copyobj&&
         ptkn->type==identifier)
      {
        printf("interpret: obj:=const %s:=%s\n",ptkn->name,ntkn->name);
          
	//overwrite if object already exists
        if(dynamic_getarg(*objlist,ptkn->name,&dummy)=='t')
	  dynamic_delarg(*objlist,ptkn->name,'d');
        
	if(ntkn->tag==real) dynamic_putarg("std.double",ptkn->name,(void *) &(ntkn->cst),SZ,objlist);
	if(ntkn->tag==integer) dynamic_putarg("std.int",ptkn->name,(void *) &(ntkn->idx),SZ,objlist);
	//function call needed to create a completely new instance, not just pointer copying
      } 

      // obj:=string assignment
      if(ntkn->type==string&&
         ctkn->type==operator&&ctkn->tag==copyobj&&
         ptkn->type==identifier)
      {
        printf("interpret: obj:=string %s:=%s\n",ptkn->name,ntkn->name);
          
	//overwrite if object already exists
        if(dynamic_getarg(*objlist,ptkn->name,&dummy)=='t')
	  dynamic_delarg(*objlist,ptkn->name,'d');
          
	dynamic_putarg("std.void",ptkn->name,(void *) ntkn->name,SZ,objlist);
	//function call needed to create a completely new instance, not just pointer copying
      } 

      // func<-
      if(ptkn->type==function&&
         ctkn->type==operator&&ctkn->tag==passarglist)
      {
        printf("interpret: func<- %s.%s.%s %c\n",ptkn->lib,ptkn->func,ptkn->inst,ptkn->parallel);

	//make the instance tag accessible to the function itself via the dynamic argument list
	dynamic_putarg("std.char","instance",(void *) ptkn->inst,SZ,&inputargs);
	
	//call the function
	dynamic_call(ptkn->lib,ptkn->func,ptkn->parallel,inputargs,&func);
	
	//store the unresolved function handle by appending to interpreter list
	dynamic_putarg("dynamic.hfunc",ptkn->name,func,SZ,funclist);
	
	//swap subsequent argument parsing from input to output
	funcflag='t';
      }
      
      // <-func
      if(ctkn->type==function&&
         ptkn->type==operator&&ptkn->tag==passarglist)
      {
        printf("interpret: <-func %s.%s[%s]\n",ctkn->lib,ctkn->func,ctkn->inst);
        if(dynamic_getarg(*funclist,ctkn->name,&func)=='t')
        {
	  dynamic_funcargs(func,'i',&inputargs); //retrieve inputargs before we destroy func 
	  dynamic_wait(func,&outputargs); //resolve function and capture output arguments	    
          dynamic_closeargs(inputargs); //inputargs can only be closed once function is resolved
	  dynamic_delarg(*funclist,ctkn->name,'p'); //remove function from list now that it is resolved
	  funcflag='t';
	  argsflag='t';
        }
      }
    }

    curr=find_node(curr->edge[0],ntokens,tokens);
    if(invalidptr(E,curr)) return;

    //printf("curr->vecr[%f,%f]\n",curr->vecr[0],curr->vecr[1]);
  }
    
  //clean up output arguments once line is fully executed
  if(argsflag=='t')
    dynamic_closeargs(outputargs);
}

void interpret_chainflip(int *ntkns,nodes ***tknchain)
{
  int ntokens;
  unsigned end;
  nodes *prev,*curr;
  nodes **tokens;

  // *********************************************** //
  // ** SWITCH TOKEN CHAIN TO EXECUTION DIRECTION ** //
  // *********************************************** //
  
  printf("interpret: chainflip:\n");

  end=str_hash("end");

  ntokens=*ntkns;
  tokens=*tknchain;
 
  //flip edges behind us as we move from start to end
  curr=find_node(str_hash("start"),ntokens,tokens);
  if(invalidptr(E,curr)) return;

  while(curr->nedges>0)
  {
    if(invalidptr(E,curr->edge)) return;

    //printf("%u %s:[%f,%f] curr->nedges=%d start=%u end=%u\n",curr->nhash,((tkns *) curr->ndata)->name,curr->vecr[0],curr->vecr[1],curr->nedges,str_hash("start"),str_hash("end"));
    prev=curr;
    curr=find_node(curr->edge[0],ntokens,tokens);
    if(invalidptr(E,curr)) return;
    if(prev==curr) return;
    rem_diredge(prev,curr);
    add_diredge(curr,prev);
    if(curr->nhash==end) return;
  }
  //printf("%p %u:[%f,%f] curr->nedges=%d start=%u end=%u\n",curr,curr->nhash,curr->vecr[0],curr->vecr[1],curr->nedges,str_hash("start"),str_hash("end"));
}

void interpret_function(int *ntkns,nodes ***tknchain)
{
  int ntokens,narrows;
  double pos[2];
  char name[MAXLEN];
  char library[MAXLEN];
  char function[MAXLEN];
  char instance[MAXLEN];
  unsigned delimiter,lessthan,fullstop,start,end;
  unsigned minus,colon,openindex,closeindex;
  nodes **tokens;
  nodes *prev,*curr,*new,*tmp;
  tkns *ptkn,*ctkn,*ntkn,*ttkn;
  
  // ********************************************* //
  // ** RESOLVE FUNCTION CALL FROM TOKEN STREAM ** //
  // ********************************************* //
  
  //serial call :      <-library.function<-  
  //parallel call:       library.function[tag]<-
  //parallel wait:     <-library.function[tag]
 
  new=NULL;
  ntokens=*ntkns;
  tokens=*tknchain;

  printf("interpret: function\n");

  //evaluate hashes only once
  delimiter=str_hash("delimiter");
  fullstop=str_hash("fullstop");
  colon=str_hash("colon");
  minus=str_hash("minus");
  lessthan=str_hash("lessthan");
  start=str_hash("start");
  end=str_hash("end");

  //follow the token chain from the start
  curr=find_node(start,ntokens,tokens);
  if(invalidptr(E,curr)) return;

  while(curr->nedges>0&&curr->nhash!=end)
  {
    if(invalidptr(E,curr->edge)) return;

    prev=curr; //store previous to reroute chain
    curr=find_node(curr->edge[0],ntokens,tokens);
    if(invalidptr(E,curr)) return;
    ptkn=(tkns *) curr->ndata;
    if(invalidptr(E,ptkn)) return;
 
    if(ptkn->type!=delimiter&&
       curr->nedges>0)
    {
      if(invalidptr(E,curr->edge)) return;
      
      tmp=curr;
      curr=find_node(curr->edge[0],ntokens,tokens);
      if(invalidptr(E,curr)) return;
      ctkn=(tkns *) curr->ndata;
      if(invalidptr(E,ctkn)) return;

      //find . delimiter 
      if(ctkn->type==delimiter&&
         ctkn->tag==fullstop&&
         curr->nedges>0)
      {
        if(invalidptr(E,curr->edge)) return;
        
	//create new node to store function details
	pos[0]=curr->vecr[0];
	pos[1]=1.0;
        sprintf(name,"%u",ntokens);
        add_node(str_hash(name),pos,ntkns,tknchain); 
        ntokens=*ntkns;
        tokens=*tknchain;
        new=find_node(str_hash(name),ntokens,tokens);
        if(invalidptr(E,new)) return;
   
	if(prev->nedges>0)
	{
	  if(invalidptr(E,prev->edge)) return;

          //replace link from prev to prev->edge[0] with prev to new
          rem_diredge(prev,find_node(prev->edge[0],ntokens,tokens));
          add_diredge(prev,new);
        }

        add_diredge(curr,new); //link dot
        add_diredge(tmp,new);  //link library name

        curr=find_node(curr->edge[0],ntokens,tokens);
        if(invalidptr(E,curr)) return;
        ntkn=(tkns *) curr->ndata;
        if(invalidptr(E,ntkn)) return;
        
        add_diredge(curr,new); //link function name

        if(curr->nedges>0)
        {
          if(invalidptr(E,curr->edge)) return;
	  
	  tmp=curr;
	  curr=find_node(curr->edge[0],ntokens,tokens);
          if(invalidptr(E,curr)) return;
          ctkn=(tkns *) curr->ndata;
          if(invalidptr(E,ctkn)) return;

	  //place identifiers for library and function in new node
          sprintf(library,"%s",ptkn->name);
          sprintf(function,"%s_%s",ptkn->name,ntkn->name);
          
	  //find any .tag pthread disambiguation
          if(ctkn->type==delimiter&&
             ctkn->tag==fullstop&&
             curr->nedges>0)
          {
            if(invalidptr(E,curr->edge)) return;
            
	    add_diredge(curr,new); //link full stop
	    
            curr=find_node(curr->edge[0],ntokens,tokens);
            if(invalidptr(E,curr)) return;
            ctkn=(tkns *) curr->ndata;
            if(invalidptr(E,ctkn)) return;
           
            if(ctkn->type!=delimiter&&
	       curr->nedges>0)
	    {
              if(invalidptr(E,curr->edge)) return;
              
	      add_diredge(curr,new); //link instance tag
              sprintf(instance,"%s",ctkn->name);

              //replace link from curr to curr->edge[0] with new to curr->edge[0]
	      add_diredge(new,find_node(curr->edge[0],ntokens,tokens));
              rem_diredge(curr,find_node(curr->edge[0],ntokens,tokens));
	    }
	  }
	  else
	  {
            sprintf(instance,"0");
        
	    if(ctkn->type==delimiter&&
	       ctkn->tag==lessthan&&
	       curr->nedges>0)
              if(invalidptr(E,curr->edge)) return;
	      {
	        //replace link from tmp to curr with new to curr
                add_diredge(new,curr);
                rem_diredge(tmp,curr);
	      }
 	  }
	}

	sprintf(name,"%s:%s:%s",library,function,instance);
	printf("interpret: %s\n",name);

	add_ndata(new,name,"function",str_hash(name));
	ntkn=(tkns *) new->ndata;
        if(invalidptr(E,ntkn)) return;
        sprintf(ntkn->lib,"%s",library);
	sprintf(ntkn->func,"%s",function);
	sprintf(ntkn->inst,"%s",instance);
      }
    }
  }
  
  //start again, this time looking for <- delimiters
  if(!invalidptr(S,new))
  {
    curr=find_node(start,ntokens,tokens);
    if(invalidptr(E,curr)) return;

    narrows=0;
    while(curr->nedges>0&&curr->nhash!=end)
    {
      if(invalidptr(E,curr->edge)) return;
      
      curr=find_node(curr->edge[0],ntokens,tokens);
      if(invalidptr(E,curr)) return;
      ptkn=(tkns *) curr->ndata;
      if(invalidptr(E,ptkn)) return;
 
      if(ptkn->type==delimiter&&
         ptkn->tag==lessthan&&
         curr->nedges>0)
      {
        if(invalidptr(E,curr->edge)) return;
        
	curr=find_node(curr->edge[0],ntokens,tokens);
        if(invalidptr(E,curr)) return;
        ntkn=(tkns *) curr->ndata;
        if(invalidptr(E,ntkn)) return;

        if(ntkn->type==delimiter&&
           ntkn->tag==minus)
        {
          narrows++;
        }
      }
    }

    //work out whether call is serial or parallel
    ntkn=(tkns *) new->ndata;
    if(!invalidptr(E,ntkn))
    {
      if(narrows==1) { ntkn->parallel='p'; printf("interpret: parallel\n"); }
      if(narrows==2) { ntkn->parallel='s'; printf("interpret: serial\n"); }
    }
  }
}

void interpret_operator(int *ntkns,nodes ***tknchain)
{
  int ntokens;
  unsigned delimiter,lessthan,minus,colon,equals,semicolon;
  unsigned passarglist,copyarg,copyobj,start,endline,end;
  double pos[2];
  char name[MAXLEN];
  nodes **tokens;
  nodes *prev,*curr,*tmp,*new;
  tkns *ctkn,*ttkn;

  // *********************************************************** //
  // ** RESOLVE OPERATORS FROM DELIMITERS IN THE TOKEN STREAM ** //
  // *********************************************************** //
  
  if(invalidptr(E,ntkns)) return;
  if(invalidptr(E,tknchain)) return;
  if(invalidptr(E,*tknchain)) return;

  printf("interpret: operator\n");

  ntokens=*ntkns;
  tokens=*tknchain;
  
  delimiter=str_hash("delimiter");
  lessthan=str_hash("lessthan");
  minus=str_hash("minus");
  colon=str_hash("colon");
  equals=str_hash("equals");
  passarglist=str_hash("passarglist");
  copyobj=str_hash("copyobj");
  copyarg=str_hash("copyarg");
  start=str_hash("start");
  endline=str_hash("endline");
  semicolon=str_hash("semicolon");
  end=str_hash("end");

  //follow the token chain from the start
  curr=find_node(start,ntokens,tokens);
  if(invalidptr(E,curr)) return;

  while(curr->nedges>0&&curr->nhash!=end)
  {
    if(invalidptr(E,curr->edge)) return;
    
    prev=curr; //store previous to reroute chain
    curr=find_node(curr->edge[0],ntokens,tokens);
    ctkn=(tkns *) curr->ndata;

    //<- operator
    if(ctkn->type==delimiter&&
       ctkn->tag==lessthan&&
       curr->nedges>0)
    {
      if(invalidptr(E,curr->edge)) return;
      
      tmp=find_node(curr->edge[0],ntokens,tokens);
      ttkn=(tkns *) tmp->ndata;
      
      if(ttkn->type==delimiter&&
         ttkn->tag==minus)
      {
        //define passarglist operator from delimiter sequence
	sprintf(name,"%u",ntokens);
	pos[0]=curr->vecr[0];
	pos[1]=1.0;
        add_node(str_hash(name),pos,ntkns,tknchain); 
        ntokens=*ntkns;
        tokens=*tknchain;
        new=find_node(str_hash(name),ntokens,tokens);
        add_ndata(new,"<-","operator",passarglist);
        add_diredge(curr,new);
        add_diredge(tmp,new);
     
        if(prev->nedges>0)
	{
	  if(invalidptr(E,prev->edge)) return;
          //replace link from prev to prev->edge[0] with prev to new
          rem_diredge(prev,find_node(prev->edge[0],ntokens,tokens));
          add_diredge(prev,new);
	}
        
	if(tmp->nedges>0)
	{
	  if(invalidptr(E,tmp->edge)) return;

	  //replace link from curr to tmp->edge[0] with new to tmp->edge[0]
          add_diredge(new,find_node(tmp->edge[0],ntokens,tokens));
          rem_diredge(tmp,find_node(tmp->edge[0],ntokens,tokens));
        }

	curr=tmp; //move active node forward
	ctkn=ttkn;
      }
    }

    // := operator
    if(ctkn->type==delimiter&&
       ctkn->tag==colon&&
       curr->nedges>0)
    {
      if(invalidptr(E,curr->edge)) return;
      
      tmp=find_node(curr->edge[0],ntokens,tokens);
      ttkn=(tkns *) tmp->ndata;
    
      sprintf(name,"%u",ntokens);
      pos[0]=curr->vecr[0];
      pos[1]=1.0;
      add_node(str_hash(name),pos,ntkns,tknchain); 
      ntokens=*ntkns;
      tokens=*tknchain;
      new=find_node(str_hash(name),ntokens,tokens);
   
      if(ttkn->type==delimiter&&
         ttkn->tag==equals)
      {
        //define object duplicate operator from delimiter sequence
        add_ndata(new,":=","operator",copyobj);
	add_diredge(curr,new);
	add_diredge(tmp,new);
        
	if(prev->nedges>0)
	{
	  if(invalidptr(E,prev->edge)) return;
        
	  //replace link from prev to prev->edge[0] with prev to new
          rem_diredge(prev,find_node(prev->edge[0],ntokens,tokens));
          add_diredge(prev,new);
        }

	if(tmp->nedges>0)
	{
	  if(invalidptr(E,tmp->edge)) return;
        
	  //replace link from curr to tmp->edge[0] with new to tmp->edge[0]
          add_diredge(new,find_node(tmp->edge[0],ntokens,tokens));
          rem_diredge(tmp,find_node(tmp->edge[0],ntokens,tokens));
        }

	curr=tmp; //move active node forward
	ctkn=ttkn;
      }
    }
 
    // : operator
    if(ctkn->type==delimiter&&
       ctkn->tag==colon&&
       curr->nedges>0)
    {
      if(invalidptr(E,curr->edge)) return;
      
      tmp=find_node(curr->edge[0],ntokens,tokens);
      ttkn=(tkns *) tmp->ndata;

      sprintf(name,"%u",ntokens);
      pos[0]=curr->vecr[0];
      pos[1]=1.0;
      add_node(str_hash(name),pos,ntkns,tknchain); 
      ntokens=*ntkns;
      tokens=*tknchain;
      new=find_node(str_hash(name),ntokens,tokens);
   
      // : and not := provided next token is not = 
      if((ttkn->type==delimiter&&ttkn->tag!=equals)||
	  ttkn->type!=delimiter)
      {
        //define statement closure operator from delimiter sequence
        add_ndata(new,":","operator",copyarg);
        add_diredge(curr,new);
        
	if(prev->nedges>0)
	{
	  if(invalidptr(E,prev->edge)) return;
          
	  //replace link from prev to prev->edge[0] with prev to new
          rem_diredge(prev,find_node(prev->edge[0],ntokens,tokens));
          add_diredge(prev,new);
        }

	if(curr->nedges>0)
	{
	  if(invalidptr(E,curr->edge)) return;
          
	  //replace link from curr to curr->edge[0] with new to curr->edge[0]
          add_diredge(new,find_node(curr->edge[0],ntokens,tokens));
          rem_diredge(curr,find_node(curr->edge[0],ntokens,tokens));
	}
      }
    }
  } 
}

void interpret_string(int *ntkns,nodes ***tknchain)
{
  int ntokens;
  unsigned delimiter,singlequote,start,end;
  double pos[2];
  char name[MAXLEN];
  char parentheses;
  nodes **tokens;
  nodes *prev,*curr,*new;
  tkns *tkn;

  // ****************************************************** //
  // ** RESOLVE QUOTED-STRING CONSTANT FROM TOKEN STREAM ** //
  // ****************************************************** //

  if(invalidptr(E,ntkns)) return;
  if(invalidptr(E,tknchain)) return;
  if(invalidptr(E,*tknchain)) return;

  printf("interpret: string\n");

  ntokens=*ntkns;
  tokens=*tknchain;
  
  delimiter=str_hash("delimiter");
  singlequote=str_hash("singlequote");
  start=str_hash("start");
  end=str_hash("end");

  //follow the token chain from the start
  curr=find_node(start,ntokens,tokens);
  if(invalidptr(E,curr)) return;

  while(curr->nedges>0)
  {
    if(invalidptr(E,curr->edge)) return;
    
    prev=curr; //store previous to reroute chain
    curr=find_node(curr->edge[0],ntokens,tokens);
    if(invalidptr(E,curr)) return;
    tkn=(tkns *) curr->ndata;
    if(invalidptr(E,tkn)) return;
    
    //if we have an opening ' delimiter
    if(tkn->type==delimiter&&
       tkn->tag==singlequote)
    {
      //create a new node to receive string constant
      sprintf(name,"%u",ntokens);
      pos[0]=curr->vecr[0];
      pos[1]=1.0;
      add_node(str_hash(name),pos,ntkns,tknchain); 
      ntokens=*ntkns;
      tokens=*tknchain;
      new=find_node(str_hash(name),ntokens,tokens);
      if(invalidptr(E,new)) return;
      
      add_diredge(curr,new);
      
      if(prev->nedges>0)
      {
	if(invalidptr(E,prev->edge)) return;
      
        //replace link from prev to prev->edge[0] with prev to new
        rem_diredge(prev,find_node(prev->edge[0],ntokens,tokens));
        add_diredge(prev,new);
      }

      //initalise string constant
      sprintf(name,"");
      
      //keep going until we've found closing ' delimiter
      parentheses='t';
      while(curr->nedges>0&&parentheses=='t')
      {
        if(invalidptr(E,curr->edge)) return;
        
	curr=find_node(curr->edge[0],ntokens,tokens);
	if(invalidptr(E,curr)) return;
        tkn=(tkns *) curr->ndata;
	if(invalidptr(E,tkn)) return;

	if(tkn->type==delimiter&&
	   tkn->tag==singlequote)
        {
	  //found closing ' delimiter
	  printf("interpret: %s\n",name);
          add_ndata(new,name,"string",0);
	  add_diredge(curr,new);
	  parentheses='f';

	  if(curr->nedges>0)
	  {
	    if(invalidptr(E,curr->edge)) return;

            //replace link from curr to curr->edge[0] with new to curr->edge[0]
            add_diredge(new,find_node(curr->edge[0],ntokens,tokens));
            rem_diredge(curr,find_node(curr->edge[0],ntokens,tokens));
	  }
	}
	else
	{
	  //append name
	  sprintf(name,"%s%s",name,tkn->name);
	  add_diredge(curr,new);
	}
      }
    }
  }
}

void interpret_constant(int *ntkns,nodes ***tknchain)
{
  int ntokens;
  double value,vpow;
  int ipow,msign,esign;
  double pos[2];
  char name[MAXLEN];
  unsigned integer,delimiter;
  unsigned fullstop,minus;
  unsigned exponent,start,end;
  unsigned datatype,real;
  nodes **tokens;
  nodes *curr,*prev,*tmp,*new;
  tkns *tkn;
 
  // **************************************************** //
  // ** RESOLVE REAL-VALUED CONSTANT FROM TOKEN STREAM ** //
  // **************************************************** //

  //accepts constants of the form: 
  //12.234E-56
  //-12.234E-56   
  //1.2E-34
  //-1.2E-34
  //1E-23
  //-1E-23
  //1E2
  //-1E2
  //1.2E3
  //-1.2E3
  //1.234E5
  //-1.234E5
  //12.345
  //-12.345
  //12
  //-12
  
  if(invalidptr(E,ntkns)) return;
  if(invalidptr(E,tknchain)) return;
  if(invalidptr(E,*tknchain)) return;

  printf("interpret: constant\n");

  ntokens=*ntkns;
  tokens=*tknchain;

  integer=str_hash("integer");
  real=str_hash("real");
  minus=str_hash("minus");
  delimiter=str_hash("delimiter");
  fullstop=str_hash("fullstop");
  exponent=str_hash("exponent");
  start=str_hash("start");
  end=str_hash("end");

  //follow the token chain from the start
  curr=find_node(start,ntokens,tokens);
  if(invalidptr(E,curr)) return;

  while(curr->nedges>0&&curr->nhash!=end)
  {
    if(invalidptr(E,curr->edge)) return;
    
    prev=curr; //store previous to reroute chain
    curr=find_node(curr->edge[0],ntokens,tokens);
    if(invalidptr(E,curr)) return;
    tkn=(tkns *) curr->ndata;
    if(invalidptr(E,tkn)) return;

    //deal with negative in the mantissa
    msign=1;
    if(tkn->type==delimiter&&
       tkn->tag==minus&&
       curr->nedges>0)
    {
      if(invalidptr(E,curr->edge)) return;
      
      tmp=curr; //store negative sign
      curr=find_node(curr->edge[0],ntokens,tokens);
      if(invalidptr(E,curr)) return;
      tkn=(tkns *) curr->ndata;
      if(invalidptr(E,tkn)) return;

      if(tkn->type==integer)
      {
        //create a new node to receive numeric constant
        sprintf(name,"%u",ntokens);
	pos[0]=curr->vecr[0];
	pos[1]=1.0;
        add_node(str_hash(name),pos,ntkns,tknchain); 
        ntokens=*ntkns;
        tokens=*tknchain;
        new=find_node(str_hash(name),ntokens,tokens);
        if(invalidptr(E,new)) return;
        add_diredge(curr,new);
        add_diredge(tmp,new);
  
	if(prev->nedges>0)
	{
	  if(invalidptr(E,prev->edge)) return;
          //replace link from prev to prev->edge[0] with prev to new
	  rem_diredge(prev,find_node(prev->edge[0],ntokens,tokens));
	  add_diredge(prev,new);
        }

	msign=-1;
      }
    }

    //if an integer token
    if(tkn->type==integer)
    {
      value=(double) tkn->tag;
      datatype=integer;
     
      if(msign==1)
      {
        //if not done already, create a new node to receive numeric constant
        sprintf(name,"%u",ntokens);
        pos[0]=curr->vecr[0];
	pos[1]=1.0;
	add_node(str_hash(name),pos,ntkns,tknchain); 
        ntokens=*ntkns;
        tokens=*tknchain;
        new=find_node(str_hash(name),ntokens,tokens);
        if(invalidptr(E,new)) return;
        add_diredge(curr,new);
        
	if(prev->nedges>0)
	{
	  if(invalidptr(E,prev->edge)) return;
	
	  //replace link from prev to prev->edge[0] with prev to new
	  rem_diredge(prev,find_node(prev->edge[0],ntokens,tokens));
	  add_diredge(prev,new);
	}
      }

      //deal with decimal in the mantissa
      if(curr->nedges>0)
      {
        if(invalidptr(E,curr->edge)) return;
      
	tmp=find_node(curr->edge[0],ntokens,tokens);
        if(invalidptr(E,tmp)) return;
        tkn=(tkns *) tmp->ndata;
        if(invalidptr(E,tkn)) return;

        if(tkn->type==delimiter&&
           tkn->tag==fullstop&&
	   tmp->nedges>0)
        {
          if(invalidptr(E,tmp->edge)) return;
          
	  add_diredge(tmp,new);

          tmp=find_node(tmp->edge[0],ntokens,tokens);
          if(invalidptr(E,tmp)) return;
          tkn=(tkns *) tmp->ndata;
          if(invalidptr(E,tkn)) return;
	  
	  if(tkn->type==integer)
	  {
	    //add decimal places
	    vpow=1;
	    while(tkn->tag/vpow>10)
	      vpow*=10;
	    value+=(double) tkn->tag/(vpow*10.0);

	    datatype=real;

	    add_diredge(tmp,new);
	    curr=tmp;  //copy back
	  }
        }
      } 

      //apply sign after adding decimals
      value*=msign;

      //deal with the exponent
      if(curr->nedges>0)
      {
        if(invalidptr(E,curr->edge)) return;
        
	tmp=find_node(curr->edge[0],ntokens,tokens);
        if(invalidptr(E,tmp)) return;
        tkn=(tkns *) tmp->ndata;
        if(invalidptr(E,tkn)) return;
        
	if(tkn->type==delimiter&&
           tkn->tag==exponent&&
	   tmp->nedges>0)
	{
          if(invalidptr(E,tmp->edge)) return;
          
	  add_diredge(tmp,new);

          tmp=find_node(tmp->edge[0],ntokens,tokens);
          if(invalidptr(E,tmp)) return;
          tkn=(tkns *) tmp->ndata;
          if(invalidptr(E,tkn)) return;
	  
	  esign=1;
          if(tkn->type==delimiter&&
             tkn->tag==minus&&
             tmp->nedges>0)
	  {
            if(invalidptr(E,tmp->edge)) return;
	    
	    add_diredge(tmp,new);

            esign=-1;

            tmp=find_node(tmp->edge[0],ntokens,tokens);
            if(invalidptr(E,tmp)) return;
            tkn=(tkns *) tmp->ndata;
            if(invalidptr(E,tkn)) return;
	  }

	  if(tkn->type==integer)
	  { 
	    //calculate exponent
	    vpow=1.0;
            for(ipow=0;ipow<tkn->tag;ipow++)
	      if(esign==-1) vpow/=10.0;
	      else          vpow*=10.0;
                
	    //apply exponent
	    value*=vpow;

	    datatype=real;
            
	    add_diredge(tmp,new);
	    curr=tmp;
          }
	}
      }

      //store real-valued constant value in token payload
      sprintf(name,"%e",value);
      add_ndata(new,name,"constant",datatype);
      tkn=(tkns *) new->ndata;
      if(invalidptr(E,tkn)) return;
      tkn->cst=value;
      tkn->idx=(int) value;
  
      if(curr->nedges>0)
      {
	if(invalidptr(E,curr->edge)) return;
      
        //replace line from curr to curr->edge[0] with new to curr->edge[0]
        add_diredge(new,find_node(curr->edge[0],ntokens,tokens));
        rem_diredge(curr,find_node(curr->edge[0],ntokens,tokens));
      }
    }
  }
}

void interpret_lexer(int nchars,char *string,int *ntokens,nodes ***tokens)
{
  int ichar,ic,itkn;
  int beg,end;
  char tkn;
  char *stringptr;
  char endstring;
  char name[MAXLEN];

  // ****************************** //
  // ** TOKENISE A BLOCK OF CODE ** //
  // ****************************** //

  if(invalidptr(E,ntokens)) return;
  if(invalidptr(E,tokens)) return;

  //initialise token chain
  *tokens=NULL;
  *ntokens=0;

  endstring=STRINGEND;
  tkn='f';
  itkn=0;
  beg=0;
  end=0;
  stringptr=string;
  for(ichar=0;ichar<nchars+1;ichar++)  //read to the end of the block
  {
    //printf("%d %c %d\n",ichar,*stringptr,nchars);
    
    if(ichar>=nchars) 
    {
      //printf("interpret: STRINGEND\n");
      stringptr=&endstring;
      
      //add a terminating node
      interpret_tokenchain(str_length("end"),"end",itkn,ntokens,tokens);
    }

    //find beginning of identifier/constant
    if(isdelimiter(*stringptr)=='f'&&tkn=='f') 
    { 
      tkn='t'; 
      beg=ichar; 
      
      //add delimiters individually to token chain
      for(ic=end;ic<beg;ic++)
      {
        sprintf(name,"%c",string[ic]);
	//printf("interpret: add delimiter [%d:%d] '%s' to chain\n",ic,ic+1,name);
        interpret_tokenchain(str_length(name),name,itkn,ntokens,tokens);
        itkn++;
      }
    }

    //once its end is found, store identifier/token 
    if(isdelimiter(*stringptr)=='t'&&tkn=='t') 
    { 
      end=ichar; 

      //isolate identifier/constant
      memset(name,STRINGEND,MAXLEN*sizeof(char));
      for(ic=beg;ic<end;ic++)
        name[ic-beg]=string[ic];
      name[end]=STRINGEND;

      //add to chain
      //printf("interpret: add identifier/constant [%d:%d] '%s' to chain\n",beg,end,name);
      interpret_tokenchain(str_length(name),name,itkn,ntokens,tokens);
      itkn++;
      
      beg=ichar; //reset for next token
      end=ichar; 
      tkn='f';
    }
    
    stringptr++;
  }
}

void interpret_tokenchain(int nchars,char *string,int itkn,int *ntokens,nodes ***tokens)
{
  char name[MAXLEN];
  double pos[2];
  nodes *curr,*prev;
  
  // ************************ //
  // ** ADD TOKEN TO CHAIN ** //
  // ************************ //

  printf("interpret: tokenchain %s %d\n",string,itkn);

  pos[0]=(double) itkn;
  pos[1]=0.0;
 
  if(str_hash(string)==str_hash("end"))
  {
    add_node(str_hash("end"),pos,ntokens,tokens);
    curr=find_node(str_hash("end"),*ntokens,*tokens);
    add_ndata(curr,"end","end",str_hash("end")); //add a dummy token so that while loops don't seg fault
  }
  else
  {
    //a chain is an directed sequence of 'dumb' code tokens with a type but no meaning attached
    sprintf(name,"tkn%u",(unsigned) itkn);
    add_node(str_hash(name),pos,ntokens,tokens);
    curr=find_node(str_hash(name),*ntokens,*tokens);

    if(!invalidptr(E,curr))
    {
      //printf("adding ndata to node %p\n",curr);
      if(isnumber(string[0])=='t')
        add_ndata(curr,string,"integer",getvalue(string));
      else if(iskeyword(string)=='t')
        add_ndata(curr,string,"keyword",getkeyword(string));
      else if(isdelimiter(string[0])=='t')
        add_ndata(curr,string,"delimiter",getdelimiter(string[0]));  
      else
        add_ndata(curr,string,"identifier",str_hash(string));
    }
  } 
 
  if(itkn==0)
  {
    //add an extra start node for good measure  
    pos[0]=-1.0;
    add_node(str_hash("start"),pos,ntokens,tokens);
    prev=find_node(str_hash("start"),*ntokens,*tokens);
    add_diredge(prev,curr);
  }
  else
  {
    sprintf(name,"tkn%u",(unsigned) itkn-1);
    prev=find_node(str_hash(name),*ntokens,*tokens);
    add_diredge(prev,curr);
  }
}

void add_ndata(nodes *curr,char *name,char *type,unsigned tag)
{
  tkns *token;

  // *************************** //
  // ** ALLOCATE NODE PAYLOAD ** //
  // *************************** //

  if(invalidptr(S,curr)) return;
  curr->ndata=imalloc(E,sizeof(tkns));
  token=(tkns *) (curr->ndata);
  if(invalidptr(E,token)) return;

  printf("interpret: %s: %s (%d)\n",name,type,(int) tag);

  token->type=str_hash(type);
  token->tag=tag;
  token->nhash=str_hash(name);
  sprintf(token->name,"%s",name);
  
  //printf("interpret: %u: %u (%u)\n",token->nhash,token->type,token->tag);
}

/*
void *interpret_openwindow(void *arglist)
{
  void *guicfg;
  void *kybcfg;
  void *clientdata;
  void *xclient;
  void *xserver;

  // *************************** //
  // ** OPEN FRESH GUI WINDOW ** //
  // *************************** //

  dynamic_getarg(arglist,"guiconfig",&guicfg);
  dynamic_getarg(arglist,"kybconfig",&kybcfg);

  //set up client-side data structure
  arglist=NULL;
  dynamic_putarg("char","kybconfig",kybcfg,sizeof(char *),&arglist);
  dynamic_call("libxclient.dylib","xclient_init",'s',arglist,&xclient);
  dynamic_wait(xclient,&clientdata);
  dynamic_closeargs(arglist);
  
  //set up arguments for xserver operation and request concurrent execution of gui with master thread
  arglist=NULL;
  dynamic_putarg("voidptr","callback",clientdata,sizeof(void *),&arglist);
  dynamic_putarg("char","guiconfig",guicfg,sizeof(char *),&arglist);
  dynamic_call("libxserver.dylib","xserver_init",'p',arglist,&xserver); 

  printf("interpret: xserver thread spawned\n"); 
  
  //wait until window is cancelled by user to close the spawned thread
  dynamic_wait(xserver,NULL);
  dynamic_closeargs(arglist);

  //clean up client-side resources for this window
  dynamic_call("libxclient.dylib","xclient_close",'s',clientdata,&xclient);
  dynamic_wait(xclient,NULL);
}
*/
