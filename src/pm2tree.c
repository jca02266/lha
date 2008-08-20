/***********************************************************
	pm2tree.c -- tree for pmext2 decoding
***********************************************************/

#ifdef __STDC__
#include <stdlib.h>
#endif
#include "pm2tree.h"
#include "lha.h"

static unsigned char tree1left[32];
static unsigned char tree1right[32];
struct tree tree1 = { 0, tree1left, tree1right };
static unsigned char table1[32];

static unsigned char tree2left[8];
static unsigned char tree2right[8];
struct tree tree2 = { 0, tree2left, tree2right };
static unsigned char table2[8];

static unsigned char tree1bound;
static unsigned char mindepth;

void maketree1()
{
    int i, nbits, x;
    tree1bound = getbits(5);
    mindepth = getbits(3);
    if (mindepth == 0)
    {
        tree_setsingle(&tree1, tree1bound - 1);
    }
    else
    {
        for (i = 0; i<32; i++) table1[i] = 0;
       	nbits = getbits(3);
       	for (i = 0; i < tree1bound; i++) {
            x = getbits(nbits);
            table1[i] = ( x == 0 ? 0 : x - 1 + mindepth );
        }
       	tree_rebuild(&tree1, tree1bound, mindepth, table1);
    }
}

void maketree2(int par_b) /* in use: 5 <= par_b <= 8 */
{
    int i, count, index;
    if (tree1bound < 10) return;
    if (tree1bound == 29 && mindepth == 0) return;
	
    for (i = 0; i < 8; i++) table2[i] = 0;
    for (i = 0; i < par_b; i++) table2[i] = getbits(3);
    index = 0;
    count = 0;
    for (i = 0; i < 8; i++)
    {
       	if (table2[i] != 0)
       	{
       	    index = i;
       	    count++;
        }
    }
    	
    if (count == 1)
    {
        tree_setsingle(&tree2, index);
    }
    else if (count > 1)
    {
       	mindepth = 1;
       	tree_rebuild(&tree2, 8, mindepth, table2);
    }
    // Note: count == 0 is possible!
    //       Excluding that possibility was a bug in version 1.

}

int tree_get(struct tree *t)
{
    int i;
    i = t->root;
    while (i < 0x80)
    {
        i = ( getbits(1) == 0 ? t->leftarr[i] : t->rightarr[i] );
    }
    return i & 0x7F;
}

void tree_setsingle(struct tree *t, unsigned char value)
{
    t->root = 128 | value;
}

void tree_rebuild(t, bound, mindepth, table)
struct tree *t;
unsigned char bound;
unsigned char mindepth;
unsigned char *table;
{
    unsigned char *parentarr, d;
    int i, curr, empty, n;
    parentarr = (unsigned char *)malloc(bound * sizeof(unsigned char));
    t->root = 0;
    for (i = 0; i < bound; i++)
    {
        t->leftarr[i]   = 0;
        t->rightarr[i]  = 0;
        parentarr[i] = 0;
    }
    	
    for (i = 0; i < mindepth - 1; i++)
    {
       	t->leftarr[i] = i + 1;
       	parentarr[i+1] = i;
    }

    curr = mindepth - 1;
    empty = mindepth;
    for (d = mindepth; TRUE; d++)
    {
        for (i = 0; i < bound; i++)
        {
            if (table[i] == d)
            {
               	if (t->leftarr[curr] == 0) t->leftarr[curr] = i | 128;
                else
                {
                    t->rightarr[curr] = i | 128;
                    n = 0;
                    while (t->rightarr[curr] != 0)
                    {
                    	if (curr == 0) /* root? -> done */
                    	{
                    	    free(parentarr);
                    	    return;
                    	}
                      	curr = parentarr[curr];
                       	n++;
                    }
                    t->rightarr[curr] = empty;
                    for (;;)
                    {
                       	parentarr[empty] = curr;
                       	curr = empty;
                       	empty++;
                        	
                       	n--; if (n == 0) break;
                       	t->leftarr[curr] = empty;
                    }
                }
            }
        }
       	if (t->leftarr[curr] == 0) t->leftarr[curr] = empty; else t->rightarr[curr] = empty;
        	
        parentarr[empty] = curr;
        curr = empty;
        empty++;
    }
}

