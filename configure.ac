# Process this file with autoconf to produce a configure script.
AC_INIT([LHa for UNIX], 1.14i-ac20020605, jca02266@nifty.ne.jp, lha)
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(config.h)

AC_DEFINE_UNQUOTED(PLATFORM, "$ac_cv_host",
		   [the type of system on which the package will run.])

# Checks for programs.
AC_PROG_CC
AC_PROG_GCC_TRADITIONAL
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_MINIX
AM_C_PROTOTYPES

# Checks for libraries.

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h limits.h sys/file.h sys/param.h sys/time.h)
AC_CHECK_HEADERS(pwd.h grp.h)

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_HEADER_TIME
AC_STRUCT_TM
AC_STRUCT_TIMEZONE

AC_CHECK_TYPES([uid_t, gid_t])
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
AC_FUNC_FORK
AC_FUNC_UTIME_NULL
AC_CHECK_FUNCS(strchr strdup memcpy memset memmove)
AC_CHECK_FUNCS(mktime timelocal tzset ftime gettimeofday utime)
AC_CHECK_FUNCS(mkdir rmdir ftruncate lchown mkstemp)
AC_CHECK_FUNCS(strcasecmp link)
AC_CHECK_FUNCS(getpwuid getgrgid getpwnam getgrnam)

if test $ac_header_dirent = no; then
  AC_LIBOBJ(lhdir)
fi

# support kanji code conversion
AC_SUBST(DEF_KCODE)
AC_ARG_ENABLE(text_conv,
  AC_HELP_STRING([--enable-text-conv],
		 [support text code convert from/to EUC [[default=yes]]]),
  , enable_text_conv=yes)

if test $enable_text_conv = yes; then
  DEF_KCODE=-DEUC
fi

# force support -lh7-
AC_SUBST(SUPPORT_LZHUFF_METHOD)
SUPPORT_LZHUFF_METHOD=-DSUPPORT_LH7

# whether use the -lh567- method
AC_MSG_CHECKING(default archive method is )
AC_ARG_WITH(default_method,
  AC_HELP_STRING([--with-default-method=[[567]]],
		 [create the -lh[[567]]- archive default [[default=5]]]),
  [case $withval in
   [567]) ;;
   *) AC_MSG_ERROR(you should have specified 5, 6 or 7);;
   esac],
  with_default_method=5)
AC_MSG_RESULT(-lh$with_default_method-)
AC_DEFINE_UNQUOTED(DEFAULT_LZHUFF_METHOD, LZHUFF${with_default_method}_METHOD_NUM, [Define it to 5, 6 or 7 which you want to use -lhX- method, default])

# decide temporary path names
AC_MSG_CHECKING(template of the temporary file is )
AC_ARG_WITH(tmp_file,
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
AC_ARG_ENABLE(indicator,
  AC_HELP_STRING([--enable-indicator],
		 [need incremental indicator [[default=yes]]]),
  ,
  # default
  enable_indicator=yes)
if test "x$enable_indicator" = xyes; then
  AC_DEFINE(NEED_INCREMENTAL_INDICATOR, 1,
	    [Define to 1 if you want to use the incremental indicator])
fi

# support multibyte filename
AC_ARG_ENABLE(multibyte-char,
  AC_HELP_STRING([--enable-multibyte-char],
		 [support multibyte filename [[default=yes]]]),
  ,
  # default
  enable_multibyte_char=yes)
if test "x$enable_multibyte_char" = xyes; then
  AC_DEFINE(MULTIBYTE_CHAR, 1,
	    [Define to 1 if you want to use multibyte filename])
fi

# make user/group name extented header
AC_ARG_ENABLE(user-name-header,
  AC_HELP_STRING([--enable-user-name-header],
		 [make user/group name extended header [[default=no]]]),
  ,
  # default
  enable_user_name_header=no)
if test "x$user_name_header" = xyes; then
  AC_DEFINE(INCLUDE_OWNER_NAME_IN_HEADER, 1,
            [Define to 1 if you want to use the user/group name extened header])
fi

AC_CONFIG_FILES([Makefile src/Makefile man/Makefile])
AC_CONFIG_FILES([tests/Makefile tests/lha-test])
AC_OUTPUT
