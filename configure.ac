dnl vim: set sw=4 sts=4 ts=4 noet ft=config foldmethod=marker foldmarker={{{,}}} :

dnl {{{ program, version
AC_PREREQ(2.59)
AC_INIT([paludis/paludis.hh])
AC_CONFIG_AUX_DIR(config)

VERSION_MAJOR=0
VERSION_MINOR=31
VERSION_MICRO=0
VERSION_SUFFIX=
VERSION_FULL="$VERSION_MAJOR.$VERSION_MINOR.$VERSION_MICRO$VERSION_SUFFIX"
VERSION="$VERSION_FULL"
VERSION_LIB_CURRENT=`expr 100 '*' $VERSION_MAJOR + $VERSION_MINOR`
VERSION_LIB_REVISION="$VERSION_MICRO"

AC_SUBST([VERSION_MAJOR])
AC_SUBST([VERSION_MINOR])
AC_SUBST([VERSION_MICRO])
AC_SUBST([VERSION_FULL])
AC_SUBST([VERSION_SUFFIX])
AC_SUBST([VERSION_LIB_CURRENT])
AC_SUBST([VERSION_LIB_REVISION])

AM_INIT_AUTOMAKE(paludis, [$VERSION_FULL])
AC_LANG([C++])

PALUDIS_CXXFLAGS=

dnl {{{ git revision
if test -d "${GIT_DIR:-${ac_top_srcdir:-./}/.git}" ; then
	GITHEAD=`git describe 2>/dev/null`
	if test -z ${GITHEAD} ; then
		GITHEAD=`git rev-parse HEAD`
	fi
	if test -n "`git diff-index -m --name-only HEAD`" ; then
		GITHEAD=${GITHEAD}-dirty
	fi
else
	GITVERSION=
fi
AC_MSG_CHECKING([for git head])
AC_ARG_WITH([git-head],
	AS_HELP_STRING([--with-git-head], [Specify Git head]),
	GITHEAD=`eval echo $withval`,
	GITHEAD=`eval echo $GITHEAD`)
AC_MSG_RESULT([$GITHEAD])
AC_SUBST([GITHEAD])
dnl }}}

PALUDIS_PC_VERSION="${VERSION_FULL}"
PALUDIS_PC_SLOT="${VERSION_MAJOR}.${VERSION_MINOR}"

if [ $((VERSION_MINOR % 2)) == 1 ] ; then
	PALUDIS_PC_SLOT="${PALUDIS_PC_SLOT}_"`printf "%.8s" ${GITHEAD}`
fi

PALUDIS_PC_CFLAGS=
PALUDIS_PC_LIBS="-lpaludis_${PALUDIS_PC_SLOT} -lpaludisutil_${PALUDIS_PC_SLOT} -lpaludisargs_${PALUDIS_PC_SLOT}"
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

dnl {{{ check for byte order
AC_C_BIGENDIAN
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

dnl {{{ check for namespace aliasing
AC_MSG_CHECKING([whether your compiler has referencing issues])
AC_COMPILE_IFELSE([
namespace n
{
	int x = 0;
	int & r_x = x;
}

using n::r_x;
],
	[AC_MSG_RESULT([no])],
	[AC_MSG_RESULT([yes])
	 AC_MSG_ERROR([Your compiler is b0rked (GCC bug 31899?)])])
dnl }}}

dnl {{{ check for whether we want an extra template<> for Implementation
AC_MSG_CHECKING([whether your compiler wants template<> for template specialisations of template classes])
AC_COMPILE_IFELSE([
template <typename T_> struct S;
template <typename T_> struct T;
template <> template <typename T_> struct S<T<T_> > { };
],
	[AC_MSG_RESULT([yes])],
	[AC_MSG_RESULT([no])
	 PALUDIS_CXXFLAGS="${PALUDIS_CXXFLAGS} -DPALUDIS_NO_DOUBLE_TEMPLATE=1"
	 PALUDIS_PC_CFLAGS="${PALUDIS_PC_CFLAGS} -DPALUDIS_NO_DOUBLE_TEMPLATE=1"
	])
dnl }}}

dnl {{{ check whether we do explicit instantiation of fully specialised templates
AC_MSG_CHECKING([whether we should explicitly instantiate fully specialised templates])
AC_COMPILE_IFELSE([
template <typename T_> struct S;
template <> struct S<int> { };
template class S<int>;
],
	[AC_MSG_RESULT([yes])],
	[AC_MSG_RESULT([no])
	 PALUDIS_CXXFLAGS="${PALUDIS_CXXFLAGS} -DPALUDIS_NO_EXPLICIT_FULLY_SPECIALISED=1"
	 PALUDIS_PC_CFLAGS="${PALUDIS_PC_CFLAGS} -DPALUDIS_NO_EXPLICIT_FULLY_SPECIALISED=1"
	])
dnl }}}

dnl {{{ check for static_assert
AC_MSG_CHECKING([for compiler support for static_assert])
AC_COMPILE_IFELSE([
int main(int, char **)
{
	static_assert(true, "works");
}
],
	[AC_MSG_RESULT([yes])
	 AC_MSG_CHECKING([whether static_assert works])
	 AC_COMPILE_IFELSE([
int main(int, char **)
{
	static_assert(false, "works");
}
],
	 [AC_MSG_RESULT([no])],
	 [AC_MSG_RESULT([yes])
	  PALUDIS_CXXFLAGS="${PALUDIS_CXXFLAGS} -DPALUDIS_HAVE_STATIC_ASSERT=1"
	  PALUDIS_PC_CFLAGS="${PALUDIS_PC_CFLAGS} -DPALUDIS_HAVE_STATIC_ASSERT=1"
	 ])],
	[AC_MSG_RESULT([no])])
dnl }}}

dnl {{{ check for concept support
AC_MSG_CHECKING([for compiler support for concepts])
AC_COMPILE_IFELSE([
auto concept Monkeyable<typename T_>
{
	void T_::monkey () const;
};

template <Monkeyable Monkey_>
void monkey_lots(const Monkey_ & m)
{
	for (int x(0) ; x < 10 ; ++x)
		m.monkey();
}

struct Bobo
{
	void monkey() const
	{
	}
};

int main(int, char **)
{
	monkey_lots(Bobo());
}
],
	[AC_MSG_RESULT([yes])
	 PALUDIS_CXXFLAGS="${PALUDIS_CXXFLAGS} -DPALUDIS_HAVE_CONCEPTS=1"
	 PALUDIS_PC_CFLAGS="${PALUDIS_PC_CFLAGS} -DPALUDIS_HAVE_CONCEPTS=1"
	],
	[AC_MSG_RESULT([no])])
dnl }}}

dnl {{{ check for hashes
AC_MSG_CHECKING([for hashed associative containers])
AC_COMPILE_IFELSE([
#include <tr1/unordered_map>
int main(int, char **)
{
	std::tr1::unordered_map<int, int> m;
	m.insert(std::make_pair(1, 2));
}
],
	[AC_MSG_RESULT([std::tr1::unordered_map])],
	[AC_MSG_RESULT([no])
	 AC_MSG_ERROR([Your compiler doesn't support tr1 unordered containers])])
dnl }}}

dnl {{{ tr1 shared pointers
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
	 AC_MSG_ERROR([Your compiler doesn't support tr1 shared pointers])])
dnl }}}

dnl {{{ tr1 type traits
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
	 AC_MSG_ERROR([Your compiler doesn't support tr1 type traits])])
dnl }}}

dnl {{{ tr1 binders
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
	 AC_MSG_ERROR([Your compiler doesn't support tr1 functional])])
dnl }}}

dnl {{{ std::next
AC_MSG_CHECKING([for std::next])
AC_COMPILE_IFELSE([
#include <algorithm>
#include <iterator>
#include <list>
int main(int, char **)
{
	std::list<int> x;
	next(x.begin());
}
],
	[AC_MSG_RESULT([yes])
	 PALUDIS_CXXFLAGS="${PALUDIS_CXXFLAGS} -DPALUDIS_HAVE_STD_NEXT=1"
	 PALUDIS_PC_CFLAGS="${PALUDIS_PC_CFLAGS} -DPALUDIS_HAVE_STD_NEXT=1"
	],
	[AC_MSG_RESULT([no])
	])
dnl }}}

dnl {{{ c++0x default and deleted functions
AC_MSG_CHECKING([for c++0x default and deleted functions])
AC_COMPILE_IFELSE([
struct S
{
	S() = default;
	S & operator= (const S &) = delete;
};
],
	[AC_MSG_RESULT([yes])
	 PALUDIS_CXXFLAGS="${PALUDIS_CXXFLAGS} -DPALUDIS_HAVE_DEFAULT_DELETED=1"
	 PALUDIS_PC_CFLAGS="${PALUDIS_PC_CFLAGS} -DPALUDIS_HAVE_DEFAULT_DELETED=1"
	],
	[AC_MSG_RESULT([no])
	])
dnl }}}

dnl {{{ c++0x enum classes
AC_MSG_CHECKING([for c++0x enum class support])
AC_COMPILE_IFELSE([
enum class C : long { c };
],
	[AC_MSG_RESULT([yes])
	 PALUDIS_CXXFLAGS="${PALUDIS_CXXFLAGS} -DPALUDIS_HAVE_ENUM_CLASS=1"
	 PALUDIS_PC_CFLAGS="${PALUDIS_PC_CFLAGS} -DPALUDIS_HAVE_ENUM_CLASS=1"
	],
	[AC_MSG_RESULT([no])
	])
dnl }}}

need_xml_check=
need_pcrepp_check=
need_syck_check=
need_gem_check=
need_gtkmm_check=

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

dnl {{{ check for listxattrf etc
AC_MSG_CHECKING([for f*xattr function family])
AC_COMPILE_IFELSE([
#include <sys/types.h>
#include <attr/xattr.h>

int main(int, char **)
{
	flistxattr(0, 0, 0);
	fgetxattr(0, 0, 0, 0);
	fsetxattr(0, 0, 0, 0, 0);
}
],
	[AC_MSG_RESULT([yes])
	 HAVE_XATTRS=yes],
	[AC_MSG_RESULT([no])
	 HAVE_XATTRS=])
if test "x$HAVE_XATTRS" = "xyes"; then
    AC_DEFINE([HAVE_XATTRS], [1], [Have xattrs support])
fi
dnl }}}

dnl {{{ check for canonicalize_file_name function
AC_CHECK_FUNCS([canonicalize_file_name])
AM_CONDITIONAL(HAVE_CANONICALIZE_FILE_NAME, test x$ac_cv_func_canonicalize_file_name = xyes)
if test "x$ac_cv_func_canonicalize_file_name" = "xyes"; then
   AC_DEFINE([HAVE_CANONICALIZE_FILE_NAME], [1], [Use canonicalize_file_name])
fi
dnl }}}

dnl {{{ check for dirent.d_type
AC_MSG_CHECKING([for dirent.d_type])
AC_COMPILE_IFELSE([
#include <sys/types.h>
#include <dirent.h>
int main(int, char **)
{
	struct dirent dent;
	dent.d_type = DT_LNK;
}
],
	[have_dirent_dtype=yes],
	[have_dirent_dtype=no])
AC_MSG_RESULT([$have_dirent_dtype])
if test "x$have_dirent_dtype" = "xyes"; then
    AC_DEFINE([HAVE_DIRENT_DTYPE], [1], [Use dirent.d_type])
fi
dnl }}}

dnl {{{ check whether we can do extern template
CAN_USE_EXTERN_TEMPLATE=
AC_MSG_CHECKING([for c++0x 'extern template' support])
AC_COMPILE_IFELSE([
template <typename T_> class S
{
	private:
		S & operator= (const S &);
};
extern template struct S<int>;
template struct S<int>;
],
	[AC_MSG_RESULT([yes])
	 AC_MSG_CHECKING([whether c++0x 'extern template' works as we expect])
	 AC_COMPILE_IFELSE([
template <typename T_> struct B
{
	T_ f();
};
extern template class B<int>;
void f()
{
	B<int> t;
	t.f();
}
],
		[AC_MSG_RESULT([yes])
		 CAN_USE_EXTERN_TEMPLATE=yes
		 PALUDIS_CXXFLAGS="${PALUDIS_CXXFLAGS} -DPALUDIS_HAVE_EXTERN_TEMPLATE=1"
		 PALUDIS_PC_CFLAGS="${PALUDIS_PC_CFLAGS} -DPALUDIS_HAVE_EXTERN_TEMPLATE=1"
		],
		[AC_MSG_RESULT([no, your compiler is weird])
		])
	 ],
	[AC_MSG_RESULT([no])])
dnl }}}

dnl {{{ check for cxxflags
if test x = x"$LET_ME_RICE"
then
	AC_MSG_CHECKING([for go faster stripes])
	for x in "enforce-eh" "fast-math" "rtti" "visibility"
	do
		if echo "${LDFLAGS} ${CFLAGS} ${CXXFLAGS}" | grep "$x" >/dev/null ; then
			flag=`echo "${LDFLAGS} ${CFLAGS} ${CXXFLAGS}" | tr ' ' '\n' | grep $x | tr '\n' ' '`
			AC_MSG_RESULT([$flag])
			AC_MSG_ERROR([Bad ricer. No bagel. Try again with non-broken compiler flags.])
		fi
	done
	AC_MSG_RESULT([no])
fi

AC_MSG_CHECKING([whether to enable threads])
PTHREAD_LIBS=""
AC_ARG_ENABLE([threads],
			  AS_HELP_STRING([--enable-threads], [Enable threads (experimental)]),
			  [ENABLE_THREADS=$enableval],
			  [ENABLE_THREADS=no])
AC_MSG_RESULT([$ENABLE_THREADS])
AC_SUBST([ENABLE_THREADS])
if test "x$ENABLE_THREADS" = "xyes" ; then
	AC_MSG_CHECKING([for compiler __thread support])
	AC_COMPILE_IFELSE([
int main(int, char **)
{
	static __thread int x(0);
	return x;
}
],
	[AC_MSG_RESULT([yes])],
	[AC_MSG_RESULT([no])
	 AC_MSG_ERROR([--enable-threads requires compiler __thread support])])

	AC_MSG_CHECKING([whether -pthread -lpthread gets us posix threads])
	save_CXXFLAGS=$CXXFLAGS
	save_LDFLAGS=$LDFLAGS
	CXXFLAGS="$CXXFLAGS -pthread"
	LDFLAGS="$LDFLAGS -pthread -lpthread"
	AC_LINK_IFELSE([
#include <pthread.h>
int main(int, char **)
{
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	return 0;
}
],
	[AC_MSG_RESULT([yes])],
	[AC_MSG_RESULT([no])
	AC_MSG_ERROR([don't know how to enable pthreads for your compiler])])
	CXXFLAGS=$save_CXXFLAGS
	LDFLAGS=$save_LDFLAGS
	PALUDIS_CXXFLAGS="$PALUDIS_CXXFLAGS -DPALUDIS_ENABLE_THREADS=1 -pthread"
	PALUDIS_PC_CFLAGS="${PALUDIS_PC_CFLAGS} -DPALUDIS_ENABLE_THREADS=1 -pthread"
	PTHREAD_LIBS="-pthread -lpthread"
	PALUDIS_PC_LIBS="${PALUDIS_PC_LIBS} -pthread -lpthread"
	PALUDIS_ENABLE_THREADS=1
else
	PALUDIS_ENABLE_THREADS=0
fi
AC_SUBST([PALUDIS_ENABLE_THREADS])
AC_SUBST([PTHREAD_LIBS])

PALUDIS_CXXFLAGS_NO_WOLD_STYLE_CAST=
PALUDIS_CXXFLAGS_NO_WREDUNDANT_DECLS=
PALUDIS_CXXFLAGS_NO_WSHADOW=
PALUDIS_CXXFLAGS_NO_WUNUSED=
PALUDIS_CXXFLAGS_NO_WEXTRA=
PALUDIS_CXXFLAGS_NO_WWRITE_STRINGS=
PALUDIS_CXXFLAGS_NO_WMISSING_NORETURN=
PALUDIS_CXXFLAGS_WITHOUT_PEDANTIC=
PALUDIS_CXXFLAGS_NO_STRICT_ALIASING=
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
template <typename T_> struct S { };
extern template struct S<int>;
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
	CHECK_CXXFLAG([-wd525])
	CHECK_CXXFLAG([-wd810])
	CHECK_CXXFLAG([-wd981])
	CHECK_CXXFLAG([-wd1125])
	CHECK_CXXFLAG([-wd1418])
	CHECK_CXXFLAG([-wd1419])
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
	if test "x$cxxflag_success" = "xyes" ; then
		PALUDIS_CXXFLAGS_NO_WMISSING_NORETURN=-Wno-missing-noreturn
	fi
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
	CHECK_CXXFLAG([-Wwrite-strings])
	CHECK_CXXFLAG([-Wsignature-shadow])
	CHECK_CXXFLAG([-Wfloat-equal])
	if test "x$cxxflag_success" = "xyes" ; then
		PALUDIS_CXXFLAGS_NO_WWRITE_STRINGS=-Wno-write-strings
	fi
	CHECK_CXXFLAG([-Wno-ignored-qualifiers])
	PALUDIS_CXXFLAGS_WITHOUT_PEDANTIC=$PALUDIS_CXXFLAGS
else
	CHECK_CXXFLAG([-Wall])
	PALUDIS_CXXFLAGS_WITHOUT_PEDANTIC=$PALUDIS_CXXFLAGS
fi
AC_MSG_RESULT([${cxxflags_message}])

AC_MSG_CHECKING([whether -pedantic works if we use extern template])
if test "x${cxx_compiler_icc}" = "xyes" ; then
	AC_MSG_RESULT([no, icc doesn't do pedantic])
else
	if test x$CAN_USE_EXTERN_TEMPLATE = xyes ; then
		save_CXXFLAGS=$CXXFLAGS
		CXXFLAGS="$CXXFLAGS -pedantic"
		AC_COMPILE_IFELSE([
template <typename T_> struct S { };
extern template struct S<int>;
	],
		[AC_MSG_RESULT([yes])
		 CHECK_CXXFLAG([-pedantic])],
		[AC_MSG_RESULT([no])])
	else
		CHECK_CXXFLAG([-pedantic])
	fi
	CXXFLAGS=$save_CXXFLAGS
fi

AC_SUBST([PALUDIS_CXXFLAGS])
AC_SUBST([PALUDIS_CXXFLAGS_WITHOUT_PEDANTIC])
AC_SUBST([PALUDIS_CXXFLAGS_NO_WOLD_STYLE_CAST])
AC_SUBST([PALUDIS_CXXFLAGS_NO_WSHADOW])
AC_SUBST([PALUDIS_CXXFLAGS_NO_WREDUNDANT_DECLS])
AC_SUBST([PALUDIS_CXXFLAGS_NO_WEXTRA])
AC_SUBST([PALUDIS_CXXFLAGS_NO_WUNUSED])
AC_SUBST([PALUDIS_CXXFLAGS_NO_WWRITE_STRINGS])
AC_SUBST([PALUDIS_CXXFLAGS_NO_WMISSING_NORETURN])

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

AC_MSG_CHECKING([how to disable strict aliasing when using dodgy Boost libraries])
old_PALUDIS_CXXFLAGS=$PALUDIS_CXXFLAGS
PALUDIS_CXXFLAGS=
CHECK_CXXFLAG([-fno-strict-aliasing])
AC_MSG_RESULT([$PALUDIS_CXXFLAGS])
PALUDIS_CXXFLAGS_NO_STRICT_ALIASING=$PALUDIS_CXXFLAGS
PALUDIS_CXXFLAGS=$old_PALUDIS_CXXFLAGS
AC_SUBST([PALUDIS_CXXFLAGS_NO_STRICT_ALIASING])
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

AC_MSG_CHECKING([whether to use external Doxygen tags])
AC_ARG_ENABLE([doxygen-tags],
			  AS_HELP_STRING([--enable-doxygen-tags], [Use 'wget' to fetch external Doxygen tags]),
			  [HAVE_DOXYGEN_TAGS=$enableval
			   AC_MSG_RESULT([$enableval])],
			  [AC_MSG_RESULT([no])])
AC_SUBST([HAVE_DOXYGEN_TAGS])
AM_CONDITIONAL([HAVE_DOXYGEN_TAGS], test "x$HAVE_DOXYGEN_TAGS" = "xyes")

if test "x$HAVE_DOXYGEN_TAGS" = xyes ; then
	DOXYGEN_TAG_FILES="\
		\"libstdc++.tag = http://gcc.gnu.org/onlinedocs/libstdc++/latest-doxygen\""
else
	DOXYGEN_TAG_FILES=""
fi
AC_SUBST([DOXYGEN_TAG_FILES])
dnl }}}

dnl {{{
AC_MSG_CHECKING([whether to use htmltidy to verify documentation])
AC_ARG_ENABLE([htmltidy],
			  AS_HELP_STRING([--enable-htmltidy], [Check generated documentation with htmltidy]),
			  [HAVE_HTMLTIDY=$enableval
			   AC_MSG_RESULT([$enableval])],
			  [HAVE_HTMLTIDY=no
			   AC_MSG_RESULT([no])])
AC_SUBST([HAVE_HTMLTIDY])
AM_CONDITIONAL([HAVE_HTMLTIDY], test "x$HAVE_HTMLTIDY" = "xyes")

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
	AC_DEFINE([ENABLE_QA], [1], [Build QA things])
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

dnl {{{ ruby documentation
AC_MSG_CHECKING([whether to build the Ruby documentation])
AC_ARG_ENABLE([ruby-doc],
   AS_HELP_STRING([--enable-ruby-doc], [Enable Ruby documentation, requires --enable-ruby (default: disable)]),
   enable_ruby_doc=$enableval,
   enable_ruby_doc=no)

if test "x$enable_ruby_doc" = "xyes" && test "x$enable_ruby" = "xno"; then
	AC_MSG_ERROR([--enable-ruby-doc requires --enable-ruby])
fi

AC_MSG_RESULT($enable_ruby_doc)

if test "x$enable_ruby_doc" = "xyes" ; then

	AC_MSG_CHECKING([for allison])
	if ruby -rubygems -e "require 'allison'" >/dev/null 2>/dev/null ; then
		AC_MSG_RESULT([yes])
	else
		AC_MSG_ERROR([allison (http://blog.evanweaver.com/files/doc/fauna/allison/files/README.html) is needed to build Ruby documentation])
	fi

	AC_MSG_CHECKING([for syntax])
	if ruby -rubygems -e "require 'syntax/convertors/html'" >/dev/null 2>/dev/null ; then
		AC_MSG_RESULT([yes])
	else
		AC_MSG_ERROR([syntax (http://syntax.rubyforge.org) is needed to build Ruby documentation])
	fi
fi
AM_CONDITIONAL([ENABLE_RUBY_DOC], test "x$enable_ruby_doc" = "xyes")
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

	AC_MSG_CHECKING([whether your gcc is new enough to enable Python])
	AC_COMPILE_IFELSE([
#ifdef __GNUC__
#  if __GNUC__ < 4
#    error too old
#  endif
#endif
	],
	[AC_MSG_RESULT([yes])],
	[AC_MSG_RESULT([no])]
	 AC_MSG_ERROR([Your gcc is too old for Python support]))

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
		AC_MSG_ERROR([boost.python (http://www.boost.org/libs/python/doc/) is required. If you think you have boost.python, \
					  you may need to reinstall it if you have upgraded Python since installing Boost.])])
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
			AC_MSG_ERROR([Can't work out how to link to boost.python. You may need to reinstall boost.python, especially \
						  if you have upgraded Python since installing Boost.])])])
	AC_SUBST(BOOST_PYTHON_LIB)
	dnl Restore the C++ flags
	LIBS="$save_LIBS"
	CXXFLAGS="$save_CXXFLAGS"
	dnl }}}

	AC_DEFINE([ENABLE_PYTHON], [1], [Enable Python])

	dnl {{{ check whether to enable Python Hooks
	AC_MSG_CHECKING([whether to enable Python Hooks])
	BOOST_MIN_VERSION="103400"
	AC_EGREP_CPP(enable,
				 [#include <boost/version.hpp>
				  #if BOOST_VERSION >= $BOOST_MIN_VERSION
				  enable
				  #endif
				  ], enable_python_hooks=yes, enable_python_hooks=no)
	if test $enable_python_hooks = "yes" ; then
		AC_DEFINE([ENABLE_PYTHON_HOOKS], [1], [Enable Python Hooks])
		AC_MSG_RESULT([yes])
	else
		AC_MSG_RESULT([no])
	fi
	dnl }}}
fi
AM_CONDITIONAL([ENABLE_PYTHON], test "x$enable_python" = "xyes")
AM_CONDITIONAL([ENABLE_PYTHON_HOOKS], test "x$enable_python_hooks" = "xyes")
dnl }}}

dnl {{{ python documentation
AC_MSG_CHECKING([whether to build the Python documentation])
AC_ARG_ENABLE([python-doc],
   AS_HELP_STRING([--enable-python-doc], [Enable Python documentation, requires --enable-python (default: disable)]),
   enable_python_doc=$enableval,
   enable_python_doc=no)

if test "x$enable_python_doc" = "xyes" && test "x$enable_python" = "xno"; then
	AC_MSG_ERROR([--enable-python-doc requires --enable-python])
fi

AC_MSG_RESULT($enable_python_doc)

if test "x$enable_python_doc" = "xyes" ; then
	AC_MSG_CHECKING([for epydoc])
	if epydoc --version >/dev/null 2>/dev/null ; then
		AC_MSG_RESULT([yes])
	else
		AC_MSG_ERROR([epydoc (http://epydoc.sourceforge.net/) is needed to build Python documentation])
	fi

	AC_MSG_CHECKING([for pygments])
	if pygmentize -V >/dev/null 2>/dev/null ; then
		AC_MSG_RESULT([yes])
	else
		AC_MSG_ERROR([pygments (http://pygments.org/) is needed to build Python documentation])
	fi
fi
AM_CONDITIONAL([ENABLE_PYTHON_DOC], test "x$enable_python_doc" = "xyes")
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
ALL_REPOSITORIES="cran e fake gems unavailable unpackaged virtuals"
DEFAULT_REPOSITORIES="e fake unavailable unpackaged virtuals"
AC_MSG_CHECKING([which repositories to build...])
AC_ARG_WITH([repositories],
			[  --with-repositories=foo,bar,...
			  Build the specified repositories:
			  all             All available repositories
			  default         Equivalent to e,fake,virtuals

			  cran            The Comprehensive R Archive Network (unusable)
			  e               Ebuild, Ebin, Exheres, VDB etc
			  fake            A fake repository for test cases and adjutrix
			  gems            Ruby gems (unusable)
			  unavailable     Unavailable
			  unpackaged      Unpackaged (for importare)
			  virtuals        Old style Gentoo virtuals],
			[repositories="`echo $with_repositories | tr ',' ' '`"],
			[repositories="$DEFAULT_REPOSITORIES"])
repositories=`echo $repositories | tr ' ' '\n' \
	| sed -e "s,^default\$,$DEFAULT_REPOSITORIES," \
	| sed -e "s,^all\$,$ALL_REPOSITORIES," \
	| tr ' ' '\n' | sort -u | tr '\n' ' '`
AC_MSG_RESULT([$repositories])
BUILD_REPOSITORIES="$repositories"
AC_SUBST([ALL_REPOSITORIES])
AC_SUBST([BUILD_REPOSITORIES])

for r in e fake ; do
	if ! echo $repositories | tr ' ' '\n' | grep "^$r\$" >/dev/null ; then
		AC_MSG_ERROR([Building without $r repository is unsupported])
	fi
done

enable_virtuals_repository=
if echo $repositories | tr ' ' '\n' | grep '^virtuals$' >/dev/null ; then
	AC_DEFINE_UNQUOTED([ENABLE_VIRTUALS_REPOSITORY], [1], [Enable virtuals repository])
	enable_virtuals_repository=yes
fi
AM_CONDITIONAL([ENABLE_VIRTUALS_REPOSITORY], test "x$enable_virtuals_repository" = "xyes")

if echo $repositories | tr ' ' '\n' | grep '^gems$' >/dev/null ; then
	need_syck_check=yes
	need_gem_check=yes
fi
dnl }}}

dnl {{{ environments
ALL_ENVIRONMENTS="adapted no_config paludis portage test"
DEFAULT_ENVIRONMENTS="adapted no_config paludis test"
AC_MSG_CHECKING([which environments to build...])
AC_ARG_WITH([environments],
			[  --with-environments=foo,bar,...
			  Build the specified environments:
			  all             All available environments
			  default         Equivalent to adapted,no_config,paludis,test

			  adapted         An environment adapter, for contrarius etc
			  no_config       Configurationless environment, for adjutrix etc
			  paludis         Standard Paludis environments
			  portage         Portage configuration (dodgy)
			  test            For test cases],
			[environments="`echo $with_environments | tr ',' ' '`"],
			[environments="$DEFAULT_ENVIRONMENTS"])
environments=`echo $environments | tr ' ' '\n' \
	|sed -e "s,^default\$,$DEFAULT_ENVIRONMENTS," \
	|sed -e "s,^all\$,$ALL_ENVIRONMENTS," \
	| tr ' ' '\n' | sort -u | tr '\n' ' '`
AC_MSG_RESULT([$environments])
BUILD_ENVIRONMENTS="$environments"
AC_SUBST([ALL_ENVIRONMENTS])
AC_SUBST([BUILD_ENVIRONMENTS])
dnl }}}

dnl {{{ distribution
AC_MSG_CHECKING([for default distribution])
AC_ARG_WITH([default-distribution],
	AS_HELP_STRING([--with-default-distribution=distro], [Specify default distribution name]),
	DEFAULT_DISTRIBUTION=`eval echo $withval`,
	DEFAULT_DISTRIBUTION=gentoo)
AC_MSG_RESULT([$DEFAULT_DISTRIBUTION])
AC_SUBST([DEFAULT_DISTRIBUTION])
AC_DEFINE_UNQUOTED([DEFAULT_DISTRIBUTION], "$DEFAULT_DISTRIBUTION", [Default distribution name])
dnl }}}

dnl {{{ clients
ALL_CLIENTS="accerso adjutrix contrarius gtkpaludis importare inquisitio instruo paludis qualudis reconcilio"
ALL_CLIENTS_HTML=""
for a in $ALL_CLIENTS ; do
	if test x"$a" = "xgtkpaludis" ; then
		ALL_CLIENTS_HTML="$ALL_CLIENTS_HTML"
	else
		ALL_CLIENTS_HTML="$ALL_CLIENTS_HTML $a.html"
	fi
done
DEFAULT_CLIENTS="adjutrix importare paludis reconcilio"
if test x"$ENABLE_QA" = "xyes" ; then
	DEFAULT_CLIENTS="$DEFAULT_CLIENTS qualudis"
fi
AC_MSG_CHECKING([which clients to build...])
AC_ARG_WITH([clients],
			[  --with-clients=foo,bar,...
			  Build the specified clients:
			  all             All available clients
			  default         Equivalent to adjutrix,paludis
			                  Also includes qualudis if --enable-qa

			  accerso         A fetch / mirror client
			  adjutrix        A tool for tree querying
			  contrarius      The cross toolchain creator
			  gtkpaludis      GUI client (unusable)
			  importare       On-the-fly unpackaged installer
			  inquisitio      A search client
			  instruo         A metadata generation client
			  paludis         The Paludis console client
			  reconcilio      A linkage repair tool
			  qualudis        The QA console client],
			[clients="`echo $with_clients | tr ',' ' '`"],
			[clients="$DEFAULT_CLIENTS"])
clients=`echo $clients | tr ' ' '\n' \
	| sed -e "s,^default\$,$DEFAULT_CLIENTS," \
	| sed -e "s,^all\$,$ALL_CLIENTS," \
	| tr ' ' '\n' | sort -u | tr '\n' ' '`
AC_MSG_RESULT([$clients])
BUILD_CLIENTS="$clients"
AC_SUBST([ALL_CLIENTS])
AC_SUBST([ALL_CLIENTS_HTML])
AC_SUBST([BUILD_CLIENTS])

if echo $clients | tr ' ' '\n' | grep '^inquisitio$' >/dev/null ; then
	need_pcrepp_check=yes
fi
if echo $clients | tr ' ' '\n' | grep '^gtkpaludis$' >/dev/null ; then
	need_gtkmm_check=yes
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

dnl {{{ syck check
if test "x$need_syck_check" = "xyes" ; then
	AC_MSG_CHECKING([for syck])
	AC_COMPILE_IFELSE([
#include <syck.h>
],
	[AC_MSG_RESULT([yes])],
	[AC_MSG_ERROR([syck is required for gems repositories])])
fi
dnl }}}

dnl {{{ gem check
if test "x$need_gem_check" = "xyes" ; then
	AC_MSG_CHECKING([for gem])
	if gem -v 1>/dev/null 2>/dev/null ; then
		AC_MSG_RESULT([yes])
	else
		AC_MSG_ERROR([gem (http://rubygems.org/) is required for gems repositories])
	fi
fi
dnl }}}

dnl {{{ gtkmm check
if test "x$need_gtkmm_check" = "xyes" ; then
	PKG_CHECK_MODULES([gtkmm], [gtkmm-2.4 >= 2.8.0])
	PKG_CHECK_MODULES([gthread], [gthread-2.0 >= 2.12.0])
	PKG_CHECK_MODULES([vte], [vte >= 0.12.1])
	PKG_CHECK_MODULES([pygtk], [pygtk-2.0 >= 2.6.0])
	AC_CHECK_FUNCS([posix_openpt], [], [AC_MSG_ERROR([I need posix_openpt])])
	GLIBMM_LIBDIR=`pkg-config --variable=libdir glibmm-2.4`
	GMMPROC_DIR=$GLIBMM_LIBDIR/glibmm-2.4/proc
	AC_SUBST(GMMPROC_DIR)
	GMMPROC=$GMMPROC_DIR/gmmproc
	AC_SUBST(GMMPROC)
fi
dnl }}}

dnl {{{ check for eselect
AC_MSG_CHECKING([for eselect])
if eselect version >/dev/null 2>/dev/null ; then
	AC_MSG_RESULT([yes])
else
	AC_MSG_RESULT([no])
	AC_MSG_ERROR([eselect is required])
fi

for m in env news ; do
	AC_MSG_CHECKING([for eselect $m])
	if eselect $m version >/dev/null 2>/dev/null ; then
		AC_MSG_RESULT([yes])
	else
		AC_MSG_RESULT([no])
		AC_MSG_ERROR([eselect $m is required])
	fi
done
dnl }}}

dnl {{{ pkg-config
AC_SUBST([PALUDIS_PC_CFLAGS])
AC_SUBST([PALUDIS_PC_LIBS])
AC_SUBST([PALUDIS_PC_VERSION])
AC_SUBST([PALUDIS_PC_SLOT])
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
dnl }}}

dnl {{{ output
AM_CONFIG_HEADER(config.h)
AC_OUTPUT(
	Makefile
	bash-completion/Makefile
	doc/Makefile
	doc/api/Makefile
	doc/api/cplusplus/doxygen.conf
	doc/api/cplusplus/Makefile
	doc/api/cplusplus/examples/Makefile
	doc/api/python/Makefile
	doc/api/ruby/Makefile
	doc/clients/Makefile
	doc/configuration/Makefile
	doc/faq/Makefile
	doc/overview/Makefile
	hooks/Makefile
	hooks/demos/Makefile
	misc/Makefile
	paludis/Makefile
	paludis/about.hh
	paludis/args/Makefile
	paludis/distributions/Makefile
	paludis/distributions/exherbo/Makefile
	paludis/distributions/gentoo/Makefile
	paludis/environments/Makefile
	paludis/environments/adapted/Makefile
	paludis/environments/no_config/Makefile
	paludis/environments/paludis/Makefile
	paludis/environments/portage/Makefile
	paludis/environments/test/Makefile
	paludis/fetchers/Makefile
	paludis/repositories/Makefile
	paludis/repositories/cran/Makefile
	paludis/repositories/cran/cran/Makefile
	paludis/repositories/e/Makefile
	paludis/repositories/e/eapis/Makefile
	paludis/repositories/e/ebuild/0/Makefile
	paludis/repositories/e/ebuild/1/Makefile
	paludis/repositories/e/ebuild/2/Makefile
	paludis/repositories/e/ebuild/Makefile
	paludis/repositories/e/ebuild/exheres-0/Makefile
	paludis/repositories/e/ebuild/kdebuild-1/Makefile
	paludis/repositories/e/ebuild/paludis-1/Makefile
	paludis/repositories/e/ebuild/pbin-1/Makefile
	paludis/repositories/e/ebuild/utils/Makefile
	paludis/repositories/e/ebuild/utils/exheres-0/Makefile
	paludis/repositories/e/ebuild/utils/kdebuild-1/Makefile
	paludis/repositories/e/qa/Makefile
	paludis/repositories/fake/Makefile
	paludis/repositories/gems/Makefile
	paludis/repositories/unavailable/Makefile
	paludis/repositories/unpackaged/Makefile
	paludis/repositories/virtuals/Makefile
	paludis/selinux/Makefile
	paludis/syncers/Makefile
	paludis/util/Makefile
	paludis/util/echo_functions.bash
	pkg-config/Makefile
	python/Makefile
	ruby/Makefile
	ruby/demos/Makefile
	src/Makefile
	src/clients/Makefile
	src/clients/accerso/Makefile
	src/clients/adjutrix/Makefile
	src/clients/contrarius/Makefile
	src/clients/gtkpaludis/Makefile
	src/clients/gtkpaludis/libgtkpaludis/Makefile
	src/clients/gtkpaludis/vtemm/Makefile
	src/clients/gtkpaludis/vtemm/converts/Makefile
	src/clients/gtkpaludis/vtemm/defs/Makefile
	src/clients/importare/Makefile
	src/clients/inquisitio/Makefile
	src/clients/instruo/Makefile
	src/clients/paludis/Makefile
	src/clients/qualudis/Makefile
	src/clients/reconcilio/Makefile
	src/clients/reconcilio/broken_linkage_finder/Makefile
	src/clients/reconcilio/littlelf/Makefile
	src/clients/reconcilio/util/Makefile
	src/output/Makefile
	test/Makefile
	vim/Makefile
	vim/ftdetect/Makefile
	vim/syntax/Makefile
	zsh-completion/Makefile
	)
dnl }}}

