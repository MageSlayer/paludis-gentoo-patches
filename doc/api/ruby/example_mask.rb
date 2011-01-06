#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=100 :

=begin description
This example demonstrates how to use Mask. It displays all the
mask keys for a particular PackageID.
=end

require 'Paludis'
require 'example_command_line'

include Paludis

exit_status = 0

# We start with an Environment, respecting the user's '--environment' choice.
env = EnvironmentFactory.instance.create(ExampleCommandLine.instance.environment)

# Fetch package IDs for 'sys-apps/paludis'
ids = env[Selection::AllVersionsSorted.new(
    Generator::Matches.new(Paludis::parse_user_package_dep_spec("sys-apps/paludis", env, []), nil, []))]

# For each ID:
ids.each do | id |
    puts id.to_s + ':'
    id.masks.each do |mask|
        puts "    Key: ".ljust(31) + mask.key
        puts "    Description: ".ljust(31) + mask.description

        # Some Mask subclasses contain more information than others
        if mask.kind_of? UserMask
            puts "    Class: ".ljust(31) + 'UserMask'
        elsif mask.kind_of? UnacceptedMask
            puts "    Class: ".ljust(31) + 'UnacceptedMask'
            puts "    Unaccepted key: ".ljust(31) + mask.unaccepted_key.raw_name unless mask.unaccepted_key.nil?
        elsif mask.kind_of? RepositoryMask
            puts "    Class: ".ljust(31) + 'RepositoryMask'
            puts "    Mask key: ".ljust(31) + mask.mask_key.raw_name unless mask.mask_key.nil?
        elsif mask.kind_of? UnsupportedMask
            puts "    Class: ".ljust(31) + 'UnsupportedMask'
            puts "    Explanation: ".ljust(31) + mask.explanation
        elsif mask.kind_of? AssociationMask
            puts "    Class: ".ljust(31) + 'AssociationMask'
            puts "    Associated package: ".ljust(31) + mask.associated_package.to_s unless mask.associated_package.nil?
        else
            puts "    Class: ".ljust(31) + 'Unknown'
        end

        puts

    end

    puts
end

exit exit_status

