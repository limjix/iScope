#include "utilities.h"

void *heap;        //utilities-global variable for an un-synced heap
uint64_t heapfree; //index of start of free heap
uint64_t heapend;  //index of end of heap
void *heaplist;    //maintain a list of objects

unsigned str_hash(char *name)
{
  // *****************************************************************
  // * CREATE A 'UNIQUE' IDENTIFIER FOR AN OBJECT WITH FNV ALGORITHM *
  // *****************************************************************

  unsigned hash;
  int nchars;
  int ch;
  int dec;

  if(invalidptr(E,name)) return 0;
    
  nchars=0;
  hash=HASHSEED;
    
  //hash the name for efficient retrieval using the FNV algorithm
  while((ch=name[nchars++])!=0)
    hash=(hash*HASHCOEFF)^ch;

  return hash;
}

int str_length(char *string)
{
  int ichar;

  ichar=0;
  while(string[ichar]!='\0')
    ichar++;

  return ichar;
}

char str_match(char *buffer,char *string)
{
  int ibuf;
  int istr;
  int ictr;
  int strln;
  int bufln;

  if(invalidptr(E,buffer)) return 'f';
  if(invalidptr(E,string)) return 'f';

  strln=str_length(string);
  bufln=str_length(buffer);

  istr=-1;
  ictr=-1;
  for(ibuf=0;ibuf<bufln;ibuf++)
  {
    if(buffer[ibuf]==string[0]&&ictr==-1)
    {
      istr=ibuf; //start of string
      ictr=0;
    }

    if(ictr>=0&&ictr<strln&&istr>=0&&istr<bufln) //protect seg fault
      if(buffer[istr]==string[ictr])
      {
        istr++;
	ictr++;
      }
      else
      {
        istr=-1;
	ictr=-1;
      }
  }

  //printf("%s %s : %d %d %d\n",buffer,string,bufln,ictr,strln);

  if(ictr==strln)
    return 't';
  else
    return 'f';
}

unsigned getvalue(char *buffer)
{
  int buflen;
  int ibuf;
  int ictr;
  int value;
  int power;

  // ******************************* //
  // ** CONVERT STRING TO INTEGER ** //
  // ******************************* //

  if(invalidptr(E,buffer)) return -1;

  buflen=str_length(buffer);

  ictr=-1;
  value=0.0;
  power=1;

  //look backwards from end of buffer for a number
  for(ibuf=buflen-1;ibuf>=0;ibuf--)
  {
    if(isnumber(buffer[ibuf])=='t'&&ictr==-1)
      ictr=ibuf;

    if(ictr>=0&&ictr<buflen)
      if(isnumber(buffer[ictr])=='t')
      {
        if(buffer[ictr]!='-')
	{
          value+=getnumber(buffer[ictr])*power;
	  power*=10;
	  ictr--;
	}
	else
	{
          value*=-1;
	}
      }
  }

  return value;
}

char iskeyword(char *buffer)
{
  // ******************************** //
  // ** IDENTIFY LANGUAGE KEYWORDS ** //
  // ******************************** //

  if(str_hash(buffer)==str_hash("for")) return 't';
  if(str_hash(buffer)==str_hash("while")) return 't';
  if(str_hash(buffer)==str_hash("if")) return 't';
  if(str_hash(buffer)==str_hash("return")) return 't';
  if(str_hash(buffer)==str_hash("void")) return 't';
  if(str_hash(buffer)==str_hash("int")) return 't';
  if(str_hash(buffer)==str_hash("real")) return 't';
  if(str_hash(buffer)==str_hash("char")) return 't';

  return 'f';
}

unsigned getkeyword(char *buffer)
{
  // *********************************** //
  // ** RETURN HASH FOR KNOWN KEYWORD ** //
  // *********************************** //

  if(str_hash(buffer)==str_hash("for")) return str_hash("for");
  if(str_hash(buffer)==str_hash("while")) return str_hash("while");
  if(str_hash(buffer)==str_hash("if")) return str_hash("if");
  if(str_hash(buffer)==str_hash("return")) return str_hash("return");
  if(str_hash(buffer)==str_hash("void")) return str_hash("void");
  if(str_hash(buffer)==str_hash("int")) return str_hash("int");
  if(str_hash(buffer)==str_hash("real")) return str_hash("real");
  if(str_hash(buffer)==str_hash("char")) return str_hash("char");
}

char isnumber(char ch) // confirm that character is a number?
{
  // ********************************* //
  // ** IDENTIFY NUMERIC CHARACTERS ** //
  // ********************************* //

  if(ch=='0') return 't';
  if(ch=='1') return 't';
  if(ch=='2') return 't';
  if(ch=='3') return 't';
  if(ch=='4') return 't';
  if(ch=='5') return 't';
  if(ch=='6') return 't';
  if(ch=='7') return 't';
  if(ch=='8') return 't';
  if(ch=='9') return 't';
  
  return 'f';
}

int getnumber(char ch)
{
  // ********************************** //
  // ** CONVERT CHARACTER TO INTEGER ** //
  // ********************************** //

  if(ch=='0') return 0;
  if(ch=='1') return 1;
  if(ch=='2') return 2;
  if(ch=='3') return 3;
  if(ch=='4') return 4;
  if(ch=='5') return 5;
  if(ch=='6') return 6;
  if(ch=='7') return 7;
  if(ch=='8') return 8;
  if(ch=='9') return 9;

  return -1;
}

char isdelimiter(char ch)
{
  // ******************************************** //
  // ** DECIDE IF FOLLOWING STRING IS DELMITER ** //
  // ******************************************** //

  if(ch==WHITESPACE) return 't';
  if(ch==SINGLEQUOTE) return 't';
  if(ch==';') return 't';
  if(ch==':') return 't';
  if(ch=='(') return 't';
  if(ch==')') return 't';
  if(ch=='{') return 't';
  if(ch=='}') return 't';
  if(ch=='[') return 't';
  if(ch==']') return 't';
  if(ch=='>') return 't';
  if(ch=='<') return 't';
  if(ch==',') return 't';
  if(ch=='.') return 't';
  if(ch=='=') return 't';
  if(ch=='+') return 't';
  if(ch=='-') return 't';
  if(ch=='*') return 't';
  if(ch=='/') return 't';
  if(ch=='!') return 't';
  if(ch=='E') return 't';

  return 'f';
}

unsigned getdelimiter(char ch)
{
  if(ch==WHITESPACE) return str_hash("whitespace");
  if(ch==SINGLEQUOTE) return str_hash("singlequote");
  if(ch==';') return str_hash("semicolon");
  if(ch==':') return str_hash("colon");
  if(ch=='(') return str_hash("openbracket");
  if(ch==')') return str_hash("closebracket");
  if(ch=='(') return str_hash("openbrace");
  if(ch==')') return str_hash("closebrace");
  if(ch=='[') return str_hash("openindex");
  if(ch==']') return str_hash("closeindex");
  if(ch=='>') return str_hash("greaterthan");
  if(ch=='<') return str_hash("lessthan");
  if(ch==',') return str_hash("comma");
  if(ch=='.') return str_hash("fullstop");
  if(ch=='=') return str_hash("equals");
  if(ch=='+') return str_hash("plus");
  if(ch=='-') return str_hash("minus");
  if(ch=='*') return str_hash("multiply");
  if(ch=='/') return str_hash("divide");
  if(ch=='!') return str_hash("not");
  if(ch=='E') return str_hash("exponent");
}

char isletter(char ch)
{
  // ****************************************** //
  // ** DECIDE IF FOLLOWING STRING IS LETTER ** //
  // ****************************************** //

  if(ch=='_') return 't'; 
  if(ch=='a') return 't'; 
  if(ch=='b') return 't'; 
  if(ch=='c') return 't'; 
  if(ch=='d') return 't'; 
  if(ch=='e') return 't'; 
  if(ch=='f') return 't'; 
  if(ch=='g') return 't'; 
  if(ch=='h') return 't'; 
  if(ch=='i') return 't'; 
  if(ch=='j') return 't'; 
  if(ch=='k') return 't'; 
  if(ch=='l') return 't'; 
  if(ch=='m') return 't'; 
  if(ch=='m') return 't'; 
  if(ch=='n') return 't'; 
  if(ch=='o') return 't'; 
  if(ch=='p') return 't'; 
  if(ch=='q') return 't'; 
  if(ch=='r') return 't'; 
  if(ch=='s') return 't'; 
  if(ch=='t') return 't'; 
  if(ch=='u') return 't'; 
  if(ch=='v') return 't'; 
  if(ch=='w') return 't'; 
  if(ch=='x') return 't'; 
  if(ch=='y') return 't'; 
  if(ch=='z') return 't'; 
 
  //upper case is reserved, in particular E is used for exponent

  return 'f';
}

int invalidptr(int line,char *filename,const char *function,char action,void *addr)
{
  // ************************************************ //
  // ** ERROR HANDLING FOR UNLIKELY POINTER VALUES ** //
  // ************************************************ //

  if((void *) addr<(void *) VALIDMEM)
  {
    if(action=='e'||action=='w')
      printf("utilities: line %04u in %s: %s pointer is invalid %p\n",line,filename,function,addr);

    if(action=='e')
      exit(1);

    return 1;
  }

  return 0;
}

void *imalloc(int line,char *filename,const char *function,char action,size_t space)
{
  void *ptr;

  // ****************************** //
  // ** MANAGE MEMORY ALLOCATION ** //
  // ****************************** //

  ptr=NULL;
  ptr=malloc(space);
  if(invalidptr(line,filename,function,action,ptr))
    return NULL;
  memset(ptr,0,space);
/*
  if(heap==NULL)
  {
    heap=malloc(MEGABYTE);
    if((void *) heap<(void *) VALIDMEM)
    {
      printf("utilities: error: heap cannot be created\n");
      exit(1);
    }
    printf("utilities: heap allocated 1MB\n");
    
    heapend=MEGABYTE;
    ptr=heap;
    heapfree=space;

    heaplist=NULL;
    dynamic_putarg("std.void",identifier,(void *) ,SZ,&heaplist);
  }
  else
  {
    if(heapfree+space>heapend)
    {
      heap=realloc(heap,heapend+MEGABYTE);
      if((void *) heap<(void *) VALIDMEM)
      {
        printf("utilities: error: heap cannot be resized\n");
        exit(1);
      }
    
      heapend+=MEGABYTE;
    }

    ptr=&(heap[heapfree]);
    heapfree+=space;
  }
*/
  return ptr;
}

void *ifree(int line,char *filename,const char *function,char action,void *addr)
{
  // *************************** //
  // ** MANAGE MEMORY RELEASE ** //
  // *************************** //

  if(!invalidptr(line,filename,function,action,addr))
    free(addr);
  //dynamic_putarg


  return NULL;
}

void iheap(void)
{
  heap=NULL;
  heapfree=0;
  heapend=0;
}
