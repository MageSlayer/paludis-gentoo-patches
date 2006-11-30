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
        class TestCase_MetadataFile < Test::Unit::TestCase
            def metadata

                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<!DOCTYPE pkgmetadata SYSTEM \"http://www.gentoo.org/dtd/metadata.dtd\">
<pkgmetadata>
       <herd>vim</herd>
       <herd>    cookie  </herd>
       <maintainer>
               <email>  foo@bar.baz  </email>
               <name>  Foo  Bar  </name>
       </maintainer>
       <maintainer>
               <email>oink@oink</email>
       </maintainer>
       <maintainer>
               <name> Fred the Fish    </name>
       </maintainer>
       <longdescription lang=\"en\">
           Some text
       </longdescription>
</pkgmetadata>"

            end

            def test_create
                #check that we can open from file as well

                file = MetadataFile.new(metadata)
            end

            def test_respond_to
                file = MetadataFile.new(metadata)
                assert_respond_to file, :herds
                assert_respond_to file, :maintainers
            end

            def test_herds
                file = MetadataFile.new(metadata)
                herds = file.herds
                assert_equal 2, herds.length
                assert_equal true, herds.include?('vim')
                assert_equal true, herds.include?('cookie')
                assert_equal false, herds.include?('monster')
            end

            def test_maintainers
                file = MetadataFile.new(metadata)
                assert_equal 3, file.maintainers.length
                file.maintainers.each do |m|
                    if m.has_key? :email
                        if m[:email] == 'foo@bar.baz'
                            assert_equal({:email => 'foo@bar.baz', :name => 'Foo Bar'}, m)
                        else
                            assert_equal({:email => 'oink@oink'}, m)
                        end
                    else
                        assert_equal({:name => 'Fred the Fish'}, m)
                    end
                end
            end
        end
    end
end

