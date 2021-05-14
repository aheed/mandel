
#include <stdio.h>
#include <time.h>
#include <malloc.h>
#include <assert.h>
#include "mandel.h"

#define COLORSUSED 200
#define MAXCOLORVAL 255  // maximum 8 bit rgb value


typedef struct BMPFILEHEADER
{
    U16  FileType;     /* File type, always 4D42h ("BM") */
    U32  FileSize;     /* Size of the file in bytes */
    U16  Reserved1;    /* Always 0 */
    U16  Reserved2;    /* Always 0 */
    U32  BitmapOffset; /* Starting position of image data in bytes */
} BMPFILEHEADER;


typedef struct BITMAPHEADER
{
    U32 Size;            /* Size of this header in bytes */
    U32 Width;           /* Image width in pixels */
    U32 Height;          /* Image height in pixels */
    U16 Planes;          /* Number of color planes */
    U16 BitsPerPixel;    /* Number of bits per pixel */
    U32 Compression;     /* Compression methods used */
    U32 SizeOfBitmap;    /* Size of bitmap in bytes */
    U32 HorzResolution;  /* Horizontal resolution in pixels per meter */
    U32 VertResolution;  /* Vertical resolution in pixels per meter */
    U32 ColorsUsed;      /* Number of colors in the image */
    U32 ColorsImportant; /* Minimum number of important colors */
} BITMAPHEADER;

typedef struct BMP24BIT
{
  BMPFILEHEADER fileheader;
  BITMAPHEADER  bitmapheader;
  char *data_p;
} BMP24BIT;


typedef struct COLOR24BIT
{
  U8 blue;
  U8 green;
  U8 red;
} COLOR24BIT;

                  
int main(int argc, char * argv[])
{  
  PIXELVAL *mandelData_p;

  int width, height;
  float rmin, rmax, imin, imax;
  char szFileName[] = "result.bmp";
  time_t starttime, endtime, elapsed;

  assert(sizeof(U32) == 4);
  assert(sizeof(U16) == 2);
  assert(sizeof(U8) == 1);

  if(argc != 7)
  { 
    printf("\nusage: %s <width> <height> <rmin> <rmax> <imin> <imax>\n\n\n" \
           "Example:\n%s 400 400 -0.751 -0.735 0.118 0.134\n\n", argv[0], argv[0]);
    return 1;
  }
  
  sscanf(argv[1], "%d", &width);
  sscanf(argv[2], "%d", &height);  
  sscanf(argv[3], "%f", &rmin);
  sscanf(argv[4], "%f", &rmax);
  sscanf(argv[5], "%f", &imin);  
  sscanf(argv[6], "%f", &imax);

  printf("width    %d\nheight   %d\nrmin   %f\nrmax   %f\nimin   %f\nimax   %f\n\n",
         width, height, rmin, rmax, imin, imax);
  

  mandelData_p = (PIXELVAL*)malloc(width * height * sizeof(PIXELVAL));
  if(!mandelData_p)
  {
    printf("out of memory\n");
    return 1;
  }

  time(&starttime);
  if(CalcMandel(width, height, rmin, rmax, imin, imax, mandelData_p))
  {
    printf("Unexpected failure\n");
    free(mandelData_p);
    return 1;
  }
  else
  {
    time(&endtime);
    elapsed = endtime - starttime;
    printf("Processing time: %lld sec\n\n", elapsed);
  }

  if(SavePic(szFileName, width, height, mandelData_p))
  {
    printf("Failed to save file\n");
    free(mandelData_p);
    return 1;
  }

  printf("Output file: %s\n", szFileName);
  
  free(mandelData_p);
  return 0;
}



void Mandelval2Color(PIXELVAL pixval, COLOR24BIT *colorval_p)
{
  int colindex;

  if(pixval == 0)
  {
    // special case pixval = 0 => Black
    colorval_p->blue = 0;
    colorval_p->green = 0;
    colorval_p->red = 0;
    return;
  }

  colindex = pixval % COLORSUSED; // pick a color index
  colindex *= (MAXCOLORVAL / COLORSUSED); // spread colorindexes over value span  
  
  if(colindex > (MAXCOLORVAL / 2))
  {
    colorval_p->blue = MAXCOLORVAL - (colindex % MAXCOLORVAL);    
  }
  else
  {
    colorval_p->blue = (colindex % MAXCOLORVAL);    
  }

  colindex = (colindex + (MAXCOLORVAL / 3)) % MAXCOLORVAL;
  if(colindex > (MAXCOLORVAL / 2))
  {
    colorval_p->green = MAXCOLORVAL - (colindex % MAXCOLORVAL);    
  }
  else
  {
    colorval_p->green = (colindex % MAXCOLORVAL);    
  }

  colindex = (colindex + (MAXCOLORVAL / 3)) % MAXCOLORVAL;
  if(colindex > (MAXCOLORVAL / 2))
  {
    colorval_p->red = MAXCOLORVAL - (colindex % MAXCOLORVAL);    
  }
  else
  {
    colorval_p->red = (colindex % MAXCOLORVAL);    
  }

  colorval_p->blue *= 2;
  colorval_p->green *= 2;
  colorval_p->red *= 2;

}

int SavePic(char * pszFile,
              int width,
              int height,               
              PIXELVAL* indata_p)
{
  int x, y;
  BMP24BIT bmpinfo;
  char *pdata;
  FILE *outfile;
  int bytecnt;
  size_t written;
  COLOR24BIT colorval;


  // assign file header info
  bmpinfo.fileheader.FileType = 0x4D42;
  bmpinfo.fileheader.FileSize = 54 + 3 * width * height; // headers + 3 bytes per pixel
  bmpinfo.fileheader.Reserved1 = 0;
  bmpinfo.fileheader.Reserved2 = 0;
  bmpinfo.fileheader.BitmapOffset = 54;

  bmpinfo.bitmapheader.Size = 40;
  bmpinfo.bitmapheader.Width = width;
  bmpinfo.bitmapheader.Height = height;
  bmpinfo.bitmapheader.Planes = 1;
  bmpinfo.bitmapheader.BitsPerPixel = 24;
  bmpinfo.bitmapheader.Compression = 0; // no compression
  bmpinfo.bitmapheader.SizeOfBitmap = 3 * width * height;
  bmpinfo.bitmapheader.HorzResolution = 1000;  // who cares?
  bmpinfo.bitmapheader.VertResolution = 1000;  // who cares?
  bmpinfo.bitmapheader.ColorsUsed = 0; // palette not used at all
  bmpinfo.bitmapheader.ColorsImportant = 0; // palette not used at all

  bmpinfo.data_p = (char*)malloc(3 * width * height);
  if(!bmpinfo.data_p)
  {
    printf("out of memory\n");
    return 1;
  }

  
  // Assign bitmap data
  pdata = bmpinfo.data_p;
  for(x=0; x < width; x++)
  {
    for(y=0; y < height; y++)
    {
      Mandelval2Color(indata_p[x * height + y], &colorval);

      *pdata++ = colorval.blue;
      *pdata++ = colorval.green;
      *pdata++ = colorval.red;
    }
  }
  
  
  outfile = fopen(pszFile, "wb");
  if(!outfile)
  {
    printf("Failed to open file %s for writing\n", pszFile);
    free(bmpinfo.data_p);
    return 1;
  }


  
  // write headers
  written = 0;
  written += fwrite(&bmpinfo.fileheader.FileType, 2, 1, outfile);
  written += fwrite(&bmpinfo.fileheader.FileSize, 4, 1, outfile);
  written += fwrite(&bmpinfo.fileheader.Reserved1, 2, 1, outfile);
  written += fwrite(&bmpinfo.fileheader.Reserved2, 2, 1, outfile);
  written += fwrite(&bmpinfo.fileheader.BitmapOffset, 4, 1, outfile);

  written += fwrite(&bmpinfo.bitmapheader.Size, 4, 1, outfile);
  written += fwrite(&bmpinfo.bitmapheader.Width, 4, 1, outfile);
  written += fwrite(&bmpinfo.bitmapheader.Height, 4, 1, outfile);
  written += fwrite(&bmpinfo.bitmapheader.Planes, 2, 1, outfile);
  written += fwrite(&bmpinfo.bitmapheader.BitsPerPixel, 2, 1, outfile);
  written += fwrite(&bmpinfo.bitmapheader.Compression, 4, 1, outfile);
  written += fwrite(&bmpinfo.bitmapheader.SizeOfBitmap, 4, 1, outfile);
  written += fwrite(&bmpinfo.bitmapheader.HorzResolution, 4, 1, outfile);
  written += fwrite(&bmpinfo.bitmapheader.VertResolution, 4, 1, outfile);
  written += fwrite(&bmpinfo.bitmapheader.ColorsUsed, 4, 1, outfile);
  written += fwrite(&bmpinfo.bitmapheader.ColorsImportant, 4, 1, outfile);
  

  if(written != 16)
  {
    printf("Failed to write file header\nWritten items %d (%d)\n", written, 15);
    fclose(outfile); // never mind return value
    free(bmpinfo.data_p);
    return 1;
  }
  

  // write image data
  if(fwrite(bmpinfo.data_p, bmpinfo.bitmapheader.SizeOfBitmap, 1, outfile) != 1)
  {
    printf("Failed to write image data to file\n");
    fclose(outfile); // never mind return value
    free(bmpinfo.data_p);
    return 1;
  }

  free(bmpinfo.data_p);
  bmpinfo.data_p = NULL;

  if(fclose(outfile))
  {
    printf("Failed to close file %s\n", pszFile);
    return 1;
  }

  
  return 0;
}


