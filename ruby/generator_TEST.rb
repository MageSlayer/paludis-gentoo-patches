#!/usr/bin/ruby
# vim: set sw=4 sts=4 et tw=80 :

#
# Copyright (c) 2008, 2011 Ciaran McCreesh
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

ENV['PALUDIS_HOME'] = Dir.getwd() + '/generator_TEST_dir/home'

module Paludis
    class TestCase_Generator < Test::Unit::TestCase
        def test_no_create
            assert_raise NoMethodError do
                Generator::new
            end
        end
    end

    class TestCase_GeneratorAll < Test::Unit::TestCase
        def test_create
            assert_nothing_raised do
                Generator::All.new
            end
        end

        def test_to_s
            assert_equal Generator::All.new.to_s, "all packages"
        end
    end

    class TestCase_GeneratorMatches < Test::Unit::TestCase
        def env
            @env or @env = EnvironmentFactory.instance.create("")
        end

        def test_create
            assert_nothing_raised do
                Generator::Matches.new(Paludis::parse_user_package_dep_spec("a/b", env, []), nil, [])
            end
        end

        def test_to_s
            assert_equal Generator::Matches.new(Paludis::parse_user_package_dep_spec("a/b", env, []), nil, []).to_s,
                "packages matching a/b"
            assert_equal Generator::Matches.new(Paludis::parse_user_package_dep_spec("a/b", env, []),
                                               nil, [:ignore_choice_requirements]).to_s,
                "packages matching a/b (ignoring choice requirements)"
        end
    end

    class TestCase_GeneratorCategory < Test::Unit::TestCase
        def test_create
            assert_nothing_raised do
                Generator::Category.new("cat")
            end
        end

        def test_to_s
            assert_equal Generator::Category.new("cat").to_s, "packages with category cat"
        end
    end

    class TestCase_GeneratorRepository < Test::Unit::TestCase
        def test_create
            assert_nothing_raised do
                Generator::InRepository.new("repo")
            end
        end

        def test_to_s
            assert_equal Generator::InRepository.new("repo").to_s, "packages with repository repo"
        end
    end

    class TestCase_GeneratorPackage < Test::Unit::TestCase
        def test_create
            assert_nothing_raised do
                Generator::Package.new("cat/pkg")
            end
        end

        def test_to_s
            assert_equal Generator::Package.new("cat/pkg").to_s, "packages named cat/pkg"
        end
    end

    class TestCase_GeneratorIntersection < Test::Unit::TestCase
        def test_create
            assert_nothing_raised do
                Generator::Intersection.new(Generator::All.new, Generator::InRepository.new("arbor"))
            end
        end

        def test_to_s
            assert_equal Generator::Intersection.new(Generator::All.new, Generator::InRepository.new("arbor")).to_s,
                "all packages intersected with packages with repository arbor"
        end
    end

    class TestCase_GeneratorAmpersand < Test::Unit::TestCase
        def test_create
            g1 = Generator::All.new
            g2 = Generator::InRepository.new("arbor")
            assert_nothing_raised do
                g1 & g2
            end
        end

        def test_to_s
            assert_equal (Generator::All.new & Generator::InRepository.new("arbor")).to_s,
                "all packages intersected with packages with repository arbor"
        end
    end
end


