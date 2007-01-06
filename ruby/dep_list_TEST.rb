#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :

#
# Copyright (c) 2007 Richard Brown <mynamewasgone@gmail.com>
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

ENV['PALUDIS_HOME'] = Dir.getwd() + '/dep_list_TEST_dir/home'

require 'test/unit'
require 'Paludis'

Paludis::Log.instance.log_level = Paludis::LogLevel::Warning

module Paludis
    module Shared
        def dlo
            DepListOptions.new(
                DepListReinstallOption::ReinstallAlways,
                DepListReinstallScmOption::ReinstallScmAlways,
                DepListTargetType::TargetPackage,
                DepListUpgradeOption::UpgradeAlways,
                DepListNewSlotsOption::NewSlotsAlways,
                DepListFallBackOption::FallBackAsNeededExceptTargets,
                DepListDepsOption::DepsDiscard,
                DepListDepsOption::DepsDiscard,
                DepListDepsOption::DepsDiscard,
                DepListDepsOption::DepsDiscard,
                DepListDepsOption::DepsDiscard,
                DepListDepsOption::DepsDiscard,
                DepListCircularOption::CircularDiscard,
                false
            )
        end

        def env
            DefaultEnvironment.instance
        end

        def dl
            DepList.new(env,dlo)
        end

        def pda
            PackageDepAtom.new('foo/bar')
        end
    end

    class TestCase_DepListOptions < Test::Unit::TestCase
        include Shared

        def options_hash
            {
                :reinstall => DepListReinstallOption::ReinstallAlways,
                :reinstall_scm => DepListReinstallScmOption::ReinstallScmAlways,
                :target_type => DepListTargetType::TargetPackage,
                :upgrade => DepListUpgradeOption::UpgradeAlways,
                :new_slots => DepListNewSlotsOption::NewSlotsAlways,
                :fall_back => DepListFallBackOption::FallBackAsNeededExceptTargets,
                :installed_deps_pre => DepListDepsOption::DepsDiscard,
                :installed_deps_runtime => DepListDepsOption::DepsDiscard,
                :installed_deps_post => DepListDepsOption::DepsDiscard,
                :uninstalled_deps_pre => DepListDepsOption::DepsDiscard,
                :uninstalled_deps_runtime => DepListDepsOption::DepsDiscard,
                :uninstalled_deps_post => DepListDepsOption::DepsDiscard,
                :circular => DepListCircularOption::CircularDiscard,
                :dependency_tags => false
            }
        end

        def test_create
            assert_nothing_raised do
                dlo
            end
        end

        def test_respond_to
            options = dlo
            options_hash.each_pair do |method, value|
                assert_respond_to options, method
                assert_equal value, options.send(method)
            end
        end
    end

    class TestCase_DepList < Test::Unit::TestCase
        include Shared

        def test_create
            assert_nothing_raised do
                dl
            end
        end

        def test_respond
            dep_list = dl
            [:add, :clear, :already_installed?, :each].each {|sym| assert_respond_to dep_list, sym}
        end

        def test_add
            assert_nothing_raised do
                dl.add(pda)
            end
        end

        def test_clear
            assert_nothing_raised do
                dl.clear
            end
        end

        def test_already_installed?
            dep_atom = pda
            dep_list = dl
            assert !dep_list.already_installed?(dep_atom)
            dep_list.add(dep_atom)
            assert dep_list.already_installed?(dep_atom)
        end

        def test_each
            assert_kind_of DepListEntry, dl.add(pda).entries.first
        end

        def test_errors
            assert_raise AllMaskedError do
                dl.add(PackageDepAtom.new('foo/ba'))
            end
        end
    end

    class TestCase_DepListEntry < Test::Unit::TestCase
        include Shared

        def dle
            dl.add(pda).entries.first
        end

        def test_create
            assert_raise NoMethodError do
                DepListEntry.new
            end
        end

        def test_methods
            dep_list_entry = dle
            {:package => PackageDatabaseEntry, :metadata => VersionMetadata,
                :destinations => Array, :state=> Integer}.each_pair do |method, returns|

                assert_respond_to dep_list_entry, method
                assert_kind_of returns, dep_list_entry.send(method)
            end
            assert_respond_to dep_list_entry, :skip_install
        end
    end
end
