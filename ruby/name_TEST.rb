#!/usr/bin/ruby
# vim: set sw=4 sts=4 et tw=80 :

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
        assert x == y
        assert y != z
        assert z < x
        assert x > z
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
        assert x.to_s == valid_name_foo()
        assert y.to_s == valid_name_bar()
    end
end

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

