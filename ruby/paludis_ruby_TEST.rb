#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :

#
# Copyright (c) 2006, 2007, 2008, 2011 Ciaran McCreesh
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

ENV["PALUDIS_HOME"] = Dir.getwd().to_s + "/paludis_ruby_TEST_dir/home";

require 'test/unit'

class TC_Basic < Test::Unit::TestCase
    def test_require
        require 'Paludis'
    end
end

require 'Paludis'

Paludis::Log.instance.log_level = Paludis::LogLevel::Warning

module Paludis

    class TestCase_Match < Test::Unit::TestCase

        def test_match_package
            env = EnvironmentFactory.instance.create("")
            spec_good = Paludis::parse_user_package_dep_spec('>=foo/bar-1', env, [])
            spec_bad = Paludis::parse_user_package_dep_spec('>=foo/bar-2', env, [])
            pid = env[Selection::RequireExactlyOne.new(Generator::Matches.new(
                Paludis::parse_user_package_dep_spec('=foo/bar-1.0::testrepo', env, []), nil, []))].first

            assert Paludis::match_package(env, spec_good, pid, nil, [])
            assert !Paludis::match_package(env, spec_bad, pid, nil, [])

        end

        def test_match_package_in_set
            env = EnvironmentFactory.instance.create("")
            world = env.set('world')
            pid = env[Selection::RequireExactlyOne.new(Generator::Matches.new(
                Paludis::parse_user_package_dep_spec('=foo/bar-1.0::testrepo', env, []), nil, []))].first

            assert Paludis::match_package_in_set(env, world, pid, [])
        end

        def test_type_errors
            env = EnvironmentFactory.instance.create("")
            spec = Paludis::parse_user_package_dep_spec('>=foo/bar-1', env, [])
            pid = env[Selection::RequireExactlyOne.new(Generator::Matches.new(
                Paludis::parse_user_package_dep_spec('=foo/bar-1.0::testrepo', env, []), nil, []))].first

            assert_raise TypeError do
                Paludis::match_package(spec,spec,pid, nil, [])
            end

            assert_raise TypeError do
                Paludis::match_package(env,spec,spec, nil, [])
            end
        end

        def test_version_spec_comparator
            one = VersionSpec.new('1')
            two = VersionSpec.new('2')
            assert Paludis::version_spec_comparator('<', one, two)
            assert Paludis::version_spec_comparator('<=', one, two)
            assert !Paludis::version_spec_comparator('>=', one, two)
            assert !Paludis::version_spec_comparator('>', one, two)
        end

        def test_bad_version_operator
            one = VersionSpec.new('1')
            two = VersionSpec.new('2')
            assert_raise BadVersionOperatorError do
                Paludis::version_spec_comparator('throw an error', one, two)
            end

            assert_raise ArgumentError do
                Paludis::version_spec_comparator('throw an error')
            end
        end
    end

end
