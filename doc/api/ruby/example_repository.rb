#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=100 :

=begin description
This example demonstrates how to use Repository.
=end

require 'Paludis'
require 'example_command_line'

include Paludis

exit_status = 0

# We start with an Environment, respecting the user's '--environment' choice.
env = EnvironmentFactory.instance.create(ExampleCommandLine.instance.environment)

# For each repository
env.repositories do |repo|
    # A repository is identified by its name.
    puts repo.name + ':'

    # Like a PackageID, a Repository has metadata. Usually metadata
    # keys will be available for all of the configuration options for
    # that repository; some repositories also provide more (ebuild
    # format repositories, for example, provide info_pkgs too). See
    # "example_metadata_key.rb" for how to display a metadata key 
    # in detail.
    puts "    Metadata Keys:".ljust(30)
    repo.each_metadata do |key|
        puts "        #{key.human_name}"
    end

    # Repositories support various methods for querying categories,
    # packages, IDs and so on. These methods are used by
    # Environment::[], but are also sometimes of direct use to
    # clients.
    puts "    Number of categories: ".ljust(31) + repo.category_names.length.to_s
    puts "    IDs for sys-apps/paludis: ".ljust(31) + repo.package_ids('sys-apps/paludis').join(' ')

end

exit exit_status

