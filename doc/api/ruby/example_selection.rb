#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=100 :

=begin description
This example demonstrates how to use the standard Selection, Generator and
Filter classes.
=end

require 'Paludis'
require 'example_command_line'

include Paludis

# Run a particular selection, and show its results.
def show_selection env, selection
    # Selections support a crude form of stringification.
    puts "#{selection}:"

    # Usually the only thing clients will do with a Selection object is pass it
    # to Environment#[].
    ids = env[selection]

    # Show the results
    ids.each {|id| puts id}
    puts
end

exit_status = 0

# We start with an Environment, respecting the user's '--environment' choice.
env = EnvironmentFactory.instance.create(ExampleCommandLine.instance.environment)

# Make some selections, and display what they give. The selection
# object used determines the number and ordering of results. In the
# simplest form, it takes a Generator as a parameter.
show_selection(env, Selection::AllVersionsSorted.new(
        Generator::Matches.new(parse_user_package_dep_spec("sys-apps/paludis", env, []), nil, [])))

# Generators can be passed through a Filter. The Selection optimises
# the code internally to avoid doing excess work.
show_selection(env, Selection::AllVersionsSorted.new(
        Generator::Matches.new(parse_user_package_dep_spec("sys-apps/paludis", env, []), nil, []) |
        Filter::InstalledAtRoot.new("/")))

# Filters can be combined. Usually Filter::NotMasked should be combined
# with Filter::SupportsAction.new(InstallAction), since installed packages
# aren't masked.
show_selection(env, Selection::AllVersionsSorted.new(
        Generator::Matches.new(parse_user_package_dep_spec("sys-apps/paludis", env, []), nil, []) |
        Filter::SupportsAction.new(InstallAction) |
        Filter::NotMasked.new))

# Selection::AllVersionsSorted can be expensive, particularly if there
# is no metadata cache. Consider using other Selection objects if
# you only need the best matching or some arbitrary matching ID.
show_selection(env, Selection::BestVersionOnly.new(
        Generator::Matches.new(parse_user_package_dep_spec("sys-apps/paludis", env, []), nil, []) |
        Filter::SupportsAction.new(InstallAction) |
        Filter::NotMasked.new))

exit exit_status

