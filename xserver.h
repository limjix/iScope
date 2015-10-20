//include standard things and constants
#include "head_std.h"
#include "head_const.h"
#ifdef XLIB
  #include <X11/Xlib.h>
#endif

//x window appearance
#define LIGHTGREY 0.9
#define SHADOW 0.75
#define ILLUMINATED 0.95
#define MIDGREY 0.5
#define WHITE 0.99
#define BLACK 0.0
#define BUTTONCHAMFER 5
#define HISTLINE 128
#define HISTPREFIX 6
#define NCOMMANDS 60

//defaults that are modifiable in gui.cfg
#define SCREENWIDTH 640
#define SCREENHEIGHT 480
#define XSCREENOFFSET 0
#define YSCREENOFFSET 22
#define XFONTSIZE 6
#define YFONTSIZE 14
#define NCONTEXTS 4
#define NBUTTONS 4
#define NPANELS 1

typedef void (*func_t)(void *,void *,void *);

#ifdef XLIB
  #include "struct_xtxt.h"
  #include "struct_xlyr.h"
  #include "struct_xbox.h"
  #include "struct_xsrv.h"
#endif
#include "struct_xreq.h"
#include "func_xserver.h"
#include "func_xserverio.h"
#include "func_xclientio.h"
#include "func_utilitiesio.h"
