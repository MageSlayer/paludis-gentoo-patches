#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :

#
# Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
# Copyright (c) 2006, 2007 Richard Brown <rbrown@gentoo.org>
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
            db.fetch_repository "installed"
        end

        def repo
            db.fetch_repository "testrepo"
        end

        def env
            @env or @env = EnvironmentMaker.instance.make_from_spec("")
        end

        def db
            env.package_database
        end

        def no_config_testrepo
            NoConfigEnvironment.new Dir.getwd().to_s + "/repository_TEST_dir/testrepo"
        end

        def p
            db.query(Query::Matches.new(PackageDepSpec.new('=foo/bar-2.0::testrepo', PackageDepSpecParseMode::Permissive)), QueryOrder::RequireExactlyOne).first
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

        def test_format
            assert_equal "vdb", installed_repo.format
            assert_equal "ebuild", repo.format
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
            assert_equal VersionSpec.new("1.0"), a[0].version
            assert_equal VersionSpec.new("2.0"), a[1].version

            assert_nothing_raised do
                repo.package_ids('foo/bar') do |pid|
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
            [:installed_interface, :mask_interface,
                :sets_interface, :syncable_interface, :use_interface,
                :world_interface, :mirrors_interface, :environment_variable_interface,
                :provides_interface, :virtuals_interface, :e_interface].each do |sym|
                assert_respond_to repo, sym
            end
        end

        def test_interfaces
            assert_equal repo.name, repo.mask_interface.name
            assert_nil repo.installed_interface

            assert_equal installed_repo.name, installed_repo.provides_interface.name
            assert_nil installed_repo.syncable_interface
        end
    end

    class TestCase_RepositoryContents < Test::Unit::TestCase
        include RepositoryTestCase

        def contents
            contents = installed_pid.contents_key.value
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
            assert_equal '//test', entries[0].to_s
            assert_equal '//test', entries[0].name
        end

        def test_second_entry
            assert_kind_of ContentsEntry, entries[1]
            assert_kind_of ContentsFileEntry, entries[1]
            assert_equal '/test/test_file', entries[1].to_s
            assert_equal '/test/test_file', entries[1].name
        end

        def test_third_entry
            assert_kind_of ContentsEntry, entries[2]
            assert_kind_of ContentsSymEntry, entries[2]
            assert_equal '/test/test_link -> /test/test_file', entries[2].to_s
            assert_equal '/test/test_file', entries[2].target
            assert_equal '/test/test_link', entries[2].name
        end
    end

    class TestCase_RepositoryQueryUse < Test::Unit::TestCase
        include RepositoryTestCase

        def test_query_use_local

            assert repo.query_use('test1',p) == true
            assert repo.query_use('test2',p) == false
            assert repo.query_use('test3',p) == true
            assert repo.query_use('test4',p) == nil
            assert repo.query_use('test5',p) == false
            assert repo.query_use('test6',p) == true
            assert repo.query_use('test7',p) == true
        end

        def test_query_use_bad
            assert_raise TypeError do
                repo.query_use('test1',{})
            end

            assert_raise ArgumentError do
                repo.query_use
                repo.query_use(1,2,3)
            end
        end
    end

    class TestCase_RepositoryQueryUseMask < Test::Unit::TestCase
        include RepositoryTestCase

        def test_query_use_mask_local

            assert ! repo.query_use_mask('test1',p)
            assert ! repo.query_use_mask('test2',p)
            assert ! repo.query_use_mask('test3',p)
            assert ! repo.query_use_mask('test4',p)
            assert repo.query_use_mask('test5',p)
            assert ! repo.query_use_mask('test6',p)
            assert ! repo.query_use_mask('test7',p)
        end

        def test_query_use_mask_bad
            assert_raise TypeError do
                repo.query_use_mask('test1',{})
            end

            assert_raise ArgumentError do
                repo.query_use_mask
                repo.query_use_mask(1,2,3)
            end
        end
    end

    class TestCase_RepositoryQueryUseForce < Test::Unit::TestCase
        include RepositoryTestCase

        def test_query_use_force_local

            assert ! repo.query_use_force('test1',p)
            assert ! repo.query_use_force('test2',p)
            assert ! repo.query_use_force('test3',p)
            assert ! repo.query_use_force('test4',p)
            assert ! repo.query_use_force('test5',p)
            assert repo.query_use_force('test6',p)
            assert repo.query_use_force('test7',p)
        end

        def test_query_use_force_bad
            assert_raise TypeError do
                repo.query_use_force('test1',{})
            end

            assert_raise ArgumentError do
                repo.query_use_force
                repo.query_use_force(1,2,3)
            end
        end
    end

    class TestCase_QueryRepositoryMasks < Test::Unit::TestCase
        include RepositoryTestCase

        def test_repository_masks
###            assert repo.query_repository_masks("foo1/bar","1.0")
###            assert repo.query_repository_masks("foo2/bar","1.0")
###            assert ! repo.query_repository_masks("foo3/bar","1.0")
###            assert ! repo.query_repository_masks("foo4/bar","1.0")
        end

        def test_repository_masks_bad
###            assert_raise TypeError do
###                repo.query_repository_masks(42,"1.0")
###            end
###            assert_raise TypeError do
###                repo.query_repository_masks("foo/bar",[])
###            end

###            assert_raise ArgumentError do
###                repo.query_repository_masks
###            end
###            assert_raise ArgumentError do
###                repo.query_repository_masks("foo/bar")
###            end
###            assert_raise ArgumentError do
###                repo.query_repository_masks("foo/bar","1.0","baz")
###            end
        end
    end

    class TestCase_QueryProfileMasks < Test::Unit::TestCase
        include RepositoryTestCase

        def test_profile_masks
###            assert repo.query_profile_masks("foo1/bar","1.0")
###            assert ! repo.query_profile_masks("foo2/bar","1.0")
###            assert repo.query_profile_masks("foo3/bar","1.0")
###            assert ! repo.query_profile_masks("foo4/bar","1.0")
        end

        def test_profile_masks_bad
###            assert_raise TypeError do
###                repo.query_profile_masks(42,"1.0")
###            end
###            assert_raise TypeError do
###                repo.query_profile_masks("foo/bar",[])
###            end

            assert_raise ArgumentError do
                repo.query_profile_masks
            end
###            assert_raise ArgumentError do
###                repo.query_profile_masks("foo/bar")
###            end
            assert_raise ArgumentError do
                repo.query_profile_masks("foo/bar","1.0","baz")
            end
        end
    end

    class TestCase_RepositoryInfo < Test::Unit::TestCase
        include RepositoryTestCase

        def test_info
            assert_kind_of RepositoryInfo, repo.info(false)
            assert_kind_of RepositoryInfo, repo.info(true)
        end

        def test_sections
            assert_kind_of Array, repo.info(false).sections
            assert_equal 1, repo.info(false).sections.length
        end

        def test_section_header
            assert_kind_of String, repo.info(false).sections.first.header
            assert_equal 'Configuration information', repo.info(false).sections.first.header
        end

        def test_section_kvs
            assert_kind_of Hash, repo.info(false).sections.first.kvs
            assert_equal 'ebuild', repo.info(false).sections.first.kvs['format']
            assert_equal 'vdb', installed_repo.info(false).sections.first.kvs['format']
        end
    end

    class TestCase_RepositoryPortageInterface < Test::Unit::TestCase
        include RepositoryTestCase

        def test_responds
            repo = no_config_testrepo.main_repository
            [:profile_variable, :profiles, :find_profile, :set_profile].each do |sym|
                assert_respond_to repo, sym
            end
        end

        def test_profiles
            repo = no_config_testrepo.main_repository
            assert_kind_of Array, repo.profiles
        end

        def test_find_profile
            repo = no_config_testrepo.main_repository
            assert_nothing_raised do
                profile = repo.find_profile(Dir.getwd().to_s + '/repository_TEST_dir/testrepo/profiles/testprofile')
                assert_kind_of ProfilesDescLine, profile
                profile = repo.find_profile('broken')
                assert profile.nil?
            end
        end

        def test_set_profile
            repo = no_config_testrepo.main_repository
            assert_nothing_raised do
                profile = repo.profiles.first
                repo.set_profile(profile)
            end
        end

        def test_profile_variable
            repo = no_config_testrepo.main_repository
            assert_nothing_raised do
                assert_equal 'test', repo.profile_variable('ARCH')
            end
        end
    end

    class TestCase_ProfilesDescLine < Test::Unit::TestCase
        include RepositoryTestCase

        def profiles
            no_config_testrepo.main_repository.profiles
        end

        def test_profiles
            assert_kind_of Array, profiles
            assert_equal 1, profiles.length
            assert_kind_of ProfilesDescLine, profiles.first
        end

        def test_respond
            assert_respond_to profiles.first, :path
            assert_respond_to profiles.first, :arch
            assert_respond_to profiles.first, :status
        end

        def test_profile_path
            assert_kind_of String, profiles.first.path
            assert_equal Dir.getwd().to_s + "/repository_TEST_dir/testrepo/profiles/testprofile",
                profiles.first.path
        end

        def test_profile_arch
            assert_kind_of String, profiles.first.arch
            assert_equal 'x86', profiles.first.arch
        end

        def test_profile_status
            assert_kind_of String, profiles.first.status
            assert_equal 'stable', profiles.first.status
        end
    end

    class TestCase_RepositoryDescribeUseFlag < Test::Unit::TestCase
        include RepositoryTestCase

        def test_responds
            assert_respond_to repo, :describe_use_flag
        end

        def test_two_args
            assert_kind_of String, repo.describe_use_flag('test1', p)
            assert_equal 'A test local use flag', repo.describe_use_flag('test2', p)
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

            assert_raise NameError do f.add_package('test') end
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
            assert_raise ArgumentError do f.add_version('foo-bar', 'baz', '3', 'abc') end

            assert_raise TypeError do f.add_version(42, '1') end
            assert_raise TypeError do f.add_version('foo-bar/baz', []) end

            assert_raise TypeError do f.add_version(proc {}, 'quux', '1.5') end
            assert_raise TypeError do f.add_version('foo-bar', {}, '9') end
            assert_raise TypeError do f.add_version('foo-bar', 'baz', Paludis) end

            assert_raise NameError do f.add_version('foo', '42') end
            assert_raise CategoryNamePartError do f.add_version('f o o/bar', '42') end
            assert_raise PackageNamePartError do f.add_version('foo/b a r' , '42') end
            assert_raise BadVersionSpecError do f.add_version('foo/bar', 'abc') end

            assert_raise CategoryNamePartError do f.add_version('f o o', 'bar', '42') end
            assert_raise PackageNamePartError do f.add_version('foo', 'b a r', '42') end
            assert_raise BadVersionSpecError do f.add_version('foo', 'bar', 'abc') end
        end
    end
end

