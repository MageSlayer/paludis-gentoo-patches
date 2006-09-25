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

class Paludis
    class TestCase_VersionSpec < Test::Unit::TestCase
        def test_create
            v = PackageDepAtom.new(">=foo/bar-1")
        end

        def test_create_error
            assert_raise TypeError do
                v = PackageDepAtom.new(0)
            end
            assert_raise PackageDepAtomError do
                v = PackageDepAtom.new("=sys-apps/foo")
            end
            assert_raise NoMethodError do
                v = DepAtom.new
            end
        end

        def test_to_s
            assert_equal ">=foo/bar-1", PackageDepAtom.new(">=foo/bar-1").to_s
        end
    end
end


