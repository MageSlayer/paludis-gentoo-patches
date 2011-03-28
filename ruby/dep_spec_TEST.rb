#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :
#
# Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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
        def env
            @env or @env = EnvironmentFactory.instance.create("")
        end

        def pda
            Paludis::parse_user_package_dep_spec('>=foo/bar-1:100::testrepo[a][-b]', env, [])
        end

        def pdb
            Paludis::parse_user_package_dep_spec('*/bar', env, [:allow_wildcards])
        end

        def pdc
            Paludis::parse_user_package_dep_spec('foo/bar::installed?', env, [])
        end

        def pdd
            Paludis::parse_user_package_dep_spec('foo/*::/??', env, [:allow_wildcards])
        end

        def pde
            Paludis::parse_user_package_dep_spec('foo/bar::testrepo->/mychroot', env, [])
        end

        def test_create
            pda
            pdb
            pdc
            pdd
            pde
        end

        def test_create_error
            assert_raise NoMethodError do
                v = PackageDepSpec.new("foo")
            end
            assert_raise PackageDepSpecError do
                Paludis::parse_user_package_dep_spec("=sys-apps/foo", env, [])
            end
            assert_raise TypeError do
                Paludis::parse_user_package_dep_spec("sys-apps/foo", env, {})
            end
            assert_raise TypeError do
                Paludis::parse_user_package_dep_spec("sys-apps/foo", env, ["foo"])
            end
            assert_raise ArgumentError do
                Paludis::parse_user_package_dep_spec("sys-apps/foo", env, [:unknown])
            end
            assert_raise TypeError do
                Paludis::parse_user_package_dep_spec("sys-apps/foo", env, [], "foo")
            end
            assert_raise ArgumentError do
                Paludis::parse_user_package_dep_spec("sys-apps/foo")
            end
        end

        def test_to_s
            assert_equal ">=foo/bar-1:100::testrepo[-b][a]", pda.to_s
            assert_equal "*/bar", pdb.to_s
            assert_equal "foo/bar::installed?", pdc.to_s
            assert_equal "foo/*::/??", pdd.to_s
            assert_equal "foo/bar::testrepo->/mychroot", pde.to_s
        end

        def test_text
            assert_equal ">=foo/bar-1:100::testrepo[-b][a]", pda.text
            assert_equal "*/bar", pdb.text
            assert_equal "foo/bar::installed?", pdc.text
            assert_equal "foo/*::/??", pdd.text
            assert_equal "foo/bar::testrepo->/mychroot", pde.text
        end

        def test_disambiguate
            assert_equal Paludis::parse_user_package_dep_spec("foo", env, []).to_s, "bar/foo"
            assert_raise NoSuchPackageError do
                Paludis::parse_user_package_dep_spec("foo", env, [], Filter::SupportsAction.new(ConfigAction))
            end
            assert_raise AmbiguousPackageNameError do
                Paludis::parse_user_package_dep_spec("bar", env, [])
            end
            assert_raise AmbiguousPackageNameError do
                Paludis::parse_user_package_dep_spec("bar", env, [], Filter::All.new())
            end
            assert_equal Paludis::parse_user_package_dep_spec("bar", env, [],
                Filter::SupportsAction.new(InstallAction)).to_s, "foo/bar"
        end

        def test_slot
            assert_kind_of SlotExactRequirement, pda.slot_requirement
            assert_equal ":100", pda.slot_requirement.to_s
            assert_equal "100", pda.slot_requirement.slot
            assert_nil pdb.slot_requirement
            assert_nil pdc.slot_requirement
            assert_nil pdd.slot_requirement
            assert_nil pde.slot_requirement
        end

        def test_package
            assert_equal QualifiedPackageName.new("foo/bar"), pda.package_name_constraint.name
            assert_nil pdb.package_name_constraint
            assert_equal QualifiedPackageName.new("foo/bar"), pdc.package_name_constraint.name
            assert_nil pdd.package_name_constraint
            assert_equal QualifiedPackageName.new("foo/bar"), pde.package_name_constraint.name
        end

        def test_from_repository
            assert_nil pda.from_repository
            assert_nil pdb.from_repository
            assert_nil pdc.from_repository
            assert_nil pdd.from_repository
            assert_equal "testrepo", pde.from_repository
        end

        def test_in_repository
            assert_equal "testrepo", pda.in_repository
            assert_nil pdb.in_repository
            assert_nil pdc.in_repository
            assert_nil pdd.in_repository
            assert_nil pde.in_repository
        end

        def test_installable_to_repository
            assert_nil pda.installable_to_repository
            assert_nil pdb.installable_to_repository
            assert_kind_of Hash, pdc.installable_to_repository
            assert_equal "installed", pdc.installable_to_repository[:repository]
            assert ! pdc.installable_to_repository[:include_masked?]
            assert_nil pdd.installable_to_repository
            assert_nil pde.installable_to_repository
        end

        def test_installed_at_path
            assert_nil pda.installed_at_path
            assert_nil pdb.installed_at_path
            assert_nil pdc.installed_at_path
            assert_nil pdd.installed_at_path
            assert_equal "/mychroot", pde.installed_at_path
        end

        def test_installable_to_path
            assert_nil pda.installable_to_path
            assert_nil pdb.installable_to_path
            assert_nil pdc.installable_to_path
            assert_kind_of Hash, pdd.installable_to_path
            assert_equal "/", pdd.installable_to_path[:path]
            assert pdd.installable_to_path[:include_masked?]
            assert_nil pde.installable_to_path
        end

        def test_package_name_part
            assert_nil pda.package_name_part
            assert_equal "bar", pdb.package_name_part
            assert_nil pdc.package_name_part
            assert_nil pdd.package_name_part
            assert_nil pde.package_name_part
        end

        def test_category_name_part
            assert_nil pda.category_name_part
            assert_nil pdb.category_name_part
            assert_nil pdc.category_name_part
            assert_equal "foo", pdd.category_name_part
            assert_nil pde.category_name_part
        end

        def test_version_requirements
            assert_kind_of Array, pda.version_requirements
            assert_equal 1, pda.version_requirements.size
            assert_equal VersionSpec.new('1'), pda.version_requirements.first[:spec]
            assert_equal ">=", pda.version_requirements.first[:operator]
            assert_equal 0, pdb.version_requirements.size
            assert_equal 0, pdc.version_requirements.size
            assert_equal 0, pdd.version_requirements.size
            assert_equal 0, pde.version_requirements.size
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
        def env
            @env or @env = EnvironmentFactory.instance.create("")
        end

        def test_create
            v = BlockDepSpec.new("!>=foo/bar-1", Paludis::parse_user_package_dep_spec(">=foo/bar-1", env, []))
        end

        def test_create_error
            assert_raise TypeError do
                v = BlockDepSpec.new("!>=foo/bar-1", 0)
            end

            assert_raise TypeError do
                v = BlockDepSpec.new("!>=foo/bar-1", PlainTextDepSpec.new('foo-bar/baz'))
            end
        end

        def test_blocked_spec
            assert_equal "foo/baz", BlockDepSpec.new("!foo/baz", Paludis::parse_user_package_dep_spec(
                "foo/baz", env, [])).blocking.to_s
        end
    end

    class TestCase_Composites < Test::Unit::TestCase
        def env
            @env or @env = EnvironmentFactory.instance.create("")
        end

        def test_composites
            spec = env[Selection::RequireExactlyOne.new(Generator::Package.new("foo/bar"))].last.build_dependencies_key.value
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

    class TestCase_URILabels < Test::Unit::TestCase
        def env
            @env or @env = EnvironmentFactory.instance.create("")
        end

        def spec_key
            env[Selection::RequireExactlyOne.new(Generator::Package.new("bar/foo"))].last.fetches_key
        end

        def test_no_create
            assert_raise NoMethodError do URILabel.new end
            [URILabel, URIMirrorsThenListedLabel, URIMirrorsOnlyLabel,
             URIListedOnlyLabel, URIListedThenMirrorsLabel,
             URILocalMirrorsOnlyLabel, URIManualOnlyLabel].each do | c |
                assert_raise NoMethodError do c.new end
            end
        end

        def test_initial_label
            assert_kind_of URIListedThenMirrorsLabel, spec_key.initial_label
            assert_equal "default", spec_key.initial_label.text
            assert_equal "default", spec_key.initial_label.to_s
        end

        def test_uri_labels_dep_spec
            specs = spec_key.value.to_a
            assert_equal 6, specs.length

            specs.each do | spec |
                assert_kind_of URILabelsDepSpec, spec
                assert_kind_of Array, spec.labels

                array_from_block = []
                spec.labels { | label | array_from_block << label }

                [spec.labels, array_from_block].each do | array |
                    assert_equal 1, array.length
                    assert_kind_of URILabel, array[0]
                    assert_equal array[0].text, array[0].to_s
                end

                assert_equal spec.labels[0].class, array_from_block[0].class
                assert_equal spec.labels[0].text,  array_from_block[0].text
            end

            assert_kind_of URIMirrorsThenListedLabel, specs.to_a[0].labels[0]
            assert_kind_of URIMirrorsOnlyLabel,       specs.to_a[1].labels[0]
            assert_kind_of URIListedOnlyLabel,        specs.to_a[2].labels[0]
            assert_kind_of URIListedThenMirrorsLabel, specs.to_a[3].labels[0]
            assert_kind_of URILocalMirrorsOnlyLabel,  specs.to_a[4].labels[0]
            assert_kind_of URIManualOnlyLabel,        specs.to_a[5].labels[0]

            assert_equal "mirrors-first", specs.to_a[0].labels[0].text
            assert_equal "mirrors-only",  specs.to_a[1].labels[0].text
            assert_equal "listed-only",   specs.to_a[2].labels[0].text
            assert_equal "listed-first",  specs.to_a[3].labels[0].text
            assert_equal "local-only",    specs.to_a[4].labels[0].text
            assert_equal "manual",        specs.to_a[5].labels[0].text
        end
    end

    class TestCase_DependencyLabels < Test::Unit::TestCase
        def env
            @env or @env = EnvironmentFactory.instance.create("")
        end

        def spec_key
            env[Selection::RequireExactlyOne.new(Generator::Package.new("bar/foo"))].last.dependencies_key
        end

        def test_initial_labels
            assert_kind_of Array, spec_key.initial_labels
            assert_kind_of DependenciesBuildLabel, spec_key.initial_labels[0]
            assert_equal "build", spec_key.initial_labels[0].text
            assert_equal "build", spec_key.initial_labels[0].to_s
        end
    end
end

