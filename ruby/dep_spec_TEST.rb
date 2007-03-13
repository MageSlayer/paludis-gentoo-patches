#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :
#
# Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
# Copyright (c) 2007 Richard Brown <mynamewasgone@gmail.com>
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

exit 0

module Paludis
    class TestCase_DepSpec < Test::Unit::TestCase
        def test_create_error
            assert_raise NoMethodError do
                v = DepSpec.new
            end
            assert_raise NoMethodError do
                v = StringDepSpec.new
            end
            assert_raise NoMethodError do
                v = AnyDepSpec.new
            end
            assert_raise NoMethodError do
                v = AllDepSpec.new
            end
        end
    end

    class TestCase_PackageDepSpec < Test::Unit::TestCase
        def pda
            PackageDepSpec.new('>=foo/bar-1:100::testrepo')
        end

        def test_create
            pda
        end

        def test_create_error
            assert_raise TypeError do
                v = PackageDepSpec.new(0)
            end
            assert_raise PackageDepSpecError do
                v = PackageDepSpec.new("=sys-apps/foo")
            end
        end

        def test_to_s
            assert_equal ">=foo/bar-1:100::testrepo", pda.to_s
        end

        def test_text
            assert_equal ">=foo/bar-1:100::testrepo", pda.text
        end

        def test_slot
            assert_equal "100", pda.slot
        end

        def test_package
            assert_equal "foo/bar", pda.package
        end

        def test_repository
            assert_equal "testrepo", pda.repository
        end

        def test_version_requirements
            assert_kind_of Array, pda.version_requirements
            assert_equal 1, pda.version_requirements.size
            assert_equal VersionSpec.new('1'), pda.version_requirements.first[:spec]
            assert_equal ">=", pda.version_requirements.first[:operator]
        end

        def test_version_requirements_mode
            assert_kind_of Fixnum, pda.version_requirements_mode
            assert_equal VersionRequirementsMode::And, pda.version_requirements_mode
        end
    end

    class TestCase_PlainTextDepSpec < Test::Unit::TestCase
        def test_create
            v = PlainTextDepSpec.new("monkey")
        end

        def test_create_error
            assert_raise TypeError do
                v = PlainTextDepSpec.new(0)
            end
        end

        def test_to_s
            assert_equal "monkey", PlainTextDepSpec.new("monkey").to_s
        end
    end

    class TestCase_BlockDepSpec < Test::Unit::TestCase
        def test_create
            v = BlockDepSpec.new(PackageDepSpec.new(">=foo/bar-1"))
            w = BlockDepSpec.new("<=foo/bar-2")
        end

        def test_create_error
            assert_raise TypeError do
                v = BlockDepSpec.new(0)
            end
            assert_raise PackageDepSpecError do
                v = BlockDepSpec.new("=foo/bar")
            end
            assert_raise TypeError do 
                v = BlockDepSpec.new(PlainTextDepSpec.new('foo-bar/baz'))
            end
        end

        def test_blocked_spec
            assert_equal "foo/bar", BlockDepSpec.new("foo/bar").blocked_spec.to_s
            assert_equal "foo/baz", BlockDepSpec.new(PackageDepSpec.new("foo/baz")).blocked_spec.to_s
        end
    end

    class TestCase_Composites < Test::Unit::TestCase
        def test_composites
            spec = PortageDepParser::parse("|| ( foo/bar foo/baz ) foo/monkey")
            assert_kind_of CompositeDepSpec, spec
            assert_kind_of AllDepSpec, spec

            assert_equal 2, spec.to_a.length

            spec.each_with_index do | a, i |
                case i
                when 0
                    assert_kind_of AnyDepSpec, a
                    assert_equal 2, a.to_a.length
                    a.each_with_index do | b, j |
                        case j
                        when 0
                            assert_kind_of PackageDepSpec, b
                            assert_equal "foo/bar", b.to_s

                        when 1
                            assert_kind_of PackageDepSpec, b
                            assert_equal "foo/baz", b.to_s

                        else
                            throw "Too many items"
                        end
                    end

                when 1
                    assert_kind_of PackageDepSpec, a
                    assert_equal "foo/monkey", a.to_s

                else
                    throw "Too many items"
                end
            end
        end
    end
end


