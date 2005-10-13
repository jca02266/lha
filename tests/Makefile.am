## Process this file with automake to produce Makefile.in
check_SCRIPTS = lha-test
EXTRA_DIST =	lha-test1 \
		lha-test2 \
		lha-test3 \
		lha-test4 \
		lha-test5 \
		lha-test6 \
		lha-test7 \
		lha-test8 \
		lha-test9 \
		lha-test10 \
		lha-test11 \
		lha-test12 \
		lha-test13 \
		lha-test14 \
		lha-test15

DISTCLEANFILES = test.log

check-local:
	@sh_for_lha_test@ ./lha-test

lha-test: $(top_builddir)/config.status $(srcdir)/lha-test.in
	cd $(top_builddir) && $(SHELL) ./config.status $(subdir)/$@
