#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :
#
# Copyright (c) 2006, 2007, 2008, 2009 Ciaran McCreesh
# Copyright (c) 2007 Richard Brown
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
    class TestCase_ActiveDependencyLabels < Test::Unit::TestCase
        def test_create
            a = ActiveDependencyLabels.new(DependencyLabelsDepSpec.new)
            a = ActiveDependencyLabels.new(a)
            a = ActiveDependencyLabels.new(a, DependencyLabelsDepSpec.new)
            a = ActiveDependencyLabels.new([])
        end
    end
end

