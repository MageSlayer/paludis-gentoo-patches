#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :

#
# Copyright (c) 2006, 2007, 2008, 2009, 2011 Ciaran McCreesh
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

ENV["PALUDIS_HOME"] = Dir.getwd().to_s + "/package_database_TEST_dir/home";

require 'test/unit'
require 'Paludis'

Paludis::Log.instance.log_level = Paludis::LogLevel::Warning

module Paludis
    class TestCase_PackageDatabase < Test::Unit::TestCase
        def test_no_create
            assert_raise NoMethodError do
                p = PackageDatabase.new
            end
        end
    end

    class TestCase_PackageDatabaseFetchUniqueQualifiedPackageName < Test::Unit::TestCase
        def env
            @env or @env = EnvironmentFactory.instance.create("")
        end

        def db
            return env.package_database
        end

        def test_package_database_fetch_unique_qualified_package_name
            assert_equal "foo/bar", db.fetch_unique_qualified_package_name("bar")
            assert_equal "foo/bar", db.fetch_unique_qualified_package_name("bar", Filter::SupportsAction.new(InstallAction))
        end

        def test_error
            assert_raise AmbiguousPackageNameError do
                db.fetch_unique_qualified_package_name('baz')
            end
            assert_raise NoSuchPackageError do
                db.fetch_unique_qualified_package_name('foobarbaz')
            end
            assert_raise NoSuchPackageError do
                db.fetch_unique_qualified_package_name('bar', Filter::SupportsAction.new(ConfigAction))
            end
        end

        def test_bad
            assert_raise ArgumentError do
                db.fetch_unique_qualified_package_name
            end
            assert_raise ArgumentError do
                db.fetch_unique_qualified_package_name(1, 2, 3)
            end
            assert_raise TypeError do
                db.fetch_unique_qualified_package_name([])
            end
            assert_raise TypeError do
                db.fetch_unique_qualified_package_name('bar', db)
            end
        end
    end

    class TestCase_PackageDatabaseRepositories < Test::Unit::TestCase
        def env
            @env or @env = EnvironmentFactory.instance.create("")
        end

        def db
            return env.package_database
        end

        def test_repositories
            if ENV["PALUDIS_ENABLE_VIRTUALS_REPOSITORY"] == "yes" then
                assert_equal 3, db.repositories.length
            else
                assert_equal 1, db.repositories.length
            end

            a = db.repositories.find_all do | repo |
                repo.name == "testrepo"
            end
            assert_equal 1, a.length

            a = db.repositories.find_all do | repo |
                repo.name == "foorepo"
            end
            assert a.empty?

            assert_equal nil, db.repositories {|repo| assert_kind_of Repository, repo}
        end

        def test_fetch_repository
            assert_equal "testrepo", db.fetch_repository("testrepo").name

            assert_raise Paludis::NoSuchRepositoryError do
                db.fetch_repository("barrepo")
            end
        end

        def test_more_important_than
            if ENV["PALUDIS_ENABLE_VIRTUALS_REPOSITORY"] == "yes" then
                assert db.more_important_than('testrepo', 'virtuals')
                assert ! db.more_important_than('virtuals', 'testrepo')
            elsif ENV["PALUDIS_ENABLE_VIRTUALS_REPOSITORY"] == "no" then
            else
                throw "oops"
            end
        end

        def test_has_repository_named?
            assert db.has_repository_named?('testrepo')
            if ENV["PALUDIS_ENABLE_VIRTUALS_REPOSITORY"] == "yes" then
                assert db.has_repository_named?('virtuals')
            else
                assert ! db.has_repository_named?('virtuals')
            end
            assert ! db.has_repository_named?('foobarbaz')
        end
    end
end

