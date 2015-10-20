typedef struct panels
{
  char active;       //flag active panel
  char panmem;       //flag memory in this structure

  int scrxmn;        //bounds of data window displayed to screen
  int scrxmx;
  int scrymn;
  int scrymx;
  
  char livezoom;     //store zoom-level for live data
  char livestream;   //flag as streaming: play/pause toggle
  char colourmap;    //raster display selection
  char vecvisible;   //collective selection of vector data
  
  rgb *rscreen;      //server-provided buffer for raster data at screen resolution 
  svg *vscreen;      //buffer for projecting vector data onto screen resolution

  int nvis;          //ordered sequence of objects that are currently rendered to this panel
  vis **visual;       
} panels;
