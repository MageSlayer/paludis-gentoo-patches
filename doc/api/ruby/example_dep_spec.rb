#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=100 :

require 'Paludis'
require 'example_command_line'

include Paludis

exit_status = 0

# We start with an Environment, respecting the user's '--environment' choice.
env = EnvironmentMaker.instance.make_from_spec(ExampleCommandLine.instance.environment)

# For each command line parameter:
ARGV.each do | arg |
    # Create a PackageDepSpec from the parameter. For user-inputted data,
    # PackageDepSpecParseMode::Permissive or PackageDepSpecParseMode::Unspecific should be used
    # (only the latter allows wildcards).
    spec = PackageDepSpec.new(arg, PackageDepSpecParseMode::Unspecific)

    # Display information about the PackageDepSpec.
    puts "Information about '#{spec}':"

    if spec.package
        puts "    Package:                #{spec.package}"
    end

    if spec.category_name_part
        puts "    Category part:          #{spec.category_name_part}"
    end

    if spec.package_name_part
        puts "    Package part:           #{spec.package_name_part}"
    end

    if spec.version_requirements and not spec.version_requirements.empty?
        print "    Version requirements:   "
        need_join = false
        spec.version_requirements.each do | r |
            if need_join
                case spec.version_requirements_mode
                when VersionRequirementsMode::And
                    print " and "
                when VersionRequirementsMode::Or
                    print " or "
                end
            end

            print r[:operator], r[:spec]
            need_join = true
        end
        puts
    end

    if spec.slot
        puts "    Slot:                   #{spec.slot}"
    end

    if spec.repository
        puts "    Repository:             #{spec.repository}"
    end

    if spec.use_requirements and not spec.use_requirements.empty?
        print "    Use requirements:       "
        need_join = false
        spec.use_requirements.each do | u |
            if need_join
                print " and "
            end

            if not u[:state]
                print "-"
            end

            print u[:flag]
            need_join = true;
        end
        puts
    end

    # And display packages matching that spec
    print "    Matches:                "
    ids = env.package_database.query(Query::Matches.new(spec), QueryOrder::OrderByVersion)
    need_indent = false
    ids.each do | id |
        if need_indent
            puts
            print "                            "
        end
        print id
        need_indent = true
    end
    puts
    puts
end

exit exit_status

