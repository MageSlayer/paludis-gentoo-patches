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

ENV["PALUDIS_HOME"] = Dir.getwd().to_s + "/environment_TEST_dir/home";

require 'test/unit'
require 'Paludis'

Paludis::Log.instance.log_level = Paludis::LogLevel::Warning

module Paludis
    class TestCase_Environment < Test::Unit::TestCase
        def test_no_create
            assert_raise NoMethodError do
                x = Environment.new()
            end
        end
    end

    class TestCase_NoConfigEnvironment < Test::Unit::TestCase
        def test_create
            assert_nothing_raised do
                e = NoConfigEnvironment.new(Dir.getwd().to_s + "/environment_TEST_dir/testrepo")
                assert_kind_of Environment, e
                assert_kind_of NoConfigEnvironment, e
            end

            assert_nothing_raised do
                e = NoConfigEnvironment.new(Dir.new(Dir.getwd().to_s + "/environment_TEST_dir/testrepo"))
                assert_kind_of Environment, e
                assert_kind_of NoConfigEnvironment, e
            end

            assert_nothing_raised do
                e = NoConfigEnvironment.new(Dir.getwd().to_s + "/environment_TEST_dir/testrepo", '/var/empty')
                assert_kind_of Environment, e
                assert_kind_of NoConfigEnvironment, e
            end

            assert_raise TypeError do
                e = NoConfigEnvironment.new(Dir.getwd().to_s + "/environment_TEST_dir/testrepo", 7)
            end

            assert_raise TypeError do
                e = NoConfigEnvironment.new(7, '/var/empty')
            end

            assert_raise ArgumentError do
                e = NoConfigEnvironment.new
            end

            assert_raise ArgumentError do
                e = NoConfigEnvironment.new(1,2,3,4)
            end
        end
    end

    class TestCase_EnvironmentUse < Test::Unit::TestCase
        def env
            @env or @env = EnvironmentMaker.instance.make_from_spec("")
        end

        def test_query_use
            assert env.query_use("enabled")
            assert ! env.query_use("not_enabled")
            assert ! env.query_use("sometimes_enabled")

            pde = PackageDatabaseEntry.new("foo/bar", VersionSpec.new("1.0"), "testrepo")

            assert env.query_use("enabled", pde)
            assert ! env.query_use("not_enabled", pde)
            assert env.query_use("sometimes_enabled", pde)
        end

        def test_query_use_bad
            assert_raise ArgumentError do
                env.query_use(1, 2, 3)
            end
            assert_raise TypeError do
                env.query_use(123)
            end
        end
    end

    class TestCase_NoConfigEnvironmentUse < Test::Unit::TestCase
        def env
            NoConfigEnvironment.new(Dir.getwd().to_s + "/environment_TEST_dir/testrepo")
        end

        def test_query_use
            assert ! env.query_use("foo")
            pde = PackageDatabaseEntry.new("foo/bar", VersionSpec.new("1.0"), "testrepo")
            assert ! env.query_use("foo", pde)
        end

        def test_query_use_bad
            assert_raise ArgumentError do
                env.query_use(1, 2, 3)
            end
            assert_raise TypeError do
                env.query_use(123)
            end
        end
    end

    class TestCase_EnvironmentAcceptKeyword < Test::Unit::TestCase
        def env
            @env or @env = EnvironmentMaker.instance.make_from_spec("")
        end

        def test_accept_keyword
            assert env.accept_keyword("test")
            assert ! env.accept_keyword("bad")
            assert ! env.accept_keyword("~test")

            pde = PackageDatabaseEntry.new("foo/bar", "1.0", "testrepo")

            assert env.accept_keyword("test", pde)
            assert ! env.accept_keyword("bad", pde)
            assert env.accept_keyword("~test", pde)
        end

        def test_accept_keyword_bad
            assert_raise ArgumentError do
                env.accept_keyword(1, 2, 3)
            end
            assert_raise TypeError do
                env.accept_keyword(123)
            end
        end
    end

    class TestCase_NoConfigEnvironmentAcceptKeyword < Test::Unit::TestCase
        def env
            NoConfigEnvironment.new(Dir.getwd().to_s + "/environment_TEST_dir/testrepo")
        end

        def test_accept_keyword
            assert env.accept_keyword("test")
            assert ! env.accept_keyword("bad")
            assert ! env.accept_keyword("~test")

            pde = PackageDatabaseEntry.new("foo/bar", "1.0", "testrepo")

            assert env.accept_keyword("test", pde)
            assert ! env.accept_keyword("bad", pde)
            assert ! env.accept_keyword("~test", pde)
        end

        def test_accept_keyword_bad
            assert_raise ArgumentError do
                env.accept_keyword(1, 2, 3)
            end
            assert_raise TypeError do
                env.accept_keyword(123)
            end
        end
    end

    class TestCase_EnvironmentAcceptLicense < Test::Unit::TestCase
        def env
            @env or @env = EnvironmentMaker.instance.make_from_spec("")
        end

        def test_accept_license
            assert env.accept_license("test")

            pde = PackageDatabaseEntry.new("foo/bar", VersionSpec.new("1.0"), "testrepo")

            assert env.accept_license("test", pde)
        end

        def test_accept_license_bad
            assert_raise ArgumentError do
                env.accept_license(1, 2, 3)
            end
            assert_raise TypeError do
                env.accept_license(123)
            end
        end
    end

    class TestCase_NoConfigEnvironmentAcceptLicense < Test::Unit::TestCase
        def env
            NoConfigEnvironment.new(Dir.getwd().to_s + "/environment_TEST_dir/testrepo")
        end

        def test_accept_license
            assert env.accept_license("test")
            pde = PackageDatabaseEntry.new("foo/bar", VersionSpec.new("1.0"), "testrepo")
            assert env.accept_license("test", pde)
        end

        def test_accept_license_bad
            assert_raise ArgumentError do
                env.accept_license(1, 2, 3)
            end
            assert_raise TypeError do
                env.accept_license(123)
            end
        end
    end

    class TestCase_NoConfigEnvironmentMaskReasons < Test::Unit::TestCase
        def env
            NoConfigEnvironment.new(Dir.getwd().to_s + "/environment_TEST_dir/testrepo")
        end

        def test_mask_reasons
            p = PackageDatabaseEntry.new("foo/bar", VersionSpec.new("1.0"), "testrepo")

            m = env.mask_reasons(p)
            assert m.empty?
        end

        def test_mask_reasons_not_empty
            p = PackageDatabaseEntry.new("foo/bar", VersionSpec.new("2.0"), "testrepo")

            m = env.mask_reasons(p)
            assert ! m.empty?
            assert m.include?(MaskReason::Keyword)
            assert_equal([MaskReason::Keyword], m.to_a)
        end

        def test_mask_reasons_no_such_repo
            p = PackageDatabaseEntry.new("foo/bar", VersionSpec.new("1.0"), "nosuchrepo")

            assert_raise Paludis::NoSuchRepositoryError do
                env.mask_reasons p
            end
        end

        def test_mask_reasons_bad
            assert_raise ArgumentError do
                env.mask_reasons(1, 2)
            end
            assert_raise TypeError do
                env.mask_reasons(123)
            end
        end
    end

    class TestCase_EnvironmentQueryUserMasks < Test::Unit::TestCase
        def env
            @env or @env = EnvironmentMaker.instance.make_from_spec("")
        end

        def test_query_user_masks
            p2 = PackageDatabaseEntry.new("foo/bar", VersionSpec.new("2.0"), "testrepo")
            p3 = PackageDatabaseEntry.new("foo/bar", VersionSpec.new("3.0"), "testrepo")

            assert ! env.query_user_masks(p2)
            assert env.query_user_masks(p3)
        end

        def test_query_user_unmasks
            p2 = PackageDatabaseEntry.new("foo/bar", VersionSpec.new("2.0"), "testrepo")
            p3 = PackageDatabaseEntry.new("foo/bar", VersionSpec.new("3.0"), "testrepo")

            assert env.query_user_unmasks(p2)
            assert ! env.query_user_unmasks(p3)
        end

        def test_query_user_masks_bad
            p = PackageDatabaseEntry.new("foo/bar", VersionSpec.new("2.0"), "testrepo")
            assert_raise ArgumentError do
                env.query_user_masks(p, p)
            end
            assert_raise TypeError do
                env.query_user_masks(123)
            end
        end

        def test_query_user_unmasks_bad
            p = PackageDatabaseEntry.new("foo/bar", VersionSpec.new("2.0"), "testrepo")
            assert_raise ArgumentError do
                env.query_user_unmasks(p, p)
            end
            assert_raise TypeError do
                env.query_user_unmasks(123)
            end
        end
    end

    class TestCase_EnvironmentPackageDatabase < Test::Unit::TestCase
        def env
            @env or @env = EnvironmentMaker.instance.make_from_spec("")
        end

        def db
            env.package_database
        end

        def test_package_database
            assert_kind_of PackageDatabase, db
            assert_equal "testrepo", db.fetch_repository("testrepo").name
        end
    end

    class TestCase_NoConfigEnvironmentPackageDatabase < Test::Unit::TestCase
        def env
            NoConfigEnvironment.new(Dir.getwd().to_s + "/environment_TEST_dir/testrepo")
        end

        def db
            env.package_database
        end

        def test_package_database
            assert_kind_of PackageDatabase, db
            assert_equal "testrepo", db.fetch_repository("testrepo").name
        end
    end

    class TestCase_EnvironmentPackageSet < Test::Unit::TestCase
        def env
            @env or @env = EnvironmentMaker.instance.make_from_spec("")
        end

        def test_package_set
            assert_kind_of DepSpec, env.package_set('everything')
        end

        def test_package_set_error
            assert_raise SetNameError do
                env.package_set('broken*')
            end
        end
    end

    class TestCase_NoConfigEnvironmentPackageSet < Test::Unit::TestCase
        def env
            NoConfigEnvironment.new(Dir.getwd().to_s + "/environment_TEST_dir/testrepo")
        end

        def test_package_set
            assert_kind_of DepSpec, env.package_set('everything')
        end

        def test_package_set_error
            assert_raise SetNameError do
                env.package_set('broken*')
            end
        end
    end

    class TestCase_NoConfigEnvirontmentPortageRepository < Test::Unit::TestCase
        def env
            NoConfigEnvironment.new(Dir.getwd().to_s + "/environment_TEST_dir/testrepo")
        end

        def test_portage_repository
            assert_kind_of PortageRepository, env.portage_repository
        end
    end

    class TestCase_NoConfigEnvirontmentMasterRepository < Test::Unit::TestCase
        def env
            NoConfigEnvironment.new(Dir.getwd().to_s + "/environment_TEST_dir/testrepo")
        end

        def env_master
            NoConfigEnvironment.new(Dir.getwd().to_s + "/environment_TEST_dir/testrepo",
                                   "/var/empty", 
                                   Dir.getwd().to_s + "/environment_TEST_dir/slaverepo")
        end

        def test_master_repository
            assert_nil env.master_repository
            assert_kind_of PortageRepository, env_master.master_repository
        end
    end

    class TestCase_EnvironmentRoot < Test::Unit::TestCase
        def test_root
            assert_kind_of String, env.root
        end
    end

    class TestCase_EnvironmentDefaultDestinations < Test::Unit::TestCase
        def test_default_destinations
            assert_kind_of Array, env.default_destinations
        end
    end

    class TestCase_EnvironmentRoot < Test::Unit::TestCase
        def env
            NoConfigEnvironment.new(Dir.getwd().to_s + "/environment_TEST_dir/testrepo")
        end

        def test_root
            assert_kind_of String, env.root
        end
    end

    class TestCase_EnvironmentDefaultDestinations < Test::Unit::TestCase
        def env
            NoConfigEnvironment.new(Dir.getwd().to_s + "/environment_TEST_dir/testrepo")
        end

        def test_default_destinations
            assert_kind_of Array, env.default_destinations
        end
    end

    class TestCase_EnvironmentSetAcceptUnstable < Test::Unit::TestCase
        def env
            NoConfigEnvironment.new(Dir.getwd().to_s + "/environment_TEST_dir/testrepo")
        end

        def test_set_accept_unstable
            assert_respond_to env, :accept_unstable=
        end
    end
end

