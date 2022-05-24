#ifndef __GFX_H
#define __GFX_H

typedef unsigned int word;
typedef unsigned char byte;

/* test drivers for use cases */
word plot_savmsc_test( void );
word plot_savmsc_test_shifts( void );
word plot_pixel_mask( void );
word plot_shifts_and_register( void );
word plot_pixel_mask_plus( void );
word plot_pixel_lookups( void );
word plot_pixel_lookups_zp( void );
word plot_pixel_lookups_zp2( void );
word plot_pixel_lookups_zp3( void );
word plot_pixel_lookups_zp4( void );
word plot_pixel_lookups_zpCheat( void );
word plot_pixel_cheat( void );
word plot_pixel_256_zp5( void );
word plot_pixel_256_asm( void );
void clear_clock( void );

#endif