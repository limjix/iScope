//include standard things
#include "head_std.h"
#include "head_const.h"

//extra libraries
#ifdef PNG
  #include <png.h>
  //a little annoyance between include and lib versions of libpng when linking X11
  //#include "/usr/X11/include/libpng12/png.h"
#endif
#include "func_exportio.h"
#include "func_utilitiesio.h"
