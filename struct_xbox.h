typedef struct xbox
{
  //basic box attributes
  int xmin;
  int xmax;
  int ymin;
  int ymax;
  char active; //activity status
  char ccontext; //context associated with this box
  int ipanel; //panel number within a context
  int ibutton;  //if a button, then which button
  int ctr;     //count mouse clicks
  int xfontsize;  //sets many sizes for boxes, not just text
  int yfontsize;
  
  //text editing attributes
  char wrtmem;
  int nwrites;
  xtxt **write;
    
  //layer display attributes
  char lyrmem;
  int nlayers;
  xlyr **layer;  

  //sub-box attributes
  char submem;
  int nsboxes;
  struct xbox **sbox;
} xbox;

