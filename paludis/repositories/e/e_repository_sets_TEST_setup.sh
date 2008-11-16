#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir e_repository_sets_TEST_dir || exit 1
cd e_repository_sets_TEST_dir || exit 1

mkdir -p repo1/{eclass,distfiles,profiles/profile,sets,metadata/glsa} || exit 1
cd repo1 || exit 1

echo "test-repo-1" > profiles/repo_name || exit 1
cat <<END > profiles/categories || exit 1
cat-one
cat-two
cat-three
cat-four
END
cat <<END > profiles/profile/make.defaults
ARCH=test
END

cat <<END > sets/set1.conf
* cat-one/foo
? >=cat-two/bar-2
? <cat-three/baz-1.2
END

mkdir -p cat-one/foo cat-two/bar cat-three/baz cat-four/xyzzy
cat <<END > cat-one/foo/foo-1.ebuild
SLOT="0"
KEYWORDS="test"
END
cp cat-one/foo/foo-1.ebuild cat-one/foo/foo-2.ebuild
cp cat-one/foo/foo-1.ebuild cat-one/foo/foo-2.1.ebuild
cp cat-one/foo/foo-1.ebuild cat-two/bar/bar-1.5.ebuild
cp cat-one/foo/foo-1.ebuild cat-two/bar/bar-1.5.1.ebuild
cp cat-one/foo/foo-1.ebuild cat-two/bar/bar-2.0.ebuild
cp cat-one/foo/foo-1.ebuild cat-three/baz/baz-1.0.ebuild
cp cat-one/foo/foo-1.ebuild cat-three/baz/baz-1.1.ebuild
cp cat-one/foo/foo-1.ebuild cat-three/baz/baz-1.1-r1.ebuild
cp cat-one/foo/foo-1.ebuild cat-three/baz/baz-1.1-r2.ebuild
cp cat-one/foo/foo-1.ebuild cat-three/baz/baz-1.2.ebuild
cp cat-one/foo/foo-1.ebuild cat-three/baz/baz-1.2-r1.ebuild
cp cat-one/foo/foo-1.ebuild cat-three/baz/baz-1.3.ebuild
cat <<END > cat-three/baz/baz-1.3.1.ebuild
SLOT="0"
KEYWORDS="~test"
END
cat <<END > cat-four/xyzzy/xyzzy-1.1.0.ebuild
SLOT="\${PV:0:1}"
KEYWORDS="test"
END
cp cat-four/xyzzy/xyzzy-1.1.0.ebuild cat-four/xyzzy/xyzzy-1.1.1.ebuild
cp cat-four/xyzzy/xyzzy-1.1.0.ebuild cat-four/xyzzy/xyzzy-2.0.1.ebuild
cp cat-four/xyzzy/xyzzy-1.1.0.ebuild cat-four/xyzzy/xyzzy-2.0.1-r1.ebuild
cp cat-four/xyzzy/xyzzy-1.1.0.ebuild cat-four/xyzzy/xyzzy-2.0.2.ebuild
cp cat-four/xyzzy/xyzzy-1.1.0.ebuild cat-four/xyzzy/xyzzy-2.0.3.ebuild

cat <<END > metadata/glsa/glsa-123456-78.xml
<?xml version="1.0" encoding="utf-8"?>
<?xml-stylesheet href="/xsl/glsa.xsl" type="text/xsl"?>
<?xml-stylesheet href="/xsl/guide.xsl" type="text/xsl"?>
<!DOCTYPE glsa SYSTEM "http://www.gentoo.org/dtd/glsa.dtd">

<glsa id="123456-78">
  <title>
    Foo: insecure XYZ
  </title>
  <synopsis>
    Foo can be made to XYZ insecurely.
  </synopsis>
  <product type="ebuild">foo</product>
  <announced>May 26, 2007</announced>
  <revised>May 26, 2007: 01</revised>
  <bug>123456</bug>
  <access>local</access>
  <affected>
    <package name="cat-one/foo" auto="yes" arch="*">
      <unaffected range="ge">2</unaffected>
      <vulnerable range="lt">2</vulnerable>
    </package>
  </affected>
  <background>
    <p>
    Foo is a Foo.
    </p>
  </background>
  <description>
    <p>
    Foo can be exploited to perform XYZ without authorisation.
    </p>
  </description>
  <impact type="high">
    <p>
    XYZ could happen insecurely.
    </p>
  </impact>
  <workaround>
    <p>
    There is no known workaround at this time.
    </p>
  </workaround>
  <resolution>
    <p>
    All Foo users should upgrade to the latest version.
    </p>
  </resolution>
  <references>
  </references>
</glsa>
END

cat <<END > metadata/glsa/glsa-314159-26.xml
<?xml version="1.0" encoding="utf-8"?>
<?xml-stylesheet href="/xsl/glsa.xsl" type="text/xsl"?>
<?xml-stylesheet href="/xsl/guide.xsl" type="text/xsl"?>
<!DOCTYPE glsa SYSTEM "http://www.gentoo.org/dtd/glsa.dtd">

<glsa id="314159-26">
  <title>
    Bar: unauthenticated QWERTY
  </title>
  <synopsis>
    Bar allows QWERTY by remote users.
  </synopsis>
  <product type="ebuild">bar</product>
  <announced>May 26, 2007</announced>
  <revised>May 26, 2007: 01</revised>
  <bug>314159</bug>
  <access>local</access>
  <affected>
    <package name="cat-two/bar" auto="yes" arch="*">
      <unaffected range="gt">1.5.1</unaffected>
      <vulnerable range="le">1.5.1</vulnerable>
    </package>
  </affected>
  <background>
    <p>
    Bar is a Bar.
    </p>
  </background>
  <description>
    <p>
    Remote users can bypass authentication to perform QWERTY.
    </p>
  </description>
  <impact type="high">
    <p>
    QWERTY can be performed by remote users.
    </p>
  </impact>
  <workaround>
    <p>
    There is no known workaround at this time.
    </p>
  </workaround>
  <resolution>
    <p>
    All Bar users should upgrade to the latest version.
    </p>
  </resolution>
  <references>
  </references>
</glsa>
END

cat <<END > metadata/glsa/glsa-987654-32.xml
<?xml version="1.0" encoding="utf-8"?>
<?xml-stylesheet href="/xsl/glsa.xsl" type="text/xsl"?>
<?xml-stylesheet href="/xsl/guide.xsl" type="text/xsl"?>
<!DOCTYPE glsa SYSTEM "http://www.gentoo.org/dtd/glsa.dtd">

<glsa id="987654-32">
  <title>
   Baz: ABC overflow
  </title>
  <synopsis>
    Baz can be made to ABC too many times.
  </synopsis>
  <product type="ebuild">bar</product>
  <announced>May 26, 2007</announced>
  <revised>May 26, 2007: 01</revised>
  <bug>987654</bug>
  <access>local</access>
  <affected>
    <package name="cat-three/baz" auto="yes" arch="*">
      <unaffected range="ge">1.3.1</unaffected>
      <vulnerable range="lt">1.3.1</vulnerable>
      <unaffected range="rge">1.2-r1</unaffected>
      <unaffected range="rle">1.1-r1</unaffected>
      <unaffected range="eq">1.3</unaffected>
    </package>
  </affected>
  <background>
    <p>
    Baz is a Baz.
    </p>
  </background>
  <description>
    <p>
    Attempting to make Baz ABC too many times causes an overflow.
    </p>
  </description>
  <impact type="high">
    <p>
    ABC can be overflowed.
    </p>
  </impact>
  <workaround>
    <p>
    There is no known workaround at this time.
    </p>
  </workaround>
  <resolution>
    <p>
    All Bar users should upgrade to the latest version.
    </p>
  </resolution>
  <references>
  </references>
</glsa>
END

cat <<END > metadata/glsa/glsa-112358-42.xml
<?xml version="1.0" encoding="utf-8"?>
<?xml-stylesheet href="/xsl/glsa.xsl" type="text/xsl"?>
<?xml-stylesheet href="/xsl/guide.xsl" type="text/xsl"?>
<!DOCTYPE glsa SYSTEM "http://www.gentoo.org/dtd/glsa.dtd">

<glsa id="112358-42">
  <title>
   Xyzzy: SQL injection
  </title>
  <synopsis>
    Xyzzy is vulnerable to SQL injection attacks.
  </synopsis>
  <product type="ebuild">xyzzy</product>
  <announced>Nov 16, 2008</announced>
  <revised>Nov 16, 2008: 01</revised>
  <bug>112358</bug>
  <access>local</access>
  <affected>
    <package name="cat-four/xyzzy" auto="yes" arch="*">
      <unaffected range="ge" slot="2">2.0.3</unaffected>
      <vulnerable range="lt" slot="2">2.0.3</vulnerable>
      <unaffected range="eq" slot="*">2.0.1-r1</unaffected>
    </package>
  </affected>
  <background>
    <p>
    Xyzzy is a magic word with an SQL backend.
    </p>
  </background>
  <description>
    <p>
    Passing magic SQL characters to Xyzzy allows an attacker to
    execute arbitrary queries.
    </p>
  </description>
  <impact type="high">
    <p>
    Arbitrary SQL queries can be executed.
    </p>
  </impact>
  <workaround>
    <p>
    There is no known workaround at this time.
    </p>
  </workaround>
  <resolution>
    <p>
    All Xyzzy users should upgrade to the latest version.
    </p>
  </resolution>
  <references>
  </references>
</glsa>
END

cd ..

