#include <stdio.h>

// For the CUDA runtime routines (prefixed with "cuda_")
#include <cuda_runtime.h>

#include <helper_cuda.h>
#include <cassert>

#define U32 uint32_t
#define U16 uint16_t
#define U8  unsigned char


#define PIXELVAL U32
#define MAXITERATION 2000

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
    char* data_p;
} BMP24BIT;


typedef struct COLOR24BIT
{
    U8 blue;
    U8 green;
    U8 red;
} COLOR24BIT;


///////////
void Mandelval2Color(PIXELVAL pixval, COLOR24BIT* colorval_p)
{
    int colindex;

    if (pixval == 0)
    {
        // special case pixval = 0 => Black
        colorval_p->blue = 0;
        colorval_p->green = 0;
        colorval_p->red = 0;
        return;
    }

    colindex = pixval % COLORSUSED; // pick a color index
    colindex *= (MAXCOLORVAL / COLORSUSED); // spread colorindexes over value span  

    if (colindex > (MAXCOLORVAL / 2))
    {
        colorval_p->blue = MAXCOLORVAL - (colindex % MAXCOLORVAL);
    }
    else
    {
        colorval_p->blue = (colindex % MAXCOLORVAL);
    }

    colindex = (colindex + (MAXCOLORVAL / 3)) % MAXCOLORVAL;
    if (colindex > (MAXCOLORVAL / 2))
    {
        colorval_p->green = MAXCOLORVAL - (colindex % MAXCOLORVAL);
    }
    else
    {
        colorval_p->green = (colindex % MAXCOLORVAL);
    }

    colindex = (colindex + (MAXCOLORVAL / 3)) % MAXCOLORVAL;
    if (colindex > (MAXCOLORVAL / 2))
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

int SavePic(char* pszFile,
    int width,
    int height,
    PIXELVAL* indata_p)
{
    int x, y;
    BMP24BIT bmpinfo;
    char* pdata;
    FILE* outfile;
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
    if (!bmpinfo.data_p)
    {
        printf("out of memory\n");
        return 1;
    }


    // Assign bitmap data
    pdata = bmpinfo.data_p;
    for (x = 0; x < width; x++)
    {
        for (y = 0; y < height; y++)
        {
            Mandelval2Color(indata_p[x * height + y], &colorval);

            *pdata++ = colorval.blue;
            *pdata++ = colorval.green;
            *pdata++ = colorval.red;
        }
    }


    outfile = fopen(pszFile, "wb");
    if (!outfile)
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


    if (written != 16)
    {
        printf("Failed to write file header\nWritten items %d (%d)\n", written, 15);
        fclose(outfile); // never mind return value
        free(bmpinfo.data_p);
        return 1;
    }


    // write image data
    if (fwrite(bmpinfo.data_p, bmpinfo.bitmapheader.SizeOfBitmap, 1, outfile) != 1)
    {
        printf("Failed to write image data to file\n");
        fclose(outfile); // never mind return value
        free(bmpinfo.data_p);
        return 1;
    }

    free(bmpinfo.data_p);
    bmpinfo.data_p = NULL;

    if (fclose(outfile))
    {
        printf("Failed to close file %s\n", pszFile);
        return 1;
    }


    return 0;
}

//////


/////
__global__ void MandelBrotIterations(const double* rval0Array, const double* ival0Array, PIXELVAL* outdata_p, int maxIterations, int numElements)
{
    double rval, ival, rval0, ival0, rtemp;
    int iteration;

    int i = blockDim.x * blockIdx.x + threadIdx.x;

    if (i >= numElements)
    {
        return;
    }

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


/////

/**
 * CUDA Kernel Device code
 *
 * Computes the vector addition of A and B into C. The 3 vectors have the same
 * number of elements numElements.
 */
__global__ void
vectorAdd(const float *A, const float *B, float *C, int numElements)
{
    int i = blockDim.x * blockIdx.x + threadIdx.x;

    if (i < numElements)
    {
        C[i] = A[i] + B[i];
    }
}

/**
 * Host main routine
 */
int
main(int argc, char* argv[])
{
    // Error code to check return values for CUDA calls
    cudaError_t err = cudaSuccess;
    
    /// ///////////////////

    int width, height;
    float rmin, rmax, imin, imax;
    char szFileName[] = "result.bmp";
    time_t starttime, endtime, elapsed;
    int x, y, index;
    double rval0, ival0;

    assert(sizeof(U32) == 4);
    assert(sizeof(U16) == 2);
    assert(sizeof(U8) == 1);

    if (argc != 7)
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
    

    /////////

    int numMandelPoints = width * height;

    PIXELVAL *mandelData_p = (PIXELVAL*)malloc(numMandelPoints * sizeof(PIXELVAL));
    double* rvals = (double*)malloc(numMandelPoints * sizeof(double));
    double* ivals = (double*)malloc(numMandelPoints * sizeof(double));

    // Verify that allocations succeeded
    if (rvals == NULL || rvals == NULL || mandelData_p == NULL)
    {
        fprintf(stderr, "Failed to allocate host vectors!\n");
        exit(EXIT_FAILURE);
    }

    // Initialize the host input vectors
    for (x = 0; x < width; x++)
    {
        rval0 = rmin + (rmax - rmin) * ((double)x / width);

        for (y = 0; y < height; y++)
        {
            ival0 = imin + (imax - imin) * ((double)y / height);

            index = y * width + x;

            rvals[index] = rval0;
            ivals[index] = ival0;
        }
    }

    // Allocate the device input vector rvals
    double* d_rvals = NULL;
    err = cudaMalloc((void**)&d_rvals, numMandelPoints * sizeof(double));

    if (err != cudaSuccess)
    {
        fprintf(stderr, "Failed to allocate device vector d_rvals (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    // Allocate the device input vector ivals
    double* d_ivals = NULL;
    err = cudaMalloc((void**)&d_ivals, numMandelPoints * sizeof(double));

    if (err != cudaSuccess)
    {
        fprintf(stderr, "Failed to allocate device vector d_ivals (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    // Allocate the device output vector d_mandelData_p
    PIXELVAL* d_mandelData_p = NULL;
    err = cudaMalloc((void**)&d_mandelData_p, numMandelPoints * sizeof(PIXELVAL));

    if (err != cudaSuccess)
    {
        fprintf(stderr, "Failed to allocate device vector d_mandelData_p (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    // Copy the host input vectors d_rvals and d_ivals in host memory to the device input vectors in
    // device memory
    printf("Copy input data from the host memory to the CUDA device\n");
    err = cudaMemcpy(d_rvals, rvals, numMandelPoints * sizeof(double), cudaMemcpyHostToDevice);
    if (err != cudaSuccess)
    {
        fprintf(stderr, "Failed to copy vector rvals from host to device (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    err = cudaMemcpy(d_ivals, ivals, numMandelPoints * sizeof(double), cudaMemcpyHostToDevice);
    if (err != cudaSuccess)
    {
        fprintf(stderr, "Failed to copy vector ivals from host to device (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    time(&starttime);

    // Launch the Mandelbrot CUDA Kernel
    int threadsPerBlock = 256;
    int blocksPerGrid = (numMandelPoints + threadsPerBlock - 1) / threadsPerBlock;
    printf("CUDA kernel launch with %d blocks of %d threads\n", blocksPerGrid, threadsPerBlock);
    MandelBrotIterations << <blocksPerGrid, threadsPerBlock >> > (d_rvals, d_ivals, d_mandelData_p, MAXITERATION, numMandelPoints);
    err = cudaGetLastError();

    if (err != cudaSuccess)
    {
        fprintf(stderr, "Failed to launch MandelBrotIterations kernel (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    // Copy the device result vector in device memory to the host result vector
    // in host memory.
    printf("Copy output data from the CUDA device to the host memory\n");
    err = cudaMemcpy(mandelData_p, d_mandelData_p, numMandelPoints * sizeof(PIXELVAL), cudaMemcpyDeviceToHost);

    if (err != cudaSuccess)
    {
        fprintf(stderr, "Failed to copy vector mandelData_p from device to host (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    time(&endtime);
    elapsed = endtime - starttime;
    printf("Processing time: %lld sec\n\n", elapsed);

    if (SavePic(szFileName, width, height, mandelData_p))
    {
        printf("Failed to save file\n");
        return 1;
    }

    printf("Output file: %s\n", szFileName);

    err = cudaFree(d_rvals);

    if (err != cudaSuccess)
    {
        fprintf(stderr, "Failed to free device vector d_rvals (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    err = cudaFree(d_ivals);

    if (err != cudaSuccess)
    {
        fprintf(stderr, "Failed to free device vector d_ivals (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    err = cudaFree(d_mandelData_p);

    if (err != cudaSuccess)
    {
        fprintf(stderr, "Failed to free device vector d_mandelData_p (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    // Free host memory
    free(rvals);
    free(ivals);
    free(mandelData_p);


    /////////////////////////
    //////////////////////////
    
    printf("Done\n");    

    return 0;
}

