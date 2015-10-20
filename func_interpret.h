
void interpret_runscript(void **objlist,void **funclist,char *filename);
int interpret_line(int ntokens,nodes **tokens,void **objlist,void **funclist,int iline,int nlines);
void interpret_runtime(int ntokens,nodes **tokens,void **objlist,void **funclist);
void interpret_binding(int *ntkns,nodes ***tknchain);
void interpret_chainflip(int *ntkns,nodes ***tknchain);
void interpret_function(int *ntkns,nodes ***tknchain);
void interpret_operator(int *ntkns,nodes ***tknchain);
void interpret_string(int *ntkns,nodes ***tknchain);
void interpret_constant(int *ntkns,nodes ***tknchain);
void interpret_lexer(int nchars,char *string,int *ntokens,nodes ***tokens);
void interpret_tokenchain(int nchars,char *name,int itkn,int *ntokens,nodes ***tokens);
void add_ndata(nodes *curr,char *name,char *type,unsigned tag);
