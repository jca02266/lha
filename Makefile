################################################################
# Makefile for LHa topdir
#		Mar. 2, 1992, written by Masaru Oki.
#		AUG. 22,1994, modified by Nobutaka Watazaki
#               Jun.  7,1999, modified by Tsugio Okamoto
################################################################

#-----------------------------------------------------------------------
# CONFIGURATION
#	SWITCHES macro definitions ... see config.eng or config.jpn.
#-----------------------------------------------------------------------

SHELL=/bin/sh
MAKE = make

#CC			= cc
CC			= gcc
SWITCHES	= -DNEED_INCREMENTAL_INDICATOR \
 -DTMP_FILENAME_TEMPLATE="\"/tmp/lhXXXXXX\""

#MACHINE     = -DSYSTIME_HAS_NO_TM -DFTIME -DEUC
#MACHINE     = -DSYSTIME_HAS_NO_TM -DEUC -DHAVE_TM_ZONE -DSYSV_SYSTEM_DIR
MACHINE     = -DSYSTIME_HAS_NO_TM -DEUC -DSYSV_SYSTEM_DIR -DMKTIME

#OPTIMIZE	= -O2 -fstrength-reduce -fomit-frame-pointer
OPTIMIZE	= -O2 -DSUPPORT_LH6

BINDIR = /usr/local/bin
MANDIR = /usr/local/man
MANSECT = n

INSTALL = install
INSTALLBIN = -s -m 755
INSTALLMAN = -m 644

SUBDIRS = src man

O		= .o

# missing library objects.
#	memset$(O) lhdir$(O)
LIBRARYOBJS	= patmatch$(O)

MYMAKEFLAGS = 'BINDIR=$(BINDIR)' 'MANDIR=$(MANDIR)' 'MANSECT=$(MANSECT)'\
 'INSTALL=$(INSTALL)' 'INSTALLBIN=$(INSTALLBIN)' 'INSTALLMAN=$(INSTALLMAN)'\
 'CC=$(CC)' 'SWITCHES=$(SWITCHES)' 'O=$(O)' 'LIBRARYOBJS=$(LIBRARYOBJS)' \
 'OPTIMIZE=$(OPTIMIZE)' 'MACHINE=$(MACHINE)'

all:
	@for i in $(SUBDIRS); do \
	echo "make all in $$i..."; \
	(cd $$i; $(MAKE) $(MFLAGS) $(MYMAKEFLAGS) all); done

install:
	@for i in $(SUBDIRS); do \
	echo "Installing in $$i..."; \
	(cd $$i; $(MAKE) $(MFLAGS) $(MYMAKEFLAGS) install); done

clean:
	@for i in $(SUBDIRS); do \
	echo "Clearing in $$i..."; \
	(cd $$i; $(MAKE) $(MFLAGS) $(MYMAKEFLAGS) clean); done

moreclean:
	@for i in $(SUBDIRS); do \
	echo "Clearing in $$i..."; \
	(cd $$i; $(MAKE) $(MFLAGS) $(MYMAKEFLAGS) moreclean); done

