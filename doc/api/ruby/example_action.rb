#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=100 :

=begin description
This example demonstarates how to use actions. It uses FetchAction to fetch source
files for all versions of sys-apps/paludis that support fetching
=end

require 'Paludis'
require 'example_command_line'

include Paludis

exit_status = 0

# We start with an Environment, respecting the user's '--environment' choice.
env = EnvironmentFactory.instance.create(ExampleCommandLine.instance.environment)

# Fetch package IDs for 'sys-apps/paludis'
ids = env[Selection::AllVersionsSorted.new(Generator::Matches.new(
    Paludis::parse_user_package_dep_spec("sys-apps/paludis", env, []), nil, []))]

# For each ID:
ids.each do | id |
    # Do we support a FetchAction? We find out by creating a SupportsActionTest object, and
    # querying via the PackageID#supports_action method.
    supports_fetch_action = SupportsActionTest.new(FetchAction)
    if not id.supports_action(supports_fetch_action)
        puts "ID #{id} does not support the fetch action."
    else
        puts "ID #{id} supports the fetch action, trying to fetch:"

        # Carry out a FetchAction. We need to specify various options when creating a FetchAction,
        # controlling whether safe resume is used and whether unneeded (e.g. due to disabled USE
        # flags) and unmirrorable source files should still be fetched.
        fetch_action = FetchAction.new(FetchActionOptions.new({
            :exclude_unmirrorable => false,
            :fetch_unneeded => false,
            :safe_resume => true
        }))

        begin
            id.perform_action(fetch_action)

        rescue FetchActionError => e
            exit_status |= 1
            puts "Caught FetchActionError, with the following details:"

            e.failures.each do | f |
                print "  * File '#{f.target_file}': "
                need_comma = false

                if f.requires_manual_fetching?
                    print "requires manual fetching"
                    need_comma = true
                end

                if f.failed_automatic_fetching?
                    if need_comma
                        print ", "
                    end
                    print "failed automatic fetching"
                    need_comma = true
                end

                if not f.failed_integrity_checks.empty?
                    if need_comma
                        print ", "
                    end
                    print "failed integrity checks: #{f.failed_integrity_checks}"
                    need_comma = true
                end
                puts
            end
        end

    end

    puts
end

exit exit_status

