#include <rtthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include "../lvgl.h"
#include "../lv_conf_internal.h"

/*******************************************************************************
 * Size: 16 px
 * Bpp: 1
 * Opts:
 ******************************************************************************/

#ifndef LV_FONT_UNSCII_16
#define LV_FONT_UNSCII_16 1
#endif

#if LV_FONT_UNSCII_16

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t gylph_bitmap[] = {
    /* U+20 " " */
    0x0,

    /* U+21 "!" */
    0xff, 0xff, 0xff, 0xff, 0xff, 0x0, 0xff,

    /* U+22 "\"" */
    0xf0, 0xff, 0xf, 0xf0, 0xff, 0xf, 0xf0, 0xff,
    0xf,

    /* U+23 "#" */
    0x3c, 0xf0, 0xf3, 0xc3, 0xcf, 0xf, 0x3c, 0xff,
    0xff, 0xff, 0xf3, 0xcf, 0xf, 0x3c, 0xff, 0xff,
    0xff, 0xf3, 0xcf, 0xf, 0x3c, 0x3c, 0xf0, 0xf3,
    0xc0,

    /* U+24 "$" */
    0xf, 0x0, 0xf0, 0x3f, 0xf3, 0xff, 0xf0, 0xf,
    0x0, 0x3f, 0xc3, 0xfc, 0x0, 0xf0, 0xf, 0xff,
    0xcf, 0xfc, 0xf, 0x0, 0xf0,

    /* U+25 "%" */
    0xf0, 0x3f, 0xc0, 0xff, 0xf, 0x3c, 0x3c, 0x3,
    0xc0, 0xf, 0x0, 0xf0, 0x3, 0xc0, 0x3c, 0x3c,
    0xf0, 0xff, 0x3, 0xfc, 0xf,

    /* U+26 "&" */
    0xf, 0xc0, 0x3f, 0x3, 0xcf, 0xf, 0x3c, 0xf,
    0xc0, 0x3f, 0x3, 0xf3, 0xcf, 0xcf, 0xf3, 0xf3,
    0xcf, 0xcf, 0xf, 0x3c, 0x3c, 0x3f, 0x3c, 0xfc,
    0xf0,

    /* U+27 "'" */
    0x3c, 0xf3, 0xcf, 0xf3, 0xc0,

    /* U+28 "(" */
    0xf, 0xf, 0x3c, 0x3c, 0xf0, 0xf0, 0xf0, 0xf0,
    0xf0, 0xf0, 0x3c, 0x3c, 0xf, 0xf,

    /* U+29 ")" */
    0xf0, 0xf0, 0x3c, 0x3c, 0xf, 0xf, 0xf, 0xf,
    0xf, 0xf, 0x3c, 0x3c, 0xf0, 0xf0,

    /* U+2A "*" */
    0x3c, 0x3c, 0x3c, 0x3c, 0xf, 0xf0, 0xf, 0xf0,
    0xff, 0xff, 0xff, 0xff, 0xf, 0xf0, 0xf, 0xf0,
    0x3c, 0x3c, 0x3c, 0x3c,

    /* U+2B "+" */
    0xf, 0x0, 0xf0, 0xf, 0x0, 0xf0, 0xff, 0xff,
    0xff, 0xf, 0x0, 0xf0, 0xf, 0x0, 0xf0,

    /* U+2C "," */
    0x3c, 0xf3, 0xcf, 0xf3, 0xc0,

    /* U+2D "-" */
    0xff, 0xff, 0xff,

    /* U+2E "." */
    0xff, 0xff,

    /* U+2F "/" */
    0x0, 0xf, 0x0, 0xf, 0x0, 0x3c, 0x0, 0x3c,
    0x0, 0xf0, 0x0, 0xf0, 0x3, 0xc0, 0x3, 0xc0,
    0xf, 0x0, 0xf, 0x0, 0x3c, 0x0, 0x3c, 0x0,
    0xf0, 0x0, 0xf0, 0x0,

    /* U+30 "0" */
    0x3f, 0xc3, 0xfc, 0xf0, 0xff, 0xf, 0xf3, 0xff,
    0x3f, 0xfc, 0xff, 0xcf, 0xf0, 0xff, 0xf, 0xf0,
    0xff, 0xf, 0x3f, 0xc3, 0xfc,

    /* U+31 "1" */
    0xf, 0x0, 0xf0, 0x3f, 0x3, 0xf0, 0xf, 0x0,
    0xf0, 0xf, 0x0, 0xf0, 0xf, 0x0, 0xf0, 0xf,
    0x0, 0xf0, 0xff, 0xff, 0xff,

    /* U+32 "2" */
    0x3f, 0xc3, 0xfc, 0xf0, 0xff, 0xf, 0x3, 0xc0,
    0x3c, 0xf, 0x0, 0xf0, 0x3c, 0x3, 0xc0, 0xf0,
    0xf, 0x0, 0xff, 0xff, 0xff,

    /* U+33 "3" */
    0x3f, 0xc3, 0xfc, 0xf0, 0xff, 0xf, 0x0, 0xf0,
    0xf, 0xf, 0xc0, 0xfc, 0x0, 0xf0, 0xf, 0xf0,
    0xff, 0xf, 0x3f, 0xc3, 0xfc,

    /* U+34 "4" */
    0x3, 0xf0, 0xf, 0xc0, 0xff, 0x3, 0xfc, 0x3c,
    0xf0, 0xf3, 0xcf, 0xf, 0x3c, 0x3c, 0xff, 0xff,
    0xff, 0xf0, 0xf, 0x0, 0x3c, 0x0, 0xf0, 0x3,
    0xc0,

    /* U+35 "5" */
    0xff, 0xff, 0xff, 0xf0, 0xf, 0x0, 0xff, 0xcf,
    0xfc, 0x0, 0xf0, 0xf, 0x0, 0xf0, 0xf, 0xf0,
    0xff, 0xf, 0x3f, 0xc3, 0xfc,

    /* U+36 "6" */
    0xf, 0xc0, 0xfc, 0x3c, 0x3, 0xc0, 0xf0, 0xf,
    0x0, 0xff, 0xcf, 0xfc, 0xf0, 0xff, 0xf, 0xf0,
    0xff, 0xf, 0x3f, 0xc3, 0xfc,

    /* U+37 "7" */
    0xff, 0xff, 0xff, 0x0, 0xf0, 0xf, 0x0, 0xf0,
    0xf, 0x3, 0xc0, 0x3c, 0xf, 0x0, 0xf0, 0xf,
    0x0, 0xf0, 0xf, 0x0, 0xf0,

    /* U+38 "8" */
    0x3f, 0xc3, 0xfc, 0xf0, 0xff, 0xf, 0xf0, 0xff,
    0xf, 0x3f, 0xc3, 0xfc, 0xf0, 0xff, 0xf, 0xf0,
    0xff, 0xf, 0x3f, 0xc3, 0xfc,

    /* U+39 "9" */
    0x3f, 0xc3, 0xfc, 0xf0, 0xff, 0xf, 0xf0, 0xff,
    0xf, 0x3f, 0xf3, 0xff, 0x0, 0xf0, 0xf, 0x3,
    0xc0, 0x3c, 0x3f, 0x3, 0xf0,

    /* U+3A ":" */
    0xff, 0xff, 0x0, 0x0, 0xff, 0xff,

    /* U+3B ";" */
    0x3c, 0xf3, 0xcf, 0x0, 0x0, 0x0, 0x3c, 0xf3,
    0xcf, 0xf3, 0xc0,

    /* U+3C "<" */
    0x3, 0xc0, 0xf0, 0xf0, 0x3c, 0x3c, 0xf, 0xf,
    0x3, 0xc0, 0x3c, 0xf, 0x0, 0xf0, 0x3c, 0x3,
    0xc0, 0xf0,

    /* U+3D "=" */
    0xff, 0xff, 0xff, 0x0, 0x0, 0x0, 0xff, 0xff,
    0xff,

    /* U+3E ">" */
    0xf0, 0x3c, 0x3, 0xc0, 0xf0, 0xf, 0x3, 0xc0,
    0x3c, 0xf, 0xf, 0x3, 0xc3, 0xc0, 0xf0, 0xf0,
    0x3c, 0x0,

    /* U+3F "?" */
    0x3f, 0xc3, 0xfc, 0xf0, 0xff, 0xf, 0x0, 0xf0,
    0xf, 0x3, 0xc0, 0x3c, 0xf, 0x0, 0xf0, 0x0,
    0x0, 0x0, 0xf, 0x0, 0xf0,

    /* U+40 "@" */
    0x3f, 0xf0, 0xff, 0xcf, 0x3, 0xfc, 0xf, 0xf3,
    0xff, 0xcf, 0xff, 0x3f, 0xfc, 0xff, 0xf3, 0xff,
    0xcf, 0xff, 0x0, 0x3c, 0x0, 0x3f, 0xf0, 0xff,
    0xc0,

    /* U+41 "A" */
    0xf, 0x0, 0xf0, 0x3f, 0xc3, 0xfc, 0xf0, 0xff,
    0xf, 0xf0, 0xff, 0xf, 0xff, 0xff, 0xff, 0xf0,
    0xff, 0xf, 0xf0, 0xff, 0xf,

    /* U+42 "B" */
    0xff, 0xcf, 0xfc, 0xf0, 0xff, 0xf, 0xf0, 0xff,
    0xf, 0xff, 0xcf, 0xfc, 0xf0, 0xff, 0xf, 0xf0,
    0xff, 0xf, 0xff, 0xcf, 0xfc,

    /* U+43 "C" */
    0x3f, 0xc3, 0xfc, 0xf0, 0xff, 0xf, 0xf0, 0xf,
    0x0, 0xf0, 0xf, 0x0, 0xf0, 0xf, 0x0, 0xf0,
    0xff, 0xf, 0x3f, 0xc3, 0xfc,

    /* U+44 "D" */
    0xff, 0xf, 0xf0, 0xf3, 0xcf, 0x3c, 0xf0, 0xff,
    0xf, 0xf0, 0xff, 0xf, 0xf0, 0xff, 0xf, 0xf3,
    0xcf, 0x3c, 0xff, 0xf, 0xf0,

    /* U+45 "E" */
    0xff, 0xff, 0xff, 0xf0, 0xf, 0x0, 0xf0, 0xf,
    0x0, 0xff, 0xcf, 0xfc, 0xf0, 0xf, 0x0, 0xf0,
    0xf, 0x0, 0xff, 0xff, 0xff,

    /* U+46 "F" */
    0xff, 0xff, 0xff, 0xf0, 0xf, 0x0, 0xf0, 0xf,
    0x0, 0xff, 0xcf, 0xfc, 0xf0, 0xf, 0x0, 0xf0,
    0xf, 0x0, 0xf0, 0xf, 0x0,

    /* U+47 "G" */
    0x3f, 0xc3, 0xfc, 0xf0, 0xff, 0xf, 0xf0, 0xf,
    0x0, 0xf3, 0xff, 0x3f, 0xf0, 0xff, 0xf, 0xf0,
    0xff, 0xf, 0x3f, 0xf3, 0xff,

    /* U+48 "H" */
    0xf0, 0xff, 0xf, 0xf0, 0xff, 0xf, 0xf0, 0xff,
    0xf, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xf, 0xf0,
    0xff, 0xf, 0xf0, 0xff, 0xf,

    /* U+49 "I" */
    0xff, 0xff, 0xff, 0xf, 0x0, 0xf0, 0xf, 0x0,
    0xf0, 0xf, 0x0, 0xf0, 0xf, 0x0, 0xf0, 0xf,
    0x0, 0xf0, 0xff, 0xff, 0xff,

    /* U+4A "J" */
    0x0, 0xf0, 0xf, 0x0, 0xf0, 0xf, 0x0, 0xf0,
    0xf, 0x0, 0xf0, 0xf, 0x0, 0xf0, 0xf, 0xf0,
    0xff, 0xf, 0x3f, 0xc3, 0xfc,

    /* U+4B "K" */
    0xf0, 0x3f, 0xc0, 0xff, 0xf, 0x3c, 0x3c, 0xf3,
    0xc3, 0xcf, 0xf, 0xf0, 0x3f, 0xc0, 0xf3, 0xc3,
    0xcf, 0xf, 0xf, 0x3c, 0x3c, 0xf0, 0x3f, 0xc0,
    0xf0,

    /* U+4C "L" */
    0xf0, 0xf, 0x0, 0xf0, 0xf, 0x0, 0xf0, 0xf,
    0x0, 0xf0, 0xf, 0x0, 0xf0, 0xf, 0x0, 0xf0,
    0xf, 0x0, 0xff, 0xff, 0xff,

    /* U+4D "M" */
    0xf0, 0x3f, 0xc0, 0xff, 0xcf, 0xff, 0x3f, 0xff,
    0xff, 0xff, 0xff, 0x33, 0xfc, 0xcf, 0xf0, 0x3f,
    0xc0, 0xff, 0x3, 0xfc, 0xf, 0xf0, 0x3f, 0xc0,
    0xf0,

    /* U+4E "N" */
    0xf0, 0x3f, 0xc0, 0xff, 0xc3, 0xff, 0xf, 0xff,
    0x3f, 0xfc, 0xff, 0x3f, 0xfc, 0xff, 0xf0, 0xff,
    0xc3, 0xff, 0x3, 0xfc, 0xf, 0xf0, 0x3f, 0xc0,
    0xf0,

    /* U+4F "O" */
    0x3f, 0xc3, 0xfc, 0xf0, 0xff, 0xf, 0xf0, 0xff,
    0xf, 0xf0, 0xff, 0xf, 0xf0, 0xff, 0xf, 0xf0,
    0xff, 0xf, 0x3f, 0xc3, 0xfc,

    /* U+50 "P" */
    0xff, 0xcf, 0xfc, 0xf0, 0xff, 0xf, 0xf0, 0xff,
    0xf, 0xff, 0xcf, 0xfc, 0xf0, 0xf, 0x0, 0xf0,
    0xf, 0x0, 0xf0, 0xf, 0x0,

    /* U+51 "Q" */
    0x3f, 0xc3, 0xfc, 0xf0, 0xff, 0xf, 0xf0, 0xff,
    0xf, 0xf0, 0xff, 0xf, 0xf0, 0xff, 0xf, 0xf3,
    0xcf, 0x3c, 0x3c, 0xf3, 0xcf,

    /* U+52 "R" */
    0xff, 0xcf, 0xfc, 0xf0, 0xff, 0xf, 0xf0, 0xff,
    0xf, 0xff, 0xcf, 0xfc, 0xf3, 0xcf, 0x3c, 0xf0,
    0xff, 0xf, 0xf0, 0xff, 0xf,

    /* U+53 "S" */
    0x3f, 0xc3, 0xfc, 0xf0, 0xff, 0xf, 0xf0, 0xf,
    0x0, 0x3f, 0xc3, 0xfc, 0x0, 0xf0, 0xf, 0xf0,
    0xff, 0xf, 0x3f, 0xc3, 0xfc,

    /* U+54 "T" */
    0xff, 0xff, 0xff, 0xf, 0x0, 0xf0, 0xf, 0x0,
    0xf0, 0xf, 0x0, 0xf0, 0xf, 0x0, 0xf0, 0xf,
    0x0, 0xf0, 0xf, 0x0, 0xf0,

    /* U+55 "U" */
    0xf0, 0xff, 0xf, 0xf0, 0xff, 0xf, 0xf0, 0xff,
    0xf, 0xf0, 0xff, 0xf, 0xf0, 0xff, 0xf, 0xf0,
    0xff, 0xf, 0x3f, 0xc3, 0xfc,

    /* U+56 "V" */
    0xf0, 0xff, 0xf, 0xf0, 0xff, 0xf, 0xf0, 0xff,
    0xf, 0xf0, 0xff, 0xf, 0xf0, 0xff, 0xf, 0x3f,
    0xc3, 0xfc, 0xf, 0x0, 0xf0,

    /* U+57 "W" */
    0xf0, 0x3f, 0xc0, 0xff, 0x3, 0xfc, 0xf, 0xf0,
    0x3f, 0xc0, 0xff, 0x33, 0xfc, 0xcf, 0xff, 0xff,
    0xff, 0xff, 0xcf, 0xff, 0x3f, 0xf0, 0x3f, 0xc0,
    0xf0,

    /* U+58 "X" */
    0xf0, 0xf, 0xf0, 0xf, 0x3c, 0x3c, 0x3c, 0x3c,
    0xf, 0xf0, 0xf, 0xf0, 0x3, 0xc0, 0x3, 0xc0,
    0xf, 0xf0, 0xf, 0xf0, 0x3c, 0x3c, 0x3c, 0x3c,
    0xf0, 0xf, 0xf0, 0xf,

    /* U+59 "Y" */
    0xf0, 0xf, 0xf0, 0xf, 0x3c, 0x3c, 0x3c, 0x3c,
    0xf, 0xf0, 0xf, 0xf0, 0x3, 0xc0, 0x3, 0xc0,
    0x3, 0xc0, 0x3, 0xc0, 0x3, 0xc0, 0x3, 0xc0,
    0x3, 0xc0, 0x3, 0xc0,

    /* U+5A "Z" */
    0xff, 0xff, 0xff, 0x0, 0xf0, 0xf, 0x3, 0xc0,
    0x3c, 0xf, 0x0, 0xf0, 0x3c, 0x3, 0xc0, 0xf0,
    0xf, 0x0, 0xff, 0xff, 0xff,

    /* U+5B "[" */
    0xff, 0xff, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
    0xf0, 0xf0, 0xf0, 0xf0, 0xff, 0xff,

    /* U+5C "\\" */
    0xf0, 0x0, 0xf0, 0x0, 0x3c, 0x0, 0x3c, 0x0,
    0xf, 0x0, 0xf, 0x0, 0x3, 0xc0, 0x3, 0xc0,
    0x0, 0xf0, 0x0, 0xf0, 0x0, 0x3c, 0x0, 0x3c,
    0x0, 0xf, 0x0, 0xf,

    /* U+5D "]" */
    0xff, 0xff, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf,
    0xf, 0xf, 0xf, 0xf, 0xff, 0xff,

    /* U+5E "^" */
    0x3, 0x0, 0xc, 0x0, 0xfc, 0x3, 0xf0, 0x3c,
    0xf0, 0xf3, 0xcf, 0x3, 0xfc, 0xf,

    /* U+5F "_" */
    0xff, 0xff, 0xff, 0xff,

    /* U+60 "`" */
    0xf0, 0xf0, 0x3c, 0x3c, 0xf, 0xf,

    /* U+61 "a" */
    0x3f, 0xc3, 0xfc, 0x0, 0xf0, 0xf, 0x3f, 0xf3,
    0xff, 0xf0, 0xff, 0xf, 0x3f, 0xf3, 0xff,

    /* U+62 "b" */
    0xf0, 0xf, 0x0, 0xf0, 0xf, 0x0, 0xff, 0xcf,
    0xfc, 0xf0, 0xff, 0xf, 0xf0, 0xff, 0xf, 0xf0,
    0xff, 0xf, 0xff, 0xcf, 0xfc,

    /* U+63 "c" */
    0x3f, 0xcf, 0xff, 0x3, 0xc0, 0xf0, 0x3c, 0xf,
    0x3, 0xc0, 0x3f, 0xcf, 0xf0,

    /* U+64 "d" */
    0x0, 0xf0, 0xf, 0x0, 0xf0, 0xf, 0x3f, 0xf3,
    0xff, 0xf0, 0xff, 0xf, 0xf0, 0xff, 0xf, 0xf0,
    0xff, 0xf, 0x3f, 0xf3, 0xff,

    /* U+65 "e" */
    0x3f, 0xc3, 0xfc, 0xf0, 0xff, 0xf, 0xff, 0xff,
    0xff, 0xf0, 0xf, 0x0, 0x3f, 0xc3, 0xfc,

    /* U+66 "f" */
    0xf, 0xc3, 0xf3, 0xc0, 0xf0, 0xff, 0xff, 0xf3,
    0xc0, 0xf0, 0x3c, 0xf, 0x3, 0xc0, 0xf0, 0x3c,
    0xf, 0x0,

    /* U+67 "g" */
    0x3f, 0xf3, 0xff, 0xf0, 0xff, 0xf, 0xf0, 0xff,
    0xf, 0x3f, 0xf3, 0xff, 0x0, 0xf0, 0xf, 0xff,
    0xcf, 0xfc,

    /* U+68 "h" */
    0xf0, 0xf, 0x0, 0xf0, 0xf, 0x0, 0xff, 0xcf,
    0xfc, 0xf0, 0xff, 0xf, 0xf0, 0xff, 0xf, 0xf0,
    0xff, 0xf, 0xf0, 0xff, 0xf,

    /* U+69 "i" */
    0x3c, 0xf, 0x0, 0x0, 0x0, 0xfc, 0x3f, 0x3,
    0xc0, 0xf0, 0x3c, 0xf, 0x3, 0xc0, 0xf0, 0x3f,
    0xcf, 0xf0,

    /* U+6A "j" */
    0x3, 0xc0, 0xf0, 0x0, 0x0, 0x3, 0xc0, 0xf0,
    0x3c, 0xf, 0x3, 0xc0, 0xf0, 0x3c, 0xf, 0x3,
    0xc0, 0xff, 0xf3, 0xfc,

    /* U+6B "k" */
    0xf0, 0xf, 0x0, 0xf0, 0xf, 0x0, 0xf0, 0xff,
    0xf, 0xf3, 0xcf, 0x3c, 0xff, 0xf, 0xf0, 0xf3,
    0xcf, 0x3c, 0xf0, 0xff, 0xf,

    /* U+6C "l" */
    0xfc, 0x3f, 0x3, 0xc0, 0xf0, 0x3c, 0xf, 0x3,
    0xc0, 0xf0, 0x3c, 0xf, 0x3, 0xc0, 0xf0, 0x3f,
    0xcf, 0xf0,

    /* U+6D "m" */
    0xf0, 0xf3, 0xc3, 0xcf, 0xff, 0xff, 0xff, 0xf3,
    0x3f, 0xcc, 0xff, 0x33, 0xfc, 0xcf, 0xf0, 0x3f,
    0xc0, 0xf0,

    /* U+6E "n" */
    0xff, 0xcf, 0xfc, 0xf0, 0xff, 0xf, 0xf0, 0xff,
    0xf, 0xf0, 0xff, 0xf, 0xf0, 0xff, 0xf,

    /* U+6F "o" */
    0x3f, 0xc3, 0xfc, 0xf0, 0xff, 0xf, 0xf0, 0xff,
    0xf, 0xf0, 0xff, 0xf, 0x3f, 0xc3, 0xfc,

    /* U+70 "p" */
    0xff, 0xcf, 0xfc, 0xf0, 0xff, 0xf, 0xf0, 0xff,
    0xf, 0xff, 0xcf, 0xfc, 0xf0, 0xf, 0x0, 0xf0,
    0xf, 0x0,

    /* U+71 "q" */
    0x3f, 0xf3, 0xff, 0xf0, 0xff, 0xf, 0xf0, 0xff,
    0xf, 0x3f, 0xf3, 0xff, 0x0, 0xf0, 0xf, 0x0,
    0xf0, 0xf,

    /* U+72 "r" */
    0xff, 0xcf, 0xfc, 0xf0, 0xff, 0xf, 0xf0, 0xf,
    0x0, 0xf0, 0xf, 0x0, 0xf0, 0xf, 0x0,

    /* U+73 "s" */
    0x3f, 0xf3, 0xff, 0xf0, 0xf, 0x0, 0x3f, 0xc3,
    0xfc, 0x0, 0xf0, 0xf, 0xff, 0xcf, 0xfc,

    /* U+74 "t" */
    0x3c, 0x3, 0xc0, 0x3c, 0x3, 0xc0, 0xff, 0xff,
    0xff, 0x3c, 0x3, 0xc0, 0x3c, 0x3, 0xc0, 0x3c,
    0x3, 0xc0, 0xf, 0xf0, 0xff,

    /* U+75 "u" */
    0xf0, 0xff, 0xf, 0xf0, 0xff, 0xf, 0xf0, 0xff,
    0xf, 0xf0, 0xff, 0xf, 0x3f, 0xf3, 0xff,

    /* U+76 "v" */
    0xf0, 0xff, 0xf, 0xf0, 0xff, 0xf, 0xf0, 0xff,
    0xf, 0x3f, 0xc3, 0xfc, 0xf, 0x0, 0xf0,

    /* U+77 "w" */
    0xf0, 0x3f, 0xc0, 0xff, 0x3, 0xfc, 0xf, 0xf3,
    0x3f, 0xcc, 0xf3, 0xff, 0xf, 0xfc, 0x3c, 0xf0,
    0xf3, 0xc0,

    /* U+78 "x" */
    0xf0, 0x3f, 0xc0, 0xf3, 0xcf, 0xf, 0x3c, 0xf,
    0xc0, 0x3f, 0x3, 0xcf, 0xf, 0x3c, 0xf0, 0x3f,
    0xc0, 0xf0,

    /* U+79 "y" */
    0xf0, 0xff, 0xf, 0xf0, 0xff, 0xf, 0xf0, 0xff,
    0xf, 0x3f, 0xf3, 0xff, 0x0, 0xf0, 0xf, 0x3f,
    0xc3, 0xfc,

    /* U+7A "z" */
    0xff, 0xff, 0xff, 0x3, 0xc0, 0x3c, 0xf, 0x0,
    0xf0, 0x3c, 0x3, 0xc0, 0xff, 0xff, 0xff,

    /* U+7B "{" */
    0x3, 0xf0, 0x3f, 0xf, 0x0, 0xf0, 0xf, 0x0,
    0xf0, 0xfc, 0xf, 0xc0, 0xf, 0x0, 0xf0, 0xf,
    0x0, 0xf0, 0x3, 0xf0, 0x3f,

    /* U+7C "|" */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,

    /* U+7D "}" */
    0xfc, 0xf, 0xc0, 0xf, 0x0, 0xf0, 0xf, 0x0,
    0xf0, 0x3, 0xf0, 0x3f, 0xf, 0x0, 0xf0, 0xf,
    0x0, 0xf0, 0xfc, 0xf, 0xc0,

    /* U+7E "~" */
    0x3f, 0x3c, 0xfc, 0xff, 0x3f, 0x3c, 0xfc,

    /* U+7F "" */
    0xf0, 0x3, 0xc0, 0xc, 0xc0, 0x33, 0x0, 0xcc,
    0xff, 0x33, 0xfc, 0xc3, 0x33, 0xc, 0xf0, 0x33,
    0xc0, 0xc0, 0x3, 0x0, 0xc, 0x0, 0x30, 0x0,
    0xc0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 256, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 16},
    {.bitmap_index = 1, .adv_w = 256, .box_w = 4, .box_h = 14, .ofs_x = 6, .ofs_y = 2},
    {.bitmap_index = 8, .adv_w = 256, .box_w = 12, .box_h = 6, .ofs_x = 2, .ofs_y = 10},
    {.bitmap_index = 17, .adv_w = 256, .box_w = 14, .box_h = 14, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 42, .adv_w = 256, .box_w = 12, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 63, .adv_w = 256, .box_w = 14, .box_h = 12, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 84, .adv_w = 256, .box_w = 14, .box_h = 14, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 109, .adv_w = 256, .box_w = 6, .box_h = 6, .ofs_x = 4, .ofs_y = 10},
    {.bitmap_index = 114, .adv_w = 256, .box_w = 8, .box_h = 14, .ofs_x = 4, .ofs_y = 2},
    {.bitmap_index = 128, .adv_w = 256, .box_w = 8, .box_h = 14, .ofs_x = 4, .ofs_y = 2},
    {.bitmap_index = 142, .adv_w = 256, .box_w = 16, .box_h = 10, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 162, .adv_w = 256, .box_w = 12, .box_h = 10, .ofs_x = 2, .ofs_y = 4},
    {.bitmap_index = 177, .adv_w = 256, .box_w = 6, .box_h = 6, .ofs_x = 4, .ofs_y = 0},
    {.bitmap_index = 182, .adv_w = 256, .box_w = 12, .box_h = 2, .ofs_x = 2, .ofs_y = 8},
    {.bitmap_index = 185, .adv_w = 256, .box_w = 4, .box_h = 4, .ofs_x = 6, .ofs_y = 2},
    {.bitmap_index = 187, .adv_w = 256, .box_w = 16, .box_h = 14, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 215, .adv_w = 256, .box_w = 12, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 236, .adv_w = 256, .box_w = 12, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 257, .adv_w = 256, .box_w = 12, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 278, .adv_w = 256, .box_w = 12, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 299, .adv_w = 256, .box_w = 14, .box_h = 14, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 324, .adv_w = 256, .box_w = 12, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 345, .adv_w = 256, .box_w = 12, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 366, .adv_w = 256, .box_w = 12, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 387, .adv_w = 256, .box_w = 12, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 408, .adv_w = 256, .box_w = 12, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 429, .adv_w = 256, .box_w = 4, .box_h = 12, .ofs_x = 6, .ofs_y = 2},
    {.bitmap_index = 435, .adv_w = 256, .box_w = 6, .box_h = 14, .ofs_x = 4, .ofs_y = 0},
    {.bitmap_index = 446, .adv_w = 256, .box_w = 10, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 464, .adv_w = 256, .box_w = 12, .box_h = 6, .ofs_x = 2, .ofs_y = 6},
    {.bitmap_index = 473, .adv_w = 256, .box_w = 10, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 491, .adv_w = 256, .box_w = 12, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 512, .adv_w = 256, .box_w = 14, .box_h = 14, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 537, .adv_w = 256, .box_w = 12, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 558, .adv_w = 256, .box_w = 12, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 579, .adv_w = 256, .box_w = 12, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 600, .adv_w = 256, .box_w = 12, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 621, .adv_w = 256, .box_w = 12, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 642, .adv_w = 256, .box_w = 12, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 663, .adv_w = 256, .box_w = 12, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 684, .adv_w = 256, .box_w = 12, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 705, .adv_w = 256, .box_w = 12, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 726, .adv_w = 256, .box_w = 12, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 747, .adv_w = 256, .box_w = 14, .box_h = 14, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 772, .adv_w = 256, .box_w = 12, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 793, .adv_w = 256, .box_w = 14, .box_h = 14, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 818, .adv_w = 256, .box_w = 14, .box_h = 14, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 843, .adv_w = 256, .box_w = 12, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 864, .adv_w = 256, .box_w = 12, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 885, .adv_w = 256, .box_w = 12, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 906, .adv_w = 256, .box_w = 12, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 927, .adv_w = 256, .box_w = 12, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 948, .adv_w = 256, .box_w = 12, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 969, .adv_w = 256, .box_w = 12, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 990, .adv_w = 256, .box_w = 12, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 1011, .adv_w = 256, .box_w = 14, .box_h = 14, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 1036, .adv_w = 256, .box_w = 16, .box_h = 14, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 1064, .adv_w = 256, .box_w = 16, .box_h = 14, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 1092, .adv_w = 256, .box_w = 12, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 1113, .adv_w = 256, .box_w = 8, .box_h = 14, .ofs_x = 4, .ofs_y = 2},
    {.bitmap_index = 1127, .adv_w = 256, .box_w = 16, .box_h = 14, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 1155, .adv_w = 256, .box_w = 8, .box_h = 14, .ofs_x = 4, .ofs_y = 2},
    {.bitmap_index = 1169, .adv_w = 256, .box_w = 14, .box_h = 8, .ofs_x = 0, .ofs_y = 8},
    {.bitmap_index = 1183, .adv_w = 256, .box_w = 16, .box_h = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1187, .adv_w = 256, .box_w = 8, .box_h = 6, .ofs_x = 6, .ofs_y = 10},
    {.bitmap_index = 1193, .adv_w = 256, .box_w = 12, .box_h = 10, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 1208, .adv_w = 256, .box_w = 12, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 1229, .adv_w = 256, .box_w = 10, .box_h = 10, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 1242, .adv_w = 256, .box_w = 12, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 1263, .adv_w = 256, .box_w = 12, .box_h = 10, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 1278, .adv_w = 256, .box_w = 10, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 1296, .adv_w = 256, .box_w = 12, .box_h = 12, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 1314, .adv_w = 256, .box_w = 12, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 1335, .adv_w = 256, .box_w = 10, .box_h = 14, .ofs_x = 4, .ofs_y = 2},
    {.bitmap_index = 1353, .adv_w = 256, .box_w = 10, .box_h = 16, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 1373, .adv_w = 256, .box_w = 12, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 1394, .adv_w = 256, .box_w = 10, .box_h = 14, .ofs_x = 4, .ofs_y = 2},
    {.bitmap_index = 1412, .adv_w = 256, .box_w = 14, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 1430, .adv_w = 256, .box_w = 12, .box_h = 10, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 1445, .adv_w = 256, .box_w = 12, .box_h = 10, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 1460, .adv_w = 256, .box_w = 12, .box_h = 12, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 1478, .adv_w = 256, .box_w = 12, .box_h = 12, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 1496, .adv_w = 256, .box_w = 12, .box_h = 10, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 1511, .adv_w = 256, .box_w = 12, .box_h = 10, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 1526, .adv_w = 256, .box_w = 12, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 1547, .adv_w = 256, .box_w = 12, .box_h = 10, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 1562, .adv_w = 256, .box_w = 12, .box_h = 10, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 1577, .adv_w = 256, .box_w = 14, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 1595, .adv_w = 256, .box_w = 14, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 1613, .adv_w = 256, .box_w = 12, .box_h = 12, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 1631, .adv_w = 256, .box_w = 12, .box_h = 10, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 1646, .adv_w = 256, .box_w = 12, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 1667, .adv_w = 256, .box_w = 4, .box_h = 14, .ofs_x = 6, .ofs_y = 2},
    {.bitmap_index = 1674, .adv_w = 256, .box_w = 12, .box_h = 14, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 1695, .adv_w = 256, .box_w = 14, .box_h = 4, .ofs_x = 0, .ofs_y = 12},
    {.bitmap_index = 1702, .adv_w = 256, .box_w = 14, .box_h = 14, .ofs_x = 0, .ofs_y = 2}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/



/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 96, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

/*Store all the custom data of the font*/
static lv_font_fmt_txt_dsc_t font_dsc = {
    .glyph_bitmap = gylph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 1,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0
};


/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
lv_font_t lv_font_unscii_16 = {
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 17,          /*The maximum line height required by the font*/
    .base_line = 0,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0)
    .underline_position = 0,
    .underline_thickness = 0,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};



#endif /*#if LV_FONT_UNSCII_16*/
