# Process this file with autoconf to produce a configure script.
AC_INIT([LHa for UNIX], 1.14i-ac20220213, jca02266@gmail.com, lha)
AC_DEFINE_UNQUOTED(LHA_CONFIGURE_OPTIONS, "$ac_configure_args",
            [specified options for the configure script.])
AC_CANONICAL_HOST
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS(config.h)

AC_DEFINE_UNQUOTED(PLATFORM, "$ac_cv_host",
                   [the type of system on which the package will run.])

# Checks for programs.
AC_PROG_CC
AC_PROG_GCC_TRADITIONAL
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_MINIX
AC_SYS_LARGEFILE
dnl Workaround for Zsh bug? on Cygwin:
dnl
dnl  Environment to trouble
dnl    Cygwin version 1.1.5.25
dnl    Zsh version 4.3.4
dnl
dnl  Examples:
dnl
dnl  o One machine(no problem)
dnl    $ uname -srvmpio
dnl    -> CYGWIN_NT-5.1 1.5.25(0.156/4/2) 2007-12-09 09:47 i686 unknown unknown Cygwin
dnl    $ zsh -c 'echo | diff - /dev/null' >/dev/null
dnl    -> $? set to 1
dnl  o Another machine(problem)
dnl    $ uname -srvmpio
dnl    -> CYGWIN_NT-5.1 1.5.25(0.156/4/2) 2008-05-26 20:37 i686 unknown unknown Cygwin
dnl    zsh -c 'echo | diff - /dev/null' >/dev/null
dnl    -> diff: -: Illegal seek
dnl    -> $? set to 2
dnl
dnl  I don't know why this problem happend.
dnl
# AC_CHECK_PROGS(sh_for_lha_test, [zsh bash ksh pdksh], [sh])
AC_CACHE_VAL([lha_cv_prog_sh_for_lha_test],[
AC_CHECK_PROG(sh_for_lha_test, [zsh], [zsh])

if test -n "$sh_for_lha_test"; then
  $sh_for_lha_test -c 'echo | diff - /dev/null' >/dev/null 2>&1
  if test $? -eq 2; then
    sh_for_lha_test=''   # cannot use Zsh
    $as_unset ac_cv_prog_sh_for_lha_test
  fi
fi

if test -z "$sh_for_lha_test"; then
  AC_CHECK_PROGS(sh_for_lha_test, [bash ksh pdksh], [sh])
fi
])

# Checks for libraries.
AC_SEARCH_LIBS(opendir, [mingwex])
AC_CHECK_LIB(applefile, af_open)

# prefer to use libiconv
AC_ARG_ENABLE(iconv,
  [AC_HELP_STRING([--enable-iconv],
     [Use iconv to support UTF-8 encoding])],
  ,enable_iconv=yes)

use_iconv=no
if test "x$enable_iconv" = xyes; then
  dnl Search iconv library
  dnl At first, search the libiconv_open() function. It is probably not part of the OS bundle.
  AC_SEARCH_LIBS(libiconv_open, [iconv])
  AC_SEARCH_LIBS(iconv_open, [iconv])

  if test "x$enable_iconv" = xyes && (test "x$ac_cv_search_libiconv_open" != xno || test "x$ac_cv_search_iconv_open" != xno); then
    use_iconv=yes
    AC_DEFINE(USE_ICONV, 1,
              [Define to 1 if you have iconv library and prefer to use it])
  fi
fi

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h limits.h sys/file.h sys/param.h sys/time.h)
AC_CHECK_HEADERS(pwd.h grp.h utime.h inttypes.h stdint.h fnmatch.h)
if test "x$enable_iconv" = xyes; then
  AC_CHECK_HEADERS(iconv.h)
fi

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_HEADER_TIME
AC_STRUCT_TM
AC_STRUCT_TIMEZONE

AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(off_t)
AC_CHECK_TYPES([uid_t, gid_t, long long, uint64_t, ssize_t])
AC_CHECK_MEMBERS([struct tm.tm_gmtoff, struct stat.st_ino],,,
[
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#if HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
])

# Checks for library functions.
AC_TYPE_SIGNAL
AC_FUNC_UTIME_NULL
AC_FUNC_FNMATCH
AC_FUNC_FSEEKO
AC_CHECK_FUNCS(strchr strdup memcpy memset memmove strcasecmp)
AC_CHECK_FUNCS(mktime timelocal tzset ftime gettimeofday utime utimes)
AC_CHECK_FUNCS(ftruncate chsize lchown mkstemp ftello)
AC_CHECK_FUNCS(getpwuid getgrgid getpwnam getgrnam)
AC_CHECK_FUNCS(libiconv iconv basename)
AC_REPLACE_FUNCS(vsnprintf fnmatch)

if test $ac_header_dirent = no; then
  AC_LIBOBJ(lhdir)
fi

AC_CHECK_DECLS([basename])

# checking whether 8 bit clean or not
AC_CACHE_CHECK([whether strchr()/strrchr() is 8bit clean],
  [lha_cv_func_strchr_8bit_clean],
  AC_TRY_RUN([
#if STDC_HEADERS
# include <string.h>
#else
# if !HAVE_STRCHR
#  define strchr index
#  define strrchr rindex
# endif
char *strchr (), *strrchr ();
#endif

int main()
{
  char *s = "\377";
  if (strchr(s, 0xff) != s) return 1;
  if (strchr(s, '\377') != s) return 1;
  if (strrchr(s, 0xff) != s) return 1;
  if (strrchr(s, '\377') != s) return 1;
  return 0; /* ok */
}
], lha_cv_func_strchr_8bit_clean=yes,
   lha_cv_func_strchr_8bit_clean=no,
   lha_cv_func_strchr_8bit_clean=no))

if test x$lha_cv_func_strchr_8bit_clean = xyes; then
  AC_DEFINE(STRCHR_8BIT_CLEAN, 1,
            [Define to 1 if strchr()/strrchr() works correctly.])
fi

# checking whether the 2nd argument of gettimeofday() is effective or not.
# note that this timezone argument is obsolete.
AC_CACHE_CHECK([whether the 2nd argument of gettimeofday() is effective],
  [lha_cv_func_gettimeofday_2nd_arg],
  AC_TRY_RUN([
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

int main()
{
    struct timeval tv;
    struct timezone tz;

    tz.tz_minuteswest = -1;
    if (gettimeofday(&tv, &tz) == -1)
        return 1;

    if (tz.tz_minuteswest == -1)
        return 1;       /* the timezone information is no given */

    return 0;
}], lha_cv_func_gettimeofday_2nd_arg=yes,
   lha_cv_func_gettimeofday_2nd_arg=no,
   lha_cv_func_gettimeofday_2nd_arg=no))

if test x$lha_cv_func_gettimeofday_2nd_arg = xyes; then
  AC_DEFINE(GETTIMEOFDAY_HAS_2ND_ARG, 1,
            [Define to 1 if the 2nd argument of gettimeofday() is effective.])
fi

# checking whether know the UTF-8-MAC encoding
dnl For example: https://github.com/fumiyas/libiconv-utf8mac
AC_CACHE_CHECK([whether iconv() know the UTF-8-MAC encoding],
  [lha_cv_func_iconv_has_utf8mac],
  AC_TRY_RUN([
#if HAVE_ICONV_H
# include <iconv.h>
#endif

int main()
{
    iconv_t ic;
    ic = iconv_open("UTF-8-MAC", "ASCII");
    if (ic == (iconv_t)-1) return -1;

    return 0;
}
], lha_cv_func_iconv_has_utf8mac=yes,
   lha_cv_func_iconv_has_utf8mac=no,
   lha_cv_func_iconv_has_utf8mac=no))

if test x$lha_cv_func_iconv_has_utf8mac = xyes; then
  AC_DEFINE(ICONV_HAS_UTF8MAC, 1,
            [Define to 1 if iconv() know the UTF-8-MAC encoding.])
fi

# support kanji code conversion
AC_SUBST(DEF_KCODE)
AC_MSG_CHECKING(kanji code conversion on text file)
AC_ARG_ENABLE(text-conv,
  AC_HELP_STRING([--enable-text-conv],
                 [support text code convert from/to EUC [[default=yes]]]),
  , enable_text_conv=yes)

AC_MSG_RESULT($enable_text_conv)
if test $enable_text_conv = yes; then
  DEF_KCODE=-DEUC
fi

# force support -lh7-
AC_SUBST(SUPPORT_LZHUFF_METHOD)
SUPPORT_LZHUFF_METHOD=-DSUPPORT_LH7

# whether use the -lh567- method
AC_MSG_CHECKING(default archive method)
AC_ARG_WITH(default-method,
  [AC_HELP_STRING([[[[--with-default-method=[567]]]]],
                 [create the -lh[567]- archive default [default=5]])],
  [case $withval in
   [[567]]) ;;
   *) AC_MSG_ERROR([you should have specified 5, 6 or 7]);;
   esac],
  with_default_method=5)
AC_MSG_RESULT(-lh$with_default_method-)
AC_DEFINE_UNQUOTED(DEFAULT_LZHUFF_METHOD, LZHUFF${with_default_method}_METHOD_NUM, [Define it to 5, 6 or 7 which you want to use -lhX- method, default])

# decide temporary path names
AC_MSG_CHECKING(template of the temporary file)
AC_ARG_WITH(tmp-file,
  AC_HELP_STRING([--with-tmp-file=TEMPLATE],
                 [temporary file template [[default=/tmp/lhXXXXXX]]]),
  [case $withval in
   yes)  with_tmp_file=/tmp/lhXXXXXX ;;
   no)   ;;
   esac],
  with_tmp_file=/tmp/lhXXXXXX)

AC_MSG_RESULT($with_tmp_file)
if test x"$with_tmp_file" != xno; then
  AC_DEFINE_UNQUOTED(TMP_FILENAME_TEMPLATE, "$with_tmp_file",
    [temporary file template for mktemp/mkstemp])
fi

# incremental indicator
AC_MSG_CHECKING(whether incremental indicator is needed)
AC_ARG_ENABLE(indicator,
  AC_HELP_STRING([--enable-indicator],
                 [need incremental indicator [[default=yes]]]),
  ,
  # default
  enable_indicator=yes)

AC_MSG_RESULT($enable_indicator)
if test "x$enable_indicator" = xyes; then
  AC_DEFINE(NEED_INCREMENTAL_INDICATOR, 1,
            [Define to 1 if you want to use the incremental indicator])
fi

# support multibyte filename
AC_MSG_CHECKING(kanji code of filename)
AC_ARG_ENABLE(multibyte-filename,
  AC_HELP_STRING([--enable-multibyte-filename],
                 [support multibyte filename. specify kanji code (euc, sjis, utf8 or auto) [[default=auto]]]),,
  # default
  enable_multibyte_filename=auto)

case $enable_multibyte_filename in
auto|yes)
  AC_EGREP_CPP(SJIS,[
#if defined(__CYGWIN__) || defined(__MINGW32__) || defined(__hpux)
SJIS
#endif], enable_multibyte_filename=CODE_SJIS,
  AC_EGREP_CPP(UTF8,[
#if USE_ICONV
UTF8
#endif], enable_multibyte_filename=CODE_UTF8,
         enable_multibyte_filename=CODE_EUC));;
sjis)    enable_multibyte_filename=CODE_SJIS;;
euc)     enable_multibyte_filename=CODE_EUC;;
utf8)    enable_multibyte_filename=CODE_UTF8;;
no)      ;;
*)       AC_MSG_ERROR([you should specify auto, sjis or euc as your system filename code.]);;
esac

AC_MSG_RESULT($enable_multibyte_filename)
if test x$enable_multibyte_filename != xno; then
  AC_DEFINE_UNQUOTED(MULTIBYTE_FILENAME, $enable_multibyte_filename,
      [Define to CODE_EUC or CODE_SJIS if you want to use multibyte filename])

  case $target_os in
  darwin*)
        # for multibyte filename
        if test $use_iconv = no; then
          LDFLAGS="$LDFLAGS -framework CoreFoundation"
        fi
        ;;
  esac
fi

# make user/group name extented header
AC_MSG_CHECKING(whether user/group name extended header is needed)
AC_ARG_ENABLE(user-name-header,
  AC_HELP_STRING([--enable-user-name-header],
                 [make user/group name extended header [[default=no]]]),
  ,
  # default
  enable_user_name_header=no)

AC_MSG_RESULT($enable_user_name_header)
if test "x$enable_user_name_header" = xyes; then
  AC_DEFINE(INCLUDE_OWNER_NAME_IN_HEADER, 1,
            [Define to 1 if you want to use the user/group name extened header])
fi

# backup old archive file
AC_MSG_CHECKING(whether backup file for old archive is left)
AC_ARG_ENABLE(backup-archive,
  AC_HELP_STRING([--enable-backup-archive],
                 [backup old archive [[default=no]]]),
  ,
  # default
  enable_backup_archive=no)

AC_MSG_RESULT($enable_backup_archive)
if test "x$enable_backup_archive" = xyes; then
  AC_DEFINE(BACKUP_OLD_ARCHIVE, 1,
            [Define to 1 if you want to leave an old archive])
fi

# additional suffixes of archive filename.
AC_MSG_CHECKING(additional suffixes of archive filename)
AC_ARG_WITH(additional-suffixes,
  [AC_HELP_STRING([[[--with-additional-suffixes=foo,bar]]],
                 [additional suffixes of archive filename, default is empty string])],
  ,
  with_additional_suffixes="")

AC_MSG_RESULT($with_additional_suffixes)
AC_DEFINE_UNQUOTED(ADDITIONAL_SUFFIXES, "$with_additional_suffixes",
            [Define to comma separated suffixes list if you wish to regard it as implicit archive suffixes.])

case $host_os in
*msdosdjgpp*)
CFLAGS="$CFLAGS -DNOT_COMPATIBLE_MODE"
;;
esac

AC_CONFIG_FILES([Makefile src/Makefile man/Makefile olddoc/Makefile])
AC_CONFIG_FILES([tests/Makefile tests/lha-test])
AC_OUTPUT
