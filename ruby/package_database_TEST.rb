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

    class TestCase_PackageDatabaseFavouriteRepository < Test::Unit::TestCase
        def env
            @env or @env = EnvironmentMaker.instance.make_from_spec("")
        end

        def test_package_database_favourite_repository
            assert_equal "testrepo", env.package_database.favourite_repository
        end
    end

    class TestCase_PackageDatabaseFetchUniqueQualifiedPackageName < Test::Unit::TestCase
        def env
            @env or @env = EnvironmentMaker.instance.make_from_spec("")
        end

        def db
            return env.package_database
        end

        def test_package_database_fetch_unique_qualified_package_name
            assert_equal "foo/bar", db.fetch_unique_qualified_package_name("bar")
        end

        def test_error
            assert_raise AmbiguousPackageNameError do
                db.fetch_unique_qualified_package_name('baz')
            end
            assert_raise NoSuchPackageError do
                db.fetch_unique_qualified_package_name('foobarbaz')
            end
        end
    end

    class TestCase_PackageDatabaseQuery < Test::Unit::TestCase
        def env
            @env or @env = EnvironmentMaker.instance.make_from_spec("")
        end

        def db
            return env.package_database
        end

        def test_arg_count
            assert_raise ArgumentError do
                db.query(1);
            end

            #outputs a deprecation warning
            assert_nothing_raised do
                db.query(PackageDepSpec.new("=foo/bar-1.0"), InstallState::Any)
            end

            assert_nothing_raised do
                db.query(PackageDepSpec.new("=foo/bar-1.0"), InstallState::Any, QueryOrder::Whatever)
            end

            assert_nothing_raised do
                db.query(Query::Matches.new(PackageDepSpec.new('=foo/bar-1.0')), QueryOrder::Whatever)
            end

            assert_raise ArgumentError do
                db.query(1,2,3,4);
            end
        end

        def test_package_database_query
            a = db.query("=foo/bar-1.0", InstallState::InstallableOnly, QueryOrder::Whatever)
            assert_equal a, [ PackageDatabaseEntry.new("foo/bar", "1.0", "testrepo") ]

            a = db.query(Query::Matches.new('=foo/bar-1.0') & Query::RepositoryHasInstallableInterface.new,
                         QueryOrder::Whatever)
            assert_equal a, [ PackageDatabaseEntry.new("foo/bar", "1.0", "testrepo") ]

            a = db.query(PackageDepSpec.new("=foo/bar-1.0"), InstallState::Any, QueryOrder::Whatever)
            assert_equal a, [ PackageDatabaseEntry.new("foo/bar", "1.0", "testrepo") ]

            a = db.query("foo/bar", InstallState::InstallableOnly, QueryOrder::OrderByVersion)
            assert_equal a, [
                PackageDatabaseEntry.new("foo/bar", "1.0", "testrepo"),
                PackageDatabaseEntry.new("foo/bar", "2.0", "testrepo")
            ]

            a = db.query(Query::Package.new('foo/bar'), QueryOrder::OrderByVersion)
            assert_equal a, [
                PackageDatabaseEntry.new("foo/bar", "1.0", "testrepo"),
                PackageDatabaseEntry.new("foo/bar", "2.0", "testrepo")
            ]

            a = db.query(">=foo/bar-27", InstallState::InstallableOnly, QueryOrder::Whatever)
            assert a.empty?

            a = db.query("foo/bar", InstallState::InstalledOnly, QueryOrder::Whatever)
            assert a.empty?
        end

        def test_package_database_query_bad
            assert_raise TypeError do
                db.query(123, InstallState::Any)
            end
            assert_raise TypeError do
                db.query(PackageDepSpec.new("foo/bar"), "Either")
            end
        end
    end

    class TestCase_PackageDatabaseRepositories < Test::Unit::TestCase
        def env
            @env or @env = EnvironmentMaker.instance.make_from_spec("")
        end

        def db
            return env.package_database
        end

        def test_repositories
            assert_equal 3, db.repositories.length

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
            assert db.more_important_than('testrepo', 'virtuals')
            assert ! db.more_important_than('virtuals', 'testrepo')
        end
    end
end

