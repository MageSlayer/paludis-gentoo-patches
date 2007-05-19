dnl vim: set sw=4 sts=4 ts=4 noet ft=config foldmethod=marker foldmarker={{{,}}} :

dnl {{{ program, version
AC_PREREQ(2.59)
AC_INIT([paludis/paludis.hh])
AC_CONFIG_AUX_DIR(config)

VERSION_MAJOR=0
VERSION_MINOR=25
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

dnl {{{ make pkg-config work... dumb macro...
PKG_PROG_PKG_CONFIG([0.9.0])
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

dnl {{{ tr1 shared pointers
need_pointer_wrappers=no
AC_MSG_CHECKING([for std::tr1::shared_ptr<>])
AC_COMPILE_IFELSE([
#include <tr1/memory>
int main(int, char **)
{
	std::tr1::shared_ptr<int> x(new int(10));
	return *x;
}
],
	[AC_MSG_RESULT([yes])],
	[AC_MSG_RESULT([no])
	 AC_MSG_CHECKING([for boost::shared_ptr<>])
	 AC_COMPILE_IFELSE([
#include <boost/shared_ptr.hpp>
int main(int, char **)
{
	boost::shared_ptr<int> x(new int(10));
	return *x;
}
],
	[AC_MSG_RESULT([yes])
	 need_pointer_wrappers=yes],
	[AC_MSG_RESULT([no])
	 AC_MSG_ERROR([Your compiler doesn't support tr1 and boost is not installed])])])
AM_CONDITIONAL([NEED_POINTER_WRAPPERS], test x$need_pointer_wrappers = "xyes")
dnl }}}

dnl {{{ tr1 type traits
need_type_traits_wrappers=no
AC_MSG_CHECKING([for std::tr1::remove_pointer<>])
AC_COMPILE_IFELSE([
#include <tr1/type_traits>
int main(int, char **)
{
	std::tr1::remove_pointer<int *>::type x(10);
	return x;
}
],
	[AC_MSG_RESULT([yes])],
	[AC_MSG_RESULT([no])
	 AC_MSG_CHECKING([for boost::remove_pointer<>])
	 AC_COMPILE_IFELSE([
#include <boost/type_traits.hpp>
int main(int, char **)
{
	boost::remove_pointer<int *>::type x(10);
	return x;
}
],
	[AC_MSG_RESULT([yes])
	 need_type_traits_wrappers=yes],
	[AC_MSG_RESULT([no])
	 AC_MSG_ERROR([Your compiler doesn't support tr1 and boost is not installed])])])
AM_CONDITIONAL([NEED_TYPE_TRAITS_WRAPPERS], test x$need_type_traits_wrappers = "xyes")
dnl }}}

dnl {{{ tr1 binders
need_bind_wrappers=no
AC_MSG_CHECKING([for std::tr1::bind<>])
AC_COMPILE_IFELSE([
#include <tr1/functional>
#include <functional>
#include <algorithm>
#include <list>
struct S
{
	void x(const int) const
	{
	}
};
int main(int, char **)
{
	using namespace std::tr1::placeholders;
	std::list<int> l;
	S s;
	std::for_each(l.begin(), l.end(),
		std::tr1::bind(std::tr1::mem_fn(&S::x), &s, _1));
}
],
	[AC_MSG_RESULT([yes])],
	[AC_MSG_RESULT([no])
	 AC_MSG_CHECKING([for boost::bind<>])
	 AC_COMPILE_IFELSE([
#include <boost/bind.hpp>
#include <boost/mem_fn.hpp>
#include <functional>
#include <algorithm>
#include <list>
struct S
{
	void x(const int) const
	{
	}
};
int main(int, char **)
{
	std::list<int> l;
	S s;
	std::for_each(l.begin(), l.end(),
		boost::bind(boost::mem_fn(&S::x), &s, _1));
}
],
	[AC_MSG_RESULT([yes])
	 need_bind_wrappers=yes],
	[AC_MSG_RESULT([no])
	 AC_MSG_ERROR([Your compiler doesn't support tr1 and boost is not installed])])])
AM_CONDITIONAL([NEED_BIND_WRAPPERS], test x$need_bind_wrappers = "xyes")
dnl }}}

need_xml_check=
need_pcrepp_check=

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

dnl {{{ we can use abi::__cxa_demangle
AC_MSG_CHECKING([for abi::__cxa_demangle])
AC_COMPILE_IFELSE([
#include <cxxabi.h>
int main(int, char **)
{
    int status(0);
    char * const name(abi::__cxa_demangle(
                "_ZN7paludis21CategoryNamePartErrorE", 0, 0, &status));
}
],
	[have_cxa_demangle=yes],
	[have_cxa_demangle=no])
AC_MSG_RESULT([$have_cxa_demangle])
if test "x$have_cxa_demangle" = "xyes"; then
   AC_DEFINE([HAVE_CXA_DEMANGLE], [1], [Include abi::__cxa_demangle support])
fi
dnl}}}

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

dnl {{{ check for lchflags function
AC_CHECK_FUNCS([lchflags])
AM_CONDITIONAL(HAVE_LCHFLAGS, test x$ac_cv_func_lchflags = xyes)
dnl }}}

dnl {{{ check for canonicalize_file_name function
AC_CHECK_FUNCS([canonicalize_file_name])
AM_CONDITIONAL(HAVE_CANONICALIZE_FILE_NAME, test x$ac_cv_func_canonicalize_file_name = xyes)
if test "x$ac_cv_func_canonicalize_file_name" = "xyes"; then
   AC_DEFINE([HAVE_CANONICALIZE_FILE_NAME], [1], [Use canonicalize_file_name])
fi
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
PALUDIS_CXXFLAGS_NO_WUNUSED=
PALUDIS_CXXFLAGS_NO_WEXTRA=
PALUDIS_CXXFLAGS_NO_WWRITE_STRINGS=
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
	else
		PALUDIS_CXXFLAGS_NO_WEXTRA=-Wno-extra
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
	CHECK_CXXFLAG([-Wunused])
	if test "x$cxxflag_success" = "xyes" ; then
		PALUDIS_CXXFLAGS_NO_WUNUSED=-Wno-unused
	fi
	CHECK_CXXFLAG([-Wunused-function])
	CHECK_CXXFLAG([-Wshadow])
	if test "x$cxxflag_success" = "xyes" ; then
		PALUDIS_CXXFLAGS_NO_WSHADOW=-Wno-shadow
	fi
	PALUDIS_CXXFLAGS_WITHOUT_PEDANTIC=$PALUDIS_CXXFLAGS
	CHECK_CXXFLAG([-Wlogical-op])
	CHECK_CXXFLAG([-Wvla])
	CHECK_CXXFLAG([-Wwrite-strings])
	if test "x$cxxflag_success" = "xyes" ; then
		PALUDIS_CXXFLAGS_NO_WWRITE_STRINGS=-Wno-write-strings
	fi
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
AC_SUBST([PALUDIS_CXXFLAGS_NO_WEXTRA])
AC_SUBST([PALUDIS_CXXFLAGS_NO_WUNUSED])
AC_SUBST([PALUDIS_CXXFLAGS_NO_WWRITE_STRINGS])

AC_MSG_CHECKING([whether to enable visibility])
AC_ARG_ENABLE([visibility],
			  AS_HELP_STRING([--enable-visibility], [Enable visibility (g++-4.1)]),
			  [ENABLE_VISIBILITY=$enableval],
			  [ENABLE_VISIBILITY=no])
AC_MSG_RESULT([$ENABLE_VISIBILITY])
AC_SUBST([ENABLE_VISIBILITY])
if test "x$ENABLE_VISIBILITY" = "xyes" ; then
	AC_MSG_CHECKING([for appropriate compiler visibility flags])
	old_PALUDIS_CXXFLAGS=$PALUDIS_CXXFLAGS
	PALUDIS_CXXFLAGS=
	CHECK_CXXFLAG([-fvisibility-inlines-hidden])
	CHECK_CXXFLAG([-fvisibility=hidden])
	AC_MSG_RESULT([$PALUDIS_CXXFLAGS])
	PALUDIS_CXXFLAGS_VISIBILITY="$PALUDIS_CXXFLAGS -DPALUDIS_ENABLE_VISIBILITY=1"
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
	need_pcrepp_check=yes
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

dnl {{{ python interface
AC_ARG_VAR(PYTHON, path to python interpreter)
AC_MSG_CHECKING([whether to build the python interface])
AC_ARG_ENABLE([python],
   AS_HELP_STRING([--enable-python], [Enable python interface (default: disable)]),
   enable_python=$enableval,
   enable_python=no)
AC_MSG_RESULT($enable_python)

if test "x$enable_python" = "xyes" ; then

	AM_PATH_PYTHON([2.3])
	if test "x$PYTHON" = "x" ; then
		AC_MSG_ERROR([Python is required for --enable-python])
	fi
	AC_SUBST([PYTHON_INSTALL_DIR])

	AC_MSG_CHECKING(for headers required to compile python extensions)
	py_prefix=`$PYTHON -c "import sys; print sys.prefix"`
	py_exec_prefix=`$PYTHON -c "import sys; print sys.exec_prefix"`
	PYTHON_INCLUDE_DIR="${py_prefix}/include/python${PYTHON_VERSION}"
	if test "$py_prefix" != "$py_exec_prefix"; then
		PYTHON_INCLUDE_DIR="$PYTHON_INCLUDE_DIR -I${py_exec_prefix}/include/python${PYTHON_VERSION}"
	fi
	AC_MSG_RESULT([$PYTHON_INCLUDE_DIR])
	AC_SUBST(PYTHON_INCLUDE_DIR)

	dnl {{{ check for Python.h
	AC_MSG_CHECKING([for Python.h])
	save_CXXFLAGS="$CXXFLAGS"
	CXXFLAGS="$CXXFLAGS -I$PYTHON_INCLUDE_DIR"
	AC_COMPILE_IFELSE([
	#include <Python.h>
	],
		[AC_MSG_RESULT([yes])],
		[AC_MSG_RESULT([no])
		AC_MSG_ERROR([Python.h not found in $PYTHON_INCLUDE_DIR])])
	dnl Restore the C++ flags
	CXXFLAGS="$save_CXXFLAGS"
	dnl }}}

	AC_ARG_WITH([python-install-dir],
		AS_HELP_STRING([--with-python-install-dir=dir], [Specify Python installation dir]),
		PYTHON_INSTALL_DIR=`eval echo $withval`,
		PYTHON_INSTALL_DIR=)
	AC_MSG_CHECKING([for Python installation dir])
	if test "x$PYTHON_INSTALL_DIR" = "x" ; then
		PYTHON_INSTALL_DIR=$pyexecdir
	fi
	AC_MSG_RESULT([$PYTHON_INSTALL_DIR])

	dnl {{{ we need boost.Python
	AC_MSG_CHECKING([for boost.python])
	save_CXXFLAGS="$CXXFLAGS"
	CXXFLAGS="$CXXFLAGS -I$PYTHON_INCLUDE_DIR"
	AC_COMPILE_IFELSE([
	#include <boost/python.hpp>
	struct Foo {};
	BOOST_PYTHON_MODULE(foo)
	{
		boost::python::class_<Foo> ("Foo");
	}
	],
		[AC_MSG_RESULT([yes])],
		[AC_MSG_RESULT([no])
		AC_MSG_ERROR([boost.python (http://www.boost.org/libs/python/doc/) is required])])
	dnl Restore the C++ flags
	CXXFLAGS="$save_CXXFLAGS"
	dnl }}}

	dnl {{{ check whether we need -lboost_python or -lboost_python-mt
	AC_MSG_CHECKING([for boost.python library name])
	save_CXXFLAGS="$CXXFLAGS"
	save_LIBS="$LIBS"
	CXXFLAGS="$CXXFLAGS -I$PYTHON_INCLUDE_DIR"
	LIBS="$LIBS -lboost_python -lpython$PYTHON_VERSION"
	AC_LINK_IFELSE([
#include <boost/python.hpp>
int main() { boost::python::throw_error_already_set(); return 0; }
],
		[BOOST_PYTHON_LIB="-lboost_python"
		AC_MSG_RESULT([$BOOST_PYTHON_LIB])],
		[LIBS="$save_LIBS -lboost_python-mt -lpython$PYTHON_VERSION"
		AC_LINK_IFELSE([
#include <boost/python.hpp>
int main() { boost::python::throw_error_already_set(); return 0; }
],
			[BOOST_PYTHON_LIB="-lboost_python-mt"
			AC_MSG_RESULT([$BOOST_PYTHON_LIB])],
			[AC_MSG_RESULT([not found])
			AC_MSG_ERROR([boost.python (http://www.boost.org/libs/python/doc/) is required])])])
	AC_SUBST(BOOST_PYTHON_LIB)
	dnl Restore the C++ flags
	LIBS="$save_LIBS"
	CXXFLAGS="$save_CXXFLAGS"
	dnl }}}

fi
AM_CONDITIONAL([ENABLE_PYTHON], test "x$enable_python" = "xyes")
dnl }}}

dnl {{{ vim
AC_MSG_CHECKING([whether to install Vim scripts])
AC_ARG_ENABLE([vim],
			  AS_HELP_STRING([--enable-vim], [Enable Vim extras]),
			  [HAVE_VIM=$enableval
			   AC_MSG_RESULT([$enableval])],
			  [AC_MSG_RESULT([no])
			   HAVE_VIM=no])
VIM_INSTALL_DIR=
if test x$HAVE_VIM = xyes ; then
	AC_MSG_CHECKING([for Vim installation dir])
	AC_ARG_WITH([vim-install-dir],
		AS_HELP_STRING([--with-vim-install-dir=dir], [Specify Vim installation dir]),
		VIM_INSTALL_DIR=`eval echo $withval`,
		VIM_INSTALL_DIR=`eval echo $datadir/vim/vimfiles`)
	AC_MSG_RESULT([$VIM_INSTALL_DIR])
fi
AC_SUBST([HAVE_VIM])
AM_CONDITIONAL([HAVE_VIM], test "x$HAVE_VIM" = "xyes")
AC_SUBST([VIM_INSTALL_DIR])
dnl }}}

dnl {{{ repositories
ALL_REPOSITORIES="cran fake gentoo gems virtuals"
DEFAULT_REPOSITORIES="fake gentoo virtuals"
AC_MSG_CHECKING([which repositories to build...])
AC_ARG_WITH([repositories],
			[  --with-repositories=foo,bar,...
			  Build the specified repositories:
			  default         Equivalent to fake,gentoo,virtuals

			  cran            The Comprehensive R Archive Network
			  fake            A fake repository for test cases and adjutrix
			  gems            Ruby Gems (requires libyaml) (UNUSABLE)
			  gentoo          Gentoo packages (ebuilds, vdb)
			  virtuals        Old style Gentoo virtuals],
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
int main(int, char **)
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

dnl {{{ environments
ALL_ENVIRONMENTS="adapted no_config paludis portage test"
DEFAULT_ENVIRONMENTS="adapted no_config paludis test"
AC_MSG_CHECKING([which environments to build...])
AC_ARG_WITH([environments],
			[  --with-environments=foo,bar,...
			  Build the specified environments:
			  default         Equivalent to adapted,no_config,paludis.test

			  adapted         An environment adapter, for contrarius etc
			  no_config       Configurationless environment, for adjutrix etc
			  paludis         Standard Paludis environments
			  portage         Portage configuration (experimental)
			  test            For test cases],
			[environments="`echo $with_environments | tr ',' ' '`"],
			[environments="$DEFAULT_ENVIRONMENTS"])
environments=`echo $environments | tr ' ' '\n' | sed -e "s,^default\$,$DEFAULT_ENVIRONMENTS," | tr ' ' '\n' | sort | uniq | tr '\n' ' '`
AC_MSG_RESULT([$environments])
BUILD_ENVIRONMENTS="$environments"
AC_SUBST([ALL_ENVIRONMENTS])
AC_SUBST([BUILD_ENVIRONMENTS])
dnl }}}

dnl {{{ clients
ALL_CLIENTS="adjutrix contrarius inquisitio paludis qualudis"
DEFAULT_CLIENTS="adjutrix paludis"
if test x"$ENABLE_QA" = "xyes" ; then
	DEFAULT_CLIENTS="$DEFAULT_CLIENTS qualudis"
fi
AC_MSG_CHECKING([which clients to build...])
AC_ARG_WITH([clients],
			[  --with-clients=foo,bar,...
			  Build the specified repositories:
			  default         Equivalent to adjutrix,paludis
			                  Also includes qualudis if --enable-qa

			  adjutrix        A tool for tree querying
			  contrarius      The cross toolchain creator
			  inquisitio      A search client
			  paludis         The Paludis console client
			  qualudis        The QA console client],
			[clients="`echo $with_clients | tr ',' ' '`"],
			[clients="$DEFAULT_CLIENTS"])
clients=`echo $clients | tr ' ' '\n' | sed -e "s,^default\$,$DEFAULT_CLIENTS," | tr ' ' '\n' | sort | uniq | tr '\n' ' '`
AC_MSG_RESULT([$clients])
BUILD_CLIENTS="$clients"
AC_SUBST([ALL_CLIENTS])
AC_SUBST([BUILD_CLIENTS])

if echo $clients | tr ' ' '\n' | grep '^inquisitio$' >/dev/null ; then
	need_pcrepp_check=yes
fi
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

dnl {{{ pcrepp check
if test "x$need_pcrepp_check" = "xyes" ; then
	AC_CHECK_PROG(HAVE_PRCEPLUSPLUS, [pcre++-config], [yes], [no])
	if test x"$HAVE_PRCEPLUSPLUS" = "xyes" ; then
		PCREPLUSPLUS_CFLAGS=`pcre++-config --cflags`
		AC_SUBST(PCREPLUSPLUS_CFLAGS)
		PCREPLUSPLUS_LIBS=`pcre++-config --libs`
		AC_SUBST(PCREPLUSPLUS_LIBS)
	else
		AC_MSG_ERROR([pcre++ (http://www.daemon.de/PCRE) is required if --enable-qa or --with-clients=inquisitio is used])
	fi
fi
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

dnl {{{ is ldconfig gnu ldconfig
AC_MSG_CHECKING([if ldconfig is GNU])
AC_ARG_ENABLE([gnu-ldconfig],
	AS_HELP_STRING([--enable-gnu-ldconfig], [Use GNU ldconfig syntax (default: autodetect)]),
	[
		HAVE_GNU_LDCONFIG=$enableval
		AC_MSG_RESULT([$HAVE_GNU_LDCONFIG])
	],
	[
		AC_MSG_RESULT([auto-detect])
		AC_MSG_CHECKING([whether ldconfig is GNU])
		if ! type ldconfig &>/dev/null ; then
			AC_MSG_RESULT([don't know])
			AC_MSG_ERROR([ldconfig not found. Use --enable-gnu-ldconfig or --disable-gnu-ldconfig])
		fi
		if ldconfig --version &>/dev/null ; then
			HAVE_GNU_LDCONFIG=yes
			AC_MSG_RESULT([$HAVE_GNU_LDCONFIG])
		else
			HAVE_GNU_LDCONFIG=no
			AC_MSG_RESULT([$HAVE_GNU_LDCONFIG])
		fi
	])
if test "x$HAVE_GNU_LDCONFIG" = "xyes"; then
	AC_DEFINE([HAVE_GNU_LDCONFIG], 1, [Define to 1 if you have GNU 'ldconfig'.])
fi
AC_SUBST([HAVE_GNU_LDCONFIG])
AM_CONDITIONAL([HAVE_GNU_LDCONFIG], test "x$HAVE_GNU_LDCONFIG" = "xyes")
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
	doc/Makefile
	doc/doxygen.conf
	doc/examples/Makefile
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
	paludis/environments/Makefile
	paludis/environments/adapted/Makefile
	paludis/environments/paludis/Makefile
	paludis/environments/portage/Makefile
	paludis/environments/no_config/Makefile
	paludis/environments/test/Makefile
	paludis/fetchers/Makefile
	paludis/hashed_containers.hh
	paludis/merger/Makefile
	paludis/qa/Makefile
	paludis/repositories/Makefile
	paludis/repositories/cran/Makefile
	paludis/repositories/cran/cran/Makefile
	paludis/repositories/fake/Makefile
	paludis/repositories/gems/Makefile
	paludis/repositories/gentoo/Makefile
	paludis/repositories/gentoo/ebuild/Makefile
	paludis/repositories/gentoo/ebuild/digests/Makefile
	paludis/repositories/gentoo/ebuild/utils/Makefile
	paludis/repositories/gentoo/ebuild/echo_functions.bash
	paludis/repositories/gentoo/ebuild/work_around_broken_utilities.bash
	paludis/repositories/virtuals/Makefile
	paludis/selinux/Makefile
	paludis/syncers/Makefile
	paludis/tasks/Makefile
	paludis/util/Makefile
	python/Makefile
	ruby/Makefile
	ruby/demos/Makefile
	src/Makefile
	src/output/Makefile
	src/common_args/Makefile
	src/clients/Makefile
	src/clients/contrarius/Makefile
	src/clients/inquisitio/Makefile
	src/clients/paludis/Makefile
	src/clients/qualudis/Makefile
	src/clients/adjutrix/Makefile
	test/Makefile
	tr1/Makefile
	vim/Makefile
	vim/ftdetect/Makefile
	vim/syntax/Makefile
	zsh-completion/Makefile
	)
dnl }}}

