/* ------------------------------------------------------------------------ */
/* LHa for UNIX    															*/
/*				lhext.c -- LHarc extract									*/
/*																			*/
/*		Copyright (C) MCMLXXXIX Yooichi.Tagawa								*/
/*		Modified          		Nobutaka Watazaki							*/
/*																			*/
/*  Ver. 0.00  Original								1988.05.23  Y.Tagawa	*/
/*  Ver. 1.00  Fixed								1989.09.22  Y.Tagawa	*/
/*  Ver. 0.03  LHa for UNIX							1991.12.17  M.Oki		*/
/*  Ver. 1.12  LHa for UNIX							1993.10.01	N.Watazaki	*/
/*  Ver. 1.13b Symbolic Link Update Bug Fix			1994.06.21	N.Watazaki	*/
/*	Ver. 1.14  Source All chagned					1995.01.14	N.Watazaki	*/
/*  Ver. 1.14e bugfix                               1999.04.30  T.Okamoto   */
/* ------------------------------------------------------------------------ */
#include "lha.h"
/* ------------------------------------------------------------------------ */
static int      skip_flg = FALSE;	/* FALSE..No Skip , TRUE..Skip */
static char	   *methods[] =
{
	LZHUFF0_METHOD, LZHUFF1_METHOD, LZHUFF2_METHOD, LZHUFF3_METHOD,
	LZHUFF4_METHOD, LZHUFF5_METHOD, LZHUFF6_METHOD,
	LARC_METHOD, LARC5_METHOD, LARC4_METHOD,
	LZHDIRS_METHOD,
	NULL
};

/* ------------------------------------------------------------------------ */
static          boolean
inquire_extract(name)
	char           *name;
{
	struct stat     stbuf;

	skip_flg = FALSE;
	if (stat(name, &stbuf) >= 0) {
		if (!is_regularfile(&stbuf)) {
			error("Already exist (not a file)", name);
			return FALSE;
		}

		if (noexec) {
			printf("EXTRACT %s but file is exist.\n", name);
			return FALSE;
		}
		else if (!force) {
			if (!isatty(0))
				return FALSE;

			switch (inquire("OverWrite ?(Yes/[No]/All/Skip)", name, "YyNnAaSs\n")) {
			case 0:
			case 1:/* Y/y */
				break;
			case 2:
			case 3:/* N/n */
			case 8:/* Return */
				return FALSE;
			case 4:
			case 5:/* A/a */
				force = TRUE;
				break;
			case 6:
			case 7:/* S/s */
				skip_flg = TRUE;
				break;
			}
		}
	}
	if (noexec)
		printf("EXTRACT %s\n", name);

	return TRUE;
}

/* ------------------------------------------------------------------------ */
static          boolean
make_parent_path(name)
	char           *name;
{
	char            path[FILENAME_LENGTH];
	struct stat     stbuf;
	register char  *p;

	/* make parent directory name into PATH for recursive call */
	strcpy(path, name);
	for (p = path + strlen(path); p > path; p--)
		if (p[-1] == '/') {
			*--p = '\0';
			break;
		}

	if (p == path) {
		message("Why?", "ROOT");
		return FALSE;	/* no more parent. */
	}

	if (GETSTAT(path, &stbuf) >= 0) {
		if (is_directory(&stbuf))
			return TRUE;
		error("Not a directory", path);
		return FALSE;
	}
	errno = 0;

	if (verbose)
		printf("Making directory \"%s\".\n", path);

	if (mkdir(path, 0777) >= 0)	/* try */
		return TRUE;	/* successful done. */
	errno = 0;

	if (!make_parent_path(path))
		return FALSE;

	if (mkdir(path, 0777) < 0) {	/* try again */
		message("Cannot make directory", path);
		return FALSE;
	}

	return TRUE;
}

/* ------------------------------------------------------------------------ */
static FILE    *
open_with_make_path(name)
	char           *name;
{
	FILE           *fp;

	if ((fp = fopen(name, WRITE_BINARY)) == NULL) {
		errno = 0;
		if (!make_parent_path(name) ||
		    (fp = fopen(name, WRITE_BINARY)) == NULL)
			error("Cannot extract", name);
		errno = 0;
	}
	return fp;
}

/* ------------------------------------------------------------------------ */
static void
adjust_info(name, hdr)
	char           *name;
	LzHeader       *hdr;
{
	time_t          utimebuf[2];

	/* adjust file stamp */
	utimebuf[0] = utimebuf[1] = hdr->unix_last_modified_stamp;

	if ((hdr->unix_mode & UNIX_FILE_TYPEMASK) != UNIX_FILE_SYMLINK)
		utime(name, utimebuf);

	if (hdr->extend_type == EXTEND_UNIX
	    || hdr->extend_type == EXTEND_OS68K
	    || hdr->extend_type == EXTEND_XOSK) {
#ifdef NOT_COMPATIBLE_MODE
		Please need your modification in this space.
#else
		if ((hdr->unix_mode & UNIX_FILE_TYPEMASK) != UNIX_FILE_SYMLINK)
			chmod(name, hdr->unix_mode);
#endif
		if (!getuid()){
#if HAVE_LCHOWN
			if ((hdr->unix_mode & UNIX_FILE_TYPEMASK) == UNIX_FILE_SYMLINK)
				lchown(name, hdr->unix_uid, hdr->unix_gid);
			else 
#endif
				chown(name, hdr->unix_uid, hdr->unix_gid);
		}
		errno = 0;
	}
}

/* ------------------------------------------------------------------------ */
static void
extract_one(afp, hdr)
	FILE           *afp;	/* archive file */
	LzHeader       *hdr;
{
	FILE           *fp;	/* output file */
	struct stat     stbuf;
	char            name[257];
	int             crc;
	int             method;
	boolean         save_quiet, save_verbose, up_flag;
	char           *q = hdr->name, c;

	if (ignore_directory && strrchr(hdr->name, '/')) {
		q = (char *) strrchr(hdr->name, '/') + 1;
	}
	else {
		if (*q == '/') {
			q++;
			/*
			 * if OSK then strip device name
			 */
			if (hdr->extend_type == EXTEND_OS68K
			    || hdr->extend_type == EXTEND_XOSK) {
				do
					c = (*q++);
				while (c && c != '/');
				if (!c || !*q)
					q = ".";	/* if device name only */
			}
		}
	}

	if (extract_directory)
		sprintf(name, "%s/%s", extract_directory, q);
	else
		strcpy(name, q);


	/* LZHDIRS_METHODを持つヘッダをチェックする */
	/* 1999.4.30 t.okamoto */
	for (method = 0;; method++) {
		if (methods[method] == NULL) {
			error("Unknown method skiped ...", name);
			return;
		}
		if (bcmp(hdr->method, methods[method], 5) == 0)
			break;
	}

	if ((hdr->unix_mode & UNIX_FILE_TYPEMASK) == UNIX_FILE_REGULAR
		&& method != LZHDIRS_METHOD_NUM) {
#if 0
		for (method = 0;; method++) {
			if (methods[method] == NULL) {
				error("Unknown method skiped ...", name);
				return;
			}
			if (bcmp(hdr->method, methods[method], 5) == 0)
				break;
		}
#endif

		reading_filename = archive_name;
		writting_filename = name;
		if (output_to_stdout || verify_mode) {
			if (noexec) {
				printf("%s %s\n", verify_mode ? "VERIFY" : "EXTRACT", name);
				if (afp == stdin) {
					int             i = hdr->packed_size;
					while (i--)
						fgetc(afp);
				}
				return;
			}

			save_quiet = quiet;
			save_verbose = verbose;
			if (!quiet && output_to_stdout) {
				printf("::::::::\n%s\n::::::::\n", name);
				quiet = TRUE;
				verbose = FALSE;
			}
			else if (verify_mode) {
				quiet = FALSE;
				verbose = TRUE;
			}

			crc = decode_lzhuf
				(afp, stdout, hdr->original_size, hdr->packed_size, name, method);
			quiet = save_quiet;
			verbose = save_verbose;
		}
		else {
			if (skip_flg == FALSE)  {
				up_flag = inquire_extract(name);
				if (up_flag == FALSE && force == FALSE) {
					return;
				}
			}

			if (skip_flg == TRUE) {	/* if skip_flg */
				if (stat(name, &stbuf) == 0 && force != TRUE) {
					if (stbuf.st_mtime >= hdr->unix_last_modified_stamp) {
						if (quiet != TRUE)
							printf("%s : Skipped...\n", name);
						return;
					}
				}
			}
			if (noexec) {
				if (afp == stdin) {
					int i = hdr->packed_size;
					while (i--)
						fgetc(afp);
				}
				return;
			}

			signal(SIGINT, interrupt);
			signal(SIGHUP, interrupt);

			unlink(name);
			errno = 0;
			remove_extracting_file_when_interrupt = TRUE;

			if ((fp = open_with_make_path(name)) != NULL) {
				crc = decode_lzhuf
					(afp, fp, hdr->original_size, hdr->packed_size, name, method);
				fclose(fp);
			}
			remove_extracting_file_when_interrupt = FALSE;
			signal(SIGINT, SIG_DFL);
			signal(SIGHUP, SIG_DFL);

			if (!fp)
				return;
		}

		errno = 0;
		if (hdr->has_crc && crc != hdr->crc)
			error("CRC error", name);
	}
	else if ((hdr->unix_mode & UNIX_FILE_TYPEMASK) == UNIX_FILE_DIRECTORY
			 || (hdr->unix_mode & UNIX_FILE_TYPEMASK) == UNIX_FILE_SYMLINK
			 || method == LZHDIRS_METHOD_NUM) {
		/* ↑これで、Symblic Link は、大丈夫か？ */
		if (!ignore_directory && !verify_mode) {
			if (noexec) {
				if (quiet != TRUE)
					printf("EXTRACT %s (directory)\n", name);
				return;
			}
			/* NAME has trailing SLASH '/', (^_^) */
			if ((hdr->unix_mode & UNIX_FILE_TYPEMASK) == UNIX_FILE_SYMLINK) {
				char            buf[256], *bb1, *bb2;
				int             l_code;
				strcpy(buf, name);
				bb1 = strtok(buf, "|");
				bb2 = strtok(NULL, "|");

#ifdef S_IFLNK
				if (skip_flg == FALSE)  {
					up_flag = inquire_extract(name);
					if (up_flag == FALSE && force == FALSE) {
						return;
					}
				} else {
					if (GETSTAT(bb1, &stbuf) == 0 && force != TRUE) {
						if (stbuf.st_mtime >= hdr->unix_last_modified_stamp) {
							if (quiet != TRUE)
								printf("%s : Skipped...\n", bb1);
							return;
						}
					}
				}

				unlink(bb1);
				l_code = symlink(bb2, bb1);
				if (l_code < 0) {
					if (quiet != TRUE)
						warning("Can't make Symbolic Link : ");
				}
				if (quiet != TRUE) {
					printf("Symbolic Link %s -> %s\n", bb1, bb2);
				}
				strcpy(name, bb1);	/* Symbolic's name set */
#else
				sprintf(buf, "%s -> %s", bb1, bb2);
				warning("Can't make Symbolic Link", buf);
				return;
#endif
			} else { /* make directory */
				if (!output_to_stdout && !make_parent_path(name))
					return;
			}
		}
	}
	else {
		error("Unknown information", name);
	}

	if (!output_to_stdout)
		adjust_info(name, hdr);
}

/* ------------------------------------------------------------------------ */
/* EXTRACT COMMAND MAIN														*/
/* ------------------------------------------------------------------------ */
void
cmd_extract()
{
	LzHeader        hdr;
	long            pos;
	FILE           *afp;

	/* open archive file */
	if ((afp = open_old_archive()) == NULL)
		fatal_error(archive_name);

	if (archive_is_msdos_sfx1(archive_name))
		skip_msdos_sfx1_code(afp);

	/* extract each files */
	while (get_header(afp, &hdr)) {
		if (need_file(hdr.name)) {
			pos = ftell(afp);
			extract_one(afp, &hdr);
			fseek(afp, pos + hdr.packed_size, SEEK_SET);
		}
		else {
			if (afp != stdin)
				fseek(afp, hdr.packed_size, SEEK_CUR);
			else {
				int             i = hdr.packed_size;
				while (i--)
					fgetc(afp);
			}
		}
	}

	/* close archive file */
	fclose(afp);

	return;
}
/* Local Variables: */
/* tab-width : 4 */
/* End: */
