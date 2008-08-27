/***********************************************************
	pm2.c -- extract pmext2 coding
***********************************************************/
#include "lha.h"
#include "pm2hist.h"
#include "pm2tree.h"

static off_t nextcount;
static unsigned long lastupdate;

/* repeated from slide.c */
static unsigned int dicsiz1;
#define offset (0x100 - 2)

void
decode_start_pm2(void)
{
    dicsiz1 = (1 << dicbit) - 1;
    init_getbits();
    hist_init();
    nextcount = 0;
    lastupdate = 0;
    getbits(1);                 /* discard bit */
}


static unsigned char gettree1;

static int historyBits[8] = {   3,   3,   4,   5,   5,   5,   6,   6 };
static int historyBase[8] = {   0,   8,  16,  32,  64,  96, 128, 192 };
static int repeatBits[6]  = {   3,   3,   5,   6,   7,   0 };
static int repeatBase[6]  = {  17,  25,  33,  65, 129, 256 };

unsigned short
decode_c_pm2(void)
{
    /* various admin: */
    while (lastupdate != loc) {
        hist_update(dtext[lastupdate]);
        lastupdate = (lastupdate + 1) & dicsiz1;
    }

    while (decode_count >= nextcount) {
        /* Actually it will never loop, because decode_count doesn't grow that fast.
           However, this is the way LHA does it.
           Probably other encoding methods can have repeats larger than 256 bytes.
           Note: LHA puts this code in decode_p...
        */

        switch (nextcount) {
        case 0x0000:
            maketree1();
            maketree2(5);
            nextcount = 0x0400;
            break;
        case 0x0400:
            maketree2(6);
            nextcount = 0x0800;
            break;
        case 0x0800:
            maketree2(7);
            nextcount = 0x1000;
            break;
        case 0x1000:
            if (getbits(1) != 0)
                maketree1();
            maketree2(8);
            nextcount = 0x2000;
            break;
        default:                /* 0x2000, 0x3000, 0x4000, ... */
            if (getbits(1) != 0) {
                maketree1();
                maketree2(8);
            }
            nextcount += 0x1000;
            break;
        }
    }
    gettree1 = tree_get(&tree1);        /* value preserved for decode_p */

    /* direct value (ret <= UCHAR_MAX) */
    if (gettree1 < 8)
        return hist_lookup(historyBase[gettree1] +
                           getbits(historyBits[gettree1]));
    /* repeats: (ret > UCHAR_MAX) */
    if (gettree1 < 23)
        return offset + 2 + (gettree1 - 8);

    return offset + repeatBase[gettree1 - 23]
        + getbits(repeatBits[gettree1 - 23]);
}

unsigned short
decode_p_pm2(void)
{
    /* gettree1 value preserved from decode_c */
    int nbits, delta, gettree2;
    if (gettree1 == 8) {        /* 2-byte repeat with offset 0..63 */
        nbits = 6;
        delta = 0;
    }
    else if (gettree1 < 28) {   /* n-byte repeat with offset 0..8191 */
        gettree2 = tree_get(&tree2);
        if (gettree2 == 0) {
            nbits = 6;
            delta = 0;
        }
        else {                  /* 1..7 */
            nbits = 5 + gettree2;
            delta = 1 << nbits;
        }
    }
    else {                      /* 256 bytes repeat with offset 0 */
        nbits = 0;
        delta = 0;
    }

    return delta + getbits(nbits);
}
