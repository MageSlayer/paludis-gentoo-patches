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

ENV["PALUDIS_HOME"] = Dir.getwd().to_s + "/default_environment_TEST_dir/home";

require 'test/unit'
require 'Paludis'

Paludis::Log.instance.log_level = Paludis::Log::LogLevel::Warning

class Paludis

    class TestCase_DefaultEnvironment < Test::Unit::TestCase
        def test_instance
            assert_equal DefaultEnvironment.instance.__id__, DefaultEnvironment.instance.__id__
        end

        def test_no_create
            assert_raise NoMethodError do
                x = DefaultEnvironment.new()
            end
        end
    end

    class TestCase_DefaultEnvironmentUse < Test::Unit::TestCase
        def test_query_use
            assert DefaultEnvironment.instance.query_use(UseFlagName.new("enabled"))
            assert ! DefaultEnvironment.instance.query_use(UseFlagName.new("not_enabled"))
            assert ! DefaultEnvironment.instance.query_use(UseFlagName.new("sometimes_enabled"))

            pde = PackageDatabaseEntry.new(QualifiedPackageName.new("foo/bar"),
                        VersionSpec.new("1.0"), RepositoryName.new("testrepo"))

            assert DefaultEnvironment.instance.query_use(UseFlagName.new("enabled"), pde)
            assert ! DefaultEnvironment.instance.query_use(UseFlagName.new("not_enabled"), pde)
            assert DefaultEnvironment.instance.query_use(UseFlagName.new("sometimes_enabled"), pde)
        end

        def test_query_use_bad
            assert_raise ArgumentError do
                DefaultEnvironment.instance.query_use(1, 2, 3)
            end
            assert_raise TypeError do
                DefaultEnvironment.instance.query_use("not_enabled")
            end
        end

    end

    class TestCase_DefaultEnvironmentAcceptKeyword < Test::Unit::TestCase
        def test_accept_keyword
            assert DefaultEnvironment.instance.accept_keyword(KeywordName.new("test"))
            assert ! DefaultEnvironment.instance.accept_keyword(KeywordName.new("bad"))
            assert ! DefaultEnvironment.instance.accept_keyword(KeywordName.new("~test"))

            pde = PackageDatabaseEntry.new(QualifiedPackageName.new("foo/bar"),
                        VersionSpec.new("1.0"), RepositoryName.new("testrepo"))

            assert DefaultEnvironment.instance.accept_keyword(KeywordName.new("test"), pde)
            assert ! DefaultEnvironment.instance.accept_keyword(KeywordName.new("bad"), pde)
            assert DefaultEnvironment.instance.accept_keyword(KeywordName.new("~test"), pde)
        end

        def test_accept_keyword_bad
            assert_raise ArgumentError do
                DefaultEnvironment.instance.accept_keyword(1, 2, 3)
            end
            assert_raise TypeError do
                DefaultEnvironment.instance.accept_keyword("foo")
            end
        end
    end

    class TestCase_DefaultEnvironmentAcceptLicense < Test::Unit::TestCase
        def test_accept_license
            assert DefaultEnvironment.instance.accept_license("test")

            pde = PackageDatabaseEntry.new(QualifiedPackageName.new("foo/bar"),
                        VersionSpec.new("1.0"), RepositoryName.new("testrepo"))

            assert DefaultEnvironment.instance.accept_license("test", pde)
        end

        def test_accept_license_bad
            assert_raise ArgumentError do
                DefaultEnvironment.instance.accept_license(1, 2, 3)
            end
            assert_raise TypeError do
                DefaultEnvironment.instance.accept_license(123)
            end
        end
    end

    class TestCase_DefaultEnvironmentMaskReasons < Test::Unit::TestCase
        def test_mask_reasons
            p = PackageDatabaseEntry.new(QualifiedPackageName.new("foo/bar"),
                        VersionSpec.new("1.0"), RepositoryName.new("testrepo"))

            m = DefaultEnvironment.instance.mask_reasons(p)
            assert m.empty?
        end

        def test_mask_reasons_not_empty
            p = PackageDatabaseEntry.new(QualifiedPackageName.new("foo/bar"),
                        VersionSpec.new("2.0"), RepositoryName.new("testrepo"))

            m = DefaultEnvironment.instance.mask_reasons(p)
            assert ! m.empty?
            assert m.include?("keyword")
            assert_equal ["keyword"], m.to_a
        end

        def test_mask_reasons_no_such_repo
            p = PackageDatabaseEntry.new(QualifiedPackageName.new("foo/bar"),
                        VersionSpec.new("1.0"), RepositoryName.new("nosuchrepo"))

            assert_raise RuntimeError do
                DefaultEnvironment.instance.mask_reasons p
            end
        end

        def test_mask_reasons_bad
            assert_raise ArgumentError do
                DefaultEnvironment.instance.mask_reasons(1, 2)
            end
            assert_raise TypeError do
                DefaultEnvironment.instance.accept_license(123)
            end
        end
    end

end

