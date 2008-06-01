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
    class TestCase_FilteredGenerator < Test::Unit::TestCase
        def test_create
            assert_nothing_raised do
                FilteredGenerator.new(Generator::All.new, Filter::All.new)
            end
        end

        def test_to_s
            assert_equal FilteredGenerator.new(Generator::All.new, Filter::All.new).to_s,
                "all packages with filter all matches"
        end

        def test_bar_generator
            assert_nothing_raised do
                Generator::All.new | Filter::All.new
            end
        end

        def test_bar_filtered_generator
            assert_nothing_raised do
                Generator::All.new | Filter::All.new | Filter::All.new
            end
        end

        def test_filtered_generator_filter
            assert_equal (Generator::All.new | Filter::NotMasked.new).filter.to_s, "all matches filtered through not masked"
        end

        def test_filtered_generator_generator
            assert_equal (Generator::Category.new("cat") | Filter::NotMasked.new).generator.to_s, "packages with category cat"
        end
    end
end

