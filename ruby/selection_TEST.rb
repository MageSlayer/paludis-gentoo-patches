#!/usr/bin/ruby
# vim: set sw=4 sts=4 et tw=80 :

#
# Copyright (c) 2008 Ciaran McCreesh
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
    class TestCase_Selection < Test::Unit::TestCase
        def test_no_create
            assert_raise NoMethodError do
                Selection::new
            end
        end
    end

    class TestCase_SelectionSomeArbitraryVersion < Test::Unit::TestCase
        def test_create
            assert_nothing_raised do
                Selection::SomeArbitraryVersion.new(Generator::All.new())
            end

            assert_nothing_raised do
                Selection::SomeArbitraryVersion.new(Generator::All.new() | Filter::All.new())
            end
        end

        def test_to_s
            assert_equal Selection::SomeArbitraryVersion.new(Generator::All.new()).to_s,
                "some arbitrary version from all packages with filter all matches"
        end
    end

    class TestCase_SelectionBestVersionOnly < Test::Unit::TestCase
        def test_create
            assert_nothing_raised do
                Selection::BestVersionOnly.new(Generator::All.new())
            end

            assert_nothing_raised do
                Selection::BestVersionOnly.new(Generator::All.new() | Filter::All.new())
            end
        end

        def test_to_s
            assert_equal Selection::BestVersionOnly.new(Generator::All.new()).to_s,
                "best version of each package from all packages with filter all matches"
        end
    end

    class TestCase_SelectionBestVersionInEachSlot < Test::Unit::TestCase
        def test_create
            assert_nothing_raised do
                Selection::BestVersionInEachSlot.new(Generator::All.new())
            end

            assert_nothing_raised do
                Selection::BestVersionInEachSlot.new(Generator::All.new() | Filter::All.new())
            end
        end

        def test_to_s
            assert_equal Selection::BestVersionInEachSlot.new(Generator::All.new()).to_s,
                "best version in each slot from all packages with filter all matches"
        end
    end

    class TestCase_SelectionAllVersionsGroupedBySlot < Test::Unit::TestCase
        def test_create
            assert_nothing_raised do
                Selection::AllVersionsGroupedBySlot.new(Generator::All.new())
            end

            assert_nothing_raised do
                Selection::AllVersionsGroupedBySlot.new(Generator::All.new() | Filter::All.new())
            end
        end

        def test_to_s
            assert_equal Selection::AllVersionsGroupedBySlot.new(Generator::All.new()).to_s,
                "all versions grouped by slot from all packages with filter all matches"
        end
    end

    class TestCase_SelectionAllVersionsUnsorted < Test::Unit::TestCase
        def test_create
            assert_nothing_raised do
                Selection::AllVersionsUnsorted.new(Generator::All.new())
            end

            assert_nothing_raised do
                Selection::AllVersionsUnsorted.new(Generator::All.new() | Filter::All.new())
            end
        end

        def test_to_s
            assert_equal Selection::AllVersionsUnsorted.new(Generator::All.new()).to_s,
                "all versions in some arbitrary order from all packages with filter all matches"
        end
    end

    class TestCase_SelectionRequireExactlyOne < Test::Unit::TestCase
        def test_create
            assert_nothing_raised do
                Selection::RequireExactlyOne.new(Generator::All.new())
            end

            assert_nothing_raised do
                Selection::RequireExactlyOne.new(Generator::All.new() | Filter::All.new())
            end
        end

        def test_to_s
            assert_equal Selection::RequireExactlyOne.new(Generator::All.new()).to_s,
                "the single version from all packages with filter all matches"
        end
    end

end

