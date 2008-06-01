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
    class TestCase_Filter < Test::Unit::TestCase
        def test_no_create
            assert_raise NoMethodError do
                Filter::new
            end
        end
    end

    class TestCase_FilterAll < Test::Unit::TestCase
        def test_create
            assert_nothing_raised do
                Filter::All.new
            end
        end

        def test_to_s
            assert_equal Filter::All.new.to_s, "all matches"
        end
    end

    class TestCase_FilterNotMasked < Test::Unit::TestCase
        def test_create
            assert_nothing_raised do
                Filter::NotMasked.new
            end
        end

        def test_to_s
            assert_equal Filter::NotMasked.new.to_s, "not masked"
        end
    end

    class TestCase_FilterInstalledAtRoot < Test::Unit::TestCase
        def test_create
            assert_nothing_raised do
                Filter::InstalledAtRoot.new("/")
            end
        end

        def test_to_s
            assert_equal Filter::InstalledAtRoot.new("/").to_s, "installed at root /"
        end
    end

    class TestCase_FilterSupportsAction < Test::Unit::TestCase
        def test_create
            assert_nothing_raised do
                Filter::SupportsAction.new(InstallAction)
            end
        end

        def test_bad_create
            assert_raise TypeError do
                Filter::SupportsAction.new(String)
            end
        end

        def test_to_s
            assert_equal Filter::SupportsAction.new(InstallAction).to_s, "supports action install"
        end
    end

    class TestCase_FilterAnd < Test::Unit::TestCase
        def test_create
            assert_nothing_raised do
                Filter::And.new(Filter::NotMasked.new, Filter::SupportsAction.new(InstallAction))
            end
        end

        def test_to_s
            assert_equal Filter::And.new(Filter::NotMasked.new, Filter::SupportsAction.new(InstallAction)).to_s,
                "not masked filtered through supports action install"
        end
    end
end

