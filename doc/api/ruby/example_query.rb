#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=100 :

=begin description
This example demonstrates how use the standard Query classes.
=end

require 'Paludis'
require 'example_command_line'

include Paludis

# Run a query, and show its results.
def show_query(env, query)
    # Queries support a crude form of stringification.
    puts "#{query}:"

    # Usually the only thing clients will do with a Query object is pass it to
    # PackageDatabase#query.
    ids = env.package_database.query(query, QueryOrder::OrderByVersion)

    # Show the results
    ids.each do | id |
        puts "    #{id}"
    end
    puts
end

# We start with an Environment, respecting the user's '--environment' choice.
env = EnvironmentMaker.instance.make_from_spec(ExampleCommandLine.instance.environment)

# Make some queries, and display what they give.
show_query(env, Query::Matches.new(Paludis::parse_user_package_dep_spec("sys-apps/paludis", [])))

# Queries can be combined. The resulting query is optimised internally,
# potentially giving better performance than doing things by hand.
show_query(env,
           Query::Matches.new(Paludis::parse_user_package_dep_spec("sys-apps/paludis", [])) &
           Query::SupportsInstalledAction.new)

# Usually Query::NotMasked should be combined with Query::SupportsInstallAction,
# since installed packages aren't masked.
show_query(env,
           Query::Matches.new(Paludis::parse_user_package_dep_spec("sys-apps/paludis", [])) &
           Query::SupportsInstallAction.new &
           Query::NotMasked.new)

