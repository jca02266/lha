/* ------------------------------------------------------------------------ */
/* LHa for UNIX                                                             */
/*              util.c -- LHarc Util                                        */
/*                                                                          */
/*      Modified                Nobutaka Watazaki                           */
/*                                                                          */
/*  Ver. 1.14   Source All chagned              1995.01.14  N.Watazaki      */
/*  Ver. 1.14e  Support for sfx archives        1999.05.28  T.Okamoto       */
/* ------------------------------------------------------------------------ */
#include "lha.h"
/*
 * util.c - part of LHa for UNIX Feb 26 1992 modified by Masaru Oki Mar  4
 * 1992 modified by Masaru Oki #ifndef USESTRCASECMP added. Mar 31 1992
 * modified by Masaru Oki #ifdef NOMEMSET added.
 */
#include <errno.h>

/* ------------------------------------------------------------------------ */
long
copyfile(f1, f2, size, text_flg, crcp)  /* return: size of source file */
    FILE           *f1;
    FILE           *f2;
    long            size;
    int text_flg;               /* 0: binary, 1: read text, 2: write text */
    unsigned int *crcp;
{
    unsigned short  xsize;
    char           *buf;
    long            rsize = 0;

    if (!text_mode)
        text_flg = 0;

    buf = (char *)xmalloc(BUFFERSIZE);
    if (crcp)
        INITIALIZE_CRC(*crcp);
    if (text_flg)
        init_code_cache();
    while (size > 0) {
        /* read */
        if (text_flg & 1) {
            xsize = fread_txt(buf, BUFFERSIZE, f1);
            if (xsize == 0)
                break;
            if (ferror(f1)) {
                fatal_error("file read error");
            }
        }
        else {
            xsize = (size > BUFFERSIZE) ? BUFFERSIZE : size;
            if (fread(buf, 1, xsize, f1) != xsize) {
                fatal_error("file read error");
            }
            size -= xsize;
        }
        /* write */
        if (f2) {
            if (text_flg & 2) {
                if (fwrite_txt(buf, xsize, f2)) {
                    fatal_error("file write error");
                }
            }
            else {
                if (fwrite(buf, 1, xsize, f2) != xsize) {
                    fatal_error("file write error");
                }
            }
        }
        /* calculate crc */
        if (crcp) {
            *crcp = calccrc(*crcp, buf, xsize);
#ifdef NEED_INCREMENTAL_INDICATOR
            put_indicator(xsize);
#endif
        }
        rsize += xsize;
    }
    free(buf);
    return rsize;
}

/* ------------------------------------------------------------------------ */
int
encode_stored_crc(ifp, ofp, size, original_size_var, write_size_var)
    FILE           *ifp, *ofp;
    long            size;
    long           *original_size_var;
    long           *write_size_var;
{
    int             save_quiet;
    unsigned int crc;

    save_quiet = quiet;
    quiet = 1;
    size = copyfile(ifp, ofp, size, 1, &crc);
    *original_size_var = *write_size_var = size;
    quiet = save_quiet;
    return crc;
}

/* ------------------------------------------------------------------------ */
/* If TRUE, archive file name is msdos SFX file name. */
boolean
archive_is_msdos_sfx1(name)
    char           *name;
{
    int             len = strlen(name);

    return ((len >= 4) &&
        (strucmp(".COM", name + len - 4) == 0 ||
         strucmp(".EXE", name + len - 4) == 0)) ||
        ((len >= 2) &&
         (strucmp(".x", name + len - 2) == 0));
}

/*
 * strdup(3)
 */

/* ------------------------------------------------------------------------ */
#ifndef HAVE_STRDUP
char           *
strdup(buf)
    char           *buf;
{
    char           *p;

    if ((p = (char *) malloc(strlen(buf) + 1)) == NULL)
        return NULL;
    strcpy(p, buf);
    return p;
}
#endif

/*
 * memmove( char *dst , char *src , size_t cnt )
 */

/* ------------------------------------------------------------------------ */
#ifndef HAVE_MEMMOVE
void           *
memmove(dst, src, cnt)
    register char  *dst, *src;
    register int    cnt;
{
    if (dst == src)
        return dst;
    if (src > dst) {
        while (--cnt >= 0)
            *dst++ = *src++;
    }
    else {
        dst += cnt;
        src += cnt;
        while (--cnt >= 0)
            *--dst = *--src;
    }
    return dst;
}
#endif

/*
 * rename - change the name of file 91.11.02 by Tomohiro Ishikawa
 * (ishikawa@gaia.cow.melco.CO.JP) 92.01.20 little modified (added #ifdef) by
 * Masaru Oki 92.01.28 added mkdir() and rmdir() by Tomohiro Ishikawa
 */

#if !defined(HAVE_FTRUNCATE) && !defined(_MINIX) && HAVE_LINK

/* ------------------------------------------------------------------------ */
int
rename(from, to)
    const char *from, *to;
{
    struct stat     s1, s2;
    extern int      errno;

    if (stat(from, &s1) < 0)
        return (-1);
    /* is 'FROM' file a directory? */
    if ((s1.st_mode & S_IFMT) == S_IFDIR) {
        errno = ENOTDIR;
        return (-1);
    }
    if (stat(to, &s2) >= 0) {   /* 'TO' exists! */
        /* is 'TO' file a directory? */
        if ((s2.st_mode & S_IFMT) == S_IFDIR) {
            errno = EISDIR;
            return (-1);
        }
        if (unlink(to) < 0)
            return (-1);
    }
    if (link(from, to) < 0)
        return (-1);
    if (unlink(from) < 0)
        return (-1);
    return (0);
}
#endif              /* !HAVE_FTRUNCATE */
/* ------------------------------------------------------------------------ */

#if !HAVE_MKDIR && HAVE_WORKING_FORK
#ifndef MKDIRPATH
#define MKDIRPATH   "/bin/mkdir"
#endif
#ifndef RMDIRPATH
#define RMDIRPATH   "/bin/rmdir"
#endif
int
rmdir(path)
    char           *path;
{
    int             stat, rtn = 0;
    char           *cmdname;
    if ((cmdname = (char *) malloc(strlen(RMDIRPATH) + 1 + strlen(path) + 1))
        == 0)
        return (-1);
    strcpy(cmdname, RMDIRPATH);
    *(cmdname + strlen(RMDIRPATH)) = ' ';
    strcpy(cmdname + strlen(RMDIRPATH) + 1, path);
    if ((stat = system(cmdname)) < 0)
        rtn = -1;   /* fork or exec error */
    else if (stat) {    /* RMDIR command error */
        errno = EIO;
        rtn = -1;
    }
    free(cmdname);
    return (rtn);
}

/* ------------------------------------------------------------------------ */
int
mkdir(path, mode)
    char           *path;
    int             mode;
{
    int             child, stat;
    char           *cmdname, *cmdpath = MKDIRPATH;
    if ((cmdname = (char *) strrchr(cmdpath, '/')) == (char *) 0)
        cmdname = cmdpath;
    if ((child = fork()) < 0)
        return (-1);    /* fork error */
    else if (child) {   /* parent process */
        while (child != wait(&stat))    /* ignore signals */
            continue;
    }
    else {          /* child process */
        int             maskvalue;
        maskvalue = umask(0);   /* get current umask() value */
        umask(maskvalue | (0777 & ~mode));  /* set it! */
        execl(cmdpath, cmdname, path, (char *) 0);
        /* never come here except execl is error */
        return (-1);
    }
    if (stat != 0) {
        errno = EIO;    /* cannot get error num. */
        return (-1);
    }
    return (0);
}
#endif

/*
 * strucmp modified: Oct 29 1991 by Masaru Oki
 */

#ifndef HAVE_STRCASECMP
static int
my_toupper(n)
    register int    n;
{
    if (n >= 'a' && n <= 'z')
        return n & (~('a' - 'A'));
    return n;
}

/* ------------------------------------------------------------------------ */
int
strucmp(s, t)
    register char  *s, *t;
{
    while (my_toupper(*s++) == my_toupper(*t++))
        if (!*s || !*t)
            break;
    if (!*s && !*t)
        return 0;
    return 1;
}
#endif

/* ------------------------------------------------------------------------ */
#ifndef HAVE_MEMSET
/* Public Domain memset(3) */
char           *
memset(s, c, n)
    char           *s;
    int             c, n;
{
    char           *p = s;
    while (n--)
        *p++ = (char) c;
    return s;
}
#endif

int
#if STDC_HEADERS
xsnprintf(char *dest, size_t size, char *fmt, ...)
#else
xsnprintf(dest, size, fmt, va_alist)
    char *dest, *fmt;
    size_t size;
    va_dcl
#endif
{
    int len;
    va_list v;

    va_init(v, fmt);
    len = vsnprintf(dest, size, fmt, v);
    va_end(v);

    if (len == -1)
        return -1;

    if (len >= size) {
        dest[size-1] = 0;
        return -1;
    }

    return 0;
}

#if !STRCHR_8BIT_CLEAN
/* 8 bit clean strchr()/strrchr()/memchr()/memrchr() */
char *
xstrchr(const char *s, int c)
{
    if (c == 0)
        return s + strlen(s);

    while (*s) {
        if ((unsigned char)*s == (unsigned char)c)
            return (char*)s;
        s++;
    }

    return 0;
}

char *
xstrrchr(const char *s, int c)
{
    char *p = 0;

    while (*s) {
        if ((unsigned char)*s == (unsigned char)c)
            p = (char*)s;
        s++;
    }

    return p;
}

char *
xmemchr(const char *s, int c, size_t n)
{
    char *end = (char*)s + n;

    while (s != end) {
        if ((unsigned char)*s == (unsigned char)c)
            return (char*)s;
        s++;
    }

    return 0;
}

char *
xmemrchr(const char *s, int c, size_t n)
{
    char *end = (char*)s-1;
    char *p = 0;

    s += n-1;
    while (s != end) {
        if ((unsigned char)*s == (unsigned char)c)
            p = (char*)s;
        s--;
    }

    return p;
}
#endif
