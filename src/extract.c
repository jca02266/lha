/* ------------------------------------------------------------------------ */
/* LHa for UNIX    															*/
/*				extract.c -- extrcat from archive							*/
/*																			*/
/*		Modified          		Nobutaka Watazaki							*/
/*																			*/
/*	Ver. 1.14 	Source All chagned				1995.01.14	N.Watazaki		*/
/* ------------------------------------------------------------------------ */
#include "lha.h"

int
decode_lzhuf(infp, outfp, original_size, packed_size, name, method)
	FILE           *infp;
	FILE           *outfp;
	long            original_size;
	long            packed_size;
	char           *name;
	int             method;
{
	interface.method = method;
	interface.dicbit = 13;	/* method + 8; -lh5- */
	interface.infile = infp;
	interface.outfile = outfp;
	interface.original = original_size;
	interface.packed = packed_size;

	switch (method) {
	case LZHUFF0_METHOD_NUM:
	case LARC4_METHOD_NUM:
		start_indicator(name, original_size
			      ,verify_mode ? "Testing " : "Melting ", 2048);
		copyfile(infp, (verify_mode ? NULL : outfp), original_size, 2);
		break;
	case LARC_METHOD_NUM:		/* -lzs- */
		interface.dicbit = 11;
		start_indicator(name, original_size
				,verify_mode ? "Testing " : "Melting "
				,1 << interface.dicbit);
		decode(&interface);
		break;
	case LZHUFF1_METHOD_NUM:		/* -lh1- */
	case LZHUFF4_METHOD_NUM:		/* -lh4- */
	case LARC5_METHOD_NUM:			/* -lz5- */
		interface.dicbit = 12;
		start_indicator(name, original_size
				,verify_mode ? "Testing " : "Melting "
				,1 << interface.dicbit);
		decode(&interface);
		break;
	case LZHUFF6_METHOD_NUM:		/* -lh6- */	/* Added N.Watazaki (^_^) */
#ifdef SUPPORT_LH7
	case LZHUFF7_METHOD_NUM:                /* -lh7- */
#endif
	    interface.dicbit = (method - LZHUFF6_METHOD_NUM) + 15;
		
	default:
		start_indicator(name, original_size
				,verify_mode ? "Testing " : "Melting "
				,1 << interface.dicbit);
		decode(&interface);
	}
	finish_indicator(name, verify_mode ? "Tested  " : "Melted  ");

	return crc;
}

/* Local Variables: */
/* mode:c */
/* tab-width:4 */
/* End: */
