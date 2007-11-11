#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :

=begin description
Basic command line handling for most examples.
=end

require 'getoptlong'
require 'singleton'
require 'Paludis'

class ExampleCommandLine < GetoptLong
    include Singleton

    def initialize
        super(
            [ '--log-level',             GetoptLong::REQUIRED_ARGUMENT ],
            [ '--environment',     '-E', GetoptLong::REQUIRED_ARGUMENT ]
        )

        @environment = ""
        each do | opt, arg |
            case opt
            when '--log-level'
                Paludis::Log.instance.log_level = Paludis::LogLevel::Debug
            when '--environment'
                @environment = arg
            end
        end
    end

    attr_reader :environment
end

