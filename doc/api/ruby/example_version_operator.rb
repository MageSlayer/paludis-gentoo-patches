#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=100 :

=begin description
This example demonstrates how use Paludis::version_spec_comparator.
=end

require 'Paludis'
require 'example_command_line'

include Paludis

#Make an array of Versions
versions = [VersionSpec.new('1.0'), VersionSpec.new('1.1'), VersionSpec.new('1.2'),
    VersionSpec.new('1.2-r1'), VersionSpec.new('2.0')]

#Make an array of VersionOperator strings
operators = ['=', '>=', '~', '<', '~>']

# Display a header
print " #{'LHS'.ljust(8)} | #{'RHS'.ljust(8)}"
operators.each do |operator|
    print " | #{operator.ljust(8)}"
end

puts

print '-' * 10
puts ('+' + ('-' * 10)) * operators.length.succ

#For each pair of versions
versions.each do |v1|
    versions.each do |v2|
        print " #{v1.to_s.ljust(8)} | #{v2.to_s.ljust(8)}"

        #Apply all of our operators, and show the results
        operators.each do |operator|
            print " | " + (Paludis::version_spec_comparator(operator, v1, v2) ? 'true' : 'false').ljust(8)
        end
        puts
    end
end
