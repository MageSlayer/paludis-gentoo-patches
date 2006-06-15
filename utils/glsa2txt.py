#!/usr/bin/env python

# vim: set sw=4 sts=4 et foldmethod=syntax :

# Copyright (c) 2005, 2006 Danny van Dyk <kugelfang@gentoo.org>
#
# This file is part of the Paludis package manager. Paludis is free software;
# you can redistribute it and/or modify it under the terms of the GNU General
# Public License version 2, as published by the Free Software Foundation.
#
# Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place, Suite 330, Boston, MA  02111-1307  USA
# Copiyright (C) 2005-2006 Danny van Dyk <kugelfang@gentoo.org>
# Distributed under the terms of the GNU General Public License v2
# $Id$

import sys, os, xml.sax

GLSA_TAGS = ['title', 'synopsis', 'package', 'unaffected', 'vulnerable', 'description', 'impact', 'workaround', 'uri', 'metadata', 'access', 'bug']

class GLSA2TXT:
    class GLSAHandler(xml.sax.ContentHandler):
        glsa = {}
        package = ""
        arch = []
        lasttag = ""
        isrge = 0

        def startElement(self, tag, attr):
            if tag in GLSA_TAGS:
                self.lasttag = tag
            if tag == 'glsa':
                self.glsa = { "Title": "", "synopsis": "", "description": "", "workaround": "", "Url": [],
                    "Committed-By": "", "Reviewed-By": "", "Access": "", "impact": "", "bug": "",
                    "Bug": []}
                self.glsa["Id"] = attr.get('id')
                self.glsa["Affected"] = []
                self.glsa["Unaffected"] = []
            if tag == 'package':
                self.package = attr.get('name')
                self.glsa[self.package + "-arch"] = attr.get('arch')
                self.glsa[self.package + "-unaffected"] = ""
                self.glsa[self.package + "-vulnerable"] = ""
            if tag == 'unaffected' or tag == 'vulnerable':
                range = attr.get('range')
                if range[0] == 'r':
                    range = range[1:]
                    self.glsa[self.package + "-" + tag + "-combo"] = 1
                else:
                    self.glsa[self.package + "-" + tag + "-combo"] = 0
                if range == 'eq':
                    range = '='
                elif range == 'ge':
                    range = '>='
                elif range == 'le':
                    range = '<='
                elif range == 'gt':
                    range = '>'
                elif range == 'lt':
                    range = '<'
                self.glsa[self.package + "-" + tag + "-range"] = range
                self.glsa[self.package + "-" + tag + "-version"] = ""
                self.glsa[self.package + "-" + tag + "-slot"] = attr.get('slot');
            if tag == 'uri':
                self.glsa["Url"].append(attr.get('link'))
            if tag == 'metadata':
                mtag = attr.get('tag')
                if mtag == 'submitter':
                    self.lasttag = 'Committed-By'
                elif mtag == 'bugReady':
                    self.lasttag = 'Reviewed-By'
                else:
                    self.lasttag = ''
            if tag == 'impact':
                self.glsa["Severity"] = attr.get('type')

        def characters(self, ch):
            if self.lasttag in ['access', 'title']:
                self.glsa[self.lasttag.capitalize()] += ch
            if self.lasttag in ['synopsis', 'description', 'workaround', 'impact', 'bug']:
                self.glsa[self.lasttag] += ch
            if self.lasttag in ['unaffected', 'vulnerable']:
                self.glsa[self.package + "-" + self.lasttag + "-version"] += ch
            if self.lasttag in ['Committed-By', 'Reviewed-By']:
                self.glsa[self.lasttag] += ch.replace('\n', '')

        def endElement(self, tag):
            if tag in GLSA_TAGS:
                self.lasttag = ""
            if tag == 'title':
                desc = self.glsa["Title"] + "\n"
                desc += '=' * len(self.glsa["Title"]) + "\n"
                self.glsa["Description"] = desc
            if tag == 'bug':
                self.glsa["Bug"].append(self.glsa["bug"])
                self.glsa["bug"] = ""
            if tag in ['synopsis', 'description', 'workaround', 'impact']:
                desc = "\n" + tag.title() + "\n"
                desc += "'" * len(tag) + "\n\n"
                lines = []
                for x in self.glsa[tag].split('\n'):
                    y = x.strip()
                    if not y == "":
                        lines.append(y)
                desc += "\n".join(lines) + "\n"
                self.glsa["Description"] += desc
            if tag in ['unaffected', 'vulnerable']:
                package = self.package
                archs = self.glsa[package + "-arch"]
                slot = self.glsa[package + "-" + tag + "-slot"]
                combo = self.glsa[package + "-" + tag + "-combo"]
                range = self.glsa[package + "-" + tag + "-range"]
                version = self.glsa[package + "-" + tag + "-version"]
                if not slot == None:
                    slot = ":" + slot
                else:
                    slot = ""
                entry  = range + package + "-" + version + slot
                if combo == 1:
                    vitems = version.split('-r')
                    if not vitems[0:-1] == []:
                        version = "-r".join(vitems[0:-1])
                    entry += " ~" + package + "-" + version + slot
                archs = self.glsa[self.package + "-arch"]
                if tag == 'vulnerable':
                    Tag = 'Affected'
                else:
                    Tag = 'Unaffected'
                if not archs == '*':
                    for arch in archs.split(' '):
                        self.glsa[Tag].append(arch + "? ( " + entry + " )")
                else:
                    self.glsa[Tag].append(entry);

    def write_advisory(self, handler):
        glsa = handler.glsa
        print "Id: " + glsa["Id"]
        print "Title: " + glsa["Title"]
        print "Access: " + glsa["Access"]
        print "Last-Modified: $Date$"
        print "Revision: 1.0"
        print "Severity: " + glsa["Severity"]
        print "Spec-Version: 1"
        for x in glsa["Affected"]:
            print "Affected: " + x
        for x in glsa["Bug"]:
            print "Bug-Id: Gentoo#" + x
        for x in glsa["Url"]:
            print "Reference: " + x
        for x in glsa["Unaffected"]:
            print "Unaffected: " + x

        print
        print glsa["Description"]

    def process(self):
        try:
            parser = xml.sax.make_parser("xml.sax.drivers2.drv_xmlproc")
            handler = GLSA2TXT.GLSAHandler()
            parser.setContentHandler(handler)
            #parser.setFeature("http://xml.org/sax/features/external-general-entities",False)
            parser.parse(sys.argv[1])

            self.write_advisory(handler)

        except (xml.sax.SAXException, IOError), e:
            print "Error parsing " + sys.argv[0] + "!"
            print e.getMessage()

if __name__ == '__main__':
    x = GLSA2TXT()
    x.process()
