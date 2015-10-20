void *xclient_exporttoraster(void *arglist);
void *xclient_importtoraster(void *arglist);
void *xclient_rastertoraster(void *arglist);
void *xclient_vectortovector(void *arglist);
void *xclient_graphtovector(void *arglist);
void drawing_raster(rgb *scr,rgb *dat,int xmn,int xmx,int ymn,int ymx,char colourmap);
void drawing_reposition(int scrnx,int scrny,int datnx,int datny,int *sxmn,int *sxmx,int *symn,int *symx,int panel,char command,char *text);
void drawing_vector(svg *scr,vecs *vdat,int xmn,int xmx,int ymn,int ymx,char colourmap);
