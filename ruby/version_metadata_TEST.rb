#!/usr/bin/ruby
# vim: set sw=4 sts=4 et tw=80 :

#
# Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

class Paludis
    class TestCase_VersionMetadata < Test::Unit::TestCase
        def test_no_create
            assert_raise NoMethodError do
                p = VersionMetadata.new
            end
        end

        def env
            unless @env
                @env = NoConfigEnvironment.new("version_metadata_TEST_dir/testrepo/")
            end
            @env
        end

        def vmd version
            env.package_database.fetch_repository("testrepo").version_metadata("foo/bar", version)
        end

        def test_license
            assert_kind_of DepAtom, vmd("1.0").license
        end

        def test_interfaces
            assert vmd("1.0").get_ebuild_interface
            assert ! vmd("1.0").get_ebin_interface
            assert ! vmd("1.0").get_cran_interface
            assert ! vmd("1.0").get_virtual_interface
        end

        def test_members
            assert_equal "Test package", vmd("1.0").description
            assert_equal "http://paludis.berlios.de/", vmd("1.0").homepage
            assert_equal "0", vmd("1.0").slot
            assert_equal "0", vmd("1.0").eapi
            assert_equal "GPL-2", vmd("1.0").license_string
        end

        def test_ebuild_members
            assert_equal "", vmd("1.0").provide_string
            assert_equal "http://example.com/bar-1.0.tar.bz2", vmd("1.0").src_uri
            assert_equal "monkey", vmd("1.0").restrict_string
            assert_equal "test", vmd("1.0").keywords.gsub(%r/\s/, "")
            assert_equal "", vmd("1.0").iuse.gsub(%r/\s/, "")
        end

        def test_deps
            assert_kind_of AllDepAtom, vmd("1.0").build_depend
            assert_kind_of AllDepAtom, vmd("1.0").run_depend
            assert_kind_of AllDepAtom, vmd("1.0").post_depend

            assert_equal 1, vmd("1.0").build_depend.to_a.length
            assert vmd("1.0").run_depend.to_a.empty?
            assert vmd("1.0").post_depend.to_a.empty?

            assert_equal "foo/bar", vmd("1.0").build_depend_string.gsub(/\s/, "")
            assert_equal "", vmd("1.0").run_depend_string.gsub(/\s/, "")
            assert_equal "", vmd("1.0").post_depend_string.gsub(/\s/, "")
        end
    end
end


