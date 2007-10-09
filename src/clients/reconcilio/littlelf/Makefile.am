AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_NO_WOLD_STYLE_CAST@ @PALUDIS_CXXFLAGS_VISIBILITY@

SUBDIRS = .

noinst_LIBRARIES = liblittlelf.a

liblittlelf_a_SOURCES = \
	Elf.cc                  Elf.cc                  \
	ElfSections.cc          ElfSections.cc          \
	ElfDynamicSection.cc    ElfDynamicSection.cc    \
	ElfRelocationSection.cc ElfRelocationSection.cc \
	ElfSymbolSection.cc     ElfSymbolSection.cc     \
	ElfTypes.hh

TESTS_ENVIRONMENT = env \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	TEST_OUTPUT_WRAPPER="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/util/outputwrapper`" \
	bash $(top_srcdir)/test/run_test.sh bash

TESTS =
check_PROGRAMS = $(TESTS)

EXTRA_DIST = \
	$(TESTS)

CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
MAINTAINERCLEANFILES = Makefile.in

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done


