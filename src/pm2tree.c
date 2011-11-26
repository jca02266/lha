/***********************************************************
	pm2tree.c -- tree for pmext2 decoding
***********************************************************/
/*
  Copyright (c) 1999 Maarten ter Huurne

  Permission is hereby granted, free of charge, to any person
  obtaining a copy of this software and associated documentation files
  (the "Software"), to deal in the Software without restriction,
  including without limitation the rights to use, copy, modify, merge,
  publish, distribute, sublicense, and/or sell copies of the Software,
  and to permit persons to whom the Software is furnished to do so,
  subject to the following conditions:

  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/
#include "lha.h"

struct tree {
    unsigned char root, *leftarr, *rightarr;
};

static unsigned char tree1left[32];
static unsigned char tree1right[32];
static struct tree tree1 = { 0, tree1left, tree1right };

static unsigned char tree2left[8];
static unsigned char tree2right[8];
static struct tree tree2 = { 0, tree2left, tree2right };

static void tree_setsingle(struct tree *t, unsigned char value);
static void tree_rebuild(struct tree *t, unsigned char bound,
                  unsigned char mindepth, unsigned char maxdepth,
                  unsigned char *table);
static void tree_setsingle(struct tree *t, unsigned char value);

static unsigned char tree1bound;
static unsigned char mindepth;

void
maketree1()
{
    int i, nbits, x;
    unsigned char table1[32];

    tree1bound = getbits(5);
    mindepth = getbits(3);
    if (mindepth == 0) {
        tree_setsingle(&tree1, tree1bound - 1);
    }
    else {
        for (i = 0; i < 32; i++)
            table1[i] = 0;
        nbits = getbits(3);
        for (i = 0; i < tree1bound; i++) {
            x = getbits(nbits);
            table1[i] = (x == 0 ? 0 : x - 1 + mindepth);
        }
        tree_rebuild(&tree1, tree1bound, mindepth, 31, table1);
    }
}

void
maketree2(int tree2bound) /* in use: 5 <= tree2bound <= 8 */
{
    int i, count, index;
    unsigned char table2[8];


    if (tree1bound < 10)
        /* tree1bound=1..8: character only, offset value is no needed. */
        /* tree1bound=9: offset value is not encoded by Huffman tree */
        return;

    if (tree1bound == 29 && mindepth == 0)
        /* the length value is just 256 and offset value is just 0 */
        return;

    /* need to build tree2 for offset value */

    for (i = 0; i < 8; i++)
        table2[i] = 0;
    for (i = 0; i < tree2bound; i++)
        table2[i] = getbits(3);

    index = 0;
    count = 0;
    for (i = 0; i < tree2bound; i++) {
        if (table2[i] != 0) {
            index = i;
            count++;
        }
    }

    if (count == 1) {
        tree_setsingle(&tree2, index);
    }
    else if (count > 1) {
        tree_rebuild(&tree2, tree2bound, 1, 7, table2);
    }
    // Note: count == 0 is possible!
    //       Excluding that possibility was a bug in version 1.

}

static int
tree_get(struct tree *t)
{
    int i;
    i = t->root;
    while (i < 0x80) {
        i = (getbits(1) == 0 ? t->leftarr[i] : t->rightarr[i]);
    }
    return i & 0x7F;
}

int
tree1_get()
{
    return tree_get(&tree1);
}

int
tree2_get()
{
    return tree_get(&tree2);
}

static void
tree_setsingle(struct tree *t, unsigned char value)
{
    t->root = 128 | value;
}

static void
tree_rebuild(struct tree *t,
             unsigned char bound,
             unsigned char mindepth,
             unsigned char maxdepth,
             unsigned char *table)
{
    unsigned char parentarr[32], d;
    int i, curr, empty, n;

    /* validate table */
    {
        unsigned int count[32];
        double total;

        memset(count, 0, sizeof(count));
        for (i = 0; i < bound; i++) {
            if (table[i] > maxdepth) {
                error("Bad table");
                exit(1);
            }
            count[table[i]]++;
        }
        total = 0.0;
        for (i = mindepth; i <= maxdepth; i++) {
            int max_leaves = (1<<i);
            if (count[i] > max_leaves) {
                error("Bad table");
                exit(1);
            }
            total += 1.0/max_leaves * count[i];
        }
        if (total != 1.0) {
            /* check the Kraft's inequality */
            error("Bad table");
            exit(1);
        }
    }

    /* initialize tree */
    t->root = 0;
    for (i = 0; i < bound; i++) {
        t->leftarr[i] = 0;
        t->rightarr[i] = 0;
        parentarr[i] = 0;
    }

    /* build tree */
    for (i = 0; i < mindepth - 1; i++) {
        t->leftarr[i] = i + 1;
        parentarr[i + 1] = i;
    }

    curr = mindepth - 1;
    empty = mindepth;
    for (d = mindepth; d <= maxdepth; d++) {
        for (i = 0; i < bound; i++) {
            if (table[i] != d)
                continue;

            if (t->leftarr[curr] == 0) {
                t->leftarr[curr] = i | 128;
                continue;
            }

            t->rightarr[curr] = i | 128;
            n = 0;
            while (t->rightarr[curr] != 0) {
                if (curr == 0) {        /* root? -> done */
                    return;
                }
                curr = parentarr[curr];
                n++;
            }

            t->rightarr[curr] = empty;
            for (;;) {
                parentarr[empty] = curr;
                curr = empty;
                empty++;

                n--;
                if (n == 0)
                    break;
                t->leftarr[curr] = empty;
            }
        }

        if (t->leftarr[curr] == 0)
            t->leftarr[curr] = empty;
        else
            t->rightarr[curr] = empty;

        parentarr[empty] = curr;
        curr = empty;
        empty++;
    }

    /* unreachable */
    error("bad table");
    exit(1);
}
