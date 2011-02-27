#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :

#
# Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
# Copyright (c) 2007, 2008 Richard Brown
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
                e = NoConfigEnvironment.new(1,2,3,4,5)
            end
        end
    end

    class TestCase_EnvironmentAcceptLicense < Test::Unit::TestCase
        def env
            @env or @env = EnvironmentFactory.instance.create("")
        end

        def pid
            env[Selection::RequireExactlyOne.new(Generator::Matches.new(
                Paludis::parse_user_package_dep_spec('=foo/bar-1.0::testrepo', env, []), nil, []))].first
        end

        def test_accept_license
            assert env.accept_license('GPL-2', pid)
            assert !env.accept_license('Failure', pid)

            pid2 = env[Selection::RequireExactlyOne.new(Generator::Matches.new(
                Paludis::parse_user_package_dep_spec('=foo/baz-1.0::testrepo', env, []), nil, []))].first
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
            @env or @env = EnvironmentFactory.instance.create("")
        end

        def pid
            env[Selection::RequireExactlyOne.new(Generator::Matches.new(
                Paludis::parse_user_package_dep_spec('=foo/bar-1.0::testrepo', env, []), nil, []))].first
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

    class TestCase_EnvironmentPackageDatabase < Test::Unit::TestCase
        def env
            @env or @env = EnvironmentFactory.instance.create("")
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
            @env or @env = EnvironmentFactory.instance.create("")
        end

        def test_package_set
            assert_kind_of DepSpec, env.set('everything')
        end

        def test_package_set_error
            assert_raise SetNameError do
                env.set('broken#')
            end
        end
    end

    class TestCase_NoConfigEnvironmentPackageSet < Test::Unit::TestCase
        def env
            NoConfigEnvironment.new(Dir.getwd().to_s + "/environment_TEST_dir/testrepo")
        end

        def test_package_set
            assert_kind_of DepSpec, env.set('everything')
        end

        def test_package_set_error
            assert_raise SetNameError do
                env.set('broken#')
            end
        end
    end

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
                                   "slaverepo",
                                   [Dir.getwd().to_s + "/environment_TEST_dir/slaverepo"])
        end

        def test_master_repository
            assert_nil env.master_repository
            assert_kind_of Repository, env_master.master_repository
        end
    end

    class TestCase_EnvironmentDistribution < Test::Unit::TestCase
        def env
            NoConfigEnvironment.new(Dir.getwd().to_s + "/environment_TEST_dir/testrepo")
        end

        def test_distribution
            assert_kind_of String, env.distribution
            assert_equal "gentoo", env.distribution
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

    class TestCase_EnvironmentMirrors < Test::Unit::TestCase
        def env
            @env or @env = EnvironmentFactory.instance.create("")
        end

        def test_respond_and_return
            assert_respond_to env, :mirrors
            assert_kind_of Array, env.mirrors('')
        end


        def test_mirrors_star
            star_mirrors = env.mirrors('*')
            assert_equal 2, star_mirrors.length
            assert star_mirrors.include?('http://a')
            assert star_mirrors.include?('http://b')
        end

        def test_named_mirror
            assert_equal ['http://c'], env.mirrors('testmirror')
        end

        def test_empty_mirror
            assert env.mirrors('missingmirror').empty?
        end
    end

    class TestCase_EnvironmentQuery < Test::Unit::TestCase
        def env
            @env or @env = EnvironmentFactory.instance.create("")
        end

        def db
            return env.package_database
        end

        def pda
            Paludis::parse_user_package_dep_spec('=foo/bar-1.0', env, [])
        end

        def pda2
            Paludis::parse_user_package_dep_spec('foo/bar', env, [])
        end

        def test_arg_count
            assert_raise ArgumentError do
                env[1, 2];
            end
        end

        def test_package_database_query
            a = env[Selection::AllVersionsSorted.new(Generator::Matches.new(pda, nil, []))]
            assert_kind_of Array, a
            assert_equal 1, a.length
            pid = a.first
            assert_kind_of PackageID, pid
            assert_equal 'foo/bar', pid.name
            assert_equal '1.0', pid.version.to_s
            assert_equal 'testrepo', pid.repository_name

            a = env[Selection::AllVersionsSorted.new(Generator::Matches.new(pda, nil, []) | Filter::SupportsAction.new(InstallAction))]
            assert_kind_of Array, a
            assert_equal 1, a.length
            pid = a.first
            assert_kind_of PackageID, pid
            assert_equal 'foo/bar', pid.name
            assert_equal '1.0', pid.version.to_s
            assert_equal 'testrepo', pid.repository_name

            a = env[Selection::AllVersionsSorted.new(Generator::Matches.new(pda, nil, []))]
            assert_kind_of Array, a
            assert_equal 1, a.length
            pid = a.first
            assert_kind_of PackageID, pid
            assert_equal 'foo/bar', pid.name
            assert_equal '1.0', pid.version.to_s
            assert_equal 'testrepo', pid.repository_name

            a = env[Selection::AllVersionsSorted.new(Generator::Matches.new(pda2, nil, []) | Filter::SupportsAction.new(InstallAction))]
            assert_kind_of Array, a
            assert_equal 2, a.length
            pid = a.shift
            assert_kind_of PackageID, pid
            assert_equal 'foo/bar', pid.name
            assert_equal '1.0', pid.version.to_s
            assert_equal 'testrepo', pid.repository_name
            pid2 = a.shift
            assert_kind_of PackageID, pid2
            assert_equal pid.name, pid2.name
            assert_equal '2.0', pid2.version.to_s
            assert_equal pid.repository_name, pid2.repository_name

            a = env[Selection::AllVersionsSorted.new(Generator::Package.new('foo/bar'))]
            assert_kind_of Array, a
            assert_equal 2, a.length
            pid = a.shift
            assert_kind_of PackageID, pid
            assert_equal 'foo/bar', pid.name
            assert_equal '1.0', pid.version.to_s
            assert_equal 'testrepo', pid.repository_name
            pid2 = a.shift
            assert_kind_of PackageID, pid2
            assert_equal pid.name, pid2.name
            assert_equal '2.0', pid2.version.to_s
            assert_equal pid.repository_name, pid2.repository_name


            a = env[Selection::AllVersionsUnsorted.new(Generator::Matches.new(Paludis::parse_user_package_dep_spec(
                '>=foo/bar-27', env, []), nil, []))]
            assert a.empty?

            a = env[Selection::AllVersionsUnsorted.new(Generator::Matches.new(pda2, nil, []) | Filter::SupportsAction.new(ConfigAction))]
            assert a.empty?
        end
    end

    class TestCase_EnvironmentMetadataKeys < Test::Unit::TestCase
        def env
            @env or @env = EnvironmentFactory.instance.create("")
        end

        def ncenv
            @ncenv or @ncenv = NoConfigEnvironment.new(Dir.getwd().to_s + "/environment_TEST_dir/testrepo")
        end

        def test_format_key
            assert_respond_to env, :format_key
            assert_not_nil env.format_key
            assert_kind_of MetadataStringKey, env.format_key
            assert_equal 'paludis', env.format_key.value

            assert_respond_to ncenv, :format_key
            assert_not_nil ncenv.format_key
            assert_kind_of MetadataStringKey, ncenv.format_key
            assert_equal 'no_config', ncenv.format_key.value
        end

        def test_config_location_key
            assert_respond_to env, :config_location_key
            assert_not_nil env.config_location_key
            assert_kind_of MetadataFSPathKey, env.config_location_key
            assert_equal Dir.getwd().to_s + "/environment_TEST_dir/home/.paludis", env.config_location_key.value

            assert_respond_to ncenv, :config_location_key
            assert_nil ncenv.config_location_key
        end
    end

    class TestCase_TestEnvironment < Test::Unit::TestCase
        def test_create
            x = TestEnvironment.new()
        end
    end
end

