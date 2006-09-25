#!/usr/bin/ruby
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

class Paludis
    class TestCase_Repository < Test::Unit::TestCase
        def test_no_create
            assert_raise NoMethodError do
                p = Repository.new
            end
        end
    end

    module RepositoryTestCase
        def repo
            db.fetch_repository "testrepo"
        end

        def db
            DefaultEnvironment.instance.package_database
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
            assert_equal "foo", a[0]
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
end

