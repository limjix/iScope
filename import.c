#include "import.h"

//gcc -g -fPIC -c import.c && gcc -g -fPIC -dynamiclib -o libimport.dylib import.o -lpng -L. -lutilities

void import_png(char *filename,int *width,int *height,uint8_t **red,uint8_t **green,uint8_t **blue)
{
  #ifdef PNG
    FILE *fptr; 
    png_structp png_ptr;
    png_infop info_ptr;
    png_byte *image;
    png_bytep *row_pointers;
    int bit_depth,colour_type;
    png_uint_32 getwidth,getheight;
    int i,j,k,m;

    // ****************************** //
    // ** READ PNG IMAGE FROM FILE ** //
    // ****************************** //

    if(invalidptr(E,filename)) return;
    if(invalidptr(E,width)) return;
    if(invalidptr(E,height)) return;
    if(invalidptr(E,red)) return;
    if(invalidptr(E,green)) return;
    if(invalidptr(E,blue)) return;

    image=NULL;
    row_pointers=NULL;
    *red=NULL;
    *green=NULL;
    *blue=NULL;

    fptr=NULL;
    if((fptr=fopen(filename,"rb"))!=NULL)
    {
      printf("import: png image '%s'\n",filename);

      png_ptr=png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
      info_ptr=png_create_info_struct(png_ptr);
      png_init_io(png_ptr,fptr);
      png_read_info(png_ptr,info_ptr);
      png_get_IHDR(png_ptr,info_ptr,&getwidth,&getheight,&bit_depth,&colour_type,NULL,NULL,NULL);

      *width=(unsigned) getwidth;
      *height=(unsigned) getheight;

      row_pointers=(png_bytep *)imalloc(E,getheight*sizeof(png_bytep));
      image=(png_byte *)imalloc(E,getwidth*getheight*3*sizeof(png_byte));
      *red=(uint8_t *)imalloc(E,getwidth*getheight*sizeof(uint8_t *));
      *green=(uint8_t *)imalloc(E,getwidth*getheight*sizeof(uint8_t *));
      *blue=(uint8_t *)imalloc(E,getwidth*getheight*sizeof(uint8_t *));
     
      if(!invalidptr(E,row_pointers)&&
         !invalidptr(E,image))
        for(k=0;k<getheight;k++)
	  row_pointers[k]=image+k*getwidth*3;

      png_read_image(png_ptr,row_pointers);
      png_read_end(png_ptr,info_ptr);
      png_destroy_read_struct(&png_ptr,&info_ptr,NULL);
          
      if(!invalidptr(E,red)&&
         !invalidptr(E,green)&&
	 !invalidptr(E,blue))
        for(i=0;i<getwidth;i++)
          for(j=0;j<getheight;j++)
          {
            k=i+(getheight-1-j)*getwidth;
	    m=i+j*getwidth;
            (*red)[m]=(uint8_t) image[k*3+0];
            (*green)[m]=(uint8_t) image[k*3+1];
            (*blue)[m]=(uint8_t) image[k*3+2];
          }

      row_pointers=ifree(E,row_pointers);
      image=ifree(E,image);
      fclose(fptr);
      //RGB arrays MUST be freed by user

      printf("import: opened '%s'\n",filename);
    }
    else
    {
      printf("image read from '%s' failed\n",filename); 
    }
  #endif
}

void import_eps(char *filename,int *width,int *height,uint8_t **red,uint8_t **green,uint8_t **blue)
{
  FILE *fptr;
  int i,j,k;
  int ctr;
  char fline[MAXLEN];
  unsigned r,g,b;

  // *********************************** //
  // ** READ ASCII EPS GRAPHICS IMAGE ** //
  // *********************************** //
   
  if(invalidptr(E,filename)) return;
  if(invalidptr(E,width)) return;
  if(invalidptr(E,height)) return;
  if(invalidptr(E,red)) return;
  if(invalidptr(E,green)) return;
  if(invalidptr(E,blue)) return;
  
  *red=NULL;
  *green=NULL;
  *blue=NULL;

  fptr=NULL;
  if((fptr=fopen(filename,"r"))!=NULL)
  {
    printf("import: eps image '%s'\n",filename);
  
    //NB>>>> THIS IS NOT A POSTCRIPT INTERPRETER, 
    //IT READS A VERY RESTRICTED FORMAT OF .EPS

    //EPS standard header
    fgets(fline,MAXLEN,fptr); //%!PS-Adobe-3.0 EPSF-3.0
    fgets(fline,MAXLEN,fptr); //%%BoundingBox: 0 0 width height
    sscanf(fline,"%%%%BoundingBox: 0 0 %d %d\n",width,height);
    fgets(fline,MAXLEN,fptr); //%%Pages: 1
    fgets(fline,MAXLEN,fptr); //%%LanguageLevel: 2
    fgets(fline,MAXLEN,fptr); //%%DocumentData: Clean7Bit
    fgets(fline,MAXLEN,fptr); //width height scale
    fgets(fline,MAXLEN,fptr); //width height 8 [width 0 0 -height 0 height]
    fgets(fline,MAXLEN,fptr); //{ currentfile 3 width mul string readhexstring pop } bind
    fgets(fline,MAXLEN,fptr); //false 3 colorimage
    
    *red=(uint8_t *)imalloc(E,*width * *height * sizeof(uint8_t));
    *green=(uint8_t *)imalloc(E,*width * *height * sizeof(uint8_t));
    *blue=(uint8_t *)imalloc(E,*width * *height * sizeof(uint8_t));

    //read in 8-bit rgb image data as 3 hexadecimal pairs per pixel
    ctr=0;
    if(!invalidptr(E,red)&&
       !invalidptr(E,green)&&
       !invalidptr(E,blue))
      for(j=0;j<*height;j++)
        for(i=0;i<*width;i++)
        {
          k=i+(*height-1-j)* *width;
          fscanf(fptr,"%02x%02x%02x",(unsigned *) &r,(unsigned *) &g,(unsigned *) &b);
          (*red)[k]=(uint8_t) r;
          (*green)[k]=(uint8_t) g;
          (*blue)[k]=(uint8_t) b;
          if(ctr%16==15) fscanf(fptr,"\n");
	  ctr++;
        }

    fclose(fptr);
  }
}

void import_vid(char *filename,int frame,int *width,int *height,uint8_t **red,uint8_t **green,uint8_t **blue)
{
  uint64_t header[FILEHEAD];
  uint64_t nframes;
  uint64_t nxsize;
  uint64_t nysize;
  uint64_t ncolours;
  uint64_t nbits;
  uint64_t framesize;
  uint64_t framestride;
  uint64_t ipt;
  uint8_t *buf;
  FILE *fptr;

  // ******************************* //
  // ** READ FRAME FROM RAW VIDEO ** //
  // ******************************* //

  if(invalidptr(E,filename)) return;
  if(invalidptr(E,width)) return;
  if(invalidptr(E,height)) return;
  if(invalidptr(E,red)) return;
  if(invalidptr(E,green)) return;
  if(invalidptr(E,blue)) return;
  
  fptr=NULL;
  if((fptr=fopen(filename,"rb"))!=NULL)
  {
    printf("import: frame %d from %s\n",frame,filename);

    fread(header,sizeof(uint64_t),FILEHEAD,fptr);
    
    nframes=header[0];
    nxsize=header[1];
    nysize=header[2];
    ncolours=header[3];
    nbits=header[4];
       
    if(nbits==sizeof(uint8_t)*8&&ncolours==3&&frame<nframes)
    {
      framesize=nxsize*nysize*sizeof(uint8_t);
      framestride=3*framesize;
      
      *red=NULL;
      *green=NULL;
      *blue=NULL;
      buf=NULL;
      *red=(uint8_t *)imalloc(E,framesize);
      *green=(uint8_t *)imalloc(E,framesize);
      *blue=(uint8_t *)imalloc(E,framesize);
      buf=(uint8_t *)imalloc(E,framestride);
      *width=nxsize;
      *height=nysize;
      if(!invalidptr(E,*red)&&
         !invalidptr(E,*green)&&
	 !invalidptr(E,*blue)&&
	 !invalidptr(E,buf))
      {
        fseek(fptr,frame*framestride,SEEK_CUR);
        fread(buf,sizeof(uint8_t),framestride,fptr);
  
        for(ipt=0;ipt<nxsize*nysize;ipt++)
        {
          (*red)[ipt]=buf[0+ipt*ncolours];
          (*green)[ipt]=buf[1+ipt*ncolours];
          (*blue)[ipt]=buf[2+ipt*ncolours];
        }

        buf=ifree(E,buf);
      }
    }

    fclose(fptr);
  }
}
