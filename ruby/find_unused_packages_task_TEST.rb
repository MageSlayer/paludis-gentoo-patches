#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :

#
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
    Log.instance.log_level = LogLevel::Warning

    class TestCase_FindUnusedPackagesTask < Test::Unit::TestCase
        def env
            NoConfigEnvironment.new(Dir.getwd().to_s + "/find_unused_packages_task_TEST_dir/testrepo")
        end

        def task
            FindUnusedPackagesTask.new(env,env.main_repository)
        end

        def test_create
            assert_nothing_raised do
                task
            end
        end

        def test_execute
            t = task
            bar = t.execute(QualifiedPackageName.new('foo/bar'))
            assert bar.empty?
            baz = t.execute(QualifiedPackageName.new('foo/baz'))
            assert_equal 1, baz.length
            assert_kind_of PackageID, baz.first
        end
    end
end

