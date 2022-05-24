/****************************************
 * gfx.c
 * 
 * An example showing multiple ways of plotting pixels
 * on a graphics 8 screen.  From slowest to fastest.
 * 
 * build:
 *  cl65 -Osir -C gfx.cfg -t atari gfx.c gr8sprite.s -o gfx.xex
 */

#include <atari.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <peekpoke.h>

#include "gfx.h"
#include "gr8.h"

typedef struct {
    bool run_test;
    char test_name[ 30 ];
    word (*func)( void );
    word jiffies;
} test;

test tests[] = {
    { true,  "SAVMSC + MATH",             &plot_savmsc_test, 0 },
    { true,  "SAVMSC + SHIFTS",           &plot_savmsc_test_shifts, 0 },
    { true,  "SAVMSC + REGISTER",         &plot_shifts_and_register, 0 },
    { true,  "SAVMSC + MASK LOOKUP",      &plot_pixel_mask, 0 },
    { true,  "SAVMSC + MASK PLUS",        &plot_pixel_mask_plus, 0 },
    { true,  "SAVMSC + FULL LOOKUPS",     &plot_pixel_lookups, 0 },
    { true,  "SAVMSC + FULL LOOKUPS-ZP",  &plot_pixel_lookups_zp, 0 },
    { true,  "SAVMSC + FULL LOOKUPS-ZP2", &plot_pixel_lookups_zp2, 0 },
    { true,  "SAVMSC + FULL LOOKUPS-ZP3", &plot_pixel_lookups_zp3, 0 },
    { true,  "FULL ZERO PAGE",            &plot_pixel_lookups_zp4, 0 },
    { true,  "SAVMSC + FULL ZP + 256",    &plot_pixel_256_zp5, 0 },
    { true,  "SAVMSC + ASM + 256",        &plot_pixel_256_asm, 0 },
    { true,  "CHEATING - INLINE",         &plot_pixel_lookups_zpCheat, 0 },
    { true,  "CHEATING - MEMSET",         &plot_pixel_cheat, 0 }
};

#define TEST_COUNT ( sizeof( tests ) / sizeof( test ) )

#define PAGE_ZERO_X 0xe0
#define PAGE_ZERO_Y 0xe2

#define lbzero( p, s ) memset( (byte *)p, 0, s )

/*
 * print_at( word x, word y, char *str )
 *
 * prints a string at text coordinate specified (0..39, 0..23).
 */
void print_at( word x, word y, char *str ) {
  word font_table = 0xe000;
  byte character_row;
  byte character_byte;
  word byte_index;
  byte y1;
  byte character;
 
  bool invert = false;
  
  y <<= 3;			/* convert text Y coordinate to gr.8 Y */

  while( *str ) {
    character = *str;

    if( character > 127 ) {
      character -= 128;
      invert = true;
    } else
      invert = false;

    if( character > 31 && character < 96 )
      character -= 32;
    else if( character < 32 )
      character += 64;

    byte_index = (word)character;
    byte_index = byte_index * 8;

    for( character_row = 0, y1 = y; character_row < 7; ++character_row, y1++ ) {
      character_byte = PEEK( font_table + byte_index + character_row);
      if( invert )
	    character_byte = ~character_byte;
      POKE( SAVMSC + (y1 * 40 ) + x , character_byte );
    }
    ++x;
    ++str;
  }
}


/****************************************************************
 * plotter test using addition, multiplication, division, and 
 * modulus
 */
void plot_savmsc( word x, word y ) {
    word address = ( y * 40 ) + ( x / 8 );
    byte mask    = 0x80;
    byte current_byte;

    current_byte = PEEK( SAVMSC + address );
    mask = 0x80 / ( x % 8 );
    POKE( SAVMSC + address, current_byte | mask );
}

word plot_savmsc_test( void ) {
    word x, y;

    lbzero( SAVMSC, 8192 );
    print_at( 0, 23, "Naive plot" );
    clear_clock();

    for( y = 0; y < 192; y++ )
        for( x = 0; x < 320; x++ )
            plot_savmsc( x, y );

    return (word) ( OS.rtclok[ 1 ] * 256 + OS.rtclok[2] );
}


/****************************************************************
 * plotter test using multiplication, shifts, and modulus.
 * notce we make two changes here...swapping out division
 * for shifts as we're doing power of 2 math here....
 */
void plot_savmsc_shifts( word x, word y ) {
    word address = ( y * 40 ) + ( x >> 3 );
    byte mask    = 0x80;
    byte current_byte;

    current_byte = PEEK( SAVMSC + address );
    mask = 0x80 >> ( x % 8 );
    POKE( SAVMSC + address, current_byte | mask );
}

word plot_savmsc_test_shifts( void ) {
    word x, y;

    lbzero( SAVMSC, 8192 );
    print_at( 0, 23, "Shifting X coord by 3" );
    clear_clock();

    for( y = 0; y < 192; y++ )
        for( x = 0; x < 320; x++ )
            plot_savmsc_shifts( x, y );

    return (word) ( OS.rtclok[ 1 ] * 256 + OS.rtclok[2] );
}

/****************************************************************
 * this time we use registers because we heard that "registers
 * speed things up!"  as you'll see here the answer is "it
 * depends."
 */
void plot_shifts_register( word x, word y ) {
    register word address = ( y * 40 ) + ( x >> 3 );
    register byte mask    = 0x80;
    register byte current_byte;

    current_byte = PEEK( SAVMSC + address );
    mask = mask >> ( x % 8 );
    POKE( SAVMSC + address, current_byte | mask );
}

word plot_shifts_and_register( void ) {
    word x, y;

    lbzero( SAVMSC, 8192 );
    print_at(0, 23, "Plot using REGISTERS");
    clear_clock();

    for( y = 0; y < 192; y++ )
        for( x = 0; x < 320; x++ )
            plot_shifts_register( x, y );

    return (word) ( OS.rtclok[ 1 ] * 256 + OS.rtclok[2] );
}


/****************************************************************
 * plotter test introducing lookups
 * we create a lookup table for the masks and use that vs.
 * doing the math at runtime.  Classic trading space for time.
 */
byte pixel_masks[ 8 ] = {
    0b10000000,                 // 0x80
    0b01000000,                 // 0x40
    0b00100000,                 // 0x20
    0b00010000,                 // 0x10
    0b00001000,                 // 0x08
    0b00000100,                 // 0x04
    0b00000010,                 // 0x02
    0b00000001                  // 0x01
};

void plot_shifts_mask( word x, word y ) {
    word address = ( y * 40 ) + ( x >> 3 );
    byte mask    = pixel_masks[ x % 8 ];
    byte current_byte;

    current_byte = PEEK( SAVMSC + address );
    POKE( SAVMSC + address, current_byte | mask );
}

word plot_pixel_mask( void ) {
    word x, y;

    lbzero( SAVMSC, 8192 );
    print_at( 0, 23, "Plot using X shift and mask tbl" );
    clear_clock();

    for( y = 0; y < 192; y++ )
        for( x = 0; x < 320; x++ )
            plot_shifts_mask( x, y );

    return (word) ( OS.rtclok[ 1 ] * 256 + OS.rtclok[2] );
}


/****************************************************************
 * in this test we introduce lookups for the row we are 
 * updating...  reuses pixel_mask[].
 * 
 * we remove the "y*40" and replace it with a lookup to our
 * graphics rows.  This allows us to remove the additions in
 * the two peek/pokes.  We also init current_byte immediatly
 * and yes variable order matters...  
 */

word row_addresses[ 192 ];

void plot_shifts_mask_plus( word x, word y ) {
    word address      = row_addresses[ y ] + ( x >> 3 );
    byte mask         = pixel_masks[ x % 8 ];
    byte current_byte = PEEK( address );

    POKE( address, current_byte | mask );
}

word plot_pixel_mask_plus( void ) {
    word x, y, i, addr;

    lbzero( SAVMSC, 8192 );

    for( i = 0, addr = (word)SAVMSC; i < 192; i++, addr += 40 )
        row_addresses[ i ] = addr;
    
    print_at( 0, 23, "mask tbl, row tbl, x shift" );
    clear_clock();

    for( y = 0; y < 192; y++ )
        for( x = 0; x < 320; x++ )
            plot_shifts_mask_plus( x, y );

    return (word) ( OS.rtclok[ 1 ] * 256 + OS.rtclok[2] );
}


/****************************************************************
* row addresses worked so well...how about column lookups?
* 
* in this case we replace the x>>3 and x % 8 with a lookup table 
* based on the column.
*
* reuses row_addresses.
*/
byte column_offsets[ 320 ];
byte column_masks[ 320 ];

void plot_full_lookups( word x, word y ) {
    word address      = row_addresses[ y ] + column_offsets[ x ];
    byte mask         = column_masks[ x ];
    byte current_byte = PEEK( address );

    POKE( address, current_byte | mask );
}

word plot_pixel_lookups( void ) {
    word x, y, i, addr;

    lbzero( SAVMSC, 8192 );

    for( i = 0, addr = (word)SAVMSC; i < 192; i++, addr += 40 )
        row_addresses[ i ] = addr;

    for( i = 0; i < 320; i++ ) {
        column_masks[ i ] = pixel_masks[ i % 8 ];
        column_offsets[ i ] = i >> 3;
    }

    print_at( 0, 23, "row/col/mask tbl" );

    clear_clock();

    for( y = 0; y < 192; y++ )
        for( x = 0; x < 320; x++ )
            plot_full_lookups( x, y );

    return (word) ( OS.rtclok[ 1 ] * 256 + OS.rtclok[2] );
}

/****************************************************************
* passing parameters can be expensive...so we use zero page to
* pass our coords.  In this case we're using the FR1 space 
* normally reserved for floating point use by the OS.
*
* reuses all the lookup tables...
*/
void plot_full_lookups_zp( void ) {
    word address      = row_addresses[ PEEK( PAGE_ZERO_Y )] + 
                        column_offsets[ PEEKW( PAGE_ZERO_X )];
    byte mask         = column_masks[ PEEKW( PAGE_ZERO_X )];
    byte current_byte = PEEK( address );

    POKE( address, current_byte | mask );
}

word plot_pixel_lookups_zp( void ) {
    word x, y;

    lbzero( SAVMSC, 8192 );
    print_at( 0, 23, "ZP Function/Vars" );
    clear_clock();

    for( y = 0; y < 192; y++ )
        for( x = 0; x < 320; x++ ) {
            POKEW( PAGE_ZERO_X, x);
            POKE( PAGE_ZERO_Y, y);
            plot_full_lookups_zp();
        }

    return (word) ( OS.rtclok[ 1 ] * 256 + OS.rtclok[2] );
}



/****************************************************************
* in this case we start trimming the variables we delcare.  do
* we really need to declare/init current_byte?  Nope.
*
* reuses all the lookup tables...
*/
void plot_full_lookups_zp2( void ) {
    word address      = row_addresses[ PEEK( PAGE_ZERO_Y )] + 
                        column_offsets[ PEEKW( PAGE_ZERO_X )];
    byte mask         = column_masks[ PEEKW( PAGE_ZERO_X )];
    
    POKE( address, PEEK( address ) | mask );
}

word plot_pixel_lookups_zp2( void ) {
    word x, y;

    lbzero( SAVMSC, 8192 );
    print_at( 0, 23, "ZP Trim var usage" );
    clear_clock();

    for( y = 0; y < 192; y++ )
        for( x = 0; x < 320; x++ ) {
            POKEW( PAGE_ZERO_X, x);
            POKE( PAGE_ZERO_Y, y);
            plot_full_lookups_zp2();
        }

    return (word) ( OS.rtclok[ 1 ] * 256 + OS.rtclok[2] );
}


/****************************************************************
* calling routines are expensive...lets inline the gfx work and
* use zero page for our x/y loops and see what the results are...
*
* reuses all the lookup tables...
*/

#define Y_VAR (*(byte *)PAGE_ZERO_Y)
#define X_VAR (*(word *)PAGE_ZERO_X)

word plot_pixel_lookups_zp3( void ) {
    word base_address;
    word address;
    byte mask;

    lbzero( SAVMSC, 8192 );

    print_at( 0, 23, "ZP inline plot with locals" );

    clear_clock();

    for( Y_VAR = 0; Y_VAR < 192; Y_VAR++ ) {
        
        base_address = row_addresses[ Y_VAR ];

        for( X_VAR = 0; X_VAR < 320; X_VAR++ ) {
            address = base_address + column_offsets[ X_VAR ];
            mask    = column_masks[ X_VAR ];
            POKE( address, PEEK( address ) | mask );
        }
    }

    return (word) ( OS.rtclok[ 1 ] * 256 + OS.rtclok[2] );
}


/****************************************************************
* use ZP for all vars...
*
* reuses all the lookup tables...  Uses FRE/FR1 ZP reserved for
* OS floating point routines.
*/

#define Y_VAR (*(byte *)PAGE_ZERO_Y)
#define X_VAR (*(word *)PAGE_ZERO_X)
#define BASE_ADDR_VAR (*(word *)0xe3)
#define ADDR_VAR (*(word *)0xe5)
#define MASK_VAR (*(byte *)0xe7)

word plot_pixel_lookups_zp4( void ) {
    lbzero( SAVMSC, 8192 );

    print_at( 0, 23, "ZP inline" );

    clear_clock();

    for( Y_VAR = 0; Y_VAR < 192; Y_VAR++ ) {       
        BASE_ADDR_VAR = row_addresses[ Y_VAR ];
        for( X_VAR = 0; X_VAR < 320; X_VAR++ ) {
            ADDR_VAR = BASE_ADDR_VAR + column_offsets[ X_VAR ];
            MASK_VAR = PEEK( ADDR_VAR) | column_masks[ X_VAR ];
            POKE( ADDR_VAR, MASK_VAR );
        }
    }

    return (word) ( OS.rtclok[ 1 ] * 256 + OS.rtclok[2] );
}


/****************************************************************
* optimize based on end case...full plot!
*
* reuses all the lookup tables...  Uses FRE/FR1 ZP reserved for
* OS floating point routines.
*/

word plot_pixel_lookups_zpCheat( void ) {

    lbzero( SAVMSC, 8192 );

    print_at( 0, 23, "ZP inline-unwrapped X loop" );

    clear_clock();
    
    for( Y_VAR = 0; Y_VAR < 192; Y_VAR++ ) {       
        BASE_ADDR_VAR = row_addresses[ Y_VAR ];
        POKE(   BASE_ADDR_VAR, 0xff );
        POKE( ++BASE_ADDR_VAR, 0xff );
        POKE( ++BASE_ADDR_VAR, 0xff );
        POKE( ++BASE_ADDR_VAR, 0xff );
        POKE( ++BASE_ADDR_VAR, 0xff );
        POKE( ++BASE_ADDR_VAR, 0xff );
        POKE( ++BASE_ADDR_VAR, 0xff );
        POKE( ++BASE_ADDR_VAR, 0xff );
        POKE( ++BASE_ADDR_VAR, 0xff );
        POKE( ++BASE_ADDR_VAR, 0xff );
        POKE( ++BASE_ADDR_VAR, 0xff );
        POKE( ++BASE_ADDR_VAR, 0xff );
        POKE( ++BASE_ADDR_VAR, 0xff );
        POKE( ++BASE_ADDR_VAR, 0xff );
        POKE( ++BASE_ADDR_VAR, 0xff );
        POKE( ++BASE_ADDR_VAR, 0xff );
        POKE( ++BASE_ADDR_VAR, 0xff );
        POKE( ++BASE_ADDR_VAR, 0xff );
        POKE( ++BASE_ADDR_VAR, 0xff );
        POKE( ++BASE_ADDR_VAR, 0xff );
        POKE( ++BASE_ADDR_VAR, 0xff );
        POKE( ++BASE_ADDR_VAR, 0xff );
        POKE( ++BASE_ADDR_VAR, 0xff );
        POKE( ++BASE_ADDR_VAR, 0xff );
        POKE( ++BASE_ADDR_VAR, 0xff );
        POKE( ++BASE_ADDR_VAR, 0xff );
        POKE( ++BASE_ADDR_VAR, 0xff );
        POKE( ++BASE_ADDR_VAR, 0xff );
        POKE( ++BASE_ADDR_VAR, 0xff );
        POKE( ++BASE_ADDR_VAR, 0xff );
        POKE( ++BASE_ADDR_VAR, 0xff );
        POKE( ++BASE_ADDR_VAR, 0xff );
        POKE( ++BASE_ADDR_VAR, 0xff );
        POKE( ++BASE_ADDR_VAR, 0xff );
        POKE( ++BASE_ADDR_VAR, 0xff );
        POKE( ++BASE_ADDR_VAR, 0xff );
        POKE( ++BASE_ADDR_VAR, 0xff );
        POKE( ++BASE_ADDR_VAR, 0xff );
        POKE( ++BASE_ADDR_VAR, 0xff );
        POKE( ++BASE_ADDR_VAR, 0xff );
    }

    return (word) ( OS.rtclok[ 1 ] * 256 + OS.rtclok[2] );
}

/****************************************************************
 * cheat mode 2 based on desired end state...
 */
word plot_pixel_cheat( void ) {
    lbzero( SAVMSC, 8192 );
    print_at( 0, 23, "Using MEMSET" );
    clear_clock();
    memset( (byte *)SAVMSC, 0xff, 8192 );
    return (word) ( OS.rtclok[ 1 ] * 256 + OS.rtclok[2] );
}



/****************************************************************
 * pretend the gr8 screen is 256 x 192
 * 
 * notice how this code is starting to look like assembly?
 */
#define X_VAR_BYTE (*(byte *)PAGE_ZERO_X)

byte column_offsets_256[ 256 ];

word plot_pixel_256_zp5( void ) {
    word i;

    // center visual display for our fake 256x192 mode
    for(i = 0; i < 256; i++ )
        column_offsets_256[ i ] = column_offsets[ i ] + 4;

    lbzero( SAVMSC, 8192 );
    print_at( 0, 23, "Full ZP faking 256x192");
    clear_clock();

    Y_VAR = 0;
draw_row:
    BASE_ADDR_VAR = row_addresses[ Y_VAR ];
    X_VAR_BYTE = 0;
next_column:
    ADDR_VAR = BASE_ADDR_VAR + column_offsets_256[ X_VAR_BYTE ];
    MASK_VAR = PEEK( ADDR_VAR) | column_masks[ X_VAR_BYTE ];
    POKE( ADDR_VAR, MASK_VAR );
    if( X_VAR_BYTE == 255 )
        goto next_row;
    ++X_VAR_BYTE;
    goto next_column;
next_row:
    if( ++Y_VAR == 192 )
        goto exit;
    goto draw_row;
exit:
    return (word) ( OS.rtclok[ 1 ] * 256 + OS.rtclok[2] );
}


word plot_pixel_256_asm( void ) {
    BASEGFX = (word)SAVMSC;
    init_fast();   

    lbzero( SAVMSC, 8192 );
    print_at( 0, 23, "Full ZP ASM faking 256x192");
    clear_clock();

    YB = 0;
draw_row:
    XB = 0;
next_column:
    plot_pixel_256();
    if( XB == 255 )
        goto next_row;
    ++XB;
    goto next_column;
next_row:
    if( ++YB == 192 )
        goto exit;
    goto draw_row;
exit:
    return (word) ( OS.rtclok[ 1 ] * 256 + OS.rtclok[2] );
}


/****************************************************************
 * code to drive the tests starts here
 */

void clear_clock( void ) {
    OS.rtclok[ 1 ] = 0;
    OS.rtclok[ 2 ] = 0;
}

void main( void ) {
    int iteration;

    _graphics( 8 + 16 );

    for( iteration = 0; iteration < TEST_COUNT; iteration++ )
        if( tests[ iteration ].run_test )
            tests[ iteration ].jiffies = tests[ iteration ].func();

    _graphics( 0 );

    for( iteration = 0; iteration < TEST_COUNT; iteration++ )
        if( tests[ iteration ].run_test )
            printf("%25s %5d %5d\n",
                tests[ iteration ].test_name,
                tests[ iteration ].jiffies,
                ( iteration == 10 || iteration == 11 ? ( 256 * 192 ) : ( 320 * 192 )) / tests[ iteration ].jiffies );

    while( true )
        ;
}   
