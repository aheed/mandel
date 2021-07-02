
#include <stdio.h>
#include <time.h>
#include <malloc.h>
#include <assert.h>
#include "mandel.h"

#define COLORSUSED 200
#define MAXCOLORVAL 255  // maximum 8 bit rgb value

typedef struct COLOR24BIT
{
  U8 blue;
  U8 green;
  U8 red;
} COLOR24BIT;
                 

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

int main(int argc, char * argv[])
{  
  int width, height;
  float rmin, rmax, imin, imax;
  char szInFileName[] = "result.raw";
  char szFileName[] = "result.rgb";
  time_t starttime, endtime, elapsed;
  FILE* infile;
  FILE* outfile;
  size_t read;
  size_t written;
  PIXELVAL pixelval;
  COLOR24BIT colorval;

  assert(sizeof(U32) == 4);
  assert(sizeof(U16) == 2);
  assert(sizeof(U8) == 1);

  

  printf("opening input file %s\n", szInFileName);
  infile = fopen(szInFileName, "rb");
  if(!infile)
  {
    printf("Failed to open file %s for reading\n", szInFileName);
    return 1;
  }

  printf("opening output file %s\n", szFileName);
  outfile = fopen(szFileName, "wb");
  if(!outfile)
  {
    printf("Failed to open file %s for writing\n", szFileName);
    return 1;
  }

  while(1)
  {
    read = fread(&pixelval, sizeof(pixelval), 1, infile);
    if(!read)
    {
      break;
    }
    
    Mandelval2Color(pixelval, &colorval);
    
    written = fwrite(&colorval, sizeof(colorval), 1, outfile);
  }

  if(fclose(infile))
  {
    printf("Failed to close input file\n");
    return 1;
  }

  if(fclose(outfile))
  {
    printf("Failed to close output file\n");
    return 1;
  }
  
  return 0;
}

