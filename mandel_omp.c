#include <stdio.h>
#include <malloc.h>
#include <omp.h>
#include "mandel.h"

void MandelBrotIterations(const double *rval0Array, const double *ival0Array, PIXELVAL *outdata_p, int maxIterations, int i)
{
  double rval, ival, rval0, ival0, rtemp;
  int iteration;

  rval0 = rval0Array[i];
  ival0 = ival0Array[i];
  rval = rval0;
  ival = ival0;
  iteration = 1;

  while ((rval * rval + ival * ival <= (2 * 2)) && (iteration < maxIterations))
  {
    rtemp = rval * rval - ival * ival + rval0;
    ival = 2 * rval * ival + ival0;
    rval = rtemp;
    iteration++;
  }

  if (iteration == maxIterations)
  {
    // special case: this point is in the mandelbrot set
    iteration = 0;
  }

  outdata_p[i] = iteration;
}

int CalcMandel(int width,
               int height,
               double rmin,
               double rmax,
               double imin,
               double imax,
               PIXELVAL *outdata_p)
{
  int x, y, index;
  double rval0, ival0;

  int numElements = width * height;
  double *rvals = (double *)malloc(numElements * sizeof(double));
  double *ivals = (double *)malloc(numElements * sizeof(double));

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

//#pragma omp for private(iteration, rval, ival, rval0, ival0, rtemp, y) schedule(runtime)
#pragma omp for private(index, rval0, ival0, x, y) schedule(runtime)
    for (x = 0; x < width; x++)
    {
      rval0 = rmin + (rmax - rmin) * ((double)x / width);

      for (y = 0; y < height; y++)
      {
        ival0 = imin + (imax - imin) * ((double)y / height);

        index = y * width + x;

        rvals[index] = rval0;
        ivals[index] = ival0;

        MandelBrotIterations(rvals, ivals, outdata_p, MAXITERATION, index);
      }

      if ((x % 10) == 9)
      {
        printf(".");
      }
    }
  }

  printf("Done calc");
  printf("\n\n");

  return 0;
}
