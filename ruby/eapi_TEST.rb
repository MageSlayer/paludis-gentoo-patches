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

ENV["PALUDIS_HOME"] = Dir.getwd().to_s + "/default_environment_TEST_dir/home";

require 'test/unit'
require 'Paludis'

module Paludis
    class TestCase_EAPIData < Test::Unit::TestCase
        def test_instance
            assert_equal Log.instance.__id__, Log.instance.__id__
        end

        def test_no_create
            assert_raise NoMethodError do
                x = EAPIData.new()
            end
        end

        def test_unknown_eapi
            assert_respond_to EAPIData.instance, :unknown_eapi
            assert_kind_of EAPI, EAPIData.instance.unknown_eapi
        end

        def test_eapi_from_string
            assert_respond_to EAPIData.instance, :eapi_from_string
            assert_kind_of EAPI, EAPIData.instance.eapi_from_string('0')
        end
    end

    class TestCase_EAPI < Test::Unit::TestCase
        def eapi0
            @eapi0 ||= EAPIData.instance.eapi_from_string('0')
        end

        def eapi_zarniwoop
            @eapi_zarniwoop ||= EAPIData.instance.eapi_from_string('zarniwoop')
        end

        def test_no_create
            assert_raise NoMethodError do
                x = EAPI.new()
            end
        end

        def test_name
            assert_equal '0', eapi0.name
            assert_equal 'zarniwoop', eapi_zarniwoop.name
        end

        def test_supported
            assert_kind_of SupportedEAPI, eapi0.supported
            assert_kind_of NilClass, eapi_zarniwoop.supported
        end
    end

    class TestCase_SupportedEAPI < Test::Unit::TestCase
        include PackageDepSpecParseMode
        include IUseFlagParseMode

        def supported
            @supported ||= EAPIData.instance.eapi_from_string('0').supported
        end

        def test_no_create
            assert_raise NoMethodError do
                x = SupportedEAPI.new()
            end
        end

        def test_package_dep_spec_parse_mode
            assert_equal Eapi0, supported.package_dep_spec_parse_mode
        end

        def test_strict_package_dep_spec_parse_mode
            assert_equal Eapi0Strict, supported.strict_package_dep_spec_parse_mode
        end

        def test_iuse_flag_parse_mode
            assert_equal Eapi0, supported.iuse_flag_parse_mode
        end

        def test_strict_iuse_flag_parse_mode
            assert_equal Eapi0Strict, supported.strict_iuse_flag_parse_mode
        end

        def test_breaks_portage?
            assert !supported.breaks_portage?
        end

        def test_has_pretend_phase?
            assert !supported.has_pretend_phase?
        end
    end
end

