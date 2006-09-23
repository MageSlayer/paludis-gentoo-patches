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

class TestCase_PackageDatabaseEntry < Test::Unit::TestCase
    def test_create
        v = PackageDatabaseEntry.new(QualifiedPackageName.new("foo/bar"),
                VersionSpec.new("0"), RepositoryName.new("moo"))
    end

    def test_create_error
        assert_raise TypeError do
            v = PackageDatabaseEntry.new("foo/bar", "0", "moo")
        end
    end
    def test_to_s
        assert_equal "foo/bar-10::moo", PackageDatabaseEntry.new(QualifiedPackageName.new("foo/bar"),
                        VersionSpec.new("10"), RepositoryName.new("moo")).to_s
    end
end

