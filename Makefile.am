CLEANFILES = *~
MAINTAINERCLEANFILES = Makefile.in configure config/* aclocal.m4 \
			config.h config.h.in INSTALL COPYING
AUTOMAKE_OPTIONS = dist-bzip2 no-dist-gzip std-options
EXTRA_DIST = autogen.bash
SUBDIRS = misc test paludis ebuild src doc

doxygen :
	$(MAKE) -C doc doxygen

all-local :
	@echo
	@echo "*********************************"
	@echo "You should run 'make check' next."
	@echo "*********************************"
	@echo

