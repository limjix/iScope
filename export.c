#include "export.h"
//
//gcc -g -fPIC -c export.c && gcc -g -fPIC -dynamiclib -o libexport.dylib export.o -lpng -L. -lutilities

void export_png(char *filename,int width,int height,uint8_t *red,uint8_t *green,uint8_t *blue)
{
  #ifdef PNG
    FILE *fptr;
    png_structp png_ptr;
    png_infop info_ptr;
    png_byte *image;
    png_bytep *row_pointers;
    int i,j,k;

    //*******************************************//
    //** WRITE PORTABLE NETWORK GRAPHICS IMAGE **//
    //*******************************************//

    if(invalidptr(E,filename)) return;
    if(invalidptr(E,red)) return;
    if(invalidptr(E,green)) return;
    if(invalidptr(E,blue)) return;

    fptr=NULL; 
    if((fptr=fopen(filename,"wb"))!=NULL)
    {
      printf("export: png image '%s'\n",filename);

      image=NULL;
      row_pointers=NULL;
      image=(png_byte *)imalloc(E,width*height*3*sizeof(png_byte));
      row_pointers=(png_bytep *)imalloc(E,height*sizeof(png_bytep));

      //  image buffer is written to in the following order:
      //  top row RGBRGBRGB... mid rows RGBRGB.......... bot row RGBRGB.........
    
      if(!invalidptr(E,image)&&
         !invalidptr(E,red)&&
	 !invalidptr(E,green)&&
	 !invalidptr(E,blue))
        for(i=0;i<width;i++)
          for(j=0;j<height;j++)
          {
            k=i+(height-1-j)*width;
            image[k*3+0]=(png_byte) red[k];
            image[k*3+1]=(png_byte) green[k];
            image[k*3+2]=(png_byte) blue[k];
          }

      if(width<=0||height<=0)
        printf("cannot make picture with negative height=%d or width=%d",(int) height,(int) width);

      png_ptr=png_create_write_struct(PNG_LIBPNG_VER_STRING,(png_voidp) NULL,NULL,NULL);
      info_ptr=png_create_info_struct(png_ptr);
      png_init_io(png_ptr,fptr);
      png_set_IHDR(png_ptr,info_ptr,width,height,8,PNG_COLOR_TYPE_RGB,PNG_INTERLACE_NONE,PNG_COMPRESSION_TYPE_DEFAULT,PNG_FILTER_TYPE_DEFAULT);
      png_write_info(png_ptr,info_ptr);

      if(!invalidptr(E,row_pointers)&&
         !invalidptr(E,image))
        for(k=0;k<height;k++)
          row_pointers[k]=image+k*width*3;

      png_write_image(png_ptr,row_pointers);
      png_write_end(png_ptr,info_ptr);
      png_destroy_write_struct(&png_ptr,&info_ptr);
      
      row_pointers=ifree(E,row_pointers);
      image=ifree(E,image);
      fclose(fptr);
      
      printf("export: closed '%s'\n",filename);
    }
  #endif
}

void export_eps(char *filename,int width,int height,uint8_t *red,uint8_t *green,uint8_t *blue)
{
  FILE *fptr;
  int i,j,k;
  int ctr;

  //************************************//
  //** WRITE ASCII EPS GRAPHICS IMAGE **//
  //************************************//
   
  if(invalidptr(E,filename)) return;
  if(invalidptr(E,red)) return;
  if(invalidptr(E,green)) return;
  if(invalidptr(E,blue)) return;
  
  fptr=NULL;
  if((fptr=fopen(filename,"w"))!=NULL)
  {
    printf("export: eps image '%s'\n",filename);
   
    //EPS standard header
    fprintf(fptr,"%%!PS-Adobe-3.0 EPSF-3.0\n");
    fprintf(fptr,"%%%%BoundingBox: %d %d %d %d\n",0,0,width,height);
    fprintf(fptr,"%%%%Pages: 1\n");
    fprintf(fptr,"%%%%LanguageLevel: 2\n");
    fprintf(fptr,"%%%%DocumentData: Clean7Bit\n");
    fprintf(fptr,"%d %d scale\n",(int) width,(int) height);
    fprintf(fptr,"%d %d 8 [%d 0 0 %d 0 %d]\n",(int) width,(int) height,(int) width,(int) -height,(int) height);
    fprintf(fptr,"{currentfile 3 %d mul string readhexstring pop } bind\n",(int) width);
    fprintf(fptr,"false 3 colorimage\n");

    //write out the 8-bit rgb image data as 3 hexadecimal pairs per pixel
    ctr=0;
    for(j=0;j<height;j++)
      for(i=0;i<width;i++)
      {
        //k=i+(height-1-j)*width;
        k=i+j*width;
        fprintf(fptr,"%02x%02x%02x",red[k],green[k],blue[k]);
        if(ctr%16==15) fprintf(fptr,"\n");
	ctr++;
      }

    fprintf(fptr,"%%%%EOF\n"); 

    fclose(fptr);
      
    printf("export: closed '%s'\n",filename);
  }
}

void export_pstricks(char *filename)
{

}
