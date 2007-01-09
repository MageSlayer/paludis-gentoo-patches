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
        class TestCase_CheckResult < Test::Unit::TestCase
            def test_create
                cr = CheckResult.new('a','b')
            end

            def test_create_error
                assert_raise TypeError do
                    cr = CheckResult.new(0, 'test')
                end

                assert_raise TypeError do
                    cr = CheckResult.new('test', 0)
                end

                assert_raise ArgumentError do
                    cr = CheckResult.new('test')
                end

                assert_raise ArgumentError do
                    cr = CheckResult.new('a','b','too_many')
                end
            end

            def test_respond_to
                cr = CheckResult.new('a','b')
                assert_respond_to cr, :empty?
                assert_respond_to cr, :most_severe_level
                assert_respond_to cr, :messages
                assert_respond_to cr, :<<
                assert_respond_to cr, :item
                assert_respond_to cr, :rule
            end

            def test_item
                cr = CheckResult.new('a','b')
                assert_equal 'a', cr.item
            end

            def test_rule
                cr = CheckResult.new('a','b')
                assert_equal 'b', cr.rule
            end

            def test_cat
                cr = CheckResult.new('a','b')
                assert_nothing_raised do
                    cr << Message.new(QALevel::Info, 'test')
                end
            end

            def test_empty
                cr = CheckResult.new('a','b')
                assert_equal true, cr.empty?
                cr << Message.new(QALevel::Info, 'test')
                assert_equal false, cr.empty?
            end

            def test_most_severe_level
                cr = CheckResult.new('a','b')
                assert_equal 0, cr.most_severe_level
                cr << Message.new(QALevel::Info, 'test')
                assert_equal QALevel::Info, cr.most_severe_level
                cr << Message.new(QALevel::Major, 'test2')
                assert_equal QALevel::Major, cr.most_severe_level
                cr << Message.new(QALevel::Maybe, 'test3')
                assert_equal QALevel::Major, cr.most_severe_level
            end

            def test_messages
                cr = CheckResult.new('a','b')
                assert_equal Array.new, cr.messages
                msgs = [
                    Message.new(QALevel::Info, 'test'),
                    Message.new(QALevel::Fatal, 'test2')
                ]
                cr << msgs[0]

                assert_nothing_raised do
                   cr.messages do |message|
                       assert_equal msgs.first.to_s, message.to_s
                   end
                end

                cr << msgs[1]
                assert_equal msgs.to_s, cr.messages.to_s
            end
        end
    end
end

