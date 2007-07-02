#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :

#
# Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
# Copyright (c) 2007 Richard Brown <rbrown@gentoo.org>
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
###            pde = PackageDatabaseEntry.new("x/x", VersionSpec.new("1.0"), "testrepo")

###            assert env.query_use("enabled", pde)
###            assert ! env.query_use("not_enabled", pde)
###            assert ! env.query_use("sometimes_enabled", pde)

            pid = env.package_database.query(Query::Matches.new(PackageDepSpec.new('=foo/bar-1.0::testrepo', PackageDepSpecParseMode::Permissive)), QueryOrder::RequireExactlyOne).first

            assert env.query_use("enabled", pid)
            assert ! env.query_use("not_enabled", pid)
            assert env.query_use("sometimes_enabled", pid)
        end

        def test_query_use_bad
            assert_raise ArgumentError do
                env.query_use(1, 2, 3)
            end
            assert_raise ArgumentError do
                env.query_use(123)
            end
        end
    end

    class TestCase_EnvironmentAcceptLicense < Test::Unit::TestCase
        def env
            @env or @env = EnvironmentMaker.instance.make_from_spec("")
        end

        def pid
            env.package_database.query(Query::Matches.new(PackageDepSpec.new('=foo/bar-1.0::testrepo', PackageDepSpecParseMode::Permissive)), QueryOrder::RequireExactlyOne).first
        end

        def test_accept_license
            assert env.accept_license('GPL-2', pid)
            assert !env.accept_license('Failure', pid)

            pid2 = env.package_database.query(Query::Matches.new(PackageDepSpec.new('=foo/baz-1.0::testrepo', PackageDepSpecParseMode::Permissive)), QueryOrder::RequireExactlyOne).first
            assert env.accept_license('GPL-2', pid2)
            assert env.accept_license('Failure', pid2)
        end

        def test_accept_license_bad
            assert_raise TypeError do
                env.accept_keywords('license','a string')
            end
        end
    end

    class TestCase_EnvironmentAcceptKeywords < Test::Unit::TestCase
        def env
            @env or @env = EnvironmentMaker.instance.make_from_spec("")
        end

        def pid
            env.package_database.query(Query::Matches.new(PackageDepSpec.new('=foo/bar-1.0::testrepo', PackageDepSpecParseMode::Permissive)), QueryOrder::RequireExactlyOne).first
        end

        def test_accept_keywords
            assert env.accept_keywords(['test'], pid)
            assert !env.accept_keywords(['test2'], pid)
            assert env.accept_keywords(['test','testtest'], pid)
            assert env.accept_keywords(['test2','testtest'], pid)
            assert !env.accept_keywords(['test2','test3'], pid)
        end

        def test_accept_keywords_bad
            assert_raise TypeError do
                env.accept_keywords('test',pid)
            end

            assert_raise TypeError do
                env.accept_keywords([],'a string')
            end
        end
    end

    class TestCase_NoConfigEnvironmentUse < Test::Unit::TestCase
        def env
            NoConfigEnvironment.new(Dir.getwd().to_s + "/environment_TEST_dir/testrepo")
        end

        def test_query_use
            pid = env.package_database.query(Query::Matches.new(PackageDepSpec.new('=foo/bar-1.0::testrepo', PackageDepSpecParseMode::Permissive)), QueryOrder::RequireExactlyOne).first
            assert ! env.query_use("foo", pid)
        end

        def test_query_use_bad
            assert_raise ArgumentError do
                env.query_use(1, 2, 3)
            end
            assert_raise ArgumentError do
                env.query_use(123)
            end
        end
    end

    class TestCase_AdaptedEnvironment < Test::Unit::TestCase
        def env
            @env or @env = AdaptedEnvironment.new(EnvironmentMaker.instance.make_from_spec(""))
        end

        def test_create
            assert_nothing_raised do
                env
            end
        end

        def test_create_bad
            assert_raise ArgumentError do
                AdaptedEnvironment.new
            end
        end
    end

    class TestCase_AdaptedEnvironmentAdaptUse < Test::Unit::TestCase
        def env
            @env or @env = AdaptedEnvironment.new(EnvironmentMaker.instance.make_from_spec(""))
        end

        def test_adapt_use
            pid = env.package_database.query(Query::Matches.new(PackageDepSpec.new('=foo/bar-1.0::testrepo', PackageDepSpecParseMode::Permissive)), QueryOrder::RequireExactlyOne).first

            assert env.query_use("enabled", pid)
            assert ! env.query_use("not_enabled", pid)
            assert env.query_use("sometimes_enabled", pid)

            env.adapt_use(PackageDepSpec.new('foo/bar', PackageDepSpecParseMode::Permissive), 'enabled', false);
            assert ! env.query_use('enabled', pid);
            env.adapt_use(PackageDepSpec.new('foo/bar', PackageDepSpecParseMode::Permissive), 'not_enabled', true);
            assert env.query_use("not_enabled", pid)
            env.adapt_use(PackageDepSpec.new('foo/bar', PackageDepSpecParseMode::Permissive), 'sometimes_enabled', false);
            assert ! env.query_use("sometimes_enabled", pid)
        end

        def test_adapt_use_bad
            assert_raise TypeError do
                env.adapt_use(PackageDepSpec.new('foo/bar', PackageDepSpecParseMode::Permissive), 'not_enabled', 'lemon');
            end
            assert_raise TypeError do
                env.adapt_use(PackageDepSpec.new(123, PackageDepSpecParseMode::Permissive), 'not_enabled', false);
            end
            assert_raise TypeError do
                env.adapt_use(PackageDepSpec.new('foo/bar', PackageDepSpecParseMode::Permissive), 7, false);
            end
        end
    end

    class TestCase_AdaptedEnvironmentClearAdaptions < Test::Unit::TestCase
        def env
            @env or @env = AdaptedEnvironment.new(EnvironmentMaker.instance.make_from_spec(""))
        end

        def test_clear_adaptions
            pid = env.package_database.query(Query::Matches.new(PackageDepSpec.new('=foo/bar-1.0::testrepo', PackageDepSpecParseMode::Permissive)), QueryOrder::RequireExactlyOne).first

            assert env.query_use("enabled", pid)

            env.adapt_use(PackageDepSpec.new('foo/bar', PackageDepSpecParseMode::Permissive), 'enabled', false);
            assert ! env.query_use('enabled', pid);

            env.clear_adaptions;
            assert env.query_use("enabled", pid)
        end
    end

    class TestCase_NoConfigEnvironmentMaskReasons < Test::Unit::TestCase
        def env
            NoConfigEnvironment.new(Dir.getwd().to_s + "/environment_TEST_dir/testrepo")
        end

        def test_mask_reasons
            pid = env.package_database.query(Query::Matches.new(PackageDepSpec.new('=foo/bar-1.0::testrepo', PackageDepSpecParseMode::Permissive)), QueryOrder::RequireExactlyOne).first
            m = env.mask_reasons(pid)
            assert m.empty?
        end

        def test_mask_reasons_not_empty
            pid = env.package_database.query(Query::Matches.new(PackageDepSpec.new('=foo/bar-2.0::testrepo', PackageDepSpecParseMode::Permissive)), QueryOrder::RequireExactlyOne).first

            m = env.mask_reasons(pid)
            assert ! m.empty?
            assert m.include?(MaskReason::Keyword)
            assert_equal([MaskReason::Keyword], m.to_a)
        end

        def test_mask_reasons_options
            pid = env.package_database.query(Query::Matches.new(PackageDepSpec.new('=foo/bar-2.0::testrepo', PackageDepSpecParseMode::Permissive)), QueryOrder::RequireExactlyOne).first
            mro = MaskReasonsOptions.new
            mro.add MaskReasonsOption::OverrideTildeKeywords
            m = env.mask_reasons(pid, mro)
            assert m.empty?
        end

        def test_mask_reasons_bad
            assert_raise ArgumentError do
                env.mask_reasons(1, 2, 3)
            end

            assert_raise TypeError do
                env.mask_reasons(123)
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

###    class TestCase_EnvironmentPackageSet < Test::Unit::TestCase
###        def env
###            @env or @env = EnvironmentMaker.instance.make_from_spec("")
###        end
###
###        def test_package_set
###            assert_kind_of DepSpec, env.set('everything')
###        end
###
###        def test_package_set_error
###            assert_raise SetNameError do
###                env.set('broken*')
###            end
###        end
###    end

###    class TestCase_NoConfigEnvironmentPackageSet < Test::Unit::TestCase
###        def env
###            NoConfigEnvironment.new(Dir.getwd().to_s + "/environment_TEST_dir/testrepo")
###        end
###
###        def test_package_set
###            assert_kind_of DepSpec, env.set('everything')
###        end
###
###        def test_package_set_error
###            assert_raise SetNameError do
###                env.set('broken*')
###            end
###        end
###    end

    class TestCase_NoConfigEnvirontmentPortageRepository < Test::Unit::TestCase
        def env
            NoConfigEnvironment.new(Dir.getwd().to_s + "/environment_TEST_dir/testrepo")
        end

        def test_portage_repository
            assert_kind_of Repository, env.main_repository
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
            assert_kind_of Repository, env_master.master_repository
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
	    assert_nothing_raised do
	    	env.accept_unstable=true
	    	env.accept_unstable=false
	    end
        end
    end
end

