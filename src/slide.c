/* ------------------------------------------------------------------------ */
/* LHa for UNIX                                                             */
/*              slide.c -- sliding dictionary with percolating update       */
/*                                                                          */
/*      Modified                Nobutaka Watazaki                           */
/*                                                                          */
/*  Ver. 1.14d  Exchanging a search algorithm  1997.01.11    T.Okamoto      */
/* ------------------------------------------------------------------------ */

#if 0
#define DEBUG 1
#endif

#include "lha.h"

#ifdef DEBUG
FILE *fout = NULL;
static int noslide = 1;
#endif

static unsigned long encoded_origsize;

/* variables for hash */
struct hash {
    unsigned int pos;
    int too_flag;               /* if 1, matching candidate is too many */
} *hash;
static unsigned int *prev;      /* previous posiion associated with hash */

/* hash function: it represents 3 letters from `pos' on `text' */
#define INIT_HASH(pos) \
        ((( (text[(pos)] << 5) \
           ^ text[(pos) + 1]  ) << 5) \
           ^ text[(pos) + 2]         ) & (unsigned)(HSHSIZ - 1);
#define NEXT_HASH(hash,pos) \
        (((hash) << 5) \
           ^ text[(pos) + 2]         ) & (unsigned)(HSHSIZ - 1);

static struct encode_option encode_define[2] = {
#if defined(__STDC__) || defined(AIX)
    /* lh1 */
    {(void (*) ()) output_dyn,
     (void (*) ()) encode_start_fix,
     (void (*) ()) encode_end_dyn},
    /* lh4, 5, 6, 7 */
    {(void (*) ()) output_st1,
     (void (*) ()) encode_start_st1,
     (void (*) ()) encode_end_st1}
#else
    /* lh1 */
    {(int (*) ()) output_dyn,
     (int (*) ()) encode_start_fix,
     (int (*) ()) encode_end_dyn},
    /* lh4, 5, 6, 7 */
    {(int (*) ()) output_st1,
     (int (*) ()) encode_start_st1,
     (int (*) ()) encode_end_st1}
#endif
};

static struct decode_option decode_define[] = {
    /* lh1 */
    {decode_c_dyn, decode_p_st0, decode_start_fix},
    /* lh2 */
    {decode_c_dyn, decode_p_dyn, decode_start_dyn},
    /* lh3 */
    {decode_c_st0, decode_p_st0, decode_start_st0},
    /* lh4 */
    {decode_c_st1, decode_p_st1, decode_start_st1},
    /* lh5 */
    {decode_c_st1, decode_p_st1, decode_start_st1},
    /* lh6 */
    {decode_c_st1, decode_p_st1, decode_start_st1},
    /* lh7 */
    {decode_c_st1, decode_p_st1, decode_start_st1},
    /* lzs */
    {decode_c_lzs, decode_p_lzs, decode_start_lzs},
    /* lz5 */
    {decode_c_lz5, decode_p_lz5, decode_start_lz5}
};

static struct encode_option encode_set;
static struct decode_option decode_set;

#define TXTSIZ (MAX_DICSIZ * 2L + MAXMATCH)
#define HSHSIZ (((unsigned long)1) <<15)
#define NIL 0
#define LIMIT 0x100             /* limit of hash chain */

static unsigned int txtsiz;
static unsigned long dicsiz;
static unsigned int remainder;

static int matchlen;
static unsigned int matchpos;

int
encode_alloc(method)
    int method;
{
    switch (method) {
    case LZHUFF1_METHOD_NUM:
        encode_set = encode_define[0];
        maxmatch = 60;
        dicbit = LZHUFF1_DICBIT;    /* 12 bits  Changed N.Watazaki */
        break;
    case LZHUFF5_METHOD_NUM:
        encode_set = encode_define[1];
        maxmatch = MAXMATCH;
        dicbit = LZHUFF5_DICBIT;    /* 13 bits */
        break;
    case LZHUFF6_METHOD_NUM:
        encode_set = encode_define[1];
        maxmatch = MAXMATCH;
        dicbit = LZHUFF6_DICBIT;    /* 15 bits */
        break;
    case LZHUFF7_METHOD_NUM:
        encode_set = encode_define[1];
        maxmatch = MAXMATCH;
        dicbit = LZHUFF7_DICBIT;    /* 16 bits */
        break;
    default:
        error("unknown method %d", method);
        exit(1);
    }

    dicsiz = (((unsigned long)1) << dicbit);
    txtsiz = dicsiz*2+maxmatch;

    if (hash) return method;

    alloc_buf();

    hash = (struct hash*)xmalloc(HSHSIZ * sizeof(struct hash));
    prev = (unsigned int*)xmalloc(MAX_DICSIZ * sizeof(unsigned int));
    text = (unsigned char*)xmalloc(TXTSIZ);

    return method;
}

static void
init_slide()
{
    unsigned int i;

    for (i = 0; i < HSHSIZ; i++) {
        hash[i].pos = NIL;
        hash[i].too_flag = 0;
    }
}

/* update dictionary */
static void
update_dict(pos, crc)
    unsigned int *pos;
    unsigned int *crc;
{
    unsigned int i, j;
    long n;

    memmove(&text[0], &text[dicsiz], txtsiz - dicsiz);

    n = fread_crc(crc, &text[txtsiz - dicsiz], dicsiz, infile);

    remainder += n;
    encoded_origsize += n;      /* total size of read bytes */

    *pos -= dicsiz;
    for (i = 0; i < HSHSIZ; i++) {
        j = hash[i].pos;
        hash[i].pos = (j > dicsiz) ? j - dicsiz : NIL;
        hash[i].too_flag = 0;
    }
    for (i = 0; i < dicsiz; i++) {
        j = prev[i];
        prev[i] = (j > dicsiz) ? j - dicsiz : NIL;
    }
}

/* associate position with token */
static void
insert_hash(token, pos)
    unsigned int token;
    unsigned int pos;
{
    prev[pos & (dicsiz - 1)] = hash[token].pos; /* chain the previous pos. */
    hash[token].pos = pos;
}

/* search the most long token matched to current token */
static void
search_dict(token, pos, min)
    unsigned int token;         /* search token */
    unsigned int pos;           /* position of token */
    int min;                    /* min. length of matching string */
{
    unsigned int off, tok, max;

    if (min < THRESHOLD - 1) min = THRESHOLD - 1;

    max = maxmatch;
    matchpos = pos;
    matchlen = min;

    if (hash[token].pos == NIL)
        return;

    off = 0;
    for (tok = token; hash[tok].too_flag && off < maxmatch - THRESHOLD; ) {
        /* If matching position is too many, The search key is
           changed into following token from `off' (for speed). */
        ++off;
        tok = NEXT_HASH(tok, pos+off);
    }
    if (off == maxmatch - THRESHOLD) {
        off = 0;
        tok = token;
    }

    for (;;) {
        unsigned int chain = 0;
        unsigned int scan_off = hash[tok].pos;
        int scan_pos = scan_off - off;
        int scan_end = pos - dicsiz;
        unsigned int len;

        while (scan_pos > scan_end) {
            chain++;

            if (text[scan_pos + matchlen] == text[pos + matchlen]) {
                {
                    /* collate token */
                    unsigned char *a = &text[scan_pos];
                    unsigned char *b = &text[pos];

                    for (len = 0; len < max && *a++ == *b++; len++);
                }

                if (len > matchlen) {
                    matchpos = scan_pos;
                    matchlen = len;
                    if (matchlen == max)
                        break;

#ifdef DEBUG
                    if (noslide) {
                      if (matchpos < dicsiz) {
                        printf("matchpos=%u scan_pos=%u dicsiz=%u\n"
                               ,matchpos, scan_pos, dicsiz);
                      }
                    }
#endif
                }
            }
            scan_off = prev[scan_off & (dicsiz - 1)];
            scan_pos = scan_off - off;
        }

        if (chain >= LIMIT)
            hash[tok].too_flag = 1;

        if (off == 0 || matchlen > off + 2)
            break;
        max = off + 2;
        off = 0;
        tok = token;
    }

    if (matchlen > remainder) matchlen = remainder;
}

/* slide dictionary */
static void
next_token(token, pos, crc)
    unsigned int *token;
    unsigned int *pos;
    unsigned int *crc;
{
    remainder--;
    if (++*pos >= txtsiz - maxmatch) {
        update_dict(pos, crc);
#ifdef DEBUG
        noslide = 0;
#endif
    }
    *token = NEXT_HASH(*token, *pos);
}

unsigned int
encode(interface)
    struct interfacing *interface;
{
    int lastmatchlen;
    unsigned int lastmatchoffset;
    unsigned int token, pos, crc;

#ifdef DEBUG
    unsigned int addr;

    addr = 0;

    fout = fopen("en", "wt");
    if (fout == NULL) exit(1);
#endif
    infile = interface->infile;
    outfile = interface->outfile;
    origsize = interface->original;
    compsize = count = 0L;
    unpackable = 0;

    INITIALIZE_CRC(crc);

    init_slide();

    encode_set.encode_start();
    memset(&text[0], ' ', TXTSIZ);

    remainder = fread_crc(&crc, &text[dicsiz], txtsiz-dicsiz, infile);
    encoded_origsize = remainder;
    matchlen = THRESHOLD - 1;

    pos = dicsiz;

    if (matchlen > remainder) matchlen = remainder;

    token = INIT_HASH(pos);
    insert_hash(token, pos);     /* associate token and pos */

    while (remainder > 0 && ! unpackable) {
        lastmatchlen = matchlen;
        lastmatchoffset = pos - matchpos - 1;

        next_token(&token, &pos, &crc);
        search_dict(token, pos, lastmatchlen-1);
        insert_hash(token, pos);

        if (matchlen > lastmatchlen || lastmatchlen < THRESHOLD) {
            /* output a letter */
            encode_set.output(text[pos - 1], 0);
#ifdef DEBUG
            fprintf(fout, "%u C %02X\n", addr, text[pos-1]);
            addr++;
#endif
            count++;
        } else {
            /* output length and offset */
            encode_set.output(lastmatchlen + (UCHAR_MAX + 1 - THRESHOLD),
                              lastmatchoffset & (dicsiz-1) );
            --lastmatchlen;

#ifdef DEBUG
            fprintf(fout, "%u M %u %u ", addr,
                    lastmatchoffset & (dicsiz-1), lastmatchlen+1);
            addr += lastmatchlen +1 ;

            {
                int t,cc;
                for (t=0; t<lastmatchlen+1; t++) {
                    cc = text[pos - lastmatchoffset - 2 + t];
                    fprintf(fout, "%02X ", cc);
                }
                fprintf(fout, "\n");
            }
#endif
            while (--lastmatchlen > 0) {
                next_token(&token, &pos, &crc);
                insert_hash(token, pos);
                count++;
            }
            next_token(&token, &pos, &crc);
            search_dict(token, pos, THRESHOLD - 1);
            insert_hash(token, pos);
        }
    }
    encode_set.encode_end();

    interface->packed = compsize;
    interface->original = encoded_origsize;

    return crc;
}

unsigned int
decode(interface)
    struct interfacing *interface;
{
    unsigned int i, j, k, c;
    unsigned int dicsiz1, offset;
    unsigned char *dtext;
    unsigned int crc;

#ifdef DEBUG
    fout = fopen("de", "wt");
    if (fout == NULL) exit(1);
#endif

    infile = interface->infile;
    outfile = interface->outfile;
    dicbit = interface->dicbit;
    origsize = interface->original;
    compsize = interface->packed;
    decode_set = decode_define[interface->method - 1];

    INITIALIZE_CRC(crc);
    prev_char = -1;
    dicsiz = 1L << dicbit;
    dtext = (unsigned char *)xmalloc(dicsiz);
    for (i=0; i<dicsiz; i++) dtext[i] = 0x20;
    decode_set.decode_start();
    dicsiz1 = dicsiz - 1;
    offset = (interface->method == LARC_METHOD_NUM) ? 0x100 - 2 : 0x100 - 3;
    count = 0;
    loc = 0;
    while (count < origsize) {
        c = decode_set.decode_c();
        if (c <= UCHAR_MAX) {
#ifdef DEBUG
          fprintf(fout, "%u C %02X\n", count, c);
#endif
            dtext[loc++] = c;
            if (loc == dicsiz) {
                fwrite_crc(&crc, dtext, dicsiz, outfile);
                loc = 0;
            }
            count++;
        }
        else {
            j = c - offset;
            i = (loc - decode_set.decode_p() - 1) & dicsiz1;
#ifdef DEBUG
            fprintf(fout, "%u M %u %u ", count, (loc-1-i) & dicsiz1, j);
#endif
            count += j;
            for (k = 0; k < j; k++) {
                c = dtext[(i + k) & dicsiz1];

#ifdef DEBUG
                fprintf(fout, "%02X ", c & 0xff);
#endif
                dtext[loc++] = c;
                if (loc == dicsiz) {
                    fwrite_crc(&crc, dtext, dicsiz, outfile);
                    loc = 0;
                }
            }
#ifdef DEBUG
            fprintf(fout, "\n");
#endif
        }
    }
    if (loc != 0) {
        fwrite_crc(&crc, dtext, loc, outfile);
    }

    free(dtext);
    return crc;
}
