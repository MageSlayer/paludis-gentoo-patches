#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=100 :

require 'Paludis'
require 'example_command_line'

include Paludis

exit_status = 0

# We start with an Environment, respecting the user's '--environment' choice.
env = EnvironmentMaker.instance.make_from_spec(ExampleCommandLine.instance.environment)

# Fetch package IDs for installed 'sys-apps/paludis'
ids = env.package_database.query(
    Query::Matches.new(PackageDepSpec.new("sys-apps/paludis", PackageDepSpecParseMode::Permissive)) &
    Query::SupportsInstalledAction.new,
    QueryOrder::OrderByVersion)

# For each ID:
ids.each do | id |
    # Do we have a contents key? PackageID _key methods can return Nil.
    if not id.contents_key
        puts "ID '#{id}' does not provide a contents key."
    else
        puts "ID '#{id}' provides contents key:"

        # Contents is made up of a collection of ContentsEntry instances.
        id.contents_key.value.each do | c |

            # Some ContentsEntry subclasses contain more information than others
            if c.kind_of? ContentsDevEntry
                puts "device    #{c.name}"

            elsif c.kind_of? ContentsMiscEntry
                puts "misc      #{c.name}"

            elsif c.kind_of? ContentsFileEntry
                puts "file      #{c.name}"

            elsif c.kind_of? ContentsDirEntry
                puts "dir       #{c.name}"

            elsif c.kind_of? ContentsFifoEntry
                puts "fifo      #{c.name}"

            elsif c.kind_of? ContentsSymEntry
                puts "sym       #{c.name} -> #{c.target}"

            else
                puts "unknown   #{c}"
            end
        end

        puts
    end
end

exit exit_status

