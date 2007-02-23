#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :

#
# Copyright (c) 2006 Richard Brown <rbrown@gentoo.org>
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

Paludis::Log.instance.log_level = Paludis::LogLevel::Warning

module Paludis
    module QA
        module MakerTests
            def instance
                maker_class.instance
            end

            def test_instance
                assert_equal instance.__id__, instance.__id__
                assert_kind_of maker_class, instance
            end

            def test_no_create
                assert_raise NoMethodError do
                    x = maker_class.new()
                end
            end

            def test_respond_to
                [:keys, :find_maker, :check_names, :find_check].each {|sym| assert_respond_to instance, sym}
            end

            def test_keys
                assert_kind_of Array, instance.keys
                assert_not_equal 0, instance.keys

                assert_equal(nil, instance.keys {|x| x})
            end

            def test_find_maker
                name = instance.keys.first
                assert_kind_of made_class, instance.find_maker(name)
            end

            def test_check_names
                assert_kind_of Array, instance.check_names
                assert_equal instance.keys, instance.check_names
            end

            def test_find_check
                name = instance.keys.first
                assert_equal instance.find_maker(name).describe, instance.find_check(name).describe
            end
        end

        class TestCase_FileCheckMaker < Test::Unit::TestCase
            include MakerTests
            def made_class; FileCheck; end
            def maker_class; FileCheckMaker; end
        end

        class TestCase_PackageDirCheckMaker < Test::Unit::TestCase
            include MakerTests
            def made_class; PackageDirCheck; end
            def maker_class; PackageDirCheckMaker; end
        end

        class TestCase_EbuildCheckMaker < Test::Unit::TestCase
            include MakerTests
            def made_class; EbuildCheck; end
            def maker_class; EbuildCheckMaker; end
        end

        class TestCase_PerProfileEbuildCheckMaker < Test::Unit::TestCase
            include MakerTests
            def made_class; PerProfileEbuildCheck; end
            def maker_class; PerProfileEbuildCheckMaker; end
        end

        class TestCase_ProfilesCheckMaker < Test::Unit::TestCase
            include MakerTests
            def made_class; ProfilesCheck; end
            def maker_class; ProfilesCheckMaker; end
        end

        class TestCase_ProfileCheckMaker < Test::Unit::TestCase
            include MakerTests
            def made_class; ProfileCheck; end
            def maker_class; ProfileCheckMaker; end
        end

        class TestCase_EbuildCheckData < Test::Unit::TestCase
            def test_create
                env = QAEnvironment.new('check_TEST_dir/repo1')
                ecd = EbuildCheckData.new('cat-one/pkg-one', "1", env)
            end
        end

        class TestCase_PerProfileEbuildCheckData < Test::Unit::TestCase
            def test_create
                env = QAEnvironment.new('check_TEST_dir/repo1')
                ecd = PerProfileEbuildCheckData.new('cat-one/pkg-one',
                    "1",
                    env,
                    'check_TEST_dir/repo1/profiles/profile')
            end
        end

        class TestCase_ProfileCheckData < Test::Unit::TestCase
            def test_create
                env = QAEnvironment.new('check_TEST_dir/repo1')
                ecd = EbuildCheckData.new('cat-one/pkg-one', "1", env)
            end
        end

        module CheckTests
            def test_respond_to
                check = get_check
                [:describe, :is_important?, :check].each {|sym| assert_respond_to get_check, sym}
            end
        end

        class TestCase_PackageDirCheck < Test::Unit::TestCase
            include CheckTests

            def get_check
                PackageDirCheckMaker.instance.find_maker('package_name')
            end

            def test_no_create
                assert_raise NoMethodError do
                    x = PackageDirCheck.new()
                end
            end

            def test_describe
                check = get_check
                assert_equal "Checks that the category/package name is valid", check.describe
            end

            def test_is_important
                check = get_check
                assert_equal true, check.is_important?
            end

            def test_check
                check = get_check
                assert_nothing_raised do
                    cr = check.check('cat-one/pkg-one')
                end
            end
        end

        class TestCase_FileCheck < Test::Unit::TestCase
            include CheckTests

            def get_check
                FileCheckMaker.instance.find_maker('whitespace')
            end

            def test_no_create
                assert_raise NoMethodError do
                    x = FileCheck.new()
                end
            end

            def test_describe
                check = get_check
                assert_equal "Checks whitespace", check.describe
            end

            def test_is_important
                check = get_check
                assert_equal false, check.is_important?
            end

            def test_check
                check = get_check
                assert_nothing_raised do
                    cr = check.check('cat-one/pkg-one/pkg-one-1.ebuild')
                end
            end
        end

        class TestCase_EbuildCheck < Test::Unit::TestCase
            include CheckTests

            def get_ecd
                env = QAEnvironment.new('check_TEST_dir/repo1')
                ecd = EbuildCheckData.new('cat-one/pkg-one', "1", env)
            end

            def get_check
                EbuildCheckMaker.instance.find_maker('create_metadata')
            end

            def test_no_create
                assert_raise NoMethodError do
                    x = EbuildCheck.new()
                end
            end

            def test_describe
                check = get_check
                assert_equal "Checks that the metadata can be generated", check.describe
            end

            def test_is_important
                check = get_check
                assert_equal true, check.is_important?
            end

            def test_check
                check = get_check
                assert_nothing_raised do
                    cr = check.check(get_ecd)
                end
            end
        end

        class TestCase_PerProfileEbuildCheck < Test::Unit::TestCase
            include CheckTests

            def get_ecd
                env = QAEnvironment.new('check_TEST_dir/repo1')
                PerProfileEbuildCheckData.new('cat-one/pkg-one', "1", env, 'check_TEST_dir/repo1/profiles/profile')
            end

            def get_check
                PerProfileEbuildCheckMaker.instance.find_maker('deps_visible')
            end

            def test_no_create
                assert_raise NoMethodError do
                    x = PerProfileEbuildCheck.new()
                end
            end

            def test_describe
                check = get_check
                assert_equal "Checks that packages in *DEPEND are visible", check.describe
            end

            def test_is_important
                check = get_check
                assert_equal false, check.is_important?
            end

            def test_check
                check = get_check
                assert_nothing_raised do
                    cr = check.check(get_ecd)
                end
            end
        end

        class TestCase_ProfilesCheck < Test::Unit::TestCase
            include CheckTests

            def get_check
                ProfilesCheckMaker.instance.find_maker('repo_name')
            end

            def test_no_create
                assert_raise NoMethodError do
                    x = ProfilesCheck.new()
                end
            end

            def test_describe
                check = get_check
                assert_equal "Checks that repo_name is sane", check.describe
            end

            def test_is_important
                check = get_check
                assert_equal false, check.is_important?
            end

            def test_check
                check = get_check
                assert_nothing_raised do
                    cr = check.check('check_TEST_dir/repo1/profiles')
                end
            end
        end

        class TestCase_ProfileCheck < Test::Unit::TestCase
            include CheckTests

            def get_pcd
                env = QAEnvironment.new('check_TEST_dir/repo1')
                ProfileCheckData.new('check_TEST_dir/repo1/profiles', env.portage_repository.profiles.first)
            end

            def get_check
                ProfileCheckMaker.instance.find_maker('profile_paths_exist')
            end

            def test_no_create
                assert_raise NoMethodError do
                    x = ProfileCheck.new()
                end
            end

            def test_describe
                check = get_check
                assert_equal "Checks that profile paths exist", check.describe
            end

            def test_is_important
                check = get_check
                assert_equal true, check.is_important?
            end

            def test_check
                check = get_check
                assert_nothing_raised do
                    cr = check.check(get_pcd)
                end
            end
        end
    end
end

