include $(top_srcdir)/misc/common-makefile.am

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_NO_WOLD_STYLE_CAST@ @PALUDIS_CXXFLAGS_VISIBILITY@

SUBDIRS = .

noinst_LIBRARIES = liblittlelf.a

liblittlelf_a_SOURCES = \
	elf.cc                    elf.hh                    \
	elf_sections.cc           elf_sections.hh           \
	elf_dynamic_section.cc    elf_dynamic_section.hh    \
	elf_relocation_section.cc elf_relocation_section.hh \
	elf_symbol_section.cc     elf_symbol_section.hh     \
	elf_types.hh

TESTS =
check_PROGRAMS = $(TESTS)

EXTRA_DIST = \
	$(TESTS)

