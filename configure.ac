dnl vim: set sw=4 sts=4 ts=4 noet ft=config foldmethod=marker foldmarker={{{,}}} :

dnl {{{ program, version
AC_PREREQ(2.59)
AC_INIT([paludis/paludis.hh])
AC_CONFIG_AUX_DIR(config)

VERSION_MAJOR=0
VERSION_MINOR=16
VERSION_MICRO=1
VERSION_FULL="$VERSION_MAJOR.$VERSION_MINOR.$VERSION_MICRO"
VERSION="$VERSION_FULL"
VERSION_LIB_CURRENT=`expr 100 '*' $VERSION_MAJOR + $VERSION_MINOR`
VERSION_LIB_REVISION="$VERSION_MICRO"

AC_SUBST([VERSION_MAJOR])
AC_SUBST([VERSION_MINOR])
AC_SUBST([VERSION_MICRO])
AC_SUBST([VERSION_FULL])
AC_SUBST([VERSION_LIB_CURRENT])
AC_SUBST([VERSION_LIB_REVISION])

AM_INIT_AUTOMAKE(paludis, [$VERSION_FULL])
AC_LANG([C++])
dnl }}}

dnl {{{ monolithic build
AC_MSG_CHECKING([whether to do a monolithic build])
AC_ARG_ENABLE([monolithic],
			  AS_HELP_STRING([--enable-monolithic], [Make a monolithic build (developer use only)]),
			  [build_monolithic=$enableval
			   AC_MSG_RESULT([$enableval])],
			  [build_monolithic=no
			   AC_MSG_RESULT([no])])
MONOLITHIC=$build_monolithic
AC_SUBST([MONOLITHIC])
AM_CONDITIONAL([MONOLITHIC], test "x$build_monolithic" = "xyes")
if test "x$build_monolithic" = "xyes" ; then
	AC_DEFINE([MONOLITHIC], [1], [Make a monolithic build])
else
	AC_DISABLE_STATIC
fi
AC_MSG_CHECKING([whether we're building shared libaries])
AC_MSG_RESULT([$enable_shared])
AC_MSG_CHECKING([whether we're building static libaries])
AC_MSG_RESULT([$enable_static])
dnl }}}

dnl {{{ basic toolchain checks
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_LIBTOOL
AC_PROG_MAKE_SET
dnl }}}

dnl {{{ check for partial specialisation of template classes
AC_MSG_CHECKING([for compiler support for partial specialisation of template classes])
AC_COMPILE_IFELSE([
template <typename T_>
struct S
{
    static const int s = 0;
};

template <typename T_>
struct S<const T_>
{
};

template <typename T_>
struct S<const T_ &>
{
    static const int t = 0;
};

int main(int, char **)
{
    return S<int &>::s + S<const int &>::t;
}
],
	[AC_MSG_RESULT([yes])],
	[
	 AC_MSG_RESULT([no])
	 AC_MSG_ERROR([Your C++ compiler doesn't compile C++ code])
	])
dnl }}}

dnl {{{ check for basic_string and string::find_last_not_of
AC_MSG_CHECKING([for a sufficiently standard std::basic_string<>])
AC_COMPILE_IFELSE([
#include <string>
int main(int, char **)
{
	std::string s("test");
	std::basic_string<int> t;
	return std::string::npos != s.find_last_not_of("abcde") ? 0 : 1;
}
],
	[AC_MSG_RESULT([yes])],
	[
	 AC_MSG_RESULT([no])
	 AC_MSG_ERROR([I need a less broken std::basic_string<> implementation])
	])
dnl }}}

dnl {{{ check for limits
AC_MSG_CHECKING([for a usable std::numeric_limits])
AC_COMPILE_IFELSE([
#include <limits>
int main(int, char **)
{
	return std::numeric_limits<unsigned>::digits >> 3;
}
],
	[AC_MSG_RESULT([yes])],
	[
	 AC_MSG_RESULT([no])
	 AC_MSG_ERROR([Your std::numeric_limits either doesn't exist or hates me])
	])
dnl }}}

dnl {{{ check for broken std::set<>::const_reverse_iterator
AC_MSG_CHECKING([for a usable std::set<>::const_reverse_iterator])
AC_COMPILE_IFELSE([
#include <set>
int main(int, char **)
{
	std::set<int> s;
	s.insert(1);
	std::set<int>::const_reverse_iterator i;
	return *i;
}
],
	[AC_MSG_RESULT([yes])],
	[
	 AC_MSG_RESULT([no])
	 AC_MSG_ERROR([Your std::set<>::const_reverse_iterator is h0rked])
	])
dnl }}}

dnl {{{ check for hashes
have_tr1_hashes=0
have_ext_hashes=0
have_std_hashes=0
AC_MSG_CHECKING([for hashed associative containers])
AC_COMPILE_IFELSE([
#include <tr1/unordered_map>
int main(int, char **)
{
	std::tr1::unordered_map<int, int> m;
	m.insert(std::make_pair(1, 2));
}
],
	[
	 AC_MSG_RESULT([std::tr1::unordered_map])
	 have_tr1_hashes=1
	],
	[
	 AC_COMPILE_IFELSE([
#include <ext/hash_map>
int main(int, char **)
{
	__gnu_cxx::hash_map<int, int> m;
	m.insert(std::make_pair(1, 2));
}
],
	 [
	  AC_MSG_RESULT([__gnu_cxx::hash_map])
	  have_ext_hashes=1
	 ],
	 [
	  AC_COMPILE_IFELSE([
#include <hash_map>
int main(int, char **)
{
	std::hash_map<int, int> m;
	m.insert(std::make_pair(1, 2));
}
],
	  [
	   AC_MSG_RESULT([std::hash_map])
	   have_std_hashes=1
	  ],
	  [
	   AC_MSG_RESULT([no, falling back to std::map (slower)])
	  ])])])

HAVE_TR1_HASHES=$have_tr1_hashes
AC_DEFINE_UNQUOTED([HAVE_TR1_HASHES], $have_tr1_hashes, [Do we have std::tr1:: hashes?])
AC_SUBST([HAVE_TR1_HASHES])
HAVE_EXT_HASHES=$have_ext_hashes
AC_DEFINE_UNQUOTED([HAVE_EXT_HASHES], $have_ext_hashes, [Do we have __gnu_cxx:: hashes?])
AC_SUBST([HAVE_EXT_HASHES])
HAVE_STD_HASHES=$have_std_hashes
AC_DEFINE_UNQUOTED([HAVE_STD_HASHES], $have_std_hashes, [Do we have std:: hashes?])
AC_SUBST([HAVE_STD_HASHES])
dnl }}}

need_xml_check=

dnl {{{ we need libebt.
dnl don't use AC_CHECK_HEADER, it does silly things
AC_MSG_CHECKING([for libebt])
AC_COMPILE_IFELSE([
#include <libebt/libebt.hh>
#include <string>
struct Tag { };
int main(int, char **)
{
	libebt::BacktraceContext<Tag> context("blah");
}
],
	[AC_MSG_RESULT([yes])],
	[AC_MSG_RESULT([no])
	 AC_MSG_ERROR([libebt (http://libebt.berlios.de/) is required])])
dnl }}}

dnl {{{ we need libwrapiter.
dnl don't use AC_CHECK_HEADER, it does silly things
AC_MSG_CHECKING([for libwrapiter])
AC_COMPILE_IFELSE([
#include <libwrapiter/libwrapiter.hh>
#include <string>
struct C {
	typedef libwrapiter::ForwardIterator<C, int> Iterator;
	Iterator begin();
	Iterator end();
};
int main(int, char **)
{
}
],
	[AC_MSG_RESULT([yes])],
	[AC_MSG_RESULT([no])
	 AC_MSG_ERROR([libwrapiter (http://libwrapiter.berlios.de/) is required])])
AC_MSG_CHECKING([libwrapiter version])
AC_COMPILE_IFELSE([
#include <libwrapiter/libwrapiter.hh>
#if LIBWRAPITER_VERSION < 1000
#  error libwrapiter 1.0 or later required
#endif
],
	[AC_MSG_RESULT([yes])],
	[AC_MSG_RESULT([no])
	 AC_MSG_ERROR([libwrapiter version must be at least 1.0])])
dnl }}}

dnl {{{ check for various misc functions.
dnl It's probably possible to make many of these optional rather than
dnl hard requirements. If any of these fail for you, send patches.
AC_CHECK_FUNCS([alarm],        [], [AC_MSG_ERROR([I need alarm])])
AC_CHECK_FUNCS([strerror],     [], [AC_MSG_ERROR([I need strerror])])
AC_CHECK_FUNCS([signal],       [], [AC_MSG_ERROR([I need signal])])
AC_CHECK_FUNCS([popen],        [], [AC_MSG_ERROR([I need popen])])
AC_CHECK_FUNCS([ptsname],      [], [AC_MSG_ERROR([I need ptsname])])
AC_CHECK_FUNCS([grantpt],      [], [AC_MSG_ERROR([I need grantpt])])
AC_CHECK_FUNCS([unlockpt],     [], [AC_MSG_ERROR([I need unlockpt])])
dnl }}}

dnl {{{ check for chflags function
AC_CHECK_FUNCS([chflags])
AM_CONDITIONAL(HAVE_CHFLAGS, test x$ac_cv_func_chflags = xyes)
dnl }}}

dnl {{{ check for cxxflags
if test x = x"$LET_ME_RICE"
then
	AC_MSG_CHECKING([for go faster stripes])
	for x in "enforce-eh" "fast-math" "rtti"
	do
		if echo "${LDFLAGS} ${CFLAGS} ${CXXFLAGS}" | grep "$x" >/dev/null ; then
			AC_MSG_RESULT([$x])
			AC_MSG_ERROR([Bad ricer. No bagel.])
		fi
	done
	AC_MSG_RESULT([no])
fi

PALUDIS_CXXFLAGS=
PALUDIS_CXXFLAGS_NO_WOLD_STYLE_CAST=
PALUDIS_CXXFLAGS_NO_WREDUNDANT_DECLS=
PALUDIS_CXXFLAGS_NO_WSHADOW=
PALUDIS_CXXFLAGS_WITHOUT_PEDANTIC=
PALUDIS_CXXFLAGS_VISIBILITY=
AC_MSG_CHECKING([whether our compiler is iccy])
AC_COMPILE_IFELSE([
#ifndef __ICC
#error nope
#endif
],
	[cxx_compiler_icc=yes],
	[cxx_compiler_icc=no])
AC_MSG_RESULT([${cxx_compiler_icc}])
AC_DEFUN([CHECK_CXXFLAG], [
	save_CXXFLAGS=$CXXFLAGS
	CXXFLAGS="$CXXFLAGS $PALUDIS_CXXFLAGS $1 -Werror"
	AC_COMPILE_IFELSE([
#include <string>
#include <iostream>
int main(int, char **)
{
	std::string s("test");
	std::cout << s << std::endl;
}
	],
	[cxxflag_success=yes],
	[cxxflag_success=no])
	CXXFLAGS="$save_CXXFLAGS"
	if test "x$cxxflag_success" = "xyes" ; then
		PALUDIS_CXXFLAGS="$PALUDIS_CXXFLAGS $1"
		cxxflags_message="${cxxflags_message} $1"
	fi
	])
AC_MSG_CHECKING([for appropriate compiler flags])
if test "x${cxx_compiler_icc}" = "xyes" ; then
	CHECK_CXXFLAG([-Wall])
	CHECK_CXXFLAG([-wd279])
	CHECK_CXXFLAG([-wd304])
	CHECK_CXXFLAG([-wd383])
	CHECK_CXXFLAG([-wd444])
	CHECK_CXXFLAG([-wd488])
	CHECK_CXXFLAG([-wd981])
	CHECK_CXXFLAG([-wd1125])
	CHECK_CXXFLAG([-wd1418])
	PALUDIS_CXXFLAGS_WITHOUT_PEDANTIC=$PALUDIS_CXXFLAGS
elif test "x${ac_cv_cxx_compiler_gnu}" = "xyes" ; then
	CHECK_CXXFLAG([-Wall])
	CHECK_CXXFLAG([-Wextra])
	if ! test "x$cxxflag_success" = "xyes" ; then
		CHECK_CXXFLAG([-W])
	fi
	CHECK_CXXFLAG([-Wold-style-cast])
	if test "x$cxxflag_success" = "xyes" ; then
		PALUDIS_CXXFLAGS_NO_WOLD_STYLE_CAST=-Wno-old-style-cast
	fi
	CHECK_CXXFLAG([-Wredundant-decls])
	if test "x$cxxflag_success" = "xyes" ; then
		PALUDIS_CXXFLAGS_NO_WREDUNDANT_DECLS=-Wno-redundant-decls
	fi
	CHECK_CXXFLAG([-Wstrict-null-sentinel])
	CHECK_CXXFLAG([-Wmissing-noreturn])
	CHECK_CXXFLAG([-Woverloaded-virtual])
	CHECK_CXXFLAG([-Winit-self])
	CHECK_CXXFLAG([-Wunreachable-code])
	CHECK_CXXFLAG([-Wunused-function])
	CHECK_CXXFLAG([-Wshadow])
	if test "x$cxxflag_success" = "xyes" ; then
		PALUDIS_CXXFLAGS_NO_WSHADOW=-Wno-shadow
	fi
	PALUDIS_CXXFLAGS_WITHOUT_PEDANTIC=$PALUDIS_CXXFLAGS
	CHECK_CXXFLAG([-pedantic])
else
	CHECK_CXXFLAG([-Wall])
fi
AC_MSG_RESULT([${cxxflags_message}])
AC_SUBST([PALUDIS_CXXFLAGS])
AC_SUBST([PALUDIS_CXXFLAGS_WITHOUT_PEDANTIC])
AC_SUBST([PALUDIS_CXXFLAGS_NO_WOLD_STYLE_CAST])
AC_SUBST([PALUDIS_CXXFLAGS_NO_WSHADOW])
AC_SUBST([PALUDIS_CXXFLAGS_NO_WREDUNDANT_DECLS])

dnl AC_MSG_CHECKING([whether to enable visibility])
dnl AC_ARG_ENABLE([visibility],
dnl 			  AS_HELP_STRING([--enable-visibility], [Enable visibility (g++-4.1)]),
dnl 			  [ENABLE_VISIBILITY=$enableval],
dnl 			  [ENABLE_VISIBILITY=no])
dnl AC_MSG_RESULT([$ENABLE_VISIBILITY])
ENABLE_VISIBILITY=no
AC_SUBST([ENABLE_VISIBILITY])
if test "x$ENABLE_VISIBILITY" = "xyes" ; then
	AC_MSG_CHECKING([for appropriate compiler visibility flags])
	old_PALUDIS_CXXFLAGS=$PALUDIS_CXXFLAGS
	PALUDIS_CXXFLAGS=
	CHECK_CXXFLAG([-fvisibility-inlines-hidden])
	CHECK_CXXFLAG([-fvisibility=hidden])
	AC_MSG_RESULT([$PALUDIS_CXXFLAGS])
	PALUDIS_CXXFLAGS_VISIBILITY=$PALUDIS_CXXFLAGS
	PALUDIS_CXXFLAGS=$old_PALUDIS_CXXFLAGS
	PALUDIS_ENABLE_VISIBILITY=1
else
	PALUDIS_ENABLE_VISIBILITY=0
fi
AC_SUBST([PALUDIS_ENABLE_VISIBILITY])
AC_SUBST([PALUDIS_CXXFLAGS_VISIBILITY])
dnl }}}

dnl {{{ sandbox
AC_MSG_CHECKING([whether to enable sandbox])
AC_ARG_ENABLE([sandbox],
			  AS_HELP_STRING([--enable-sandbox], [Enable sandbox]),
			  [HAVE_SANDBOX=$enableval
			   AC_MSG_RESULT([$enableval])],
			  [AC_MSG_RESULT([autodetect])
			   AC_CHECK_PROG(HAVE_SANDBOX, [sandbox], [yes], [no])])
AC_SUBST([HAVE_SANDBOX])
AM_CONDITIONAL([HAVE_SANDBOX], test "x$HAVE_SANDBOX" = "xyes")
if test x"$HAVE_SANDBOX" = "xyes" ; then
	AC_DEFINE([HAVE_SANDBOX], 1, [Do we have sandbox?])
else
	AC_DEFINE([HAVE_SANDBOX], 0)
fi
dnl }}}

dnl {{{ doxygen
AC_MSG_CHECKING([whether to enable doxygen])
AC_ARG_ENABLE([doxygen],
			  AS_HELP_STRING([--enable-doxygen], [Enable 'make doxygen' (developer docs)]),
			  [HAVE_DOXYGEN=$enableval
			   AC_MSG_RESULT([$enableval])],
			  [AC_MSG_RESULT([autodetect])
			   AC_CHECK_PROG(HAVE_DOXYGEN, [doxygen], [yes], [no])])
AC_SUBST([HAVE_DOXYGEN])
AM_CONDITIONAL([HAVE_DOXYGEN], test "x$HAVE_DOXYGEN" = "xyes")
dnl }}}

dnl {{{ check for whether to build qa stuff
AC_MSG_CHECKING([whether to build QA tools])
AC_ARG_ENABLE([qa],
			  AS_HELP_STRING([--enable-qa], [Build QA things (needs pcre++, libxml2)]),
			  [ENABLE_QA=$enableval
			   AC_MSG_RESULT([$enableval])],
			  [ENABLE_QA=no
			   AC_MSG_RESULT([no])])
if test x"$ENABLE_QA" = "xyes" ; then
   AC_CHECK_PROG(HAVE_PRCEPLUSPLUS, [pcre++-config], [yes], [no])
   if test x"$HAVE_PRCEPLUSPLUS" = "xyes" ; then
		PCREPLUSPLUS_CFLAGS=`pcre++-config --cflags`
		AC_SUBST(PCREPLUSPLUS_CFLAGS)
		PCREPLUSPLUS_LIBS=`pcre++-config --libs`
		AC_SUBST(PCREPLUSPLUS_LIBS)
	else
		AC_MSG_ERROR([pcre++ (http://www.daemon.de/PCRE) is required if --enable-qa is used])
	fi
	need_xml_check=yes
	AC_DEFINE([ENABLE_RUBY_QA], [1], [Enable QA support in ruby])
fi
AC_SUBST([ENABLE_QA])
AM_CONDITIONAL([ENABLE_QA], test "x$ENABLE_QA" = "xyes")
dnl }}}

dnl {{{ check for whether to build glsa stuff
AC_MSG_CHECKING([whether to build GLSA support])
AC_ARG_ENABLE([glsa],
			  AS_HELP_STRING([--enable-glsa], [Build GLSA things (needs libxml2)]),
			  [ENABLE_GLSA=$enableval
			   AC_MSG_RESULT([$enableval])],
			  [ENABLE_GLSA=no
			   AC_MSG_RESULT([no])])
if test x"$ENABLE_GLSA" = "xyes" ; then
	need_xml_check=yes
	AC_DEFINE([ENABLE_GLSA], [1], [Build GLSA support])
fi
AC_SUBST([ENABLE_GLSA])
AM_CONDITIONAL([ENABLE_GLSA], test "x$ENABLE_GLSA" = "xyes")
dnl }}}

dnl {{{ libxml2 check
NEED_XML=$need_xml_check
if test "x$need_xml_check" = "xyes" ; then
	PKG_CHECK_MODULES(LIBXML2DEPS, [libxml-2.0 >= 2.6], [],
		[AC_MSG_ERROR([libxml2 (http://xmlsoft.org/) is required if --enable-qa or --enable-glsa is used])])
	AC_SUBST(LIBXML2DEPS_CFLAGS)
	AC_SUBST(LIBXML2DEPS_LIBS)
fi
AC_SUBST([NEED_XML])
AM_CONDITIONAL([NEED_XML], test "x$NEED_XML" = "xyes")
dnl }}}

dnl {{{ colourschemes
AC_MSG_CHECKING([whether we like pink])
AC_ARG_ENABLE([pink],
			  AS_HELP_STRING([--enable-pink], [Make it pink]),
			[WANT_PINK=$enableval
			 AC_MSG_RESULT([$enableval])
			 if test x$enableval = xyes ; then
				 COLOURSCHEME=pink
			 else
				 COLOURSCHEME=default
			 fi
			 ],
			[WANT_PINK=no
			 AC_MSG_RESULT([no :(])
			 COLOURSCHEME=default])
AM_CONDITIONAL([WANT_PINK], test "x$WANT_PINK" = "xyes")
AC_SUBST([COLOURSCHEME])
if test x"$WANT_PINK" = "xyes"; then
	AC_DEFINE([PALUDIS_COLOUR_PINK], 1, [Are we pinkified?])
else
	AC_DEFINE([PALUDIS_COLOUR_PINK], 0)
fi
dnl }}}

dnl {{{ Check for dlopen symbol and set DYNAMIC_LD_LIBS.
dnl
dnl AM_DL()
dnl

AC_CHECK_LIB(c, dlopen,
[DYNAMIC_LD_LIBS=""
have_dl=yes])

if test x$have_dl != "xyes"; then
AC_CHECK_LIB(dl, dlopen,
 [DYNAMIC_LD_LIBS="-ldl"
  have_dl=yes])
fi

if test x$have_dl != "xyes"; then
AC_MSG_ERROR(dynamic linker needed)
fi

AC_SUBST(DYNAMIC_LD_LIBS)

dnl }}}

dnl {{{ selinux support
AC_MSG_CHECKING([whether to enable selinux support])
AC_ARG_ENABLE([selinux],
   AS_HELP_STRING([--enable-selinux], [Enable selinux support (default: check)]),
   enable_selinux=$enableval,
   enable_selinux=autodetect)
AC_MSG_RESULT($enable_selinux)

if test "x$enable_selinux" != "xno"; then
   AC_CHECK_LIB([selinux],
				[is_selinux_enabled],
				found_selinux=yes,
				found_selinux=no)
   if test "x$enable_selinux" = "xyes" && test "x$found_selinux" != "xyes"; then
      AC_MSG_ERROR([SElinux support requested but not found])
   fi
fi

if test "x$found_selinux" = "xyes"; then
   AC_DEFINE([HAVE_SELINUX], [1], [Build selinux support])
fi
dnl }}}

dnl {{{ gtk support
AC_MSG_CHECKING([whether to build the gtk+ client])
AC_ARG_ENABLE([gtk],
   AS_HELP_STRING([--enable-gtk], [Enable gtk+ client (default: disable) (UNUSABLE)]),
   enable_gtk=$enableval,
   enable_gtk=no)
AC_MSG_RESULT($enable_gtk)

if test "x$enable_gtk" = "xyes"; then
	AC_CHECK_FUNCS([posix_openpt], [], [AC_MSG_ERROR([I need posix_openpt])])
	GLIBMM_LIBDIR=`pkg-config --variable=libdir glibmm-2.4`
	GMMPROC_DIR=$GLIBMM_LIBDIR/glibmm-2.4/proc
	AC_SUBST(GMMPROC_DIR)
	GMMPROC=$GMMPROC_DIR/gmmproc
	AC_SUBST(GMMPROC)
	PKG_CHECK_MODULES(GTKDEPS, [gtkmm-2.4 >= 2.6.0 glibmm-2.4 >= 2.6.0 pangomm-1.4 gdkmm-2.4 gthread-2.0 >= 2.6.0 vte >= 0.12.1],
		[], [AC_MSG_ERROR([gtkmm-2.4 and vte-0.12.1 or later are required if --enable-gtk is used])])
	AC_SUBST(GTKDEPS_CFLAGS)
	AC_SUBST(GTKDEPS_LIBS)

	# for vte
	PKG_CHECK_MODULES(PYGTKDEPS, [pygtk-2.0 >= 2.6.0],
		[], [AC_MSG_ERROR([pygtk-2.6 is required if --enable-gtk is used])])

	AC_MSG_CHECKING([whether Gtk::TreeView::set_grid_lines is available])
	old_CXXFLAGS=$CXXFLAGS
	CXXFLAGS="$CXXFLAGS $GTKDEPS_CFLAGS"
	AC_COMPILE_IFELSE([
#include <gtkmm/treeview.h>
int main(int, char *[])
{
	Gtk::TreeViewGridLines g;
	g = Gtk::TREE_VIEW_GRID_LINES_VERTICAL;
}
],
		[have_tree_view_grid_lines=yes],
		[have_tree_view_grid_lines=no])
	CXXFLAGS="$old_CXXFLAGS"
	AC_MSG_RESULT([$have_tree_view_grid_lines])
	if test "x$have_tree_view_grid_lines" = "xyes" ; then
		AC_MSG_CHECKING([whether Gtk::TreeView::set_grid_lines works])
		PKG_CHECK_MODULES(GTKTREEVIEWDEPS, [gtk+-2.0 >= 2.11],
						  [have_tree_view_grid_lines=yes],
						  [have_tree_view_grid_lines=no])
		AC_MSG_RESULT([$have_tree_view_grid_lines])
	fi
	HAVE_TREE_VIEW_GRID_LINES=$have_tree_view_grid_lines
	if test "x$have_tree_view_grid_lines" = "xyes" ; then
		AC_DEFINE_UNQUOTED([HAVE_TREE_VIEW_GRID_LINES], 1, [Do we have Gtk::TreeView::set_grid_lines?])
	fi
	AC_SUBST([HAVE_TREE_VIEW_GRID_LINES])
fi
AM_CONDITIONAL([ENABLE_GTK], test "x$enable_gtk" = "xyes")

enable_gtk_tests=monkey
AC_MSG_CHECKING([whether to build tests for the gtk+ client])
AC_ARG_ENABLE([gtktests],
   AS_HELP_STRING([--enable-gtktests], [Enable tests for the gtk+ client (default: disable)]),
   enable_gtk_tests=$enableval,
   enable_gtk_tests=no)
AC_MSG_RESULT($enable_gtk_tests)
AM_CONDITIONAL([ENABLE_GTK_TESTS], test "x$enable_gtk_tests" = "xyes")
if test "x$enable_gtk_tests" = "xyes"; then
	PKG_CHECK_MODULES(GTKTESTDEPS, [atkmm-1.6 >= 2.6.0 gtkmm-2.4 >= 2.10.0],
		[], [AC_MSG_ERROR([atkmm-2.6 or later and gtkmm-2.10 or later is required if --enable-gtk-tests is used])])
	AC_SUBST(GTKTESTDEPS_CFLAGS)
	AC_SUBST(GTKTESTDEPS_LIBS)
fi
dnl }}}

dnl {{{ ruby interface
AC_ARG_VAR(RUBY,path to ruby interpreter)
AC_MSG_CHECKING([whether to build the ruby interface])
AC_ARG_ENABLE([ruby],
   AS_HELP_STRING([--enable-ruby], [Enable ruby interface (default: disable)]),
   enable_ruby=$enableval,
   enable_ruby=no)
AC_MSG_RESULT($enable_ruby)

if test "x$enable_ruby" = "xyes" ; then
	AC_CHECK_PROGS(RUBY, ruby, )
	if test "x$RUBY" = "x" ; then
		AC_MSG_ERROR([Ruby is required for --enable-ruby])
	fi

	AC_MSG_CHECKING([for Ruby archdir])
	RUBY_ARCHDIR=`$RUBY -r rbconfig -e 'puts Config::CONFIG@<:@"archdir"@:>@'`
	if test "x$RUBY_ARCHDIR" = "x" ; then
		AC_MSG_ERROR([Could not find Ruby archdir])
	fi
	AC_MSG_RESULT([$RUBY_ARCHDIR])
	AC_SUBST([RUBY_ARCHDIR])

	AC_MSG_CHECKING([for Ruby sitearchdir])
	RUBY_SITEARCHDIR=`$RUBY -r rbconfig -e 'puts Config::CONFIG@<:@"sitearchdir"@:>@'`
	if test "x$RUBY_SITEARCHDIR" = "x" ; then
		AC_MSG_ERROR([Could not find Ruby sitearchdir])
	fi
	AC_MSG_RESULT([$RUBY_SITEARCHDIR])
	AC_SUBST([RUBY_SITEARCHDIR])

	AC_MSG_CHECKING([for Ruby soname])
	RUBY_SONAME=`$RUBY -r rbconfig -e 'puts Config::CONFIG@<:@"RUBY_SO_NAME"@:>@'`
	if test "x$RUBY_SONAME" = "x" ; then
		AC_MSG_ERROR([Could not find Ruby soname])
	fi
	AC_MSG_RESULT([$RUBY_SONAME])
	AC_SUBST([RUBY_SONAME])

	AC_ARG_WITH([ruby-install-dir],
		AS_HELP_STRING([--with-ruby-install-dir=dir], [Specify Ruby installation dir]),
		RUBY_INSTALL_DIR=`eval echo $withval`,
		RUBY_INSTALL_DIR=)
	AC_MSG_CHECKING([for Ruby installation dir])
	if test "x$RUBY_INSTALL_DIR" = "x" ; then
		RUBY_INSTALL_DIR=$RUBY_SITEARCHDIR
	fi
	AC_MSG_RESULT([$RUBY_INSTALL_DIR])
	AC_SUBST([RUBY_INSTALL_DIR])
fi

AM_CONDITIONAL([ENABLE_RUBY], test "x$enable_ruby" = "xyes")
dnl }}}

dnl {{{ repositories
ALL_REPOSITORIES="cran fake nothing portage vdb gems virtuals"
DEFAULT_REPOSITORIES="fake nothing portage vdb virtuals"
AC_MSG_CHECKING([which repositories to build...])
AC_ARG_WITH([repositories],
			[  --with-repositories=foo,bar,...
			  Build the specified repositories:
			  default         Equivalent to fake,nothing,portage,vdb,virtuals

			  cran            The Comprehensive R Archive Network
			  fake            A fake repository for test cases
			  gems            Ruby Gems (requires libyaml) (UNUSABLE)
			  nothing         A repository with no content
			  portage         Gentoo Portage
			  vdb             Gentoo installed packages
			  virtuals        Old style Portage virtuals],
			[repositories="`echo $with_repositories | tr ',' ' '`"],
			[repositories="$DEFAULT_REPOSITORIES"])
repositories=`echo $repositories | tr ' ' '\n' | sed -e "s,^default\$,$DEFAULT_REPOSITORIES," | tr ' ' '\n' | sort | uniq | tr '\n' ' '`
AC_MSG_RESULT([$repositories])
BUILD_REPOSITORIES="$repositories"
AC_SUBST([ALL_REPOSITORIES])
AC_SUBST([BUILD_REPOSITORIES])
dnl }}}

dnl {{{ gems
AC_MSG_CHECKING([whether we're building the gems repository])
if echo $repositories | tr ' ' '\n' | sort | uniq | grep '^gems$' >/dev/null ; then
	AC_MSG_RESULT([yes])
	AC_MSG_CHECKING([for libyaml])
	AC_COMPILE_IFELSE([
#include <yaml.h>
int main(int, char *[])
{
	yaml_get_version_string();
	return 0;
}
],
		[AC_MSG_RESULT([yes])],
		[AC_MSG_RESULT([no])
		 AC_MSG_ERROR([libyaml (http://pyyaml.org/wiki/LibYAML) is required for gems support])])
else
	AC_MSG_RESULT([no])
fi
dnl }}}

dnl {{{ clients
ALL_CLIENTS="adjutrix contrarius gtkpaludis paludis qualudis"
DEFAULT_CLIENTS="adjutrix paludis"
if test x"$ENABLE_QA" = "xyes" ; then
	DEFAULT_CLIENTS="$DEFAULT_CLIENTS qualudis"
fi
if test "x$enable_gtk" = "xyes"; then
	DEFAULT_CLIENTS="$DEFAULT_CLIENTS gtkpaludis"
fi
AC_MSG_CHECKING([which clients to build...])
AC_ARG_WITH([clients],
			[  --with-clients=foo,bar,...
			  Build the specified repositories:
			  default         Equivalent to adjutrix,paludis
			                  Also includes qualudis if --enable-qa
			                  Also includes gtkpaludis if --enable-gtk

			  adjutrix        A tool for tree querying
			  contrarius      The cross toolchain creator
			  gtkpaludis      The Gtk+ client (UNUSABLE)
			  paludis         The Paludis console client
			  qualudis        The QA console client],
			[clients="`echo $with_clients | tr ',' ' '`"],
			[clients="$DEFAULT_CLIENTS"])
clients=`echo $clients | tr ' ' '\n' | sed -e "s,^default\$,$DEFAULT_CLIENTS," | tr ' ' '\n' | sort | uniq | tr '\n' ' '`
AC_MSG_RESULT([$clients])
BUILD_CLIENTS="$clients"
AC_SUBST([ALL_CLIENTS])
AC_SUBST([BUILD_CLIENTS])
dnl }}}

dnl {{{ is sed gnu sed

AC_MSG_CHECKING([if we need a sed wrapper])
AC_SUBST([NEED_SED_WRAPPER])
AC_CACHE_VAL(paludis_need_sed_wrapper,[sed --version &>/dev/null || paludis_need_sed_wrapper=yes])
AM_CONDITIONAL([NEED_SED_WRAPPER], test x$paludis_need_sed_wrapper = "xyes")
if test x$paludis_need_sed_wrapper = "xyes"; then
	AC_DEFINE([NEED_SED_WRAPPER], 1, [Do we need sed wrapper?])
	AC_MSG_RESULT([yes])
else
	AC_DEFINE([NEED_SED_WRAPPER], 0)
	AC_MSG_RESULT([no])
fi
dnl }}}

dnl {{{ is awk gnu awk
AC_MSG_CHECKING([if we need an awk wrapper])
AC_SUBST([NEED_AWK_WRAPPER])
AC_CACHE_VAL(paludis_need_awk_wrapper,[awk --version &>/dev/null || paludis_need_awk_wrapper=yes])
AM_CONDITIONAL([NEED_AWK_WRAPPER], test x$paludis_need_awk_wrapper = "xyes")
if test x$paludis_need_awk_wrapper = "xyes"; then
	AC_DEFINE([NEED_AWK_WRAPPER], 1, [Do we need awk wrapper?])
	AC_MSG_RESULT([yes])
else
	AC_DEFINE([NEED_AWK_WRAPPER], 0)
	AC_MSG_RESULT([no])
fi
dnl }}}

dnl {{{ is make gnu make
AC_MSG_CHECKING([if we need a make wrapper])
AC_SUBST([NEED_MAKE_WRAPPER])
AC_CACHE_VAL(paludis_need_make_wrapper,[make --version &>/dev/null || paludis_need_make_wrapper=yes])
AM_CONDITIONAL([NEED_MAKE_WRAPPER], test x$paludis_need_make_wrapper = "xyes")
if test x$paludis_need_make_wrapper = "xyes"; then
	AC_DEFINE([NEED_MAKE_WRAPPER], 1, [Do we need make wrapper?])
	AC_MSG_RESULT([yes])
else
	AC_DEFINE([NEED_MAKE_WRAPPER], 0)
	AC_MSG_RESULT([no])
fi
dnl }}}

dnl {{{ is patch gnu patch
AC_MSG_CHECKING([if we need a patch wrapper])
AC_SUBST([NEED_PATCH_WRAPPER])
AC_CACHE_VAL(paludis_need_patch_wrapper,[patch -g0 < /dev/null &>/dev/null || paludis_need_patch_wrapper=yes])
AM_CONDITIONAL([NEED_PATCH_WRAPPER], test x$paludis_need_patch_wrapper = "xyes")
if test x$paludis_need_patch_wrapper = "xyes"; then
	AC_DEFINE([NEED_PATCH_WRAPPER], 1, [Do we need patch wrapper?])
	AC_MSG_RESULT([yes])
else
	AC_DEFINE([NEED_PATCH_WRAPPER], 0)
	AC_MSG_RESULT([no])
fi
dnl }}}

dnl {{{ built sources
GENERATED_FILE=misc/generated-file.txt
AC_SUBST_FILE(GENERATED_FILE)

BUILDUSER=`whoami`
AC_SUBST([BUILDUSER])
BUILDHOST=`hostname`
AC_SUBST([BUILDHOST])
BUILDDATE=`date +%Y-%m-%dT%H:%M:%S%z`
AC_SUBST([BUILDDATE])
if test -d "${ac_top_srcdir:-./}/.svn" ; then
	SVNVERSION=`svnversion "${ac_top_srcdir:-./}"`
else
	SVNVERSION=
fi
AC_SUBST([SVNVERSION])
dnl }}}

dnl {{{ output
AM_CONFIG_HEADER(config.h)
AC_OUTPUT(
	Makefile
	bash-completion/Makefile
	cran/Makefile
	doc/Makefile
	doc/doxygen.conf
	doc/examples/Makefile
	ebuild/Makefile
	ebuild/digests/Makefile
	ebuild/fetchers/Makefile
	ebuild/syncers/Makefile
	ebuild/utils/Makefile
	ebuild/echo_functions.bash
	ebuild/work_around_broken_utilities.bash
	eselect/Makefile
	hooks/Makefile
	hooks/demos/Makefile
	misc/Makefile
	misc/svn-version-filter-data.bash
	paludis/Makefile
	paludis/about.hh
	paludis/args/Makefile
	paludis/dep_list/Makefile
	paludis/digests/Makefile
	paludis/environment/Makefile
	paludis/environment/default/Makefile
	paludis/environment/no_config/Makefile
	paludis/environment/test/Makefile
	paludis/hashed_containers.hh
	paludis/libxml/Makefile
	paludis/qa/Makefile
	paludis/repositories/Makefile
	paludis/repositories/cran/Makefile
	paludis/repositories/fake/Makefile
	paludis/repositories/gems/Makefile
	paludis/repositories/nothing/Makefile
	paludis/repositories/portage/Makefile
	paludis/repositories/vdb/Makefile
	paludis/repositories/virtuals/Makefile
	paludis/selinux/Makefile
	paludis/tasks/Makefile
	paludis/util/Makefile
	paludis/util/attributes.hh
	ruby/Makefile
	ruby/demos/Makefile
	src/Makefile
	src/output/Makefile
	src/common_args/Makefile
	src/clients/Makefile
	src/clients/contrarius/Makefile
	src/clients/paludis/Makefile
	src/clients/gtkpaludis/Makefile
	src/clients/gtkpaludis/cellrendererbutton/Makefile
	src/clients/gtkpaludis/vtemm/Makefile
	src/clients/gtkpaludis/vtemm/converts/Makefile
	src/clients/gtkpaludis/vtemm/defs/Makefile
	src/clients/qualudis/Makefile
	src/clients/adjutrix/Makefile
	test/Makefile
	zsh-completion/Makefile
	)
dnl }}}

