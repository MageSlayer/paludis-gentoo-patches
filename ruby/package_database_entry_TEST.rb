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
    class TestCase_PackageDatabaseEntry < Test::Unit::TestCase
        def test_create
            v = PackageDatabaseEntry.new("foo/bar", VersionSpec.new("0"), "moo")
            w = PackageDatabaseEntry.new("foo/bar", "0", "moo")
        end

        def test_create_error
            assert_raise TypeError do
                v = PackageDatabaseEntry.new("foo/bar", "0", 123)
            end
            assert_raise TypeError do
                v = PackageDatabaseEntry.new("foo/bar", 123, "moo")
            end
            assert_raise NameError do
                v = PackageDatabaseEntry.new("asdf", "0", "moo")
            end
        end

        def test_to_s
            assert_equal "foo/bar-10::moo", PackageDatabaseEntry.new("foo/bar", "10", "moo").to_s
        end

        def test_name
            pde = PackageDatabaseEntry.new('foo/bar', VersionSpec.new('1.0-r1'), 'moo')
            assert_instance_of String, pde.name
            assert_equal pde.name ,'foo/bar'
        end

        def test_name_set
            pde = PackageDatabaseEntry.new('foo/bar', VersionSpec.new('1.0-r1'), 'moo')
            pde.name='foo/baz'
            assert_equal pde.name, 'foo/baz'
        end

        def test_version
            pde = PackageDatabaseEntry.new('foo/bar', VersionSpec.new('1.0-r1'), 'moo')
            assert_instance_of VersionSpec, pde.version
            assert_equal pde.version ,VersionSpec.new('1.0-r1')
            pde = PackageDatabaseEntry.new('foo/bar', '1.0-r1', 'moo')
            assert_instance_of VersionSpec, pde.version
            assert_equal pde.version ,VersionSpec.new('1.0-r1')
        end

        def test_version_set
            pde = PackageDatabaseEntry.new('foo/bar', VersionSpec.new('1.0-r1'), 'moo')
            pde.version = VersionSpec.new('7')
            assert_equal pde.version, VersionSpec.new('7')
            pde.version = '7.1'
            assert_equal pde.version, VersionSpec.new('7.1')
        end

        def test_repository
            pde = PackageDatabaseEntry.new('foo/bar', VersionSpec.new('1.0-r1'), 'moo')
            assert_instance_of String, pde.repository
            assert_equal pde.repository ,'moo'
        end
        
        def test_repository_set
            pde = PackageDatabaseEntry.new('foo/bar', VersionSpec.new('1.0-r1'), 'moo')
            pde.repository='testrepo'
            assert_equal pde.repository, 'testrepo'
        end

    end
end

