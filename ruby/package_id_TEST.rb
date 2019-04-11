#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :

#
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

ENV["PALUDIS_HOME"] = Dir.getwd().to_s + "/package_id_TEST_dir/home";

require 'test/unit'
require 'Paludis'

Paludis::Log.instance.log_level = Paludis::LogLevel::Warning

module Paludis
    module TestStuff
        def env
            unless @env
                @env = EnvironmentFactory.instance.create("")
            end
            @env
        end

        def pid_testrepo
            env.fetch_repository("testrepo").package_ids("foo/bar").first
        end

        def pid_bad
            env.fetch_repository("testrepo").package_ids("bad/pkg").first
        end

        def pid_scm
            env.fetch_repository("exheresrepo").package_ids("scm/scm").first
        end

        def pid_installed
            env.fetch_repository("installed").package_ids("cat-one/pkg-one").first
        end
    end

    class TestCase_MetadataKey < Test::Unit::TestCase
        include TestStuff

        def classes
            [MetadataStringKey, MetadataTimeKey, MetadataKeywordNameSetKey, MetadataStringSetKey]
        end

        def all_classes
            classes.unshift MetadataKey
        end

        def mk
            pid_testrepo.short_description_key
        end

        def test_no_create
            all_classes.each do |x|
                assert_raise NoMethodError do
                    x.new
                end
            end
        end

        def test_raw_name
            assert_respond_to mk, :raw_name
            assert_kind_of String, mk.raw_name
            assert_equal 'DESCRIPTION', mk.raw_name
        end

        def test_human_name
            assert_respond_to mk, :human_name
            assert_kind_of String, mk.human_name
            assert_equal 'Description', mk.human_name
        end

        def test_type
            assert_respond_to mk, :type
            assert_kind_of Fixnum, mk.type
        end

        def test_value
            assert_respond_to mk, :parse_value
            assert_kind_of String, mk.parse_value
        end
    end


    class TestCase_PackageID < Test::Unit::TestCase
        include TestStuff
        def test_no_create
            assert_raise NoMethodError do
                p = PackageID.new
            end
        end

        def test_members
            { :name => QualifiedPackageName, :version => VersionSpec, :repository_name => String,
                :keywords_key => MetadataKeywordNameSetKey, :choices_key => MetadataChoicesKey,
                :short_description_key => MetadataStringKey, :long_description_key => MetadataStringKey,
                :installed_time_key => MetadataTimeKey,
                :from_repositories_key => Array, :masks => Array, :overridden_masks => Array
            }.each_pair do | method, type |

                assert_respond_to pid_testrepo, method
                x = pid_testrepo.send(method)
                if x
                    assert_kind_of type, x
                else
                    assert_nil x
                end
            end
        end

        def test_canonical_form
            assert_equal 'foo/bar-1.0::testrepo', pid_testrepo.canonical_form(PackageIDCanonicalForm::Full)
            assert_equal '1.0', pid_testrepo.canonical_form(PackageIDCanonicalForm::Version)
            assert_equal 'foo/bar::testrepo', pid_testrepo.canonical_form(PackageIDCanonicalForm::NoVersion)
        end

        def test_stringify
            assert_equal 'foo/bar-1.0::testrepo', "#{pid_testrepo}"
        end

        def test_supports_action_test
            assert pid_testrepo.supports_action(SupportsActionTest.new(FetchAction))
            assert_raise TypeError do
                pid_testrepo.supports_action(1)
            end
            assert_raise TypeError do
                pid_testrepo.supports_action(pid_testrepo)
            end
        end

        def test_=
            assert_equal pid_testrepo, pid_testrepo
        end

        def test_subscript
            assert_equal pid_testrepo["DESCRIPTION"].parse_value, "Test package"
            assert_nil pid_testrepo["PRESCRIPTION"]
        end

        def test_each_metadata
            keys = { "DESCRIPTION" => 1, "INHERITED" => 1, "KEYWORDS" => 1, "EAPI" => 1,
                "DEPEND" => 1, "LICENSE" => 1,
                "RESTRICT" => 1, "SRC_URI" => 1, "HOMEPAGE" => 1, "EBUILD" => 1, "IUSE" => 1,
                "PALUDIS_CHOICES" => 1, "DEFINED_PHASES" => 1, "SLOT" => 1 }
            pid_testrepo.each_metadata do | key |
                assert keys.has_key?(key.raw_name), "no key #{key.raw_name} -> #{key.parse_value}"
                keys.delete key.raw_name
            end
            assert keys.empty?, "keys are #{keys.map { | k, v | k }.join ', '}"
        end

        def test_masked?
            assert pid_testrepo.masked?
            assert !pid_installed.masked?
        end

        def test_masks
            masks = pid_testrepo.masks
            assert_equal 1, masks.length
            mask = masks.first
            assert_kind_of RepositoryMask, mask
        end

        def test_overridden_masks
            masks = pid_testrepo.overridden_masks
            assert_equal 1, masks.length
            mask = masks.first
            assert_kind_of OverriddenMask, mask
            assert_kind_of UnacceptedMask, mask.mask
            assert_equal MaskOverrideReason::AcceptedUnstable, mask.override_reason
        end

        def test_mask_tokens
            masks = pid_scm.masks
            assert_equal 1, masks.length
            mask = masks.first
            assert_kind_of RepositoryMask, mask
            assert_equal "scm", mask.token
        end

        def test_hash
            a = pid_testrepo
            b = pid_testrepo
            assert_equal a.hash, b.hash
        end

        def test_eql
            a = pid_testrepo
            b = pid_testrepo
            assert a.eql?(b)
        end

        def test_uniquely_identifying_spec
            assert_kind_of PackageDepSpec, pid_testrepo.uniquely_identifying_spec
        end
    end

    class TestCase_ERepo < Test::Unit::TestCase
        include TestStuff
        def test_name
            assert_kind_of QualifiedPackageName, pid_testrepo.name
            assert_equal QualifiedPackageName.new('foo/bar'), pid_testrepo.name
        end

        def test_version
            assert_kind_of VersionSpec, pid_testrepo.version
            assert_equal VersionSpec.new('1.0'), pid_testrepo.version
        end

        def test_repository_name
            assert_kind_of String, pid_testrepo.repository_name
            assert_equal 'testrepo', pid_testrepo.repository_name
        end

        def test_slot
            assert_kind_of String, pid_testrepo.slot_key.parse_value
            assert_equal '0', pid_testrepo.slot_key.parse_value
        end

        def test_short_description
            assert_kind_of MetadataStringKey, pid_testrepo.short_description_key
            assert_equal 'Test package', pid_testrepo.short_description_key.parse_value
        end

        def test_long_description
            #only gems has this atm
            assert_nil pid_testrepo.long_description_key
        end

        def test_installed_time_key
            assert_nil pid_testrepo.installed_time_key
        end

        def test_keywords_key
            assert_kind_of MetadataKeywordNameSetKey, pid_testrepo.keywords_key
            assert_kind_of Array, pid_testrepo.keywords_key.parse_value
            assert_equal ['~test'], pid_testrepo.keywords_key.parse_value
        end

        def test_build_dependencies_target_key
            assert_kind_of MetadataDependencySpecTreeKey, pid_testrepo.build_dependencies_target_key
            assert_kind_of AllDepSpec, pid_testrepo.build_dependencies_target_key.parse_value
        end

        def test_build_dependencies_host_key
            assert_kind_of MetadataDependencySpecTreeKey, pid_testrepo.build_dependencies_host_key
            assert_kind_of AllDepSpec, pid_testrepo.build_dependencies_host_key.parse_value
        end

        def test_homepage_key
            assert_kind_of MetadataSimpleURISpecTreeKey, pid_testrepo.homepage_key
            assert_kind_of AllDepSpec, pid_testrepo.homepage_key.parse_value
        end

        def test_fetches_key
            assert_kind_of MetadataFetchableURISpecTreeKey, pid_testrepo.fetches_key
            assert_kind_of AllDepSpec, pid_testrepo.fetches_key.parse_value
            assert_respond_to pid_testrepo.fetches_key, :initial_label
            assert_kind_of URILabel, pid_testrepo.fetches_key.initial_label
            assert_kind_of URIMirrorsThenListedLabel, pid_testrepo.fetches_key.initial_label
            assert_equal "default", pid_testrepo.fetches_key.initial_label.text
        end
    end

    class TestCase_VDBRepo < Test::Unit::TestCase
        include TestStuff

        def test_name
            assert_kind_of QualifiedPackageName, pid_installed.name
            assert_equal QualifiedPackageName.new('cat-one/pkg-one'), pid_installed.name
        end

        def test_version
            assert_kind_of VersionSpec, pid_installed.version
            assert_equal VersionSpec.new('1'), pid_installed.version
        end

        def test_repository
            assert_kind_of String, pid_installed.repository_name
            assert_equal 'installed', pid_installed.repository_name
        end

        def test_slot
            assert_kind_of String, pid_installed.slot_key.parse_value
            assert_equal 'test_slot', pid_installed.slot_key.parse_value
        end

        def test_short_description
            assert_kind_of MetadataStringKey, pid_installed.short_description_key
            assert_equal 'a description', pid_installed.short_description_key.parse_value
        end

        def test_long_description
            assert_nil pid_installed.long_description_key
        end

        def test_contents
            assert_kind_of Contents, pid_installed.contents
        end

        def test_installed_time_key
            assert_kind_of MetadataTimeKey, pid_installed.installed_time_key
            assert_kind_of Time, pid_installed.installed_time_key.parse_value
        end

        def test_from_repositories_key
            assert_kind_of MetadataStringSetKey, pid_installed.from_repositories_key
            assert_equal ['origin_test'], pid_installed.from_repositories_key.parse_value
        end

        def test_keywords_key
            assert_nil pid_installed.keywords_key
        end
    end

    class TestCase_BadKeys < Test::Unit::TestCase
        include TestStuff

        def test_keywords_key
            assert_raise NameError do
                pid_bad.keywords_key.parse_value
            end
        end

        def test_build_dependencies_target_key
            assert_raise NameError do
                pid_bad.build_dependencies_target_key.parse_value
            end
        end

        def test_build_dependencies_host_key
            assert_raise NameError do
                pid_bad.build_dependencies_host_key.parse_value
            end
        end
    end
end


