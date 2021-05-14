#include <stdint.h>

#define U32 uint32_t
#define U16 uint16_t
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
	    
