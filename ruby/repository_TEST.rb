#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :

#
# Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
# Copyright (c) 2006, 2007, 2008 Richard Brown
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

ENV["PALUDIS_HOME"] = Dir.getwd().to_s + "/repository_TEST_dir/home";

require 'test/unit'
require 'Paludis'

Paludis::Log.instance.log_level = Paludis::LogLevel::Warning

module Paludis

    class TestCase_Repository < Test::Unit::TestCase
        def test_no_create
            assert_raise NoMethodError do
                p = Repository.new
            end
        end
    end

    module RepositoryTestCase
        def installed_repo
            env.fetch_repository "installed"
        end

        def repo
            env.fetch_repository "testrepo"
        end

        def env
            @env or @env = EnvironmentFactory.instance.create("")
        end

        def no_config_testrepo
            NoConfigEnvironment.new Dir.getwd().to_s + "/repository_TEST_dir/testrepo"
        end

        def p
            env[Selection::RequireExactlyOne.new(Generator::Matches.new(
                Paludis::parse_user_package_dep_spec('=foo/bar-2.0::testrepo', env, []), nil, []))].first
        end

        def installed_pid
            installed_repo.package_ids('cat-one/pkg-one').first
        end
    end

    class TestCase_Repository < Test::Unit::TestCase
        include RepositoryTestCase

        def test_name
            assert_equal "installed", installed_repo.name
            assert_equal "testrepo", repo.name
        end
    end

    class TestCase_RepositoryHasCategoryNamed < Test::Unit::TestCase
        include RepositoryTestCase

        def test_has_category_named
            assert repo.has_category_named?("foo")
            assert ! repo.has_category_named?("bar")
        end
    end

    class TestCase_RepositoryHasPackageNamed < Test::Unit::TestCase
        include RepositoryTestCase

        def test_has_package_named
            assert repo.has_package_named?("foo/bar")
            assert ! repo.has_package_named?("bar/bar")
            assert ! repo.has_package_named?("foo/foo")
        end
    end

    class TestCase_RepositoryVersionSpecs < Test::Unit::TestCase
        include RepositoryTestCase

        def test_package_ids
            a = repo.package_ids "foo/bar"
            assert_equal 2, a.length
            a = a.sort_by { | id | id.version }
            assert_equal VersionSpec.new("1.0"), a[0].version
            assert_equal VersionSpec.new("2.0"), a[1].version

            assert_nothing_raised do
                repo.package_ids('foo/bar') do |pid|
                    next if pid.version == VersionSpec.new("2.0")
                    assert_equal VersionSpec.new('1.0'), pid.version
                    break
                end
            end

            b = repo.package_ids "bar/baz"
            assert b.empty?
        end
    end

    class TestCase_RepositoryCategoryNames < Test::Unit::TestCase
        include RepositoryTestCase

        def test_category_names
            a = repo.category_names
            assert_equal 1, a.length
            assert_equal "foo", a.first
            assert_nothing_raised do
                repo.category_names do |name|
                    assert_equal 'foo', name
                end
            end
        end
    end

    class TestCase_RepositoryCategoryNamesContainingPackage < Test::Unit::TestCase
        include RepositoryTestCase

        def test_category_names_containing_package
            a = repo.category_names_containing_package('bar')
            assert_equal 1, a.length
            assert_equal "foo", a.first
            assert_nothing_raised do
                repo.category_names_containing_package('bar') do |name|
                    assert_equal 'foo', name
                end
            end
        end
    end

    class TestCase_RepositoryPackageNames < Test::Unit::TestCase
        include RepositoryTestCase

        def test_package_names
            a = repo.package_names "foo"
            assert_equal 1, a.length
            assert_equal "foo/bar", a[0]

            assert_nothing_raised do
                repo.package_names('foo') do |name|
                    assert_equal 'foo/bar', name
                end
            end

            assert repo.package_names("bar").empty?
        end
    end

    class TestCase_RepositoryInterfaces < Test::Unit::TestCase
        include RepositoryTestCase

        def test_responds
            repo = no_config_testrepo.main_repository
            [
                :environment_variable_interface, :virtuals_interface].each do |sym|
                assert_respond_to repo, sym
            end
        end

        def test_interfaces
            assert_equal installed_repo.name, installed_repo.environment_variable_interface.name
            assert_nil installed_repo.virtuals_interface
        end

        def text_repository_environment_interface
            assert_not_nil repo.environment_variable_interface
        end

        def test_get_environment_variable
            pid = env[Selection::BestVersionOnly.new(Generator::Matches.new(Paludis::parse_user_package_dep_spec(
                '=foo/bar-1.0', env, []), nil, []))].first;
            assert_equal "hello", repo.get_environment_variable(pid, "TEST_ENV_VAR")
            assert_equal "", repo.get_environment_variable(pid, "TEST_UNSET_ENV_VAR")
        end
    end

    class TestCase_RepositoryContents < Test::Unit::TestCase
        include RepositoryTestCase

        def contents
            contents = installed_pid.contents_key.parse_value
        end

        def entries
            entries = contents.entries
        end

        def test_contents
            assert_kind_of Contents, contents
        end

        def test_contents_entries
            assert_kind_of Array, entries
            assert_equal 3, entries.length
        end

        def test_first_entry
            assert_kind_of ContentsEntry, entries[0]
            assert_kind_of ContentsDirEntry, entries[0]
            assert_equal '/test', entries[0].location_key.parse_value
        end

        def test_second_entry
            assert_kind_of ContentsEntry, entries[1]
            assert_kind_of ContentsFileEntry, entries[1]
            assert_equal '/test/test_file', entries[1].location_key.parse_value
        end

        def test_third_entry
            assert_kind_of ContentsEntry, entries[2]
            assert_kind_of ContentsSymEntry, entries[2]
            assert_equal '/test/test_file', entries[2].target_key.parse_value
            assert_equal '/test/test_link', entries[2].location_key.parse_value
        end
    end

    class TestCase_RepositorySomeIdsMightSupport < Test::Unit::TestCase
        include RepositoryTestCase

        def test_some_ids_might_support
            assert repo.some_ids_might_support_action(SupportsActionTest.new(InstallAction))
            assert ! installed_repo.some_ids_might_support_action(SupportsActionTest.new(InstallAction))
        end
    end

    class TestCase_FakeRepository < Test::Unit::TestCase
        include RepositoryTestCase

        def fake
            FakeRepository.new(env, 'fake')
        end

        def test_new
            f = fake

            assert_kind_of FakeRepository, f
            assert_kind_of FakeRepositoryBase, f
            assert_equal 'fake', f.name
        end

        def test_new_bad
            assert_raise ArgumentError do FakeRepository.new end
            assert_raise ArgumentError do FakeRepository.new(env) end
            assert_raise ArgumentError do FakeRepository.new(env, 'fake', 42) end

            assert_raise TypeError do FakeRepository.new(repo, 'fake') end
            assert_raise TypeError do FakeRepository.new(env, 42) end

            assert_raise NameError do FakeRepository.new(env, 'f a k e') end
        end

        def test_add_category
            f = fake

            assert_equal [], f.category_names
            f.add_category('foo-bar')
            assert_equal ['foo-bar'], f.category_names
            f.add_category('foo-bar')
            assert_equal ['foo-bar'], f.category_names
            f.add_category('bar-foo')
            assert_equal ['bar-foo', 'foo-bar'], f.category_names
        end

        def test_add_category_bad
            f = fake

            assert_raise ArgumentError do f.add_category end
            assert_raise ArgumentError do f.add_category('foo-bar', 'bar-foo') end

            assert_raise TypeError do f.add_category(42) end

            assert_raise CategoryNamePartError do f.add_category('foo bar') end
        end

        def test_add_package
            f = fake

            foobar_baz = QualifiedPackageName.new('foo-bar', 'baz')
            foobar_quux = QualifiedPackageName.new('foo-bar', 'quux')
            barfoo_xyzzy = QualifiedPackageName.new('bar-foo', 'xyzzy')

            f.add_category('foo-bar')
            assert_equal [], f.package_names('foo-bar')
            f.add_package(foobar_baz)
            assert_equal [foobar_baz], f.package_names('foo-bar')
            f.add_package('foo-bar/baz')
            assert_equal [foobar_baz], f.package_names('foo-bar')
            f.add_package(foobar_quux)
            assert_equal [foobar_baz, foobar_quux], f.package_names('foo-bar')

            f.add_package(barfoo_xyzzy)
            assert_equal ['bar-foo', 'foo-bar'], f.category_names
            assert_equal [barfoo_xyzzy], f.package_names('bar-foo')
        end

        def test_add_package_bad
            f = fake

            assert_raise ArgumentError do f.add_package end
            assert_raise ArgumentError do
                f.add_package(Paludis::QualifiedPackageName.new('foo-bar', 'baz'), 42)
            end

            assert_raise TypeError do f.add_package(42) end

            assert_raise CategoryNamePartError do f.add_package('test') end
            assert_raise CategoryNamePartError do f.add_package('f o o/bar') end
            assert_raise PackageNamePartError do f.add_package('foo/b a r') end
        end

        def test_add_version
            f = fake

            f.add_category('foo-bar')
            f.add_package('foo-bar/baz')
            assert_equal [], f.package_ids('foo-bar/baz')
            pkg = f.add_version('foo-bar/baz', '123')
            assert_equal [pkg], f.package_ids('foo-bar/baz')
            assert_kind_of PackageID, pkg
            assert_equal QualifiedPackageName.new('foo-bar/baz'), pkg.name
            assert_equal VersionSpec.new('123'), pkg.version

            f.add_version('bar-foo/quux', '42')
            assert_equal ['bar-foo', 'foo-bar'], f.category_names
            assert_equal [QualifiedPackageName.new('bar-foo/quux')], f.package_names('bar-foo')

            pkg2 = f.add_version('abc-def', 'ghi', 'scm')
            assert_kind_of PackageID, pkg2
            assert_equal QualifiedPackageName.new('abc-def/ghi'), pkg2.name
            assert_equal VersionSpec.new('scm'), pkg2.version
            assert_equal ['abc-def', 'bar-foo', 'foo-bar'], f.category_names
            assert_equal [QualifiedPackageName.new('abc-def/ghi')], f.package_names('abc-def')
            assert_equal [pkg2], f.package_ids('abc-def/ghi')
        end

        def test_add_version_bad
            f = fake

            assert_raise ArgumentError do f.add_version end
            assert_raise ArgumentError do f.add_version('foo-bar/baz') end
            assert_raise ArgumentError do f.add_version('foo-bar', 'baz', '3', 'abc', '123') end

            assert_raise TypeError do f.add_version(42, '1') end
            assert_raise TypeError do f.add_version('foo-bar/baz', []) end

            assert_raise TypeError do f.add_version(proc {}, 'quux', '1.5') end
            assert_raise TypeError do f.add_version('foo-bar', {}, '9') end
            assert_raise TypeError do f.add_version('foo-bar', 'baz', Paludis) end

            assert_raise CategoryNamePartError do f.add_version('foo', '42') end
            assert_raise CategoryNamePartError do f.add_version('f o o/bar', '42') end
            assert_raise PackageNamePartError do f.add_version('foo/b a r' , '42') end
            assert_raise BadVersionSpecError do f.add_version('foo/bar', 'abc') end

            assert_raise CategoryNamePartError do f.add_version('f o o', 'bar', '42') end
            assert_raise PackageNamePartError do f.add_version('foo', 'b a r', '42') end
            assert_raise BadVersionSpecError do f.add_version('foo', 'bar', 'abc') end
        end
    end

    class TestCase_Repository < Test::Unit::TestCase
        include RepositoryTestCase

        def test_format_key
            assert_kind_of MetadataStringKey, repo.format_key
            assert_equal 'e', repo.format_key.parse_value
            assert_kind_of MetadataStringKey, installed_repo.format_key
            assert_equal 'vdb', installed_repo.format_key.parse_value
        end

        def test_installed_root_key
            assert_nil repo.installed_root_key
            assert_kind_of MetadataFSPathKey, installed_repo.installed_root_key
            assert_equal '/', installed_repo.installed_root_key.parse_value
        end

        def test_each_metadata
            assert_respond_to repo, :each_metadata
        end

        def test_subscript
            assert_respond_to repo, :[]
            assert_kind_of MetadataStringKey, repo['format']
            assert_equal 'e', repo['format'].parse_value
            assert_nil repo['monkey']
        end

        def test_sync
            assert_kind_of MetadataStringStringMapKey, repo['sync']
            assert_kind_of Hash, repo['sync'].parse_value
            assert_equal 3, repo['sync'].parse_value.size
            assert_equal 'normalsync', repo['sync'].parse_value['']
            assert_equal 'foosync', repo['sync'].parse_value['foo']
            assert_equal 'barsync', repo['sync'].parse_value['bar']
        end
    end
end

