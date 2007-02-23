#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :

#
# Copyright (c) 2007 Richard Brown <rbrown@gentoo.org>
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

    class TestCase_RepositoryHasInstallableInterface < Test::Unit::TestCase
        def get_query
            Query::RepositoryHasInstallableInterface.new
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

    class TestCase_RepositoryHasUninstallableInterface < Test::Unit::TestCase
        def get_query
            Query::RepositoryHasUninstallableInterface.new
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

    class TestCase_RepositoryHasInstalledInterface < Test::Unit::TestCase
        def get_query
            Query::RepositoryHasInstalledInterface.new
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
            Query::Matches.new(PackageDepSpec.new('>=foo-bar/baz-1'))
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
end

