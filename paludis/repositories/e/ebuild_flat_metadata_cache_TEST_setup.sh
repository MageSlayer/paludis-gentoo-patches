#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir ebuild_flat_metadata_cache_TEST_dir || exit 1
cd ebuild_flat_metadata_cache_TEST_dir || exit 1

mkdir extra_eclasses || exit 1
touch extra_eclasses/bar.eclass || exit 1
TZ=UTC touch -t 197001010003 extra_eclasses/bar.eclass || exit 1

mkdir -p repo/{eclass,exlibs,distfiles,profiles/profile} || exit 1
mkdir -p repo/cat/exlibs || exit 1
mkdir -p repo/metadata/cache/cat
cd repo || exit 1
echo "test-repo" > profiles/repo_name || exit 1
cat <<END > profiles/categories || exit 1
cat
END
cat <<END > profiles/profile/make.defaults
ARCH=test
END
cat <<END > eclass/foo.eclass
DEPEND="cat/baz"
IUSE="quux"
END
cat <<END > exlibs/foo.exlib
DEPENDENCIES="cat/baz"
MYOPTIONS="quux"
END
TZ=UTC touch -t 197001010003 eclass/foo.eclass eclass/bar.eclass || exit 1
TZ=UTC touch -t 197001010003 exlibs/foo.exlib exlibs/bar.exlib || exit 1
TZ=UTC touch -t 197001010003 cat/exlibs/bar.exlib || exit 1

mkdir cat/flat_list
cat <<END > cat/flat_list/flat_list-1.ebuild || exit 1
END
cat <<END > metadata/cache/cat/flat_list-1 || exit 1
the/depend
the/rdepend
the-slot
the-src-uri
the-restrict
the-homepage
the-license
the-description-flat_list
the-keywords

the-iuse
unused
the/pdepend
the/provide
0
END

mkdir cat/flat_list-stale
cat <<END > cat/flat_list-stale/flat_list-stale-1.ebuild || exit 1
DESCRIPTION="The Generated Description flat_list-stale"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
DEPEND=""
END
cat <<END > metadata/cache/cat/flat_list-stale-1 || exit 1
the/depend
the/rdepend
the-slot
the-src-uri
the-restrict
the-homepage
the-license
The Stale Description
the-keywords

the-iuse
unused
the/pdepend
the/provide
0
END
touch -t 199901010101 metadata/cache/cat/flat_list-stale-1 || exit 2

mkdir cat/flat_list-eclass
cat <<END > cat/flat_list-eclass/flat_list-eclass-1.ebuild || exit 1
END
cat <<END > metadata/cache/cat/flat_list-eclass-1 || exit 1
the/depend
the/rdepend
the-slot
the-src-uri
the-restrict
the-homepage
the-license
the-description-flat_list-eclass
the-keywords
foo
the-iuse
unused
the/pdepend
the/provide
0
END

mkdir cat/flat_list-eclass-stale
cat <<END > cat/flat_list-eclass-stale/flat_list-eclass-stale-1.ebuild || exit 1
DESCRIPTION="The Generated Description flat_list-eclass-stale"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
DEPEND=""
END
cat <<END > metadata/cache/cat/flat_list-eclass-stale-1 || exit 1
the/depend
the/rdepend
the-slot
the-src-uri
the-restrict
the-homepage
the-license
The Stale Description
the-keywords
foo
the-iuse
unused
the/pdepend
the/provide
0
END
touch -t 197001010001 cat/flat_list-eclass-stale/flat_list-eclass-stale-1.ebuild || exit 2
touch -t 197001010002 metadata/cache/cat/flat_list-eclass-stale-1 || exit 2

mkdir cat/flat_list-eclass-wrong
cat <<END > cat/flat_list-eclass-wrong/flat_list-eclass-wrong-1.ebuild || exit 1
DESCRIPTION="The Generated Description flat_list-eclass-wrong"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
DEPEND=""
END
cat <<END > metadata/cache/cat/flat_list-eclass-wrong-1 || exit 1
the/depend
the/rdepend
the-slot
the-src-uri
the-restrict
the-homepage
the-license
The Stale Description
the-keywords
bar
the-iuse
unused
the/pdepend
the/provide
0
END

mkdir cat/flat_list-eclass-gone
cat <<END > cat/flat_list-eclass-gone/flat_list-eclass-gone-1.ebuild || exit 1
DESCRIPTION="The Generated Description flat_list-eclass-gone"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
DEPEND=""
END
cat <<END > metadata/cache/cat/flat_list-eclass-gone-1 || exit 1
the/depend
the/rdepend
the-slot
the-src-uri
the-restrict
the-homepage
the-license
The Stale Description
the-keywords
baz
the-iuse
unused
the/pdepend
the/provide
0
END

mkdir cat/flat_list-detection
cat <<END > cat/flat_list-detection/flat_list-detection-1.ebuild || exit 1
END
cat <<END > metadata/cache/cat/flat_list-detection-1 || exit 1
the/package =the/depend-1
the/package =the/rdepend-1
the-slot
the-src-uri
the-restrict
the-homepage
the-license
the-description-flat_list-detection
the-keywords

the-iuse
unused
the/pdepend
the/provide
0
END

mkdir cat/flat_hash
cat <<END > cat/flat_hash/flat_hash-1.ebuild || exit 1
END
cat <<END > metadata/cache/cat/flat_hash-1 || exit 1
_mtime_=60
DEPEND=the/depend
RDEPEND=the/rdepend
SLOT=the-slot
SRC_URI=the-src-uri
RESTRICT=the-restrict
HOMEPAGE=the-homepage
LICENSE=the-license
DESCRIPTION=the-description-flat_hash
KEYWORDS=the-keywords
IUSE=the-iuse
PDEPEND=the/pdepend
PROVIDE=the/provide
EAPI=0
END
TZ=UTC touch -t 197001010001 cat/flat_hash/flat_hash-1.ebuild || exit 2

mkdir cat/flat_hash-empty
cat <<END > cat/flat_hash-empty/flat_hash-empty-1.ebuild || exit 1
DESCRIPTION="The Generated Description flat_hash-empty"
END
cat <<END > metadata/cache/cat/flat_hash-empty-1 || exit 1
_mtime_=60
DEPEND=the/depend
RDEPEND=the/rdepend
SLOT=the-slot
SRC_URI=the-src-uri
RESTRICT=the-restrict
HOMEPAGE=the-homepage
LICENSE=the-license
KEYWORDS=the-keywords
IUSE=the-iuse
PDEPEND=the/pdepend
PROVIDE=the/provide
EAPI=0
END
TZ=UTC touch -t 197001010001 cat/flat_hash-empty/flat_hash-empty-1.ebuild || exit 2

mkdir cat/flat_hash-stale
cat <<END > cat/flat_hash-stale/flat_hash-stale-1.ebuild || exit 1
DESCRIPTION="The Generated Description flat_hash-stale"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
DEPEND=""
END
cat <<END > metadata/cache/cat/flat_hash-stale-1 || exit 1
_mtime_=915152460
DEPEND=the/depend
RDEPEND=the/rdepend
SLOT=the-slot
SRC_URI=the-src-uri
RESTRICT=the-restrict
HOMEPAGE=the-homepage
LICENSE=the-license
DESCRIPTION=The Stale Description
KEYWORDS=the-keywords
IUSE=the-iuse
PDEPEND=the/pdepend
PROVIDE=the/provide
EAPI=0
END

mkdir cat/flat_hash-no-mtime
cat <<END > cat/flat_hash-no-mtime/flat_hash-no-mtime-1.ebuild || exit 1
DESCRIPTION="The Generated Description flat_hash-no-mtime"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
DEPEND=""
END
cat <<END > metadata/cache/cat/flat_hash-no-mtime-1 || exit 1
DEPEND=the/depend
RDEPEND=the/rdepend
SLOT=the-slot
SRC_URI=the-src-uri
RESTRICT=the-restrict
HOMEPAGE=the-homepage
LICENSE=the-license
DESCRIPTION=The Stale Description
KEYWORDS=the-keywords
IUSE=the-iuse
PDEPEND=the/pdepend
PROVIDE=the/provide
EAPI=0
END

mkdir cat/flat_hash-no-eapi
cat <<END > cat/flat_hash-no-eapi/flat_hash-no-eapi-1.ebuild || exit 1
DESCRIPTION="The Generated Description flat_hash-no-eapi"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
DEPEND=""
END
cat <<END > metadata/cache/cat/flat_hash-no-eapi-1 || exit 1
_mtime_=60
DEPEND=the/depend
RDEPEND=the/rdepend
SLOT=the-slot
SRC_URI=the-src-uri
RESTRICT=the-restrict
HOMEPAGE=the-homepage
LICENSE=the-license
DESCRIPTION=The Stale Description
KEYWORDS=the-keywords
IUSE=the-iuse
PDEPEND=the/pdepend
PROVIDE=the/provide
END
TZ=UTC touch -t 197001010001 cat/flat_hash-no-eapi/flat_hash-no-eapi-1.ebuild || exit 2

mkdir cat/flat_hash-duplicate-key
cat <<END > cat/flat_hash-duplicate-key/flat_hash-duplicate-key-1.ebuild || exit 1
DESCRIPTION="The Generated Description flat_hash-duplicate-key"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
DEPEND=""
END
cat <<END > metadata/cache/cat/flat_hash-duplicate-key-1 || exit 1
_mtime_=60
DEPEND=the/depend
RDEPEND=the/rdepend
SLOT=the-slot
SRC_URI=the-src-uri
RESTRICT=the-restrict
HOMEPAGE=the-homepage
LICENSE=the-license
DESCRIPTION=The Stale Description
KEYWORDS=the-keywords
IUSE=the-iuse
PDEPEND=the/pdepend
PROVIDE=the/provide
EAPI=0
FOO=bar
FOO=bar
END
TZ=UTC touch -t 197001010001 cat/flat_hash-duplicate-key/flat_hash-duplicate-key-1.ebuild || exit 2

mkdir cat/flat_hash-eclass
cat <<END > cat/flat_hash-eclass/flat_hash-eclass-1.ebuild || exit 1
END
cat <<END > metadata/cache/cat/flat_hash-eclass-1 || exit 1
_mtime_=60
_eclasses_=foo	180
DEPEND=the/depend
RDEPEND=the/rdepend
SLOT=the-slot
SRC_URI=the-src-uri
RESTRICT=the-restrict
HOMEPAGE=the-homepage
LICENSE=the-license
DESCRIPTION=the-description-flat_hash-eclass
KEYWORDS=the-keywords
IUSE=the-iuse
PDEPEND=the/pdepend
PROVIDE=the/provide
EAPI=0
END
TZ=UTC touch -t 197001010001 cat/flat_hash-eclass/flat_hash-eclass-1.ebuild || exit 2

mkdir cat/flat_hash-eclass-stale
cat <<END > cat/flat_hash-eclass-stale/flat_hash-eclass-stale-1.ebuild || exit 1
DESCRIPTION="The Generated Description flat_hash-eclass-stale"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
DEPEND=""
END
cat <<END > metadata/cache/cat/flat_hash-eclass-stale-1 || exit 1
_mtime_=60
_eclasses_=foo	120
DEPEND=the/depend
RDEPEND=the/rdepend
SLOT=the-slot
SRC_URI=the-src-uri
RESTRICT=the-restrict
HOMEPAGE=the-homepage
LICENSE=the-license
DESCRIPTION=The Stale Description
KEYWORDS=the-keywords
IUSE=the-iuse
PDEPEND=the/pdepend
PROVIDE=the/provide
EAPI=0
END
TZ=UTC touch -t 197001010001 cat/flat_hash-eclass-stale/flat_hash-eclass-stale-1.ebuild || exit 2

mkdir cat/flat_hash-eclass-wrong
cat <<END > cat/flat_hash-eclass-wrong/flat_hash-eclass-wrong-1.ebuild || exit 1
DESCRIPTION="The Generated Description flat_hash-eclass-wrong"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
DEPEND=""
END
cat <<END > metadata/cache/cat/flat_hash-eclass-wrong-1 || exit 1
_mtime_=60
_eclasses_=bar	180
DEPEND=the/depend
RDEPEND=the/rdepend
SLOT=the-slot
SRC_URI=the-src-uri
RESTRICT=the-restrict
HOMEPAGE=the-homepage
LICENSE=the-license
DESCRIPTION=The Stale Description
KEYWORDS=the-keywords
IUSE=the-iuse
PDEPEND=the/pdepend
PROVIDE=the/provide
EAPI=0
END
TZ=UTC touch -t 197001010001 cat/flat_hash-eclass-wrong/flat_hash-eclass-wrong-1.ebuild || exit 2

mkdir cat/flat_hash-eclass-gone
cat <<END > cat/flat_hash-eclass-gone/flat_hash-eclass-gone-1.ebuild || exit 1
DESCRIPTION="The Generated Description flat_hash-eclass-gone"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
DEPEND=""
END
cat <<END > metadata/cache/cat/flat_hash-eclass-gone-1 || exit 1
_mtime_=60
_eclasses_=baz	180
DEPEND=the/depend
RDEPEND=the/rdepend
SLOT=the-slot
SRC_URI=the-src-uri
RESTRICT=the-restrict
HOMEPAGE=the-homepage
LICENSE=the-license
DESCRIPTION=The Stale Description
KEYWORDS=the-keywords
IUSE=the-iuse
PDEPEND=the/pdepend
PROVIDE=the/provide
EAPI=0
END
TZ=UTC touch -t 197001010001 cat/flat_hash-eclass-gone/flat_hash-eclass-gone-1.ebuild || exit 2

mkdir cat/flat_hash-full-eclass
cat <<END > cat/flat_hash-full-eclass/flat_hash-full-eclass-1.ebuild || exit 1
END
cat <<END > metadata/cache/cat/flat_hash-full-eclass-1 || exit 1
_mtime_=60
_eclasses_=foo	ebuild_flat_metadata_cache_TEST_dir/repo/eclass	180
DEPEND=the/depend
RDEPEND=the/rdepend
SLOT=the-slot
SRC_URI=the-src-uri
RESTRICT=the-restrict
HOMEPAGE=the-homepage
LICENSE=the-license
DESCRIPTION=the-description-flat_hash-full-eclass
KEYWORDS=the-keywords
IUSE=the-iuse
PDEPEND=the/pdepend
PROVIDE=the/provide
EAPI=0
END
TZ=UTC touch -t 197001010001 cat/flat_hash-full-eclass/flat_hash-full-eclass-1.ebuild || exit 2

mkdir cat/flat_hash-full-eclass-nonstandard
cat <<END > cat/flat_hash-full-eclass-nonstandard/flat_hash-full-eclass-nonstandard-1.ebuild || exit 1
END
cat <<END > metadata/cache/cat/flat_hash-full-eclass-nonstandard-1 || exit 1
_mtime_=60
_eclasses_=foo	ebuild_flat_metadata_cache_TEST_dir/repo/eclass	180	bar	ebuild_flat_metadata_cache_TEST_dir/extra_eclasses	180
DEPEND=the/depend
RDEPEND=the/rdepend
SLOT=the-slot
SRC_URI=the-src-uri
RESTRICT=the-restrict
HOMEPAGE=the-homepage
LICENSE=the-license
DESCRIPTION=the-description-flat_hash-full-eclass-nonstandard
KEYWORDS=the-keywords
IUSE=the-iuse
PDEPEND=the/pdepend
PROVIDE=the/provide
EAPI=0
END
TZ=UTC touch -t 197001010001 cat/flat_hash-full-eclass-nonstandard/flat_hash-full-eclass-nonstandard-1.ebuild || exit 2

mkdir cat/flat_hash-full-eclass-stale
cat <<END > cat/flat_hash-full-eclass-stale/flat_hash-full-eclass-stale-1.ebuild || exit 1
DESCRIPTION="The Generated Description flat_hash-full-eclass-stale"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
DEPEND=""
END
cat <<END > metadata/cache/cat/flat_hash-full-eclass-stale-1 || exit 1
_mtime_=60
_eclasses_=foo	ebuild_flat_metadata_cache_TEST_dir/repo/eclass	120
DEPEND=the/depend
RDEPEND=the/rdepend
SLOT=the-slot
SRC_URI=the-src-uri
RESTRICT=the-restrict
HOMEPAGE=the-homepage
LICENSE=the-license
DESCRIPTION=The Stale Description
KEYWORDS=the-keywords
IUSE=the-iuse
PDEPEND=the/pdepend
PROVIDE=the/provide
EAPI=0
END
TZ=UTC touch -t 197001010001 cat/flat_hash-full-eclass-stale/flat_hash-full-eclass-stale-1.ebuild || exit 2

mkdir cat/flat_hash-full-eclass-wrong
cat <<END > cat/flat_hash-full-eclass-wrong/flat_hash-full-eclass-wrong-1.ebuild || exit 1
DESCRIPTION="The Generated Description flat_hash-full-eclass-wrong"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
DEPEND=""
END
cat <<END > metadata/cache/cat/flat_hash-full-eclass-wrong-1 || exit 1
_mtime_=60
_eclasses_=foo	ebuild_flat_metadata_cache_TEST_dir/repo/eclass	180	bar	ebuild_flat_metadata_cache_TEST_dir/repo/eclass	180
DEPEND=the/depend
RDEPEND=the/rdepend
SLOT=the-slot
SRC_URI=the-src-uri
RESTRICT=the-restrict
HOMEPAGE=the-homepage
LICENSE=the-license
DESCRIPTION=The Stale Description
KEYWORDS=the-keywords
IUSE=the-iuse
PDEPEND=the/pdepend
PROVIDE=the/provide
EAPI=0
END
TZ=UTC touch -t 197001010001 cat/flat_hash-full-eclass-wrong/flat_hash-full-eclass-wrong-1.ebuild || exit 2

mkdir cat/flat_hash-full-eclass-gone
cat <<END > cat/flat_hash-full-eclass-gone/flat_hash-full-eclass-gone-1.ebuild || exit 1
DESCRIPTION="The Generated Description flat_hash-full-eclass-gone"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
DEPEND=""
END
cat <<END > metadata/cache/cat/flat_hash-full-eclass-gone-1 || exit 1
_mtime_=60
_eclasses_=baz	ebuild_flat_metadata_cache_TEST_dir/repo/eclass	180
DEPEND=the/depend
RDEPEND=the/rdepend
SLOT=the-slot
SRC_URI=the-src-uri
RESTRICT=the-restrict
HOMEPAGE=the-homepage
LICENSE=the-license
DESCRIPTION=The Stale Description
KEYWORDS=the-keywords
IUSE=the-iuse
PDEPEND=the/pdepend
PROVIDE=the/provide
EAPI=0
END
TZ=UTC touch -t 197001010001 cat/flat_hash-full-eclass-gone/flat_hash-full-eclass-gone-1.ebuild || exit 2

mkdir cat/flat_hash-eclasses-truncated
cat <<END > cat/flat_hash-eclasses-truncated/flat_hash-eclasses-truncated-1.ebuild || exit 1
DESCRIPTION="The Generated Description flat_hash-eclasses-truncated"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
DEPEND=""
END
cat <<END > metadata/cache/cat/flat_hash-eclasses-truncated-1 || exit 1
_mtime_=60
_eclasses_=foo	ebuild_flat_metadata_cache_TEST_dir/repo/eclass
DEPEND=the/depend
RDEPEND=the/rdepend
SLOT=the-slot
SRC_URI=the-src-uri
RESTRICT=the-restrict
HOMEPAGE=the-homepage
LICENSE=the-license
DESCRIPTION=The Stale Description
KEYWORDS=the-keywords
IUSE=the-iuse
PDEPEND=the/pdepend
PROVIDE=the/provide
EAPI=0
END
TZ=UTC touch -t 197001010001 cat/flat_hash-eclasses-truncated/flat_hash-eclasses-truncated-1.ebuild || exit 2
cat <<END > cat/flat_hash-eclasses-truncated/flat_hash-eclasses-truncated-2.ebuild || exit 1
DESCRIPTION="The Generated Description flat_hash-eclasses-truncated-2"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
DEPEND=""
END
cat <<END > metadata/cache/cat/flat_hash-eclasses-truncated-2 || exit 1
_mtime_=60
_eclasses_=foo
DEPEND=the/depend
RDEPEND=the/rdepend
SLOT=the-slot
SRC_URI=the-src-uri
RESTRICT=the-restrict
HOMEPAGE=the-homepage
LICENSE=the-license
DESCRIPTION=The Stale Description
KEYWORDS=the-keywords
IUSE=the-iuse
PDEPEND=the/pdepend
PROVIDE=the/provide
EAPI=0
END
TZ=UTC touch -t 197001010001 cat/flat_hash-eclasses-truncated/flat_hash-eclasses-truncated-2.ebuild || exit 2

mkdir cat/flat_hash-eclasses-bad-mtime
cat <<END > cat/flat_hash-eclasses-bad-mtime/flat_hash-eclasses-bad-mtime-1.ebuild || exit 1
DESCRIPTION="The Generated Description flat_hash-eclasses-bad-mtime"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
DEPEND=""
END
cat <<END > metadata/cache/cat/flat_hash-eclasses-bad-mtime-1 || exit 1
_mtime_=60
_eclasses_=foo	bar
DEPEND=the/depend
RDEPEND=the/rdepend
SLOT=the-slot
SRC_URI=the-src-uri
RESTRICT=the-restrict
HOMEPAGE=the-homepage
LICENSE=the-license
DESCRIPTION=The Stale Description
KEYWORDS=the-keywords
IUSE=the-iuse
PDEPEND=the/pdepend
PROVIDE=the/provide
EAPI=0
END
TZ=UTC touch -t 197001010001 cat/flat_hash-eclasses-bad-mtime/flat_hash-eclasses-bad-mtime-1.ebuild || exit 2

mkdir cat/flat_hash-eclasses-spaces
cat <<END > cat/flat_hash-eclasses-spaces/flat_hash-eclasses-spaces-1.ebuild || exit 1
DESCRIPTION="The Generated Description flat_hash-eclasses-spaces"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
DEPEND=""
END
cat <<END > metadata/cache/cat/flat_hash-eclasses-spaces-1 || exit 1
_mtime_=60
_eclasses_=foo 180
DEPEND=the/depend
RDEPEND=the/rdepend
SLOT=the-slot
SRC_URI=the-src-uri
RESTRICT=the-restrict
HOMEPAGE=the-homepage
LICENSE=the-license
DESCRIPTION=The Stale Description
KEYWORDS=the-keywords
IUSE=the-iuse
PDEPEND=the/pdepend
PROVIDE=the/provide
EAPI=0
END
TZ=UTC touch -t 197001010001 cat/flat_hash-eclasses-spaces/flat_hash-eclasses-spaces-1.ebuild || exit 2

mkdir cat/flat_hash-exlib
cat <<END > cat/flat_hash-exlib/flat_hash-exlib-1.ebuild || exit 1
END
cat <<END > metadata/cache/cat/flat_hash-exlib-1 || exit 1
_mtime_=60
_exlibs_=foo	ebuild_flat_metadata_cache_TEST_dir/repo/exlibs	180
DEPENDENCIES=the/depend
SLOT=the-slot
SRC_URI=the-src-uri
RESTRICT=the-restrict
HOMEPAGE=the-homepage
LICENSE=the-license
SUMMARY=the-description-flat_hash-exlib
PLATFORMS=the-keywords
MYOPTIONS=the-iuse
EAPI=exheres-0
END
TZ=UTC touch -t 197001010001 cat/flat_hash-exlib/flat_hash-exlib-1.ebuild || exit 2

mkdir cat/flat_hash-exlib-percat
cat <<END > cat/flat_hash-exlib-percat/flat_hash-exlib-percat-1.ebuild || exit 1
END
cat <<END > metadata/cache/cat/flat_hash-exlib-percat-1 || exit 1
_mtime_=60
_exlibs_=foo	ebuild_flat_metadata_cache_TEST_dir/repo/exlibs	180	bar	ebuild_flat_metadata_cache_TEST_dir/repo/cat/exlibs	180
DEPENDENCIES=the/depend
SLOT=the-slot
SRC_URI=the-src-uri
RESTRICT=the-restrict
HOMEPAGE=the-homepage
LICENSE=the-license
SUMMARY=the-description-flat_hash-exlib-percat
PLATFORMS=the-keywords
MYOPTIONS=the-iuse
EAPI=exheres-0
END
TZ=UTC touch -t 197001010001 cat/flat_hash-exlib-percat/flat_hash-exlib-percat-1.ebuild || exit 2

mkdir cat/flat_hash-exlib-stale
cat <<END > cat/flat_hash-exlib-stale/flat_hash-exlib-stale-1.ebuild || exit 1
SUMMARY="The Generated Description flat_hash-exlib-stale"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
MYOPTIONS=""
LICENSE="GPL-2"
PLATFORMS="test"
DEPENDENCIES=""
END
cat <<END > metadata/cache/cat/flat_hash-exlib-stale-1 || exit 1
_mtime_=60
_exlibs_=foo	ebuild_flat_metadata_cache_TEST_dir/repo/exlibs	120
DEPENDENCIES=the/depend
SLOT=the-slot
SRC_URI=the-src-uri
RESTRICT=the-restrict
HOMEPAGE=the-homepage
LICENSE=the-license
SUMMARY=The Stale Description
PLATFORMS=the-keywords
MYOPTIONS=the-iuse
EAPI=exheres-0
END
TZ=UTC touch -t 197001010001 cat/flat_hash-exlib-stale/flat_hash-exlib-stale-1.ebuild || exit 2

mkdir cat/flat_hash-exlib-wrong
cat <<END > cat/flat_hash-exlib-wrong/flat_hash-exlib-wrong-1.ebuild || exit 1
SUMMARY="The Generated Description flat_hash-exlib-wrong"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
MYOPTIONS=""
LICENSE="GPL-2"
PLATFORMS="test"
DEPENDENCIES=""
END
cat <<END > metadata/cache/cat/flat_hash-exlib-wrong-1 || exit 1
_mtime_=60
_exlibs_=foo	ebuild_flat_metadata_cache_TEST_dir/repo/exlibs	180	bar	ebuild_flat_metadata_cache_TEST_dir/repo/exlibs	180
DEPENDENCIES=the/depend
SLOT=the-slot
SRC_URI=the-src-uri
RESTRICT=the-restrict
HOMEPAGE=the-homepage
LICENSE=the-license
SUMMARY=The Stale Description
PLATFORMS=the-keywords
MYOPTIONS=the-iuse
EAPI=exheres-0
END
TZ=UTC touch -t 197001010001 cat/flat_hash-exlib-wrong/flat_hash-exlib-wrong-1.ebuild || exit 2

mkdir cat/flat_hash-exlib-gone
cat <<END > cat/flat_hash-exlib-gone/flat_hash-exlib-gone-1.ebuild || exit 1
SUMMARY="The Generated Description flat_hash-exlib-gone"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
MYOPTIONS=""
LICENSE="GPL-2"
PLATFORMS="test"
DEPENDENCIES=""
END
cat <<END > metadata/cache/cat/flat_hash-exlib-gone-1 || exit 1
_mtime_=60
_exlibs_=baz	ebuild_flat_metadata_cache_TEST_dir/repo/exlibs	180
DEPENDENCIES=the/depend
SLOT=the-slot
SRC_URI=the-src-uri
RESTRICT=the-restrict
HOMEPAGE=the-homepage
LICENSE=the-license
SUMMARY=The Stale Description
PLATFORMS=the-keywords
MYOPTIONS=the-iuse
EAPI=exheres-0
END
TZ=UTC touch -t 197001010001 cat/flat_hash-exlib-gone/flat_hash-exlib-gone-1.ebuild || exit 2

mkdir cat/flat_hash-exlibs-truncated
cat <<END > cat/flat_hash-exlibs-truncated/flat_hash-exlibs-truncated-1.ebuild || exit 1
SUMMARY="The Generated Description flat_hash-exlibs-truncated"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
MYOPTIONS=""
LICENSE="GPL-2"
PLATFORMS="test"
DEPENDENCIES=""
END
cat <<END > metadata/cache/cat/flat_hash-exlibs-truncated-1 || exit 1
_mtime_=60
_exlibs_=foo	ebuild_flat_metadata_cache_TEST_dir/repo/exlibs
DEPENDENCIES=the/depend
SLOT=the-slot
SRC_URI=the-src-uri
RESTRICT=the-restrict
HOMEPAGE=the-homepage
LICENSE=the-license
SUMMARY=The Stale Description
PLATFORMS=the-keywords
MYOPTIONS=the-iuse
EAPI=exheres-0
END
TZ=UTC touch -t 197001010001 cat/flat_hash-exlibs-truncated/flat_hash-exlibs-truncated-1.ebuild || exit 2
cat <<END > cat/flat_hash-exlibs-truncated/flat_hash-exlibs-truncated-2.ebuild || exit 1
SUMMARY="The Generated Description flat_hash-exlibs-truncated-2"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
MYOPTIONS=""
LICENSE="GPL-2"
PLATFORMS="test"
DEPENDENCIES=""
END
cat <<END > metadata/cache/cat/flat_hash-exlibs-truncated-2 || exit 1
_mtime_=60
_exlibs_=foo
DEPENDENCIES=the/depend
SLOT=the-slot
SRC_URI=the-src-uri
RESTRICT=the-restrict
HOMEPAGE=the-homepage
LICENSE=the-license
SUMMARY=The Stale Description
PLATFORMS=the-keywords
MYOPTIONS=the-iuse
EAPI=exheres-0
END
TZ=UTC touch -t 197001010001 cat/flat_hash-exlibs-truncated/flat_hash-exlibs-truncated-2.ebuild || exit 2

mkdir cat/flat_hash-exlibs-bad-mtime
cat <<END > cat/flat_hash-exlibs-bad-mtime/flat_hash-exlibs-bad-mtime-1.ebuild || exit 1
SUMMARY="The Generated Description flat_hash-exlibs-bad-mtime"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
MYOPTIONS=""
LICENSE="GPL-2"
PLATFORMS="test"
DEPENDENCIES=""
END
cat <<END > metadata/cache/cat/flat_hash-exlibs-bad-mtime-1 || exit 1
_mtime_=60
_exlibs_=foo	bar
DEPENDENCIES=the/depend
SLOT=the-slot
SRC_URI=the-src-uri
RESTRICT=the-restrict
HOMEPAGE=the-homepage
LICENSE=the-license
SUMMARY=The Stale Description
PLATFORMS=the-keywords
MYOPTIONS=the-iuse
EAPI=exheres-0
END
TZ=UTC touch -t 197001010001 cat/flat_hash-exlibs-bad-mtime/flat_hash-exlibs-bad-mtime-1.ebuild || exit 2

mkdir cat/flat_hash-exlibs-spaces
cat <<END > cat/flat_hash-exlibs-spaces/flat_hash-exlibs-spaces-1.ebuild || exit 1
SUMMARY="The Generated Description flat_hash-exlibs-spaces"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
MYOPTIONS=""
LICENSE="GPL-2"
PLATFORMS="test"
DEPENDENCIES=""
END
cat <<END > metadata/cache/cat/flat_hash-exlibs-spaces-1 || exit 1
_mtime_=60
_exlibs_=foo 180
DEPENDENCIES=the/depend
SLOT=the-slot
SRC_URI=the-src-uri
RESTRICT=the-restrict
HOMEPAGE=the-homepage
LICENSE=the-license
SUMMARY=The Stale Description
PLATFORMS=the-keywords
MYOPTIONS=the-iuse
EAPI=exheres-0
END
TZ=UTC touch -t 197001010001 cat/flat_hash-exlibs-spaces/flat_hash-exlibs-spaces-1.ebuild || exit 2

mkdir cat/write
cat <<END > cat/write/write-1.ebuild || exit 1
DESCRIPTION="A nice package"
HOMEPAGE="http://example.com/"
SRC_URI=""
LICENSE="GPL-2"
SLOT="0"
KEYWORDS="test"
IUSE="bar"
DEPEND="cat/foo bar? ( cat/bar )"
END
TZ=UTC touch -t 197001010001 cat/write/write-1.ebuild || exit 2

mkdir cat/write-eclasses
cat <<END > cat/write-eclasses/write-eclasses-1.ebuild || exit 1
inherit foo bar
DESCRIPTION="A nice package"
HOMEPAGE="http://example.com/"
SRC_URI=""
LICENSE="GPL-2"
SLOT="0"
KEYWORDS="test"
IUSE="bar"
DEPEND="cat/foo bar? ( cat/bar )"
END
TZ=UTC touch -t 197001010001 cat/write-eclasses/write-eclasses-1.ebuild || exit 2

mkdir cat/write-exlibs
cat <<END > cat/write-exlibs/write-exlibs-1.ebuild || exit 1
require foo bar
SUMMARY="A nice package"
HOMEPAGE="http://example.com/"
SRC_URI=""
LICENSE="GPL-2"
SLOT="0"
PLATFORMS="test"
MYOPTIONS="bar"
DEPENDENCIES="build: cat/foo bar? ( cat/bar )"
END
TZ=UTC touch -t 197001010001 cat/write-exlibs/write-exlibs-1.ebuild || exit 2

cd ..

mkdir -p cache/{test-repo,expected}/cat

cat <<END > cache/expected/cat/write-1
_mtime_=60
DEPEND=cat/foo bar? ( cat/bar )
RDEPEND=cat/foo bar? ( cat/bar )
SLOT=0
HOMEPAGE=http://example.com/
LICENSE=GPL-2
DESCRIPTION=A nice package
KEYWORDS=test
IUSE=bar
EAPI=0
END

cat <<END > cache/expected/cat/write-eclasses-1
_mtime_=60
_eclasses_=bar	ebuild_flat_metadata_cache_TEST_dir/extra_eclasses	180	foo	ebuild_flat_metadata_cache_TEST_dir/repo/eclass	180
DEPEND=cat/foo bar? ( cat/bar ) cat/baz
RDEPEND=cat/foo bar? ( cat/bar )
SLOT=0
HOMEPAGE=http://example.com/
LICENSE=GPL-2
DESCRIPTION=A nice package
KEYWORDS=test
IUSE=bar quux
EAPI=0
END

cat <<END > cache/expected/cat/write-exlibs-1
_mtime_=60
_exlibs_=bar	ebuild_flat_metadata_cache_TEST_dir/repo/cat/exlibs	180	foo	ebuild_flat_metadata_cache_TEST_dir/repo/exlibs	180
DEPENDENCIES=build: cat/foo bar? ( cat/bar ) cat/baz build: bar? ( ) cat/baz build: bar? ( ) 
SLOT=0
HOMEPAGE=http://example.com/
LICENSE=GPL-2
SUMMARY=A nice package
PLATFORMS=test
MYOPTIONS=bar quux
EAPI=exheres-0
END

