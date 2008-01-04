#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :
#
# Copyright (c) 2006, 2007 Ciaran McCreesh
# Copyright (c) 2007 Richard Brown
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

ENV['PALUDIS_HOME'] = Dir.getwd() + '/dep_spec_TEST_dir/home'

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
            Paludis::parse_user_package_dep_spec('>=foo/bar-1:100::testrepo[a][-b]', [])
        end

        def pdb
            Paludis::parse_user_package_dep_spec('*/bar', [:allow_wildcards])
        end

        def test_create
            pda
            pdb
        end

        def test_create_error
            assert_raise NoMethodError do
                v = PackageDepSpec.new("foo")
            end
            assert_raise PackageDepSpecError do
                Paludis::parse_user_package_dep_spec("=sys-apps/foo", [])
            end
            assert_raise TypeError do
                Paludis::parse_user_package_dep_spec("sys-apps/foo", {})
            end
            assert_raise TypeError do
                Paludis::parse_user_package_dep_spec("sys-apps/foo", ["foo"])
            end
            assert_raise ArgumentError do
                Paludis::parse_user_package_dep_spec("sys-apps/foo", [:unknown])
            end
            assert_raise ArgumentError do
                Paludis::parse_user_package_dep_spec("sys-apps/foo")
            end
        end

        def test_to_s
            assert_equal ">=foo/bar-1:100::testrepo[-b][a]", pda.to_s
            assert_equal "*/bar", pdb.to_s
        end

        def test_text
            assert_equal ">=foo/bar-1:100::testrepo[-b][a]", pda.text
            assert_equal "*/bar", pdb.text
        end

        def test_slot
            assert_equal "100", pda.slot
            assert_nil pdb.slot
        end

        def test_package
            assert_equal "foo/bar", pda.package
            assert_nil pdb.package
        end

        def test_repository
            assert_equal "testrepo", pda.repository
            assert_nil pdb.package
        end

        def test_package_name_part
            assert_nil pda.package_name_part
            assert_equal "bar", pdb.package_name_part
        end

        def test_category_name_part
            assert_nil pda.category_name_part
            assert_nil pdb.category_name_part
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

###        def test_use_requirements
###            assert_kind_of Array, pda.use_requirements
###            assert_equal 2, pda.use_requirements.size
###
###            assert_equal 'a', pda.use_requirements[0][:flag]
###            assert_equal true, pda.use_requirements[0][:state]
###
###            assert_equal 'b', pda.use_requirements[1][:flag]
###            assert_equal false, pda.use_requirements[1][:state]
###        end

        def test_without_use_requirements
            assert_equal ">=foo/bar-1:100::testrepo", pda.without_use_requirements.to_s
            assert_equal "*/bar", pdb.without_use_requirements.to_s
        end

        def test_tag
            assert_nil pda.tag
            assert_nil pdb.tag
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
            v = BlockDepSpec.new(Paludis::parse_user_package_dep_spec(">=foo/bar-1", []))
        end

        def test_create_error
            assert_raise TypeError do
                v = BlockDepSpec.new(0)
            end

            assert_raise TypeError do
                v = BlockDepSpec.new(PlainTextDepSpec.new('foo-bar/baz'))
            end
        end

        def test_blocked_spec
            assert_equal "foo/baz", BlockDepSpec.new(Paludis::parse_user_package_dep_spec("foo/baz", [])).blocked_spec.to_s
        end
    end

    class TestCase_Composites < Test::Unit::TestCase
        def env
            @env or @env = EnvironmentMaker.instance.make_from_spec("")
        end

        def test_composites
            spec = env.package_database.query(Query::All.new, QueryOrder::RequireExactlyOne).last.build_dependencies_key.value
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

