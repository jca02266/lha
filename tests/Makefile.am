## Process this file with automake to produce Makefile.in
check_SCRIPTS = lha-test
check-local:
	bash $(check_SCRIPTS)

lha-test: $(top_builddir)/config.status $(srcdir)/lha-test.in
	cd $(top_builddir) && $(SHELL) ./config.status $(subdir)/$@
