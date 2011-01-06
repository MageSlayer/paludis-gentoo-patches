#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=100 :

=begin description
This example demonstrates how to use contents. It displays details about
the files installed by 'sys-apps/paludis'.
=end

require 'Paludis'
require 'example_command_line'

include Paludis

exit_status = 0

# We start with an Environment, respecting the user's '--environment' choice.
env = EnvironmentFactory.instance.create(ExampleCommandLine.instance.environment)

# Fetch all installed packages
ids = env[Selection::AllVersionsSorted.new(
    Generator::All.new | Filter::InstalledAtRoot.new("/"))]

# Fetch the 'system' and 'world' sets. Ordinarily we should check for
# Nil here, but these two sets will always exist.
system = env.set('system')
world = env.set('world')

# For each ID:
ids.each do | id |
    # Is it paludis?
    if match_package(env, parse_user_package_dep_spec('sys-apps/paludis', env, []), id, nil, [])
        puts id.to_s.ljust(49) + ': paludis'
    elsif match_package_in_set(env, system, id, [])
        puts id.to_s.ljust(49) + ': system'
    elsif match_package_in_set(env, world, id, [])
        puts id.to_s.ljust(49) + ': world'
    else
        puts id.to_s.ljust(49) + ': nothing'
    end
end

exit exit_status

