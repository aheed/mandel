#include <stdio.h>
#include <omp.h>
#include "mandel.h"

int CalcMandel(int width,
               int height,
               double rmin,
               double rmax,
               double imin,
               double imax,
               PIXELVAL* outdata_p)
{
  int x, y;
  double rval, ival, rval0, ival0, rtemp;
  int iteration;
  
  #pragma omp parallel
  {
  
#ifdef _OPENMP
  #pragma omp single
  printf("Num threads is %d\n", omp_get_num_threads());

#ifndef _MSC_VER // no Visual Studio support for OpenMP 3
  omp_sched_t kind;
  int chunkSize;
  #pragma omp single
  {
    omp_get_schedule(&kind, &chunkSize);
    printf("Schedule kind %d  chunk size %d\n", kind, chunkSize);
  }
#endif
#endif

  #pragma omp for private(iteration, rval, ival, rval0, ival0, rtemp, y) schedule(runtime)
  for(x=0; x < width; x++)
  {
    rval0 = rmin + (rmax - rmin) * ((double)x / width);

    for(y=0; y < height; y++)
    {      
      ival0 = imin + (imax - imin) * ((double)y / height);

      rval = rval0;
      ival = ival0;      
      iteration = 1;
   
      while ( (rval*rval + ival*ival <= (2*2))  &&  (iteration < MAXITERATION) )
      {
        rtemp = rval * rval - ival * ival + rval0;
        ival = 2 * rval * ival + ival0;
        rval = rtemp;
        iteration++;
      }

 
      if ( iteration == MAXITERATION ) 
      {
        // special case: this point is in the mandelbrot set
        outdata_p[y * width + x] = 0;
      }
      else
      {
        outdata_p[y * width + x] = iteration;
      }

    }

    if((x % 10) == 9)
    {
      printf(".");
    }
  }

  }
  
  printf("\n\n");

  return 0;
}

