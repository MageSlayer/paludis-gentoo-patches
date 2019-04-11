#!/usr/bin/ruby
# vim: set sw=4 sts=4 et tw=100 :

=begin description
This example demonstrates how to handle dependency specs. It looks through
all installed packages, and picks out any package whose dependencies include
'app-arch/unzip', or whose fetchable files includes any with a '.zip'
extension.
=end

require 'Paludis'
require 'example_command_line'

include Paludis

# Collect dependencies upon 'app-arch/unzip' and store them in our results
# table.
def collect_dependencies env, id, spec, results, recursing_sets = {}
    # We need to handle every type that could be in a dependency heirarchy.
    if spec.instance_of? AllDepSpec
        # For an AllDepSpec, just collect all of our children
        spec.each do | child |
            collect_dependencies(env, id, child, results, recursing_sets)
        end

    elsif spec.instance_of? AnyDepSpec
        # Same for an AnyDepSpec
        spec.each do | child |
            collect_dependencies(env, id, child, results, recursing_sets)
        end

    elsif spec.instance_of? ConditionalDepSpec
        # Was this condition satisfied when we built this package?
        if spec.condition_met?
            spec.each do | child |
                collect_dependencies(env, id, child, results, recursing_sets)
            end
        end

    elsif spec.instance_of? PackageDepSpec
        # spec.package may be a zero pointer if it's a wildcarded dep.
        if spec.package and spec.package == "app-arch/unzip"
            results[id.to_s][:has_dep] = true
        end

    elsif spec.instance_of? NamedSetDepSpec
        # For named set specs, we visit the set
        set = env.set(spec.name())

        # First complication: we might have a name referring to a set that
        # doesn't exist.
        if ! set
            Log.instance.message(LogLevel::Warning, "Unknown set '" + spec.name() + "'")
            return
        end

        # Second complication: we need to handle sets that contain themselves.
        # Although this shouldn't happen, user-defined sets can be made to
        # include themselves, possibly with other sets inbetween (a includes b
        # includes a).
        if recursing_sets[set.name()]
            Log.instance.message(LogLevel::Warning, "Recursively defined set '" + spec.name() + "'")
            return
        end

        recursing_sets[set.name()] = true
        collect_dependencies(env, id, child, results, recursing_sets)
        recursing_sets.delete(set.name())

    elsif spec.instance_of? BlockDepSpec
        # Do nothing

    elsif spec.instance_of? DependencyLabelsDepSpec
        # Do nothing

    else
        raise TypeError, "Got unexpected type '#{spec.class}'"
    end
end

# Collect files with extension '.zip' and store them in our results table.
def collect_extensions env, id, spec, results, recursing_sets = {}
    # We need to handle every type that could be in a fetchable URI heirarchy.
    if spec.instance_of? AllDepSpec
        # For an AllDepSpec, just collect all of our children
        spec.each do | child |
            collect_extensions(env, id, child, results, recursing_sets)
        end

    elsif spec.instance_of? ConditionalDepSpec
        # Was this condition satisfied when we built this package?
        if spec.condition_met?
            spec.each do | child |
                collect_extensions(env, id, child, results, recursing_sets)
            end
        end

    elsif spec.instance_of? FetchableURIDepSpec
        # We need to be careful not to assume that the filename has an
        # extension.
        if spec.filename[%r/\.zip$/]
            results[id.to_s][:has_ext] = true
        end

    elsif spec.instance_of? URILabelsDepSpec
        # Do nothing

    else
        raise TypeError, "Got unexpected type '#{spec.class}'"
    end

end

# We start with an Environment, respecting the user's '--environment' choice.
env = EnvironmentFactory.instance.create(ExampleCommandLine.instance.environment)

# Fetch package IDs for all installed packages.
ids = env[Selection::AllVersionsSorted.new(
    Generator::All.new | Filter::InstalledAtRoot.new("/"))]

# Our results table, mapping the ID to { :has_dep => ?, :has_ext => ? }
results = { }

# For each ID:
ids.each do | id |
    # Make a default result for this ID.
    results[id.to_s] = { :has_dep => false, :has_ext => false }

    # IDs can potentially have four dependency-related keys. Each of thse keys
    # may return nil. If it doesn't, collect 'app-arch/unzip' dependencies.
    [ :build_dependencies_target_key, :build_dependencies_host_key, :run_dependencies_key, :post_dependencies_key ].each do | key |
        if id.send(key)
            collect_dependencies(env, id, id.send(key).parse_value, results)
        end
    end

    # And the same for '.zip' file extensions
    if id.fetches_key
        collect_extensions(env, id, id.fetches_key.parse_value, results)
    end
end

# Display our results
print "Package".ljust(60), "| ", "Dep".ljust(4), "| ", "Ext".ljust(4), "\n"
print "-" * 60, "+", "-" * 5, "+", "-" * 5, "\n"
results.keys.sort.each do | id |
    print id.ljust(60), "| ", (results[id][:has_dep] ? "yes" : "no").ljust(4), "| ",
        (results[id][:has_ext] ? "yes" : "no").ljust(4), "\n"
end
puts

