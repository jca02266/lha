/* ------------------------------------------------------------------------ */
/* LHa for UNIX    Archiver Driver											*/
/*																			*/
/*		Modified          		Nobutaka Watazaki							*/
/*																			*/
/*	Ver. 1.14 	Soruce All chagned				1995.01.14	N.Watazaki		*/
/*	Ver. 1.14i 	Modified and bug fixed			2000.10.06	t.okamoto   	*/
/* ------------------------------------------------------------------------ */
/*
	Included...
		lharc.h		interface.h		slidehuf.h
*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <signal.h>

#if STDC_HEADERS
# include <stdarg.h>
#else
# include <varargs.h>
#endif

#if HAVE_PWD_H
# include <pwd.h>
#endif
#if HAVE_GRP_H
# include <grp.h>
#endif

#if !HAVE_UID_T
typedef int uid_t;
#endif
#if !HAVE_GID_T
typedef int gid_t;
#endif

#include "lha_macro.h"

struct encode_option {
#if defined(__STDC__) || defined(AIX)
	void            (*output) ();
	void            (*encode_start) ();
	void            (*encode_end) ();
#else
	int             (*output) ();
	int             (*encode_start) ();
	int             (*encode_end) ();
#endif
};

struct decode_option {
	unsigned short  (*decode_c) ();
	unsigned short  (*decode_p) ();
#if defined(__STDC__) || defined(AIX)
	void            (*decode_start) ();
#else
	int             (*decode_start) ();
#endif
};

/* ------------------------------------------------------------------------ */
/*	LHa File Type Definition												*/
/* ------------------------------------------------------------------------ */
struct string_pool {
	int             used;
	int             size;
	int             n;
	char           *buffer;
};

typedef struct LzHeader {
	unsigned char   header_size;
	char            method[METHOD_TYPE_STRAGE];
	long            packed_size;
	long            original_size;
	long            last_modified_stamp;
	unsigned char   attribute;
	unsigned char   header_level;
	char            name[256];
	unsigned short  crc;
	boolean         has_crc;
	unsigned char   extend_type;
	unsigned char   minor_version;

	/* extend_type == EXTEND_UNIX  and convert from other type. */
	time_t          unix_last_modified_stamp;
	unsigned short  unix_mode;
	unsigned short  unix_uid;
	unsigned short  unix_gid;
    char            user[256];
    char            group[256];
}  LzHeader;

struct interfacing {
	FILE			*infile;
	FILE			*outfile;
	unsigned long   original;
	unsigned long   packed;
	int             dicbit;
	int             method;
};


/* ------------------------------------------------------------------------ */
/*	Option switch variable													*/
/* ------------------------------------------------------------------------ */
/* command line options (common options) */
EXTERN boolean  quiet;
EXTERN boolean  text_mode;
EXTERN boolean  verbose;
EXTERN boolean  noexec;		/* debugging option */
EXTERN boolean  force;
EXTERN boolean  prof;
EXTERN boolean  delete_after_append;
EXTERN int		compress_method;
EXTERN int		header_level;
/* EXTERN int		quiet_mode; */   /* 1996.8.13 t.okamoto */
#ifdef EUC
EXTERN boolean	euc_mode;
#endif

/* list command flags */
EXTERN boolean  verbose_listing;

/* extract/print command flags */
EXTERN boolean  output_to_stdout;

/* add/update/delete command flags */
EXTERN boolean  new_archive;
EXTERN boolean  update_if_newer;
EXTERN boolean  generic_format;

EXTERN boolean	remove_temporary_at_error;
EXTERN boolean	recover_archive_when_interrupt;
EXTERN boolean	remove_extracting_file_when_interrupt;
EXTERN boolean	get_filename_from_stdin;
EXTERN boolean	ignore_directory;
EXTERN boolean	verify_mode;

/* Indicator flag */
EXTERN int		quiet_mode;

/* ------------------------------------------------------------------------ */
/*	Globale Variable														*/
/* ------------------------------------------------------------------------ */
EXTERN char		**cmd_filev;
EXTERN int      cmd_filec;

EXTERN char		*archive_name;
EXTERN char     temporary_name[FILENAME_LENGTH];
EXTERN char     backup_archive_name[FILENAME_LENGTH];

extern char		*extract_directory;
EXTERN char		*reading_filename, *writting_filename;

/* 1996.8.13 t.okamoto */
#if 0
EXTERN boolean  remove_temporary_at_error;
EXTERN boolean  recover_archive_when_interrupt;
EXTERN boolean  remove_extracting_file_when_interrupt;
#endif

EXTERN int      archive_file_mode;
EXTERN int      archive_file_gid;

EXTERN struct	interfacing interface;
EXTERN node		*next;
/* EXTERN unsigned short crc; */  /* 1996.8.13 t.okamoto */

EXTERN int      noconvertcase; /* 2000.10.6 */

/* slide.c */
EXTERN int      unpackable;
EXTERN unsigned long origsize, compsize;
EXTERN unsigned short dicbit;
EXTERN unsigned short maxmatch;
EXTERN unsigned long count;
EXTERN unsigned long loc;			/* short -> long .. Changed N.Watazaki */
EXTERN unsigned char *text;
EXTERN int		prev_char;

/* huf.c */
#ifndef LHA_MAIN_SRC  /* t.okamoto 96/2/20 */
EXTERN unsigned short left[], right[];
EXTERN unsigned char c_len[], pt_len[];
EXTERN unsigned short c_freq[], c_table[], c_code[];
EXTERN unsigned short p_freq[], pt_table[], pt_code[], t_freq[];
#endif

/* append.c */
#ifdef NEED_INCREMENTAL_INDICATOR
EXTERN long		indicator_count;
EXTERN long		indicator_threshold;
#endif

/* crcio.c */
EXTERN FILE		*infile, *outfile;
EXTERN unsigned short crc, bitbuf;
EXTERN int      dispflg;
EXTERN long		reading_size;

/* from dhuf.c */
EXTERN unsigned int n_max;

/* lhadd.c */
EXTERN int temporary_fd;

/* ------------------------------------------------------------------------ */
/*	Functions																*/
/* ------------------------------------------------------------------------ */
#include "prototypes.h"

/* Local Variables: */
/* mode:c */
/* tab-width:4 */
/* End: */
/* vi: set tabstop=4: */
