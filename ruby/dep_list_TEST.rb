#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :
#
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
                DepListOverrideMasksFunctions.new,
                false,
                []
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
                :override_masks => DepListOverrideMasksFunctions.new,
                :dependency_tags => false,
                :match_package_options => []
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
                :override_masks => DepListOverrideMasksFunctions.new,
                :dependency_tags => true,
                :match_package_options => []
            }
        end

        def dlo_default
            DepListOptions.new()
        end

        def env
            @env or @env = EnvironmentFactory.instance.create("")
        end

        def dl
            DepList.new(env,dlo)
        end

        def pda
            Paludis::parse_user_package_dep_spec('foo/bar', env, [])
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
                if :override_masks == method
                    assert_nil options.send(method)
                else
                    assert_equal value, options.send(method)
                end
                #check setters work
                assert_nothing_raised do
                    options.send("#{method}=", options_hash[method])
                    if :override_masks == method
                        assert_equal options_hash[method].class, options.send(method).class
                    else
                        assert_equal options_hash[method], options.send(method)
                    end
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

###        def test_respond
###            dep_list = dl
###            [:add, :clear, :already_installed?, :each, :options].each {|sym| assert_respond_to dep_list, sym}
###        end

        def test_add
            assert_nothing_raised do
                dl.add(pda, dd)
            end
        end

        def test_add_set
            assert_nothing_raised do
                dl.add(env.set("world"), dd)
            end
        end

        def test_add_bad_tree
            assert_raise TypeError do
                dl.add(env[Selection::BestVersionOnly.new(Generator::Matches.new(
                    pda, nil, []))].last.build_dependencies_key.value, dd)
            end
        end

        def test_clear
            assert_nothing_raised do
                dl.clear
            end
        end

###        def test_already_installed?
###            dep_spec = pda
###            dep_list = dl
###            assert !dep_list.already_installed?(dep_spec, dd)
###            dep_list.add(dep_spec, dd)
###            assert dep_list.already_installed?(dep_spec, dd)
###        end

###        def test_each
###            assert_kind_of DepListEntry, dl.add(pda, dd).entries.first
###        end

###        def test_errors
###            assert_raise AllMaskedError do
###                dl.add(PackageDepSpec.new('foo/ba', PackageDepSpecParseMode::Permissive), dd)
###            end
###
###            begin
###                dl.add(PackageDepSpec.new('foo/ba', PackageDepSpecParseMode::Permissive), dd)
###            rescue AllMaskedError => error
###                assert_equal 'foo/ba', error.query
###            end
###
###        end

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
                :package_id => PackageID, :state=> Integer, :tags => [String]
            }.each_pair do |method, returns|
                assert_respond_to dep_list_entry, method
                if returns.kind_of? Array
                    assert_kind_of Array, dep_list_entry.send(method)
                    dep_list_entry.send(method).each {|x| assert_kind_of returns.first, x}
                else
                    assert_kind_of returns, dep_list_entry.send(method)
                end
            end
        end
    end

    class TestCase_DepListOverrideMasksFunctions < Test::Unit::TestCase
        include Shared

        def test_create
            assert_nothing_raised do
                DepListOverrideMasksFunctions.new
            end
        end

        def test_bind
            dlo = DepListOverrideMasksFunctions.new
            assert_equal dlo, dlo.bind(:tilde_keywords, env)
            assert_equal dlo, dlo.bind(:unkeyworded, env)
            assert_equal dlo, dlo.bind(:repository_masks)
            assert_equal dlo, dlo.bind(:license)
        end
    end
end

