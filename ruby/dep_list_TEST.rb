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
                DepListReinstallOption::Never,
                DepListReinstallScmOption::Never,
                DepListTargetType::Package,
                DepListUpgradeOption::Always,
                DepListDowngradeOption::AsNeeded,
                DepListNewSlotsOption::Always,
                DepListFallBackOption::AsNeededExceptTargets,
                DepListDepsOption::Discard,
                DepListDepsOption::TryPost,
                DepListDepsOption::TryPost,
                DepListDepsOption::Pre,
                DepListDepsOption::PreOrPost,
                DepListDepsOption::Post,
                DepListDepsOption::TryPost,
                DepListSuggestedOption::Show,
                DepListCircularOption::Error,
                DepListUseOption::Standard,
                DepListBlocksOption::Accumulate,
                DepListOverrideMasks.new,
                false
            )
        end

        def default_options
            {
                :reinstall => DepListReinstallOption::Never,
                :reinstall_scm => DepListReinstallScmOption::Never,
                :target_type => DepListTargetType::Package,
                :upgrade => DepListUpgradeOption::Always,
                :downgrade => DepListDowngradeOption::AsNeeded,
                :new_slots => DepListNewSlotsOption::Always,
                :fall_back => DepListFallBackOption::AsNeededExceptTargets,
                :installed_deps_pre => DepListDepsOption::Discard,
                :installed_deps_runtime => DepListDepsOption::TryPost,
                :installed_deps_post => DepListDepsOption::TryPost,
                :uninstalled_deps_pre => DepListDepsOption::Pre,
                :uninstalled_deps_runtime => DepListDepsOption::PreOrPost,
                :uninstalled_deps_post => DepListDepsOption::Post,
                :uninstalled_deps_suggested => DepListDepsOption::TryPost,
                :suggested => DepListSuggestedOption::Show,
                :circular => DepListCircularOption::Error,
                :use => DepListUseOption::Standard,
                :blocks => DepListBlocksOption::Accumulate,
                :override_masks => DepListOverrideMasks.new,
                :dependency_tags => false
            }
        end

        def dlo_hash
            DepListOptions.new(options_hash)
        end

        def options_hash
            {
                :reinstall => DepListReinstallOption::Always,
                :reinstall_scm => DepListReinstallScmOption::Always,
                :target_type => DepListTargetType::Set,
                :upgrade => DepListUpgradeOption::AsNeeded,
                :downgrade => DepListDowngradeOption::Warning,
                :new_slots => DepListNewSlotsOption::AsNeeded,
                :fall_back => DepListFallBackOption::AsNeeded,
                :installed_deps_pre => DepListDepsOption::Pre,
                :installed_deps_runtime => DepListDepsOption::Discard,
                :installed_deps_post => DepListDepsOption::Discard,
                :uninstalled_deps_pre => DepListDepsOption::Discard,
                :uninstalled_deps_runtime => DepListDepsOption::Discard,
                :uninstalled_deps_post => DepListDepsOption::Discard,
                :uninstalled_deps_suggested => DepListDepsOption::Discard,
                :suggested => DepListSuggestedOption::Discard,
                :circular => DepListCircularOption::Discard,
                :use => DepListUseOption::TakeAll,
                :blocks => DepListBlocksOption::Error,
                :override_masks => DepListOverrideMasks.new,
                :dependency_tags => true
            }
        end

        def dlo_default
            DepListOptions.new()
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

        def dd
            env.default_destinations
        end
    end

    class TestCase_DepListOptions < Test::Unit::TestCase
        include Shared

        def test_create
            assert_nothing_raised do
                dlo
            end

            assert_nothing_raised do
                dlo_hash
            end

            assert_nothing_raised do
                dlo_default
            end
        end

        def test_members
            options = dlo_default
            #This will fail if the defaults change, please also update the rdoc.
            default_options.each_pair do |method, value|
                assert_respond_to options, method
                assert_equal value, options.send(method)
                #check setters work
                assert_nothing_raised do
                    options.send("#{method}=", options_hash[method])
                    assert_equal options_hash[method], options.send(method)
                end
            end
        end

        def test_bad_create
            options_hash.each_key do |key|
                assert_raises ArgumentError do
                    DepListOptions.new(options_hash.delete(key))
                end
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
            [:add, :clear, :already_installed?, :each, :options].each {|sym| assert_respond_to dep_list, sym}
        end

        def test_add
            assert_nothing_raised do
                dl.add(pda, dd)
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
            assert !dep_list.already_installed?(dep_atom, dd)
            dep_list.add(dep_atom, dd)
            assert dep_list.already_installed?(dep_atom, dd)
        end

        def test_each
            assert_kind_of DepListEntry, dl.add(pda, dd).entries.first
        end

        def test_errors
            assert_raise AllMaskedError do
                dl.add(PackageDepAtom.new('foo/ba'), dd)
            end

            begin
                dl.add(PackageDepAtom.new('foo/ba'), dd)
            rescue AllMaskedError => error
                assert_equal 'foo/ba', error.query
            end

        end

        def test_options
            dep_list = dl
            assert_kind_of DepListOptions, dep_list.options
            assert_equal DepListReinstallOption::Never, dep_list.options.reinstall

            assert_nothing_raised do
                dep_list.options.reinstall = DepListReinstallOption::Always
            end

            assert_equal DepListReinstallOption::Always, dep_list.options.reinstall

        end
    end

    class TestCase_DepListEntry < Test::Unit::TestCase
        include Shared

        def dle
            dl.add(pda, dd).entries.first
        end

        def test_create
            assert_raise NoMethodError do
                DepListEntry.new
            end
        end

        def test_methods
            dep_list_entry = dle
            {
                :package => PackageDatabaseEntry, :metadata => VersionMetadata,
                :state=> Integer, :tags => Array, :destination => Repository
            }.each_pair do |method, returns|
                assert_respond_to dep_list_entry, method
                assert_kind_of returns, dep_list_entry.send(method)
            end
        end
    end


    class TestCase_DepListOverrideMasks < Test::Unit::TestCase
        def test_create
            m = DepListOverrideMasks.new
        end

        def test_each
            m = DepListOverrideMasks.new
            assert_equal [], m.to_a
        end

        def test_empty
            m = DepListOverrideMasks.new
            assert m.empty?
            m.set DepListOverrideMask::Licenses
            assert !m.empty?
        end

        def test_set
            m = DepListOverrideMasks.new
            m.set DepListOverrideMask::Licenses
            m.set DepListOverrideMask::ProfileMasks

            assert ! m.empty?
            assert_equal 2, m.entries.length

            assert m.include?(DepListOverrideMask::Licenses)
            assert m.include?(DepListOverrideMask::ProfileMasks)
        end

        def test_clear
            m = DepListOverrideMasks.new
            m.set DepListOverrideMask::Licenses
            m.set DepListOverrideMask::ProfileMasks
            m.set DepListOverrideMask::TildeKeywords

            assert_equal 3, m.entries.length
            m.reset DepListOverrideMask::TildeKeywords
            assert_equal 2, m.entries.length
            m.reset
            assert m.empty?
        end
    end
end
