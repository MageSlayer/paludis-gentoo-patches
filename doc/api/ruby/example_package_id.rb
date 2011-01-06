#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=100 :

=begin description
This example demonstrates how to use PackageID.

See "example_action.rb" for more on actions. See "example_metadata_key.rb" for more on 
metadata keys. See "example_mask.rb" for more on masks.
=end

require 'Paludis'
require 'example_command_line'

include Paludis

exit_status = 0

# We start with an Environment, respecting the user's '--environment' choice.
env = EnvironmentFactory.instance.create(ExampleCommandLine.instance.environment)

# Fetch package IDs for installed 'sys-apps/paludis'
ids = env[Selection::AllVersionsSorted.new(
    Generator::Matches.new(Paludis::parse_user_package_dep_spec("sys-apps/paludis", env, []), nil, []))]

# For each ID:
ids.each do | id |
    puts "#{id}:"

    # Start by outputting some basic properties:
    puts "    Name: ".ljust(40) + id.name
    puts "    Version: ".ljust(40) + id.version.to_s
    puts "    Repository: ".ljust(40) + id.repository_name

    # The PackageID.canonical_form method should be used when
    # outputting a package
    puts "    PackageIDCanonicalForm::Full: ".ljust(40) + id.canonical_form(PackageIDCanonicalForm::Full)
    puts "    PackageIDCanonicalForm::Version: ".ljust(40) + id.canonical_form(PackageIDCanonicalForm::Version)
    puts "    PackageIDCanonicalForm::NoVersion: ".ljust(40) + id.canonical_form(PackageIDCanonicalForm::NoVersion)

    # Let's see what keys we have. Other examples cover keys in depth,
    # so we'll just use the basic methods here.
    puts "    Keys: ".ljust(40)
    id.each_metadata do |key|
        puts "        #{key.raw_name}: ".ljust(40) + key.human_name
    end

    # And what about masks? Again, these are covered in depth
    # elsewhere.
    if id.masked?
        puts "    Masks: ".ljust(40)
        id.masks.each do |mask|
            puts "        #{mask.key}: ".ljust(40) + mask.description
        end
    end

    # Let's see which actions we support. There's no particularly nice
    # way of doing this, since it's not something we'd expect to be
    # doing.
    actions = []
    actions << "install" if id.supports_action(SupportsActionTest.new(InstallAction))
    actions << "uninstall" if id.supports_action(SupportsActionTest.new(UninstallAction))
    actions << "pretend" if id.supports_action(SupportsActionTest.new(PretendAction))
    actions << "config" if id.supports_action(SupportsActionTest.new(ConfigAction))
    actions << "fetch" if id.supports_action(SupportsActionTest.new(FetchAction))
    actions << "info" if id.supports_action(SupportsActionTest.new(InfoAction))

    puts "    Actions: ".ljust(40) + actions.join(' ')

    puts
end

exit exit_status

