/***********************************************************
	pm2tree.h -- tree for pmext2 decoding
***********************************************************/

struct tree {
    unsigned char root, *leftarr, *rightarr;
};

extern struct tree tree1, tree2;

void maketree1();
void maketree2(int par_b);
int tree_get(struct tree *t);
void tree_setsingle(struct tree *t, unsigned char value);
void tree_rebuild(struct tree *t, unsigned char bound,
                  unsigned char mindepth, unsigned char *table);
