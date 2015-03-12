#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :
#
# Copyright (c) 2007, 2008, 2009, 2010 Ciaran McCreesh
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

ENV["PALUDIS_HOME"] = Dir.getwd().to_s + "/action_TEST_dir/home";

require 'test/unit'
require 'Paludis'

module Paludis
    module InstallActionModule
        def env
            @env or @env = EnvironmentFactory.instance.create("")
        end

        def hash_args
            InstallActionOptions.new(
                {:destination => destination}
            )
        end

        def long_args
            InstallActionOptions.new(destination)
        end

        def destination
            @destination or @destination = FakeRepository.new(env, 'fake');
        end
    end

    class TestCase_SupportsFetchActionTest < Test::Unit::TestCase
        def test_create
            assert_kind_of SupportsActionTest, SupportsActionTest.new(FetchAction)
        end
    end

    class TestCase_SupportsInfoActionTest < Test::Unit::TestCase
        def test_create
            assert_kind_of SupportsActionTest, SupportsActionTest.new(InfoAction)
        end
    end

    class TestCase_SupportsConfigActionTest < Test::Unit::TestCase
        def test_create
            assert_kind_of SupportsActionTest, SupportsActionTest.new(ConfigAction)
        end
    end

    class TestCase_SupportsInstallActionTest < Test::Unit::TestCase
        def test_create
            assert_kind_of SupportsActionTest, SupportsActionTest.new(InstallAction)
        end
    end

    class TestCase_SupportsUninstallActionTest < Test::Unit::TestCase
        def test_create
            assert_kind_of SupportsActionTest, SupportsActionTest.new(UninstallAction)
        end
    end

    class TestCase_FetchActionOptions < Test::Unit::TestCase
        def test_create
            assert_kind_of FetchActionOptions, FetchActionOptions.new(false, false, false)
            assert_kind_of FetchActionOptions, FetchActionOptions.new(
                {:safe_resume => false, :fetch_unneeded => false, :exclude_unmirrorable => false})
        end
    end

    class TestCase_InstallActionOptions < Test::Unit::TestCase
        include InstallActionModule

        def test_create
            assert_kind_of InstallActionOptions, long_args
            assert_kind_of InstallActionOptions, hash_args
        end

        def test_methods_hash_args
            opts = hash_args
            assert_equal destination.name, opts.destination.name
        end

        def test_methods_long_args
            opts = long_args
            assert_equal destination.name, opts.destination.name
        end
    end

    class TestCase_FetchActionFailure < Test::Unit::TestCase
        def test_create
            assert_kind_of FetchActionFailure, FetchActionFailure.new('target_file', false, false, 'fic')
            assert_kind_of FetchActionFailure, FetchActionFailure.new(
                {
                :requires_manual_fetching => false, :failed_automatic_fetching => false,
                :target_file => 'target_file', :failed_integrity_checks => 'fic'
                }
            )
        end

        def test_methods_hash_args
             failure =  FetchActionFailure.new(
                {
                :requires_manual_fetching => false, :failed_automatic_fetching => false,
                :target_file => 'target_file', :failed_integrity_checks => 'fic'
                }
             )
             assert_equal 'target_file', failure.target_file;
             assert ! failure.requires_manual_fetching?
             assert ! failure.failed_automatic_fetching?
             assert_equal 'fic', failure.failed_integrity_checks;
        end

        def test_methods_4_args
             failure = FetchActionFailure.new('target_file', false, false, 'fic')
             assert_equal 'target_file', failure.target_file;
             assert ! failure.requires_manual_fetching?
             assert ! failure.failed_automatic_fetching?
             assert_equal 'fic', failure.failed_integrity_checks;
        end
    end

    class TestCase_FetchAction < Test::Unit::TestCase
        def test_create
            assert_kind_of FetchAction, FetchAction.new(FetchActionOptions.new(false, false, false))
            assert_kind_of Action, FetchAction.new(FetchActionOptions.new(false, false, false))

            assert_kind_of FetchAction, FetchAction.new(FetchActionOptions.new(
                {:safe_resume => false, :fetch_unneeded => false, :exclude_unmirrorable => false}))
        end

        def test_bad_create
            assert_raise TypeError do
                FetchAction.new("foo")
            end

            assert_raise ArgumentError do
                FetchAction.new(FetchActionOptions.new({:monkey => false}))
            end
        end

        def test_options
            a = FetchAction.new(FetchActionOptions.new(false, true, false))
            assert_kind_of FetchActionOptions, a.options
            assert a.options.safe_resume?

            a = FetchAction.new(FetchActionOptions.new({:safe_resume => false, :fetch_unneeded => true, :exclude_unmirrorable => false}))
            assert_kind_of FetchActionOptions, a.options
            assert !a.options.safe_resume?
            assert !a.options.exclude_unmirrorable?
        end
    end

    class TestCase_InfoAction < Test::Unit::TestCase
        def test_create
            assert_kind_of InfoAction, InfoAction.new
            assert_kind_of Action, InfoAction.new
        end

        def test_bad_create
            assert_raise ArgumentError do
                InfoAction.new('')
            end
        end
    end

    class TestCase_ConfigAction < Test::Unit::TestCase
        def test_create
            assert_kind_of ConfigAction, ConfigAction.new
            assert_kind_of Action, ConfigAction.new
        end

        def test_bad_create
            assert_raise ArgumentError do
                ConfigAction.new('')
            end
        end
    end

    class TestCase_InstallAction < Test::Unit::TestCase
        include InstallActionModule

        def test_create
            assert_kind_of InstallAction, InstallAction.new(hash_args)
            assert_kind_of Action, InstallAction.new(hash_args)

            assert_kind_of InstallAction, InstallAction.new(long_args)
        end

        def test_bad_create
            assert_raise TypeError do
                InstallAction.new("foo")
            end

            assert_raise ArgumentError do
                InstallAction.new(InstallActionOptions.new({:monkey => false}))
            end
        end

        def test_options_hash_args
            action = InstallAction.new(hash_args)
            assert_kind_of InstallActionOptions, action.options
            assert_equal destination.name, action.options.destination.name
        end
    end

    class TestCase_UninstallAction < Test::Unit::TestCase
        def test_create
            assert_kind_of UninstallAction, UninstallAction.new(UninstallActionOptions.new("monkey"))
            assert_kind_of Action, UninstallAction.new(UninstallActionOptions.new("monkey"))
        end

        def test_bad_create
            assert_raise TypeError do
                UninstallAction.new("foo")
            end

            assert_raise TypeError do
                UninstallAction.new(UninstallActionOptions.new({:monkey => false}))
            end
        end
    end

    class TestCase_PretendAction < Test::Unit::TestCase
        def env
            @env or @env = EnvironmentFactory.instance.create("")
        end

        def test_create
            assert_kind_of PretendAction, PretendAction.new(PretendActionOptions.new(destination))
            assert_kind_of Action, PretendAction.new(PretendActionOptions.new(destination))
        end

        def test_bad_create
            assert_raise ArgumentError do
                PretendAction.new()
            end
        end

        def test_methods
            action = PretendAction.new(PretendActionOptions.new(destination))
            assert !action.failed?

            assert_nothing_raised do
                action.set_failed
            end

            assert action.failed?
        end

        def destination
            @destination or @destination = FakeRepository.new(env, 'fake');
        end
    end
end


