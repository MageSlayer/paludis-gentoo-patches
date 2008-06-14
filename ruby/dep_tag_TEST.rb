#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :
#
# Copyright (c) 2007 Richard Brown
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

ENV['PALUDIS_HOME'] = Dir.getwd() + '/dep_tag_TEST_dir/home'

module Paludis
    class TestCase_DepTag < Test::Unit::TestCase
        def test_no_create
            assert_raise NoMethodError do
                ce = DepTag.new('test')
            end
        end
    end

    class TestCase_DependencyDepTag < Test::Unit::TestCase
        def env
            @env or @env = EnvironmentMaker.instance.make_from_spec("")
        end

        def get_dt
            DependencyDepTag.new(env[Selection::RequireExactlyOne.new(Generator::All.new)].last,
                                 Paludis::parse_user_package_dep_spec('foo/bar', env, []))
        end

        def test_create
            assert_kind_of DependencyDepTag, get_dt
        end

        def test_create_error
            assert_raise ArgumentError do
                DependencyDepTag.new
            end

            assert_raise ArgumentError do
                DependencyDepTag.new('a','b','c')
            end

            assert_raise TypeError do
                DependencyDepTag.new('a','b')
            end

            assert_raise ArgumentError do
                DependencyDepTag.new(1)
            end
        end

        def test_methods
            dt = get_dt
            {:short_text => 'foo/bar-1.0::testrepo', :category=>'dependency'}.each do |method, val|
                assert_respond_to dt, method
                assert_equal val, dt.send(method)
            end
        end

        def test_package_id
            dt = get_dt
            assert_respond_to dt, :package_id
            id = dt.package_id
            assert_kind_of PackageID, id
            assert_equal "foo/bar-1.0::testrepo", id.to_s
        end

        def test_dependency
            dt = get_dt
            assert_respond_to dt, :dependency
            pds = dt.dependency
            assert_kind_of PackageDepSpec, pds
            assert_equal "foo/bar", pds.to_s
        end

        # XXX extend this once we can define a non-trival condition in ruby
        def test_conditions
            dt = get_dt
            assert_respond_to dt, :conditions
            cond = dt.conditions
            assert_kind_of AllDepSpec, cond
            assert_equal 0, cond.to_a.length
        end
    end

    class TestCase_GLSADepTag < Test::Unit::TestCase
        def get_dt
            GLSADepTag.new("id", "title", "/path")
        end

        def test_create
            assert_kind_of GLSADepTag, get_dt
        end

        def test_create_error
            assert_raise ArgumentError do
                GLSADepTag.new
            end

            assert_raise ArgumentError do
                GLSADepTag.new('a')
            end

            assert_raise TypeError do
                GLSADepTag.new(1,1,3)
            end
        end

        def test_respond_to
            dt = get_dt
            {
                :short_text => 'GLSA-id',
                :category=>'glsa',
                :glsa_title => 'title',
                :glsa_file => "/path"
            }.each do |method, val|
                assert_respond_to dt, method
                assert_equal val, dt.send(method)
            end
        end
    end

    class TestCase_GeneralSetDepTag < Test::Unit::TestCase
        def get_dt
            GeneralSetDepTag.new("world", "title")
        end

        def test_create
            assert_kind_of GeneralSetDepTag, get_dt
        end

        def test_create_error
            assert_raise ArgumentError do
                GeneralSetDepTag.new
            end

            assert_raise ArgumentError do
                GeneralSetDepTag.new('a')
            end

            assert_raise TypeError do
                GeneralSetDepTag.new(1,1)
            end
        end

        def test_respond_to
            dt = get_dt
            {
                :short_text => 'world',
                :category=>'general',
                :source => 'title'
            }.each do |method, val|
                assert_respond_to dt, method
                assert_equal val, dt.send(method)
            end
        end
    end

    class TestCase_TargetDepTag < Test::Unit::TestCase
        def get_dt
            TargetDepTag.new
        end

        def test_create
            assert_kind_of TargetDepTag, get_dt
        end

        def test_create_error
            assert_raise ArgumentError do
                TargetDepTag.new(1)
            end
        end

        def test_respond_to
            dt = get_dt
            {
                :short_text => 'target',
                :category=>'target',
            }.each do |method, val|
                assert_respond_to dt, method
                assert_equal val, dt.send(method)
            end
        end
    end
end

