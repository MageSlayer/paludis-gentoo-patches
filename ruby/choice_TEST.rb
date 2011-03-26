#!/usr/bin/env ruby
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

Paludis::Log.instance.log_level = Paludis::LogLevel::Warning

module Paludis
    module TestStuff
        def env
            unless @env
                @env = NoConfigEnvironment.new("choice_TEST_dir/testrepo/", '/var/empty')
            end
            @env
        end

        def pid
            env.fetch_repository("testrepo").package_ids("foo/bar").first
        end

        def choices
            pid.choices_key.value
        end

        def use_choice
            choices.each do | choice |
                return choice if choice.raw_name == "USE"
            end
            raise "oops"
        end

        def linguas_choice
            choices.each do | choice |
                return choice if choice.raw_name == "LINGUAS"
            end
            raise "oops"
        end

        def use_flag1
            choices.find_by_name_with_prefix("flag1")
        end

        def use_flag2
            choices.find_by_name_with_prefix("flag2")
        end

        def linguas_en
            choices.find_by_name_with_prefix("linguas_en")
        end
    end

    class TestCase_Choices < Test::Unit::TestCase
        include TestStuff

        def test_choices
            assert choices
        end

        def test_each
            count = 0
            choices.each do | choice |
                assert_kind_of Choice, choice
                count += 1
            end
            assert count > 0
        end

        def test_find_by_name_with_prefix
            assert_kind_of ChoiceValue, choices.find_by_name_with_prefix("flag1")
            assert_nil choices.find_by_name_with_prefix("monkey")
        end

        def test_has_matching_contains_every_value_prefix
            assert choices.has_matching_contains_every_value_prefix?("linguas_de")
            assert ! choices.has_matching_contains_every_value_prefix?("giant_hamster")
        end
    end

    class TestCase_Choice < Test::Unit::TestCase
        include TestStuff

        def test_choice
            assert use_choice
            assert linguas_choice
        end

        def test_each
            count = 0
            use_choice.each do | choice |
                assert_kind_of ChoiceValue, choice
                count += 1
            end
            assert count > 0
        end

        def test_raw_name
            assert_equal "USE", use_choice.raw_name
            assert_equal "LINGUAS", linguas_choice.raw_name
        end

        def test_human_name
            assert_equal "USE", use_choice.human_name
            assert_equal "linguas", linguas_choice.human_name
        end

        def test_prefix
            assert_equal "", use_choice.prefix
            assert_equal "linguas", linguas_choice.prefix
        end

        def test_contains_every_value
            assert ! use_choice.contains_every_value?
            assert linguas_choice.contains_every_value?
        end

        def test_hidden
            assert ! use_choice.hidden?
            assert ! linguas_choice.hidden?
        end

        def test_show_with_no_prefix
            assert use_choice.show_with_no_prefix?
            assert ! linguas_choice.show_with_no_prefix?
        end

        def test_consider_added_or_changed
            assert use_choice.consider_added_or_changed?
            assert linguas_choice.consider_added_or_changed?
        end
    end

    class TestCase_ChoiceValue < Test::Unit::TestCase
        include TestStuff

        def test_choice_value
            assert use_flag1
            assert use_flag2
            assert linguas_en
        end

        def test_unprefixed_name
            assert_equal "flag1", use_flag1.unprefixed_name
            assert_equal "flag2", use_flag2.unprefixed_name
            assert_equal "en", linguas_en.unprefixed_name
        end

        def test_name_with_prefix
            assert_equal "flag1", use_flag1.name_with_prefix
            assert_equal "flag2", use_flag2.name_with_prefix
            assert_equal "linguas_en", linguas_en.name_with_prefix
        end

        def test_enabled
            assert use_flag1.enabled?
            assert ! use_flag2.enabled?
            assert ! linguas_en.enabled?
        end

        def test_enabled_by_default
            assert use_flag1.enabled_by_default?
            assert ! use_flag2.enabled_by_default?
            assert ! linguas_en.enabled_by_default?
        end

        def test_locked
            assert ! use_flag1.locked?
            assert ! use_flag2.locked?
            assert ! linguas_en.locked?
        end

        def test_description
            assert_equal "", use_flag1.description
            assert_equal "", use_flag2.description
            assert_equal "", linguas_en.description
        end

        def test_explicitly_listed
            assert use_flag1.explicitly_listed?
            assert use_flag2.explicitly_listed?
            assert linguas_en.explicitly_listed?
        end
    end
end

#!/usr/bin/ruby
# vim: set sw=4 sts=4 et tw=80 :

