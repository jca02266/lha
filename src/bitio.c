/* ------------------------------------------------------------------------ */
/* LHa for UNIX                                                             */
/*              bitio.c -- bit stream                                       */
/*                                                                          */
/*      Modified                Nobutaka Watazaki                           */
/*                                                                          */
/*  Ver. 1.14   Source All chagned              1995.01.14  N.Watazaki      */
/*              Separated from crcio.c          2002.10.26  Koji Arai       */
/* ------------------------------------------------------------------------ */
#include "lha.h"

static unsigned char subbitbuf, bitcount;
static unsigned int buffered_bits;
static int getbits_initialized;

void
fillbuf(unsigned char n)  /* Shift bitbuf n bits left, read n bits */
{
    unsigned int old_buffered_bits = buffered_bits;
    unsigned int bytes_read = 0;

    if (getbits_initialized) {
        unsigned int needed_bits = n > buffered_bits ? n - buffered_bits : 0;
        unsigned int needed_bytes =
            (needed_bits + CHAR_BIT - 1) / CHAR_BIT;

        if (needed_bytes > compsize)
            fatal_error("Unexpected end of compressed data");
    }

    while (n > bitcount) {
        n -= bitcount;
        bitbuf = (bitbuf << bitcount) + (subbitbuf >> (CHAR_BIT - bitcount));
        if (compsize != 0) {
            int c = getc(infile);
            compsize--;
            if (c == EOF) {
                fatal_error("cannot read stream");
            }
            subbitbuf = (unsigned char)c;
            bytes_read++;
        }
        else
            subbitbuf = 0;
        bitcount = CHAR_BIT;
    }
    bitcount -= n;
    bitbuf = (bitbuf << n) + (subbitbuf >> (CHAR_BIT - n));
    subbitbuf <<= n;

    if (getbits_initialized)
        buffered_bits = old_buffered_bits + bytes_read * CHAR_BIT - n;
    else
        buffered_bits = bytes_read * CHAR_BIT;
}

unsigned short
getbits(unsigned char n)
{
    unsigned short  x;

    x = bitbuf >> (2 * CHAR_BIT - n);
    fillbuf(n);
    return x;
}

void
putcode(unsigned char n, unsigned short x)  /* Write leftmost n bits of x */
{
    while (n >= bitcount) {
        n -= bitcount;
        subbitbuf += x >> (USHRT_BIT - bitcount);
        x <<= bitcount;
        if (compsize < origsize) {
            if (fwrite(&subbitbuf, 1, 1, outfile) == 0) {
                fatal_error("Write error in bitio.c(putcode)");
            }
            compsize++;
        }
        else
            unpackable = 1;
        subbitbuf = 0;
        bitcount = CHAR_BIT;
    }
    subbitbuf += x >> (USHRT_BIT - bitcount);
    bitcount -= n;
}

void
putbits(unsigned char n, unsigned short x)  /* Write rightmost n bits of x */
{
    x <<= USHRT_BIT - n;
    putcode(n, x);
}

void
init_getbits( /* void */ )
{
    bitbuf = 0;
    subbitbuf = 0;
    bitcount = 0;
    buffered_bits = 0;
    getbits_initialized = 0;
    fillbuf(2 * CHAR_BIT);
    getbits_initialized = 1;
}

void
init_putbits( /* void */ )
{
    getbits_initialized = 0;
    bitcount = CHAR_BIT;
    subbitbuf = 0;
}
