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
        def test_create
            assert_nothing_raised do
                Generator::Matches.new(Paludis::parse_user_package_dep_spec("a/b", []))
            end
        end

        def test_to_s
            assert_equal Generator::Matches.new(Paludis::parse_user_package_dep_spec("a/b", [])).to_s, "packages matching a/b"
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
                Generator::Repository.new("repo")
            end
        end

        def test_to_s
            assert_equal Generator::Repository.new("repo").to_s, "packages with repository repo"
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

    class TestCase_GeneratorSomeIDsMightSupportAction < Test::Unit::TestCase
        def test_create
            assert_nothing_raised do
                Generator::SomeIDsMightSupportAction.new(InstallAction)
            end
        end

        def test_bad_create
            assert_raise TypeError do
                Generator::SomeIDsMightSupportAction.new(String)
            end
        end

        def test_to_s
            assert_equal Generator::SomeIDsMightSupportAction.new(InstallAction).to_s, "packages that might support action install"
        end
    end

    class TestCase_GeneratorUnion < Test::Unit::TestCase
        def test_create
            assert_nothing_raised do
                Generator::Union.new(Generator::All.new, Generator::SomeIDsMightSupportAction.new(InstallAction))
            end
        end

        def test_to_s
            assert_equal Generator::Union.new(Generator::All.new, Generator::SomeIDsMightSupportAction.new(InstallAction)).to_s,
                "all packages intersected with packages that might support action install"
        end
    end

    class TestCase_GeneratorAmpersand < Test::Unit::TestCase
        def test_create
            g1 = Generator::All.new
            g2 = Generator::SomeIDsMightSupportAction.new(InstallAction)
            assert_nothing_raised do
                g1 & g2
            end
        end

        def test_to_s
            assert_equal (Generator::All.new & Generator::SomeIDsMightSupportAction.new(InstalledAction)).to_s,
                "all packages intersected with packages that might support action installed"
        end
    end
end


