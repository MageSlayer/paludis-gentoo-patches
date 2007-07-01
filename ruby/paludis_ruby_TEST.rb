#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :

#
# Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

ENV["PALUDIS_HOME"] = Dir.getwd().to_s + "/paludis_ruby_TEST_dir/home";

require 'test/unit'

class TC_Basic < Test::Unit::TestCase
    def test_require
        require 'Paludis'
    end
end

require 'Paludis'

Paludis::Log.instance.log_level = Paludis::LogLevel::Warning

module Paludis

    class TestCase_Match < Test::Unit::TestCase

        def test_match
            env = EnvironmentMaker.instance.make_from_spec("")
            spec_good = PackageDepSpec.new('>=foo/bar-1', PackageDepSpecParseMode::Permissive)
            spec_bad = PackageDepSpec.new('>=foo/bar-2', PackageDepSpecParseMode::Permissive)
            pid = env.package_database.query(Query::Matches.new(PackageDepSpec.new('=foo/bar-1.0::testrepo', PackageDepSpecParseMode::Permissive)), QueryOrder::RequireExactlyOne).first

            assert Paludis::match_package(env, spec_good, pid)
            assert !Paludis::match_package(env, spec_bad, pid)

        end

        def test_type_errors
            env = EnvironmentMaker.instance.make_from_spec("")
            spec = PackageDepSpec.new('>=foo/bar-1', PackageDepSpecParseMode::Permissive)
            pid = env.package_database.query(Query::Matches.new(PackageDepSpec.new('=foo/bar-1.0::testrepo', PackageDepSpecParseMode::Permissive)), QueryOrder::RequireExactlyOne).first

            assert_raise TypeError do
                Paludis::match_package(spec,spec,pid)
            end

            assert_raise TypeError do
                Paludis::match_package(env,spec,spec)
            end
        end
    end

end
