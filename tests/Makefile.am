## Process this file with automake to produce Makefile.in
check_SCRIPTS = lha-test \
	lha-test2 \
	lha-test3 \
	lha-test4 \
	lha-test5 \
	lha-test6 \
	lha-test7 \
	lha-test8

check-local:
	bash ./lha-test

lha-test: $(top_builddir)/config.status $(srcdir)/lha-test.in
	cd $(top_builddir) && $(SHELL) ./config.status $(subdir)/$@
