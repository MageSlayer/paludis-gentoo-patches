vte_defs_included = \
	vte_methods.defs \
	vte_vfuncs.defs \
	vte_enums.defs \
	vte_signals.defs

vte_defs = \
	$(vte_defs_included) \
	vte.defs

noinst_DATA = $(vte_defs)
noinst_PROGRAMS = force_automake_to_include_cxxcompile

INCLUDES = $(gtkmm_CFLAGS) $(vte_CFLAGS)

force_automake_to_include_cxxcompile_SOURCES = force_automake_to_include_cxxcompile.cc

force_automake_to_include_cxxcompile.cc :
	echo 'int main(int, char *[]) { }' > $@

generate_defs_vte : generate_defs_vte.cc
	$(CXXCOMPILE) $(gtkmm_LIBS) $(vte_LIBS) -lglibmm_generate_extra_defs-2.4 -o $@ $<

CLEANFILES = *~ force_automake_to_include_cxxcompile.cc generate_defs_vte
MAINTAINERCLEANFILES = Makefile.in $(vte_defs)

vte_methods.defs :
	if ! python `pkg-config --variable=datadir pygtk-2.0`/pygtk/2.0/codegen/h2def.py \
	    `pkg-config --variable=includedir vte`/vte/*.h \
	    > $@ ; then rm $@ ; false ; fi

vte_enums.defs :
	if ! perl $(srcdir)/enum.pl `pkg-config --variable=includedir vte`/vte/*.h > $@ ; \
	    then rm $@ ; false ; fi

vte_signals.defs : generate_defs_vte
	./generate_defs_vte | sed -e 's~const-gchar\*~gchar*~' > $@

vte.defs :
	echo > $@
	for a in $(vte_defs_included) ; do echo "(include $${a})" >> $@ ; done

vte_vfuncs.defs :
	touch $@

EXTRA_DIST = enum.pl generate_defs_vte.cc

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

distcheck-deps : $(DISTCHECK_DEPS) distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s distcheck-deps || exit 1 ; done


