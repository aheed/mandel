# mandel

Renders an image from the Mandelbrot set

## To build and run

Requires a compiler with Openmp support or omp pragmas will be ignored.
gcc version 4.2 or higher supports openmp.

### To build with gcc

gcc main.c mandel_omp.c -fopenmp

### To Run

$ ./a.out 600 600 -2.0 0.6 -1.3 1.3

To force a certain number of parallel threads when running this program:
Set the environment variable OMP_NUM_THREADS to desired number of threads.

Example:

$ OMP_NUM_THREADS=4 OMP_SCHEDULE=dynamic,8 ./a.out 300 300 -0.751 -0.735 0.118 0.134

## To build and run, Windows 10, Visual Studio

### Prerequisites

VS Installer -> Desktop development with C++

### Open Developer prompt

Windows start menu -> Developer Command Prompt

### Build

cl main.c mandel_omp.c /openmp

### Run (example)

set OMP_NUM_THREADS=4 & set OMP_SCHEDULE=dynamic,8 & main.exe 2600 2600 -0.751 -0.735 0.118 0.134

## CUDA implementation

This is an alternative implementation targeting nVIDIA GPUs. Requires a compatible GPU, Visual Studio 2019, CUDA Toolkit.

### Build with VS 2019

Open cuda\mandel.sln.
Build the solution (Ctrl+Shift+B)

### Run (example)

cuda\bin\win64\Release\mandelcuda.exe 2600 2600 -0.751 -0.735 0.118 0.134
