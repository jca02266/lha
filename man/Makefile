################################################################
# Makefile for LHa man directory
################################################################

NROFF = nroff
#NROFF = jman

all:	lha.man

lha.man: lha.n
	$(NROFF) -man lha.n > lha.man

install:
	$(INSTALL) $(INSTALLMAN) lha.n $(MANDIR)/man$(MANSECT)/lha.$(MANSECT)

clean:
	rm -f *~

moreclean:
	rm -f lha.man core *~ *.orig
