/***********************************************************
	pm2hist.c -- history for pmext2 decoding
***********************************************************/

/* Circular double-linked list. */

static unsigned char prev[0x100];
static unsigned char next[0x100];
static unsigned char lastbyte;

void
hist_init()
{
    int i;
    for (i = 0; i < 0x100; i++) {
        prev[(0xFF + i) & 0xFF] = i;
        next[(0x01 + i) & 0xFF] = i;
    }
    prev[0x7F] = 0x00; next[0x00] = 0x7F;
    prev[0xDF] = 0x80; next[0x80] = 0xDF;
    prev[0x9F] = 0xE0; next[0xE0] = 0x9F;
    prev[0x1F] = 0xA0; next[0xA0] = 0x1F;
    prev[0xFF] = 0x20; next[0x20] = 0xFF;
    lastbyte = 0x20;
}

unsigned char
hist_lookup(int n)
{
    int i;
    unsigned char *direction = prev;
    if (n >= 0x80) {
        /* Speedup: If you have to process more than half the ring,
           it's faster to walk the other way around.
         */
        direction = next;
        n = 0x100 - n;
    }
    for (i = lastbyte; n != 0; n--)
        i = direction[i];

    return i;
}

void
hist_update(unsigned char data)
{
    unsigned char oldNext, oldPrev, newNext;

    if (data == lastbyte)
        return;

    /* detach from old position */
    oldNext = next[data];
    oldPrev = prev[data];
    prev[oldNext] = oldPrev;
    next[oldPrev] = oldNext;

    /* attach to new next */
    newNext = next[lastbyte];
    prev[newNext] = data;
    next[data] = newNext;

    /* attach to new prev */
    prev[data] = lastbyte;
    next[lastbyte] = data;

    lastbyte = data;
}
