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

# Mostly PackageDatabase is used by Environment. But other methods are useful:
if env.package_database.has_repository_named?('gentoo')
    repo = env.package_database.fetch_repository('gentoo')
    puts "Repository 'gentoo' exists, and has format '" +
        (repo.format_key ? repo.format_key.value : '') + "'"
end

begin
    name = env.package_database.fetch_unique_qualified_package_name('git')
    puts "The only package named 'git' is '#{name}'"
rescue NoSuchPackageError
    puts "There is no package named 'git'"
rescue AmbiguousPackageNameError
    puts "There are several packages named 'git':"
    $!.options.each do |o|
        puts "    #{o}"
    end
end

exit exit_status

