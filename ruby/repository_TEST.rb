#!/usr/bin/env ruby
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

    class TestCase_PortageRepository < Test::Unit::TestCase
        def test_no_create
            assert_raise NoMethodError do
                p = PortageRepository.new
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

        def db
            DefaultEnvironment.instance.package_database
        end

        def no_config_testrepo
            NoConfigEnvironment.new Dir.getwd().to_s + "/repository_TEST_dir/testrepo"
        end
    end

    class TestCase_RepositoryHasVersion < Test::Unit::TestCase
        include RepositoryTestCase

        def test_has_version
            assert repo.has_version?("foo/bar", "1.0")
            assert repo.has_version?("foo/bar", "2.0")

            assert repo.has_version?("foo/bar", VersionSpec.new("1.0"))
            assert repo.has_version?("foo/bar", VersionSpec.new("2.0"))

            assert ! repo.has_version?("foo/barbar", "1.0")
            assert ! repo.has_version?("foo/bar", "3.0")

            assert ! repo.has_version?("foo/barbar", VersionSpec.new("1.0"))
            assert ! repo.has_version?("foo/bar", VersionSpec.new("3.0"))
        end

        def test_has_version_error
            assert_raise TypeError do
                repo.has_version?('foo/bar', PackageDepAtom.new('foo-bar/baz'))
            end
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

        def test_version_specs
            a = repo.version_specs "foo/bar"
            assert_equal 2, a.length
            assert_equal VersionSpec.new("1.0"), a[0]
            assert_equal VersionSpec.new("2.0"), a[1]

            b = repo.version_specs "bar/baz"
            assert b.empty?
        end
    end

    class TestCase_RepositoryCategoryNames < Test::Unit::TestCase
        include RepositoryTestCase

        def test_category_names
            a = repo.category_names
            assert_equal 1, a.length
            assert_equal "foo", a.first
        end
    end

    class TestCase_RepositoryCategoryNamesContainingPackage < Test::Unit::TestCase
        include RepositoryTestCase

        def test_category_names_containing_package
            a = repo.category_names_containing_package('bar')
            assert_equal 1, a.length
            assert_equal "foo", a.first
        end
    end

    class TestCase_RepositoryPackageNames < Test::Unit::TestCase
        include RepositoryTestCase

        def test_package_names
            a = repo.package_names "foo"
            assert_equal 1, a.length
            assert_equal "foo/bar", a[0]

            assert repo.package_names("bar").empty?
        end
    end

    class TestCase_RepositoryInterfaces < Test::Unit::TestCase
        include RepositoryTestCase

        def test_interfaces
            assert_equal repo.name, repo.installable_interface.name
            assert_nil repo.installed_interface

            assert_equal installed_repo.name, installed_repo.installed_interface.name
            assert_nil installed_repo.installable_interface
        end
    end

    class TestCase_RepositoryContents < Test::Unit::TestCase
        include RepositoryTestCase

        def entries
            contents = installed_repo.contents('cat-one/pkg-one','1')
            entries = contents.entries
        end

        def test_contents
            contents = installed_repo.contents('cat-one/pkg-one','1')
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

    class TestCase_RepositoryInstalledTime < Test::Unit::TestCase
        include RepositoryTestCase

        def test_time
            time = installed_repo.installed_time('cat-one/pkg-one','1')
            assert_kind_of Time, time
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

    class TestCase_PortageRepositoryProfilesDescLine < Test::Unit::TestCase
        include RepositoryTestCase

        def profiles
            no_config_testrepo.portage_repository.profiles
        end

        def test_profiles
            assert_kind_of Array, profiles
            assert_equal 1, profiles.length
            assert_kind_of PortageRepositoryProfilesDescLine, profiles.first
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
end

