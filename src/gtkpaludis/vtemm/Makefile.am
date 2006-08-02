EXTRA_DIST = \
	terminal_widget.hg terminal_widget.ccg \
	reaper.hg          reaper.ccg

AM_CXXFLAGS = -I$(top_srcdir) \
	@PALUDIS_CXXFLAGS@ \
	@PALUDIS_CXXFLAGS_NO_WOLD_STYLE_CAST@ \
	@PALUDIS_CXXFLAGS_NO_WREDUNDANT_DECLS@

DEFS = \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DGLIBMM_EXCEPTIONS_ENABLED=1

vte_generated_sources = \
	terminal_widget.cc terminal_widget.hh private/terminal_widget_p.hh \
	reaper.cc          reaper.hh          private/reaper_p.hh

SUBDIRS = defs converts .

if ENABLE_GTK

BUILT_SOURCES = terminal_widget.hh reaper.hh $(vte_generated_sources)

terminal_widget.hh terminal_widget.cc : terminal_widget.hg terminal_widget.ccg
	mkdir -p private
	@GMMPROC@ --unwrapped -I converts --defs defs terminal_widget . .
	sed -e 's@/\* vim: set @/* vim: set ro @' < ./terminal_widget.h > ./terminal_widget.hh
	rm terminal_widget.h
	mv ./terminal_widget.cc ./terminal_widget.cc.tmp
	sed \
	    -e 's@#include <vtemm/terminal_widget.h>@#include "terminal_widget.hh"@' \
	    -e 's@#include <vtemm/private/terminal_widget_p.h>@#include "private/terminal_widget_p.hh"@' \
	    -e 's@/\* vim: set @/* vim: set ro @' \
	    < ./terminal_widget.cc.tmp > ./terminal_widget.cc
	rm ./terminal_widget.cc.tmp
	mv ./private/terminal_widget_p.h ./private/terminal_widget_p.hh

reaper.hh reaper.cc : reaper.hg reaper.ccg
	mkdir -p private
	@GMMPROC@ --unwrapped -I converts --defs defs reaper . .
	sed -e 's@/\* vim: set @/* vim: set ro @' < ./reaper.h > ./reaper.hh
	rm reaper.h
	mv ./reaper.cc ./reaper.cc.tmp
	sed \
	    -e 's@#include <vtemm/reaper.h>@#include "reaper.hh"@' \
	    -e 's@#include <vtemm/private/reaper_p.h>@#include "private/reaper_p.hh"@' \
	    -e 's@/\* vim: set @/* vim: set ro @' \
	    < ./reaper.cc.tmp > ./reaper.cc
	rm ./reaper.cc.tmp
	mv ./private/reaper_p.h ./private/reaper_p.hh

noinst_LIBRARIES = libvtemm.a

nodist_libvtemm_a_SOURCES = \
	$(vte_generated_sources)

INCLUDES = $(DEPS_CFLAGS)

clean-local :
	rm -fr private || true

endif

CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda $(vte_generated_sources)
MAINTAINERCLEANFILES = Makefile.in

