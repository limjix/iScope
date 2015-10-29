#include "head_std.h"
#include "head_const.h"
#include "struct_rgb.h"
#include "struct_svg.h"
#include "struct_vecs.h"
#include "struct_nodes.h"
#include "struct_hgph.h"
#include "struct_vis.h"
#include "struct_panels.h"
#include "struct_keys.h"
#include "struct_hobj.h"
#include "struct_xclient.h"

#include "func_callbackio.h"
#include "func_dynamicio.h"
#include "func_xclientio.h"
#include "func_drawingio.h"
#include "func_xserverio.h"
#include "func_utilitiesio.h"
#include "func_johnio.h"

  // ************************************** //
  // ** John's Library of Test Functions ** //
  // ************************************** //

void *john_testhelloworld(void *arglist)
{
  // ***************************** //
  // ** Hello World Test Script	** //
  // ***************************** //

	void *xptr;
	void *callback;
	char string[MAXLEN];

	if(dynamic_getarg(arglist,"xclient",&xptr)=='f') return NULL;
        printf("xclient=%p\n",xptr);

	sprintf(string,"Hello World\n");

	arglist=NULL;
	dynamic_putarg("std.void","xclient",xptr,SZ,&arglist);
	dynamic_putarg("std.char","string",(void *) string,SZ,&arglist); 
	dynamic_call("xclient","xclient_callback_write",'s',arglist,&callback);
	dynamic_wait(callback,NULL);
	dynamic_closeargs(arglist);

   return NULL;
}

