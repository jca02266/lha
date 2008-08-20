/***********************************************************
	pm2hist.h -- history for pmext2 decoding
***********************************************************/

/* Circular double-linked list. */

void hist_init();
unsigned char hist_lookup(int n);
void hist_update(unsigned char data);
