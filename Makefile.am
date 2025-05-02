## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign
EXTRA_DIST = README.md README.jp.md header.doc.md header.doc.jp Hacking_of_LHa autogen.sh build-aux/gen-build-version.sh
SUBDIRS= man olddoc src tests

.tarball-version:
	sh $(top_srcdir)/build-aux/gen-build-version.sh --dist > .tarball-version

pre-configure: .tarball-version

dist-hook: .tarball-version
	mv .tarball-version $(distdir)
