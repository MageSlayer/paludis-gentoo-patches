#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :

#
# Copyright (c) 2007 Richard Brown
# Copyright (c) 2007 Alexander Færøy
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
    class TestCase_Query < Test::Unit::TestCase
        def test_no_create
            assert_raise NoMethodError do
                Query::Query.new;
            end
        end
    end

    class TestCase_NotMasked < Test::Unit::TestCase
        def get_query
            Query::NotMasked.new
        end

        def test_create
            assert_nothing_raised do 
                get_query
            end
        end

        def test_and
            q = get_query
            assert_respond_to q, :&
            assert_kind_of Query::Query, q & q
        end

        def test_kind_of
            assert_kind_of Query::Query, get_query
        end
    end

    class TestCase_Matches < Test::Unit::TestCase
        def get_query
            Query::Matches.new(PackageDepSpec.new('>=foo-bar/baz-1', PackageDepSpecParseMode::Permissive))
        end

        def test_create
            assert_nothing_raised do 
                get_query
            end
        end

        def test_and
            q = get_query
            assert_respond_to q, :&
            assert_kind_of Query::Query, q & q
        end

        def test_kind_of
            assert_kind_of Query::Query, get_query
        end
    end

    class TestCase_Package < Test::Unit::TestCase
        def get_query
            Query::Package.new(QualifiedPackageName.new('foo-bar/baz'))
        end

        def test_create
            assert_nothing_raised do 
                get_query
            end
        end

        def test_and
            q = get_query
            assert_respond_to q, :&
            assert_kind_of Query::Query, q & q
        end

        def test_kind_of
            assert_kind_of Query::Query, get_query
        end
    end

    class TestCase_Repository < Test::Unit::TestCase
        def get_query
            Query::Repository.new("gentoo")
        end

        def test_create
            assert_nothing_raised do
                get_query
            end
        end

        def test_and
            q = get_query
            assert_respond_to q, :&
            assert_kind_of Query::Query, get_query
        end
    end

    class TestCase_All < Test::Unit::TestCase
        def get_query
            Query::All.new
        end

        def test_create
            assert_nothing_raised do
                get_query
            end
        end

        def test_and
            q = get_query
            assert_respond_to q, :&
            assert_kind_of Query::Query, get_query
        end
    end

    class TestCase_Category < Test::Unit::TestCase
        def get_query
            Query::Category.new("foo-bar")
        end

        def test_create
            assert_nothing_raised do
                get_query
            end
        end

        def test_and
            q = get_query
            assert_respond_to q, :&
            assert_kind_of Query::Query, get_query
        end
    end

    class TestCase_SupportsInstallAction < Test::Unit::TestCase
        def get_query
            Query::SupportsInstallAction.new
        end

        def test_create
            assert_nothing_raised do
                get_query
            end
        end

        def test_and
            q = get_query
            assert_respond_to q, :&
            assert_kind_of Query::Query, get_query
        end
    end

    class TestCase_SupportsUninstallAction < Test::Unit::TestCase
        def get_query
            Query::SupportsUninstallAction.new
        end

        def test_create
            assert_nothing_raised do
                get_query
            end
        end

        def test_and
            q = get_query
            assert_respond_to q, :&
            assert_kind_of Query::Query, get_query
        end
    end

    class TestCase_SupportsInstalledAction < Test::Unit::TestCase
        def get_query
            Query::SupportsInstalledAction.new
        end

        def test_create
            assert_nothing_raised do
                get_query
            end
        end

        def test_and
            q = get_query
            assert_respond_to q, :&
            assert_kind_of Query::Query, get_query
        end
    end

    class TestCase_SupportsPretendAction < Test::Unit::TestCase
        def get_query
            Query::SupportsPretendAction.new
        end

        def test_create
            assert_nothing_raised do
                get_query
            end
        end

        def test_and
            q = get_query
            assert_respond_to q, :&
            assert_kind_of Query::Query, get_query
        end
    end

    class TestCase_SupportsConfigAction < Test::Unit::TestCase
        def get_query
            Query::SupportsConfigAction.new
        end

        def test_create
            assert_nothing_raised do
                get_query
            end
        end

        def test_and
            q = get_query
            assert_respond_to q, :&
            assert_kind_of Query::Query, get_query
        end
    end
end

