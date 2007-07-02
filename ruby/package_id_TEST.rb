#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :

#
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

require 'test/unit'
require 'Paludis'

Paludis::Log.instance.log_level = Paludis::LogLevel::Warning

module Paludis
    module TestStuff
        def env
            unless @env
                @env = NoConfigEnvironment.new("package_id_TEST_dir/testrepo/", '/var/empty')
            end
            @env
        end

        def env_vdb
            unless @env_vdb
                @env_vdb = NoConfigEnvironment.new("package_id_TEST_dir/installed/")
            end
            @env_vdb
        end

        def pid_testrepo
            env.package_database.fetch_repository("testrepo").package_ids("foo/bar").first
        end

        def pid_installed
            env_vdb.package_database.fetch_repository("installed").package_ids("cat-one/pkg-one").first
        end
    end

    class TestCase_MetadataKey < Test::Unit::TestCase
        include TestStuff

        def classes
            [MetadataStringKey, MetadataContentsKey, MetadataTimeKey, MetadataUseFlagNameCollectionKey, MetadataKeywordNameCollectionKey, MetadataIUseFlagCollectionKey, MetadataInheritedCollectionKey]
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
            assert_respond_to mk, :value
            assert_kind_of String, mk.value
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
            [:name, :version, :slot, :repository, :eapi, :==, :keywords_key,
                :use_key, :iuse_key, :inherited_key, :short_description_key,
                :long_description_key, :contents_key, :installed_time_key,
                :source_origin_key, :binary_origin_key].each do |method|

                assert_respond_to pid_testrepo, method
            end
        end

        def test_canonical_form
            assert_equal 'foo/bar-1.0::testrepo', pid_testrepo.canonical_form(PackageIDCanonicalForm::Full)
            assert_equal '1.0', pid_testrepo.canonical_form(PackageIDCanonicalForm::Version)
            assert_equal 'foo/bar::testrepo', pid_testrepo.canonical_form(PackageIDCanonicalForm::NoVersion)
        end

        def test_=
            assert_equal pid_testrepo, pid_testrepo
        end
    end

    class TestCase_ERepo < Test::Unit::TestCase
        include TestStuff
        def test_name
            assert_kind_of String, pid_testrepo.name
            assert_equal 'foo/bar', pid_testrepo.name
        end

        def test_version
            assert_kind_of VersionSpec, pid_testrepo.version
            assert_equal VersionSpec.new('1.0'), pid_testrepo.version
        end

        def test_repository
            assert_kind_of Repository, pid_testrepo.repository
            assert_equal 'testrepo', pid_testrepo.repository.name
        end

        def test_eapi
            assert_kind_of EAPI, pid_testrepo.eapi
            assert_equal '0', pid_testrepo.eapi.name
        end

        def test_slot
            assert_kind_of String, pid_testrepo.slot
            assert_equal '0', pid_testrepo.slot
        end

        def test_short_description
            assert_kind_of MetadataStringKey, pid_testrepo.short_description_key
            assert_equal 'Test package', pid_testrepo.short_description_key.value
        end

        def test_long_description
            #only gems has this atm
            assert_nil pid_testrepo.long_description_key
        end

        def test_contents_key
            assert_nil pid_testrepo.contents_key
        end

        def test_installed_time_key
            assert_nil pid_testrepo.installed_time_key
        end

        def test_source_origin_key
            assert_nil pid_testrepo.source_origin_key
        end

        def test_binary_origin_key
            assert_nil pid_testrepo.binary_origin_key
        end

        def test_keywords_key
            assert_kind_of MetadataKeywordNameCollectionKey, pid_testrepo.keywords_key
            assert_kind_of Array, pid_testrepo.keywords_key.value
            assert_equal ['test'], pid_testrepo.keywords_key.value
        end

        def test_use_key
            assert_nil pid_testrepo.use_key
            #assert_kind_of MetadataUseFlagNameCollectionKey, pid_testrepo.use_key
            #assert_kind_of Array, pid_testrepo.use_key.value
            #assert_equal ['test'], pid_testrepo.use_key.value
        end

        def test_iuse_key
            assert_kind_of MetadataIUseFlagCollectionKey, pid_testrepo.iuse_key
            assert_kind_of Array, pid_testrepo.iuse_key.value
            assert_equal ['testflag'], pid_testrepo.iuse_key.value
        end

        def test_inherited_key
            assert_nil pid_testrepo.inherited_key
            #assert_kind_of MetadataInheritCollectionKey, pid_testrepo.inherited_key
            #assert_kind_of Array, pid_testrepo.iuse_key.value
            #assert_equal ['testflag'], pid_testrepo.iuse_key.value
        end
    end

    class TestCase_VDBRepo < Test::Unit::TestCase
        include TestStuff
        def test_name
            pid_installed = env_vdb.package_database.fetch_repository("installed").package_ids("cat-one/pkg-one").first
        end
        def test_name
            assert_kind_of String, pid_installed.name
            assert_equal 'cat-one/pkg-one', pid_installed.name
        end

        def test_version
            assert_kind_of VersionSpec, pid_installed.version
            assert_equal VersionSpec.new('1'), pid_installed.version
        end

        def test_repository
            assert_kind_of Repository, pid_installed.repository
            assert_equal 'installed', pid_installed.repository.name
        end

        def test_eapi
            assert_kind_of EAPI, pid_installed.eapi
            assert_equal '0', pid_installed.eapi.name
        end

        def test_slot
            assert_kind_of String, pid_installed.slot
            assert_equal 'test_slot', pid_installed.slot
        end

        def test_short_description
            assert_kind_of MetadataStringKey, pid_installed.short_description_key
            assert_equal 'a description', pid_installed.short_description_key.value
        end

        def test_long_description
            assert_nil pid_installed.long_description_key
        end

        def test_contents_key
            assert_kind_of MetadataContentsKey, pid_installed.contents_key
            assert_kind_of Contents, pid_installed.contents_key.value
        end

        def test_installed_time_key
            assert_kind_of MetadataTimeKey, pid_installed.installed_time_key
            assert_kind_of Time, pid_installed.installed_time_key.value
        end

        def test_source_origin_key
            assert_kind_of MetadataStringKey, pid_installed.source_origin_key
            assert_equal 'origin_test', pid_installed.source_origin_key.value
        end

        def test_binary_origin_key
            assert_nil pid_installed.binary_origin_key
        end

        def test_keywords_key
            assert_nil pid_installed.keywords_key
        end

        def test_use_key
            assert_kind_of MetadataUseFlagNameCollectionKey, pid_installed.use_key
            assert_kind_of Array, pid_installed.use_key.value
            assert_equal ['test', 'test_use'], pid_installed.use_key.value
        end

        def test_iuse_key
            assert_kind_of MetadataIUseFlagCollectionKey, pid_installed.iuse_key
            assert_kind_of Array, pid_installed.iuse_key.value
            assert_equal ['test', 'test_iuse'], pid_installed.iuse_key.value
        end

        def test_inherited_key
            assert_kind_of MetadataInheritedCollectionKey, pid_installed.inherited_key
            assert_kind_of Array, pid_installed.inherited_key.value
            assert_equal ['test_inherited'], pid_installed.inherited_key.value
        end
    end
end


