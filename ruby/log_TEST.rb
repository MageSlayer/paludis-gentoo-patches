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

class Paludis
    class TestCase_Log < Test::Unit::TestCase
        def test_instance
            assert_equal Log.instance.__id__, Log.instance.__id__
        end

        def test_no_create
            assert_raise NoMethodError do
                x = Log.new()
            end
        end
    end

    class TestCase_LogLogLevel < Test::Unit::TestCase
        def test_log_level
            assert_equal Log.instance.log_level, Log.instance.log_level
            assert Log.instance.log_level >= Log::LogLevel::Debug
            assert Log.instance.log_level <= Log::LogLevel::Silent
        end

        def test_log_level_set
            Log.instance.log_level = Log::LogLevel::Debug;
            assert_equal Log::LogLevel::Debug, Log.instance.log_level

            Log.instance.log_level = Log::LogLevel::Warning;
            assert_equal Log::LogLevel::Warning, Log.instance.log_level
        end
    end

    class TestCase_LogMessage < Test::Unit::TestCase
        def test_log_message
            Log.instance.message Log::LogLevel::Warning, "This is a test warning message"
        end

        def test_log_message_bad
            assert_raise ArgumentError do
                Log.instance.message "This should fail"
            end

            assert_raise TypeError do
                Log.instance.message "Warning", "This should fail"
            end
        end
    end
end

