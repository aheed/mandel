
#define U32 unsigned long
#define S32 long
#define U16 unsigned short
#define U8  unsigned char
#define PIXELVAL U32
#define MAXITERATION 2000

int CalcMandel(int width,
               int height,
               double rmin,
               double rmax,
               double imin,
               double imax,
               PIXELVAL* outdata_p);
	    
