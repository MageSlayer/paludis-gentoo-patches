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

module Paludis
    class TestCase_DepAtom < Test::Unit::TestCase
        def test_create_error
            assert_raise NoMethodError do
                v = DepAtom.new
            end
            assert_raise NoMethodError do
                v = StringDepAtom.new
            end
            assert_raise NoMethodError do
                v = AnyDepAtom.new
            end
            assert_raise NoMethodError do
                v = AllDepAtom.new
            end
        end
    end

    class TestCase_PackageDepAtom < Test::Unit::TestCase
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
        end

        def test_to_s
            assert_equal ">=foo/bar-1", PackageDepAtom.new(">=foo/bar-1").to_s
        end
    end

    class TestCase_PlainTextDepAtom < Test::Unit::TestCase
        def test_create
            v = PlainTextDepAtom.new("monkey")
        end

        def test_create_error
            assert_raise TypeError do
                v = PlainTextDepAtom.new(0)
            end
        end

        def test_to_s
            assert_equal "monkey", PlainTextDepAtom.new("monkey").to_s
        end
    end

    class TestCase_BlockDepAtom < Test::Unit::TestCase
        def test_create
            v = BlockDepAtom.new(PackageDepAtom.new(">=foo/bar-1"))
            w = BlockDepAtom.new("<=foo/bar-2")
        end

        def test_create_error
            assert_raise TypeError do
                v = BlockDepAtom.new(0)
            end
            assert_raise PackageDepAtomError do
                v = BlockDepAtom.new("=foo/bar")
            end
            assert_raise TypeError do 
                v = BlockDepAtom.new(PlainTextDepAtom.new('foo-bar/baz'))
            end
        end

        def test_blocked_atom
            assert_equal "foo/bar", BlockDepAtom.new("foo/bar").blocked_atom.to_s
            assert_equal "foo/baz", BlockDepAtom.new(PackageDepAtom.new("foo/baz")).blocked_atom.to_s
        end
    end

    class TestCase_Composites < Test::Unit::TestCase
        def test_composites
            atom = PortageDepParser::parse("|| ( foo/bar foo/baz ) foo/monkey")
            assert_kind_of CompositeDepAtom, atom
            assert_kind_of AllDepAtom, atom

            assert_equal 2, atom.to_a.length

            atom.each_with_index do | a, i |
                case i
                when 0:
                    assert_kind_of AnyDepAtom, a
                    assert_equal 2, a.to_a.length
                    a.each_with_index do | b, j |
                        case j
                        when 0:
                            assert_kind_of PackageDepAtom, b
                            assert_equal "foo/bar", b.to_s

                        when 1:
                            assert_kind_of PackageDepAtom, b
                            assert_equal "foo/baz", b.to_s

                        else
                            throw "Too many items"
                        end
                    end

                when 1:
                    assert_kind_of PackageDepAtom, a
                    assert_equal "foo/monkey", a.to_s

                else
                    throw "Too many items"
                end
            end
        end
    end
end


