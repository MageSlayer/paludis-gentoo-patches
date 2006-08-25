dnl vim: set sw=4 sts=4 ts=4 noet ft=config foldmethod=marker foldmarker={{{,}}} :

dnl {{{ program, version
AC_PREREQ(2.59)
AC_INIT([paludis/paludis.hh])
AC_CONFIG_AUX_DIR(config)

VERSION_MAJOR=0
VERSION_MINOR=5
VERSION_MICRO=0
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
dnl }}}

dnl {{{ basic toolchain checks
AC_LANG([C++])
AC_DISABLE_STATIC
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

int main(int, char *[])
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
#if LIBWRAPITER_VERSION < 200
#  error libwrapiter 0.2 or later required
#endif
],
	[AC_MSG_RESULT([yes])],
	[AC_MSG_RESULT([no])
	 AC_MSG_ERROR([libwrapiter version must be at least 0.2])])
dnl }}}

dnl {{{ check for various misc functions.
dnl It's probably possible to make many of these optional rather than
dnl hard requirements. If any of these fail for you, send patches.
AC_CHECK_FUNCS([alarm],        [], [AC_MSG_ERROR([I need alarm])])
AC_CHECK_FUNCS([strerror],     [], [AC_MSG_ERROR([I need strerror])])
AC_CHECK_FUNCS([signal],       [], [AC_MSG_ERROR([I need signal])])
AC_CHECK_FUNCS([popen],        [], [AC_MSG_ERROR([I need popen])])
AC_CHECK_FUNCS([posix_openpt], [], [AC_MSG_ERROR([I need posix_openpt])])
AC_CHECK_FUNCS([ptsname],      [], [AC_MSG_ERROR([I need ptsname])])
AC_CHECK_FUNCS([grantpt],      [], [AC_MSG_ERROR([I need grantpt])])
AC_CHECK_FUNCS([unlockpt],     [], [AC_MSG_ERROR([I need unlockpt])])
dnl }}}

dnl {{{ check for cxxflags
AC_MSG_CHECKING([for go faster stripes])
if echo "${LDFLAGS} ${CFLAGS} ${CXXFLAGS}" | grep "as-needed" >/dev/null ; then
	AC_MSG_RESULT([as-needed])
	AC_MSG_ERROR([Bad ricer. No bagel.])
else
	if echo "${LDFLAGS} ${CFLAGS} ${CXXFLAGS}" | grep "visibility" >/dev/null ; then
		AC_MSG_RESULT([visibility])
		AC_MSG_ERROR([Bad ricer. No bagel.])
	else
		AC_MSG_RESULT([no])
	fi
fi

PALUDIS_CXXFLAGS=
PALUDIS_CXXFLAGS_NO_WOLD_STYLE_CAST=
PALUDIS_CXXFLAGS_NO_WREDUNDANT_DECLS=
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
	CHECK_CXXFLAG([-pedantic])
else
	CHECK_CXXFLAG([-Wall])
fi
AC_MSG_RESULT([${cxxflags_message}])
AC_SUBST([PALUDIS_CXXFLAGS])
AC_SUBST([PALUDIS_CXXFLAGS_NO_WOLD_STYLE_CAST])
AC_SUBST([PALUDIS_CXXFLAGS_NO_WREDUNDANT_DECLS])
dnl }}}

dnl {{{ sandbox
AC_MSG_CHECKING([whether to enable sandbox])
AC_ARG_ENABLE([sandbox],
			  [  --enable-sandbox        Enable sandbox],
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

dnl {{{ doxygen, dot
AC_MSG_CHECKING([whether to enable doxygen])
AC_ARG_ENABLE([doxygen],
			  [  --enable-doxygen        Enable 'make doxygen' (developer docs)],
			  [HAVE_DOXYGEN=$enableval
			   AC_MSG_RESULT([$enableval])],
			  [AC_MSG_RESULT([autodetect])
			   AC_CHECK_PROG(HAVE_DOXYGEN, [doxygen], [yes], [no])])
AC_SUBST([HAVE_DOXYGEN])
AM_CONDITIONAL([HAVE_DOXYGEN], test "x$HAVE_DOXYGEN" = "xyes")

AC_MSG_CHECKING([whether to enable dot])
AC_ARG_ENABLE([dot],
			  [  --enable-dot            Enable 'HAVE_DOT' in doxygen.conf],
			[HAVE_DOT=$enableval
			 AC_MSG_RESULT([$enableval])],
			[AC_MSG_RESULT([autodetect])
			 AC_CHECK_PROG(HAVE_DOT, [dot], [yes], [no])])
AC_SUBST([HAVE_DOT])
AM_CONDITIONAL([HAVE_DOT], test "x$HAVE_DOT" = "xyes")
dnl }}}

dnl {{{ check for whether to build qa stuff
AC_MSG_CHECKING([whether to build QA tools])
AC_ARG_ENABLE([qa],
			  [  --enable-qa             Build QA things (needs pcre++)],
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
		AC_MSG_ERROR([pcre++ (http://www.daemon.de/PCRE) is required if --enable-qa is used])])
	fi
fi
AC_SUBST([ENABLE_QA])
AM_CONDITIONAL([ENABLE_QA], test "x$ENABLE_QA" = "xyes")
dnl }}}

dnl {{{ colourschemes
AC_MSG_CHECKING([whether we like pink])
AC_ARG_ENABLE([pink],
			  [ --enable-pink             Make it pink],
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
   AS_HELP_STRING([--enable-gtk], [Enable gtk+ client (default: disable)]),
   enable_gtk=$enableval,
   enable_gtk=no)
AC_MSG_RESULT($enable_gtk)

if test "x$enable_gtk" = "xyes"; then
	GLIBMM_LIBDIR=`pkg-config --variable=libdir glibmm-2.4`
	GMMPROC_DIR=$GLIBMM_LIBDIR/glibmm-2.4/proc
	AC_SUBST(GMMPROC_DIR)
	GMMPROC=$GMMPROC_DIR/gmmproc
	AC_SUBST(GMMPROC)
	PKG_CHECK_MODULES(GTKDEPS, [gtkmm-2.4 >= 2.6.0 glibmm-2.4 >= 2.6.0 pangomm-1.4 gdkmm-2.4 vte >= 0.12.1])
	AC_SUBST(GTKDEPS_CFLAGS)
	AC_SUBST(GTKDEPS_LIBS)
fi

AM_CONDITIONAL([ENABLE_GTK], test "x$enable_gtk" = "xyes")
dnl }}}

dnl {{{ is sed gnu sed
AC_MSG_CHECKING([if we need a sed wrapper])
AC_SUBST([NEED_SED_WRAPPER])
AM_CONDITIONAL([NEED_SED_WRAPPER], test x`sed --version &>/dev/null || echo yes` = "xyes")
if test x`sed --version &>/dev/null || echo yes` = "xyes"; then
	AC_DEFINE([NEED_SED_WRAPPER], 1, [Do we need sed wrapper?])
	AC_MSG_RESULT([yes])
else
	AC_DEFINE([NEED_SED_WRAPPER], 0)
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
	cran/Makefile
	doc/Makefile
	doc/doxygen.conf
	ebuild/Makefile
	ebuild/digests/Makefile
	ebuild/fetchers/Makefile
	ebuild/utils/Makefile
	ebuild/echo_functions.bash
	eselect/Makefile
	hooks/Makefile
	misc/Makefile
	misc/svn-version-filter-data.bash
	paludis/Makefile
	paludis/about.hh
	paludis/args/Makefile
	paludis/digests/Makefile
	paludis/hashed_containers.hh
	paludis/qa/Makefile
	paludis/repositories/Makefile
	paludis/repositories/cran/Makefile
	paludis/repositories/fake/Makefile
	paludis/repositories/nothing/Makefile
	paludis/repositories/portage/Makefile
	paludis/repositories/vdb/Makefile
	paludis/repositories/virtuals/Makefile
	paludis/selinux/Makefile
	paludis/tasks/Makefile
	paludis/util/Makefile
	src/Makefile
	src/paludis/Makefile
	src/gtkpaludis/Makefile
	src/gtkpaludis/vtemm/Makefile
	src/gtkpaludis/vtemm/converts/Makefile
	src/gtkpaludis/vtemm/defs/Makefile
	src/qualudis/Makefile
	src/adjutrix/Makefile
	test/Makefile
	)
dnl }}}

