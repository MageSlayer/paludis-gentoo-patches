#!/usr/bin/ruby
# vim: set sw=4 sts=4 et tw=80 :

#
# Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

module NameTestCaseBase
    def valid_name_foo
        return "foo"
    end

    def valid_name_bar
        return "bar"
    end

    def bad_name
        return "foo~"
    end

    def test_create
        x = name_type().new(valid_name_foo())
    end

    def test_assign
        x = name_type().new(valid_name_foo())
        y = x
    end

    def test_compare
        x = name_type().new(valid_name_foo())
        y = name_type().new(valid_name_foo())
        z = name_type().new(valid_name_bar())
        assert_equal x, y
        assert_not_equal y, z
        assert_operator z, :<, x
        assert_operator x, :>, z
        assert_raise TypeError do
            x < valid_name_foo()
        end
    end

    def test_create_errors
        assert_raise error_type() do
            x = name_type().new(bad_name())
        end
        assert_raise error_type() do
            x = name_type().new("")
        end
    end

    def test_to_s
        x = name_type().new(valid_name_foo())
        y = name_type().new(valid_name_bar())
        assert_equal valid_name_foo(), x.to_s
        assert_equal valid_name_bar(), y.to_s
    end
end

class Paludis
    class TestCase_PackageNamePart < Test::Unit::TestCase
        include NameTestCaseBase

        def error_type
            return PackageNamePartError
        end

        def name_type
            return PackageNamePart
        end
    end

    class TestCase_CategoryNamePart < Test::Unit::TestCase
        include NameTestCaseBase

        def error_type
            return CategoryNamePartError
        end

        def name_type
            return CategoryNamePart
        end

        def test_plus
            q = CategoryNamePart.new("foo") + PackageNamePart.new("bar")
            assert_equal "foo/bar", q.to_s
        end

        def test_plus_bad
            assert_raise TypeError do
                q = CategoryNamePart.new("foo") + "bar"
            end
        end
    end

    class TestCase_UseFlagName < Test::Unit::TestCase
        include NameTestCaseBase

        def error_type
            return UseFlagNameError
        end

        def name_type
            return UseFlagName
        end
    end

    class TestCase_RepositoryName < Test::Unit::TestCase
        include NameTestCaseBase

        def error_type
            return RepositoryNameError
        end

        def name_type
            return RepositoryName
        end
    end

    class TestCase_SlotName < Test::Unit::TestCase
        include NameTestCaseBase

        def error_type
            return SlotNameError
        end

        def name_type
            return SlotName
        end
    end

    class TestCase_KeywordName < Test::Unit::TestCase
        include NameTestCaseBase

        def error_type
            return KeywordNameError
        end

        def name_type
            return KeywordName
        end
    end

    class TestCase_QualifiedPackageName < Test::Unit::TestCase
        include NameTestCaseBase

        def error_type
            return QualifiedPackageNameError
        end

        def name_type
            return QualifiedPackageName
        end

        def valid_name_foo
            return "foo/foo"
        end

        def valid_name_bar
            return "bar/bar"
        end

        def test_two_arg_create
            x = QualifiedPackageName.new(CategoryNamePart.new("foo"), PackageNamePart.new("bar"))
            assert_equal "foo/bar", x.to_s
        end

        def test_bad_arg_count_create
            assert_raise ArgumentError do
                x = QualifiedPackageName.new("foo", "bar", "baz")
            end
        end

        def test_bad_type_create
            assert_raise TypeError do
                x = QualifiedPackageName.new("foo", "bar")
            end
        end
    end
end

