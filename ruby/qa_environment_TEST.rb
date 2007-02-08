#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :

#
# Copyright (c) 2006 Richard Brown <mynamewasgone@gmail.com>
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

Paludis::Log.instance.log_level = Paludis::LogLevel::Silent

module Paludis
    module QA
        class TestCase_QAEnvironment < Test::Unit::TestCase
            def test_create
                env = QAEnvironment.new('qa_environment_TEST_dir/repo1')
                env = QAEnvironment.new('qa_environment_TEST_dir/repo1', '/var/empty')
            end

            def test_create_error
                assert_raise ArgumentError do
                    env = QAEnvironment.new
                end

                assert_raise ArgumentError do
                    env = QAEnvironment.new("1","2","3","4")
                end
            end

            def test_as_environment
                env = QAEnvironment.new('qa_environment_TEST_dir/repo1')
                assert_kind_of PackageDatabase, env.package_database
            end

            def test_pass
                env = QAEnvironment.new('qa_environment_TEST_dir/repo1')
                check = EbuildCheckMaker.instance.find_maker('create_metadata')
                ecd = EbuildCheckData.new('cat-one/pkg-one', "1", env)
                cr = check.check(ecd)
                assert_equal true, cr.empty?
            end

            def test_fail
                env = QAEnvironment.new('qa_environment_TEST_dir/repo1')
                check = EbuildCheckMaker.instance.find_maker('create_metadata')
                ecd = EbuildCheckData.new('cat-one/pkg-one', "2", env)
                # This outputs stuff to stderr. Don't Panic.
                cr = check.check(ecd)
                assert_equal false, cr.empty?
            end
        end
    end
end

