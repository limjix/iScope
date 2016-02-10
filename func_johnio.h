void *john_testhelloworld(void *arglist);
void *john_tnodes(void *arglist);
void *john_kalmaninit(void *arglist);
void *john_kalmancreatenodes(void *arglist);
void *john_makeplotvector(void *arglist);
void *john_nodeplot(void *arglist);

void add_hfactortondata(nodes *curr,hfactor *hfac);
hfactor *create_hfactor();
void *john_testfunc(void *arglist);
nodes *createnode(int x, int y,char str[MAXLEN],hgph *graph, char type);
