include $(top_srcdir)/misc/common-makefile.am

AM_CXXFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src \
	      @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DLIBDIR=\"$(libdir)\"

SUBDIRS = .

bin_PROGRAMS = appareo
man_MANS = appareo.1

SUFFIXES = .1 .txt .xml .html-man-fragment

man_MANS_txts = appareo.txt
man_MANS_xmls = appareo.xml
man_MANS_html_man_fragments = appareo.html-man-fragment

if USE_PREBUILT_DOCUMENTATION
else

noinst_PROGRAMS = man-appareo

$(man_MANS_txts) : man-appareo
	if ! ./man-appareo `echo $@ | sed -e 's.^appareo-*..' -e 's,\.txt$$,,'` > $@ ; then rm -f $@ ; exit 1 ; fi

.txt.xml : $(man_MANS_txts)
	asciidoc --doctype=manpage --backend=docbook --out-file=$@ $<

.xml.1 : $(man_MANS_xmls)
	xmlto man $<

.xml.html-man-fragment : $(man_MANS_xmls)
	xmlto html $< -o $@-dir
	sed \
	    -e '1s,^.*\(<div class="refnamediv">\),\1,' \
	    -e '$$s,</body>.*,,' -e '$$s,</div><div class="navfooter"><hr></div>,,' \
	    -e 's,<a name="[^"]*"></a>,,g' \
	    $@-dir/index.html > $@
	rm -fr $@-dir

man_appareo_SOURCES = \
	man_appareo.cc \
	command_line.hh \
	command_line.cc

man_appareo_LDADD = \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/args/libpaludisargs_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/src/output/liboutput.a \
	$(DYNAMIC_LD_LIBS)

endif

appareo_SOURCES = \
	command_line.hh      command_line.cc      \
	appareo.cc

appareo_LDADD = \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/args/libpaludisargs_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/src/output/liboutput.a \
	$(DYNAMIC_LD_LIBS)

TESTS = version_TEST \
	help_TEST

noinst_DATA = $(man_MANS_html_man_fragments)

EXTRA_DIST = \
	$(man_MANS) \
	$(man_MAN_html_man_fragments) \
	$(TESTS)

DISTCLEANFILES = \
	$(man_MANS) \
	$(noinst_DATA) \
	$(man_MANS_txts) \
	$(man_MANS_xmls) \
	$(man_MAN_html_man_fragments)

