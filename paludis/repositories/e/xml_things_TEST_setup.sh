#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir xml_things_TEST_dir || exit 1
cd xml_things_TEST_dir || exit 1

cat <<"END" > glsa-123456-78.xml
<?xml version="1.0" encoding="utf-8"?>
<?xml-stylesheet href="/xsl/glsa.xsl" type="text/xsl"?>
<?xml-stylesheet href="/xsl/guide.xsl" type="text/xsl"?>
<!DOCTYPE glsa SYSTEM "http://www.gentoo.org/dtd/glsa.dtd">

<glsa id="123456-78">
  <title>
    Kittens: Too Adorable
  </title>
  <synopsis>
    Kittens are too adorable. This can lead to excess cuteness.
  </synopsis>
  <product type="ebuild">kitten</product>
  <announced>October 10, 2006</announced>
  <revised>October 10, 2006: 01</revised>
  <bug>123456</bug>
  <access>remote</access>
  <affected>
    <package name="animal-feline/kitten" auto="yes" arch="*">
      <unaffected range="ge">1.23</unaffected>
      <vulnerable range="lt">1.22</vulnerable>
    </package>
  </affected>
  <background>
    <p>
    Kittens are small cats.
    </p>
  </background>
  <description>
    <p>
    By being adorable, kittens can get away with too much misbehaviour.
    </p>
  </description>
  <impact type="high">
    <p>
    A kitten could get away with going undrowned because of its adorableness.
    </p>
  </impact>
  <workaround>
    <p>
    There is no known workaround at this time.
    </p>
  </workaround>
  <resolution>
    <p>
    All kitten users should upgrade to the latest version.
    </p>
  </resolution>
  <references>
  </references>
</glsa>
END

cat <<"END" > glsa-987654-32.xml
<?xml version="1.0" encoding="utf-8"?>
<?xml-stylesheet href="/xsl/glsa.xsl" type="text/xsl"?>
<?xml-stylesheet href="/xsl/guide.xsl" type="text/xsl"?>
<!DOCTYPE glsa SYSTEM "http://www.gentoo.org/dtd/glsa.dtd">

<glsa id="987654-32">
  <title>
    Python: Retarded
  </title>
  <synopsis>
    Python is retarded. Reading it can make your eyes bleed.
  </synopsis>
  <product type="ebuild">python</product>
  <announced>October 10, 2006</announced>
  <revised>October 10, 2006: 01</revised>
  <bug>987654</bug>
  <access>remote</access>
  <affected>
    <package name="dev-lang/python" auto="yes" arch="x86 sparc mips">
      <unaffected range="ge">12.34</unaffected>
      <vulnerable range="lt">12.34</vulnerable>
    </package>
  </affected>
  <background>
    <p>
    Python purports to be a programming language.
    </p>
  </background>
  <description>
    <p>
    Python abuses whitespace for block structures. This makes anyone reading it
    suffer severe brain ache.
    </p>
  </description>
  <impact type="high">
    <p>
    Anyone reading python code could go crazy.
    </p>
  </impact>
  <workaround>
    <p>
    Use a real programming language.
    </p>
  </workaround>
  <resolution>
    <p>
    All python users should get their brains examined.
    </p>
  </resolution>
  <references>
  </references>
</glsa>
END

