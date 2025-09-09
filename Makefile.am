## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign
EXTRA_DIST = README.md README.jp.md header.doc.md header.doc.jp Hacking_of_LHa autogen.sh build-aux/gen-build-version.sh
SUBDIRS= man olddoc src tests

## Re-run autoconf to regenerate configure and update PACKAGE_VERSION via .tarball-version
dist-hook:
	cd $(top_srcdir) && \
		build-aux/gen-build-version.sh --package --check-clean -o .tarball-version && \
		$(AUTOCONF) --force
	mv $(top_srcdir)/.tarball-version $(distdir)
