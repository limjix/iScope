//common constants
#define FILEHEAD 16
#define MAXLEN 511
#define CHWHITE 255
#define HASHSEED 2166136261
#define HASHCOEFF 16777619
#define SMALL 1e-5
#define LARGE 1e12
#define ND 2
#define VALIDMEM 0xffff
#define MEGABYTE (1024*1024*8)

//character handling
#define DELETE 127
#define BACKSPACE 8
#define CRETURN 13
#define SINGLEQUOTE 39
#define DOUBLEQUOTE 34
#define NEWLINE '\n'
#define STRINGEND '\0'
#define WHITESPACE ' '
#define ENDOFFILE EOF

//conditional compilation of external libraries
#define PTHREAD
#define GENI
#define XLIB
#define PNG
#define NETWK
#define FFTW
#define DYLIB

//handle dynamic library file format
#define LIBPRE "lib"
//#define LIBPOST ".dylib"
#define LIBPOST ".so"

//macro for error handling
#define ILINE __LINE__
#define IFILE __FILE__
#define IFUNC __PRETTY_FUNCTION__
//#define IFUNC __func__  //c99 standard
#define E ILINE,IFILE,IFUNC,'e'  //error
#define W ILINE,IFILE,IFUNC,'w'  //warn
#define S ILINE,IFILE,IFUNC,'s'  //silent

#define SZ 1
