#ifndef __GR8_H
#define __GR8_H

#include "common.h"

#define SAVMSC *((word *) 0x0058)
#define WSYNC  *((byte *) 0xd40a)
#define COLOR0 *((byte *) 0x02c4)
#define COLOR1 *((byte *) 0x02c5)
#define COLOR2 *((byte *) 0x02c6)
#define COLOR3 *((byte *) 0x02c7)
#define COLBK  *((byte *) 0x02c8)
#define CHBAS  *((word *) 0x02f4)
#define RANDOM *((byte *) 0xd20a)
#define RTCLOK *((byte *) 0x0012)
#define RT2    *((byte *) 0x0013)
#define RT3    *((byte *) 0x0014)
#define VCOUNT *((byte *) 0xd40b)
#define RANDOM *((byte *) 0xd20a)

/*
 * zero page bits
 */
#define DESTINATION *((word *) 0xd4)
#define SOURCE      *((word *) 0xd6)
#define BASEGFX     *((word *) 0xe0)
#define XX          *((word *) 0xe2)
#define YY          *((word *) 0xe4)
#define ARGUMENT    *((word *) 0xe6)
#define WIDTH       *((word *) 0xe8)
#define HEIGHT      *((word *) 0xea)
#define XX2         *((word *) 0xec)
#define YY2         *((word *) 0xee)
#define XB          *((byte *) 0xe2)
#define YB          *((byte *) 0xe3)
#define WB	        *((byte *) 0xe8)
#define HB          *((byte *) 0xe9)
#define BITMAP      *((word *) 0xf0)

  
/*
 * declare the assembly routines defined in gr8sprite.s
 */
extern void plot_pixel_256_opt( void );
extern void plot_pixel_256( void );
extern void plot_pixel_256_fast( void );
extern void init_fast( void );
extern void plot_pixel_256_full( void );

#endif