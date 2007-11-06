#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :

#
# Copyright (c) 2006, 2007 Ciaran McCreesh
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

module Paludis
    class TestCase_VersionSpec < Test::Unit::TestCase
        def test_create
            v = VersionSpec.new("0")
        end

        def test_create_error
            assert_raise TypeError do
                v = VersionSpec.new(0)
            end

            assert_raise BadVersionSpecError do
                v = VersionSpec.new("1.0-r1-x")
            end
        end

        def test_compare
            v0 = VersionSpec.new("0")
            v1 = VersionSpec.new("0.1")
            v2 = VersionSpec.new("1.0")

            assert_operator v0, :<, v1
            assert_operator v0, :<, v2
            assert_operator v1, :<, v2

            assert_operator v0, :<=, v0
            assert_operator v0, :<=, v1
            assert_operator v0, :<=, v2
            assert_operator v1, :<=, v2
        end

        def test_to_s
            assert_equal "0.1_alpha2", VersionSpec.new("0.1_alpha2").to_s
        end

        def test_remove_revision
            assert_equal VersionSpec.new('0.1'), VersionSpec.new('0.1-r1').remove_revision
        end

        def test_version_only
            assert_equal 'r9', VersionSpec.new('0.1-r9').revision_only
        end

        def test_is_scm?
            assert_equal true, VersionSpec.new('scm').is_scm?
            assert_equal false, VersionSpec.new('0.1').is_scm?
        end

        def test_has_scm_part?
            assert_equal true, VersionSpec.new('scm').has_scm_part?
            assert_equal false, VersionSpec.new('0.1').has_scm_part?
        end

        def test_has_try_part?
            assert_equal true, VersionSpec.new('1.2-try3').has_try_part?
            assert_equal false, VersionSpec.new('0.1').has_try_part?
        end

        def test_bump
            assert_equal VersionSpec.new('2'), VersionSpec.new('1').bump
            assert_equal VersionSpec.new('2'), VersionSpec.new('1.2').bump
            assert_equal VersionSpec.new('1.3'), VersionSpec.new('1.2.3').bump
            assert_equal VersionSpec.new('1.3'), VersionSpec.new('1.2.3-r4').bump
            assert_equal VersionSpec.new('scm'), VersionSpec.new('scm').bump
        end
    end
end

