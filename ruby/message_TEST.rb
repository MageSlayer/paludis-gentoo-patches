#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :

#
# Copyright (c) 2006 Richard Brown <mynamewasgone@gmail.com>
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
    module QA
        class TestCase_Message < Test::Unit::TestCase
            def test_create
                msg = Message.new(QALevel::Info,'test')
            end

            def test_create_error
                assert_raise TypeError do
                    msg = Message.new('wrong', 'test')
                end

                assert_raise TypeError do
                    msg = Message.new(QALevel::Info, 0)
                end

                assert_raise ArgumentError do
                    msg = Message.new(QALevel::Info)
                end

                assert_raise ArgumentError do
                    msg = Message.new(QALevel::Info,'test','too_many')
                end

            end

            def test_respond_to
                msg = Message.new(QALevel::Info,'test')
                assert_respond_to msg, :to_s
                assert_respond_to msg, :msg
                assert_respond_to msg, :level
            end

            def test_to_s
                msg = Message.new(QALevel::Info,'test')
                assert_equal '(info) test', msg.to_s
            end

            def test_msg
                msg = Message.new(QALevel::Info,'test')
                assert_equal 'test', msg.msg
            end

            def test_level
                msg = Message.new(QALevel::Info, 'test')
                assert_equal QALevel::Info, msg.level
            end
        end
    end
end

