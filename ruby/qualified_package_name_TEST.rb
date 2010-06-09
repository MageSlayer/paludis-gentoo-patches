#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :

#
# Copyright (c) 2006, 2007, 2008, 2010 Ciaran McCreesh
# Copyright (c) 2008 Richard Brown
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
    class TestCase_QualifiedPackageName < Test::Unit::TestCase
        def test_create
            qpn = QualifiedPackageName.new('foo-bar','baz')
            qpn2 = QualifiedPackageName.new('foo-bar/baz')
        end

        def test_create_error
            assert_raise TypeError do
                qpn = QualifiedPackageName.new(0)
            end

            assert_raise ArgumentError do
                qpn = QualifiedPackageName.new()
            end

            assert_raise ArgumentError do
                qpn = QualifiedPackageName.new('a','b','c')
            end

            assert_raise CategoryNamePartError do
                qpn = QualifiedPackageName.new('foo*','baz')
            end

            assert_raise PackageNamePartError do
                qpn = QualifiedPackageName.new('foo-bar','baz*')
            end
        end

        def test_to_s
            assert_equal 'foo-bar/baz', QualifiedPackageName.new('foo-bar','baz').to_s
        end

        def test_compare
            qpn0 = QualifiedPackageName.new('foo-bar/baz')
            qpn1 = QualifiedPackageName.new('foo-bar/quux')
            qpn2 = QualifiedPackageName.new('foo-baz/bar')

            assert_operator qpn0, :<, qpn1
            assert_operator qpn0, :<, qpn2
            assert_operator qpn1, :<, qpn2

            assert_operator qpn0, :<=, qpn0
            assert_operator qpn0, :<=, qpn2

            assert_equal qpn0, 'foo-bar/baz'
            assert_equal 'foo-bar/baz', qpn0
        end

        def test_getters
            cat = 'foo-bar'
            pkg = 'baz'
            qpn = QualifiedPackageName.new(cat,pkg)

            assert_equal cat, qpn.category
            assert_equal pkg, qpn.package
        end

        def test_to_value_type_error
            assert_raise TypeError do
                QualifiedPackageName.new('foo-bar/baz') <=> 0
            end
        end

        def test_hash
            qpn = QualifiedPackageName.new('a/b')
            qpn2 = QualifiedPackageName.new('a','b')
            assert_equal qpn.hash, qpn2.hash
        end

        def test_eql?
            qpn = QualifiedPackageName.new('a/b')
            qpn2 = QualifiedPackageName.new('a','b')
            assert qpn.eql?(qpn2)
        end
    end
end

