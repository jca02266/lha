## Process this file with automake to produce Makefile.in
check_SCRIPTS = lha-test
EXTRA_DIST =	lha-test2 \
		lha-test3 \
		lha-test4 \
		lha-test5 \
		lha-test6 \
		lha-test7 \
		lha-test8 \
		lha-test9 \
		lha-test10

DISTCLEANFILES = test.log

check-local:
	bash ./lha-test

lha-test: $(top_builddir)/config.status $(srcdir)/lha-test.in
	cd $(top_builddir) && $(SHELL) ./config.status $(subdir)/$@
