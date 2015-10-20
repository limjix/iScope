typedef struct tkns
{
  char name[MAXLEN]; //identifiers
  unsigned nhash;    
  unsigned type;     
  unsigned tag;
  double cst;        //constant info
  int idx;  
  char lib[MAXLEN];  //function info
  char func[MAXLEN];
  char inst[MAXLEN];
  char parallel;     //execution format
} tkns;
