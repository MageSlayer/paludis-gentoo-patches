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

require 'test/unit'
require 'Paludis'

Paludis::Log.instance.log_level = Paludis::LogLevel::Warning

module Paludis
    class TestCase_PackageID < Test::Unit::TestCase
        def test_no_create
            assert_raise NoMethodError do
                p = PackageID.new
            end
        end

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

        def pid
            env.package_database.fetch_repository("testrepo").package_ids("foo/bar").first
        end

        def test_members
            assert_equal 'foo/bar', pid.name
            assert_kind_of VersionSpec, pid.version
            assert_equal VersionSpec.new('1.0'), pid.version
            assert_equal '0', pid.slot
            assert_kind_of Repository, pid.repository
            assert_equal 'testrepo', pid.repository.name
            assert_kind_of EAPI, pid.eapi
            assert_equal '0', pid.eapi.name
            assert_equal pid, pid
        end

        def test_canonical_form
            assert_equal 'foo/bar-1.0::testrepo', pid.canonical_form(PackageIDCanonicalForm::Full)
            assert_equal '1.0', pid.canonical_form(PackageIDCanonicalForm::Version)
            assert_equal 'foo/bar::testrepo', pid.canonical_form(PackageIDCanonicalForm::NoVersion)
        end
    end
end


