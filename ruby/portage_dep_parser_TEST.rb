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
    class TestCase_Policy < Test::Unit::TestCase
        def test_package
            assert_nothing_raised do
                policy = PortageDepParser::Policy::text_is_package_dep_spec(true, PackageDepSpecParseMode::Permissive)
            end
        end

        def test_text
            assert_nothing_raised do
                policy = PortageDepParser::Policy::text_is_text_dep_spec(true)
            end
        end

        def test_no_create
            assert_raise NoMethodError do
                x = PortageDepParser::Policy.new()
            end
        end
    end

    class TestCase_PortageDepParser < Test::Unit::TestCase
        def text_false
            PortageDepParser::Policy::text_is_text_dep_spec(false)
        end

        def text_true
            PortageDepParser::Policy::text_is_text_dep_spec(true)
        end

        def package_false
            PortageDepParser::Policy::text_is_package_dep_spec(false, PackageDepSpecParseMode::Permissive);
        end

        def package_true
            PortageDepParser::Policy::text_is_package_dep_spec(true, PackageDepSpecParseMode::Permissive);
        end

        def test_many_args
            spec = PortageDepParser::parse("foo/monkey", text_false)
            assert_kind_of AllDepSpec, spec
            assert_equal 1, spec.to_a.length
            assert_equal "foo/monkey", spec.to_a.first.to_s
            assert_kind_of PlainTextDepSpec, spec.to_a.first

            spec = PortageDepParser::parse("foo/monkey", text_true)
            assert_kind_of AllDepSpec, spec
            assert_equal 1, spec.to_a.length
            assert_equal "foo/monkey", spec.to_a.first.to_s
            assert_kind_of PlainTextDepSpec, spec.to_a.first

            spec = PortageDepParser::parse("foo/monkey", package_false)
            assert_kind_of AllDepSpec, spec
            assert_equal 1, spec.to_a.length
            assert_equal "foo/monkey", spec.to_a.first.to_s
            assert_kind_of PackageDepSpec, spec.to_a.first

            spec = PortageDepParser::parse("foo/monkey", package_true)
            assert_kind_of AllDepSpec, spec
            assert_equal 1, spec.to_a.length
            assert_equal "foo/monkey", spec.to_a.first.to_s
            assert_kind_of PackageDepSpec, spec.to_a.first

            assert_raise DepStringParseError do
                PortageDepParser::parse("|| ( foo/bar )", package_false)
            end

            assert_nothing_raised do
                PortageDepParser::parse("|| ( foo/bar )", package_true)
            end

            assert_raise DepStringParseError do
                PortageDepParser::parse("|| ( foo/bar )", text_false)
            end

            assert_nothing_raised do
                PortageDepParser::parse("|| ( foo/bar )", text_true)
            end
        end

        def test_dep_string_nesting_error
            assert_raise DepStringNestingError do
                PortageDepParser::parse("|| ( foo/var ", package_true)
            end
        end
    end
end


