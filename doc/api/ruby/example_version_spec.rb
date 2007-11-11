#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=100 :

=begin description
This example demonstrates how use VersionSpec.
=end

require 'Paludis'

include Paludis

# Make a set of versions
versions = %w{1.0 1.1 1.2 1.2-r1 2.0 2.0-try1 2.0-scm 9999}.map do | v |
    VersionSpec.new v
end.sort

# For each version...
versions.each do | v |
    puts "#{v}:"

    # Show the output of various members.
    puts "    Remove revision:             #{v.remove_revision}"
    puts "    Revision only:               #{v.revision_only}"
    puts "    Bump:                        #{v.bump}"
    puts "    Is SCM?                      #{v.is_scm?}"
    puts "    Has -try?                    #{v.has_try_part?}"
    puts "    Has -scm?                    #{v.has_scm_part?}"
    puts
end

