#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :

=begin description
This example demonstrates how to use EnvironmentFactory and the resultant
Environment.
=end

require 'Paludis'
require 'example_command_line'

include Paludis

exit_status = 0

# We use EnvironmentFactory to construct an environment from the user's
# --environment commandline choice. With an empty string, this uses the
# distribution-defined default environment. With a non-empty string, it
# is split into two parts upon the first colon (if there is no colon,
# the second part is considered empty). The first part is the name of
# the environment class to use (e.g. 'paludis', 'portage') and the
# second part is passed as parameters to be handled by that
# environment's constructor.
env = EnvironmentFactory.instance.create(ExampleCommandLine.instance.environment)

# A lot of the Environment members aren't very useful to clients. The mask
# related methods are used by PackageID, and shouldn't usually be called
# directly from clients. The system information and mirror functions are mostly
# for use by Repository subclasses. The [] operator, for selections, is covered
# in other examples. That leaves the package database, sets and (currently,
# although this may well change in the future) use flag queries. The package
# database has its own examples, so we'll start with sets:

world = env.set('world')
if (world)
    # see examples_dep_tree.rb for how to make use of this set
    puts "World set exists"
else
    puts "No world set defined"
end

# And use flags, for which we need package IDs:
ids = env[Selection::AllVersionsSorted.new(
    Generator::Matches.new(Paludis::parse_user_package_dep_spec('sys-apps/paludis', [])) |
    Filter::SupportsAction.new(InstalledAction))]

if (ids.length > 0)
    id = ids.last
    print "Use flag 'ruby' for ID '#{id.to_s}' is "
    puts env.query_use('ruby', id) ? 'enabled' : 'disabled'
end

