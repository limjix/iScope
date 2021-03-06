void request_event_keystroke(char *inst,char ch,int panel,int *running);
void request_event_mouse(char *inst,int x,int y,int b);
void request_event_write(char *inst,int nstring,char *string);
void request_event_screen(char *inst,unsigned command,int panel);
void request_event_vector(char *inst,unsigned lyrhash,int panel,unsigned drawtype,char fill,int npts,int *xpt,int *ypt,int *size,uint8_t *red,uint8_t *green,uint8_t *blue,uint8_t *alpha);
void request_event_raster(char *inst,unsigned lyrhash,int panel,int nx,int ny,uint8_t *red,uint8_t *green,uint8_t *blue,uint8_t *alpha);
void request_event_layer(char *inst,unsigned lyrhash,int panel,unsigned command);
void *xserver_init(void *arglist);
void *xserver_nullwindow(void *arglist);
void *xserver_xsrv_destroy(void *arglist);
