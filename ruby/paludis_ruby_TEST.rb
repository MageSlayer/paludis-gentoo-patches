#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :

require 'test/unit'

class TC_Basic < Test::Unit::TestCase
    def test_require
        require 'Paludis'
    end
end

