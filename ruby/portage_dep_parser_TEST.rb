#!/usr/bin/ruby
# vim: set sw=4 sts=4 et tw=80 :

#
# Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#

require 'test/unit'
require 'Paludis'

class Paludis
    class TestCase_PortageDepParser < Test::Unit::TestCase
        def test_one_arg
            atom = PortageDepParser::parse("foo/monkey")
            assert_kind_of AllDepAtom, atom
            assert_equal 1, atom.to_a.length
            assert_equal "foo/monkey", atom.to_a[0].to_s
            assert_kind_of PackageDepAtom, atom.to_a[0]
        end

        def test_many_args
            atom = PortageDepParser::parse("foo/monkey", PortageDepParser::PlainTextDepAtom, false)
            assert_kind_of AllDepAtom, atom
            assert_equal 1, atom.to_a.length
            assert_equal "foo/monkey", atom.to_a[0].to_s
            assert_kind_of PlainTextDepAtom, atom.to_a[0]

            atom = PortageDepParser::parse("foo/monkey", PortageDepParser::PlainTextDepAtom, true)
            assert_kind_of AllDepAtom, atom
            assert_equal 1, atom.to_a.length
            assert_equal "foo/monkey", atom.to_a[0].to_s
            assert_kind_of PlainTextDepAtom, atom.to_a[0]

            atom = PortageDepParser::parse("foo/monkey", PortageDepParser::PackageDepAtom, false)
            assert_kind_of AllDepAtom, atom
            assert_equal 1, atom.to_a.length
            assert_equal "foo/monkey", atom.to_a[0].to_s
            assert_kind_of PackageDepAtom, atom.to_a[0]

            atom = PortageDepParser::parse("foo/monkey", PortageDepParser::PackageDepAtom, true)
            assert_kind_of AllDepAtom, atom
            assert_equal 1, atom.to_a.length
            assert_equal "foo/monkey", atom.to_a[0].to_s
            assert_kind_of PackageDepAtom, atom.to_a[0]

            assert_raise DepStringError do
                PortageDepParser::parse("|| ( foo/bar )", PortageDepParser::PackageDepAtom, false)
            end

            PortageDepParser::parse("|| ( foo/bar )", PortageDepParser::PackageDepAtom, true)

            assert_raise DepStringError do
                PortageDepParser::parse("|| ( foo/bar )", PortageDepParser::PlainTextDepAtom, false)
            end

            PortageDepParser::parse("|| ( foo/bar )", PortageDepParser::PlainTextDepAtom, true)
        end
    end
end


