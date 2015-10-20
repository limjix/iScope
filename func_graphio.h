
void add_diredge(nodes *inode,nodes *jnode);
void rem_diredge(nodes *inode,nodes *jnode);
void add_edge(nodes *inode,nodes *jnode);
void rem_edge(nodes *inode,nodes *jnode);
void add_node(unsigned nhash,double *vecr,int *nnodes,nodes ***nodelist);
nodes *find_node(unsigned nhash,int nnodes,nodes **nodelist);
void rem_node(unsigned nhash,int *nnodes,nodes ***nodelist);
