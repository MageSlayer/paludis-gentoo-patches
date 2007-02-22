#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :

#
# Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

module Paludis
    class TestCase_PortageDepParser < Test::Unit::TestCase
        def test_one_arg
            spec = PortageDepParser::parse("foo/monkey")
            assert_kind_of AllDepSpec, spec
            assert_equal 1, spec.to_a.length
            assert_equal "foo/monkey", spec.to_a[0].to_s
            assert_kind_of PackageDepSpec, spec.to_a[0]
        end

        def test_many_args
            spec = PortageDepParser::parse("foo/monkey", PortageDepParser::PlainTextDepSpec, false)
            assert_kind_of AllDepSpec, spec
            assert_equal 1, spec.to_a.length
            assert_equal "foo/monkey", spec.to_a[0].to_s
            assert_kind_of PlainTextDepSpec, spec.to_a[0]

            spec = PortageDepParser::parse("foo/monkey", PortageDepParser::PlainTextDepSpec, true)
            assert_kind_of AllDepSpec, spec
            assert_equal 1, spec.to_a.length
            assert_equal "foo/monkey", spec.to_a[0].to_s
            assert_kind_of PlainTextDepSpec, spec.to_a[0]

            spec = PortageDepParser::parse("foo/monkey", PortageDepParser::PackageDepSpec, false)
            assert_kind_of AllDepSpec, spec
            assert_equal 1, spec.to_a.length
            assert_equal "foo/monkey", spec.to_a[0].to_s
            assert_kind_of PackageDepSpec, spec.to_a[0]

            spec = PortageDepParser::parse("foo/monkey", PortageDepParser::PackageDepSpec, true)
            assert_kind_of AllDepSpec, spec
            assert_equal 1, spec.to_a.length
            assert_equal "foo/monkey", spec.to_a[0].to_s
            assert_kind_of PackageDepSpec, spec.to_a[0]

            assert_raise DepStringParseError do
                PortageDepParser::parse("|| ( foo/bar )", PortageDepParser::PackageDepSpec, false)
            end

            PortageDepParser::parse("|| ( foo/bar )", PortageDepParser::PackageDepSpec, true)

            assert_raise DepStringParseError do
                PortageDepParser::parse("|| ( foo/bar )", PortageDepParser::PlainTextDepSpec, false)
            end

            PortageDepParser::parse("|| ( foo/bar )", PortageDepParser::PlainTextDepSpec, true)
        end
        
        def test_dep_string_nesting_error
            assert_raise DepStringNestingError do
                PortageDepParser::parse("|| ( foo/var ", PortageDepParser::PackageDepSpec,true)
            end
        end
    end
end


