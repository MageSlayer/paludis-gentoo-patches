#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :

require 'Paludis'
require 'getoptlong'

Paludis::Log.instance.log_level = Paludis::LogLevel::Warning
Paludis::Log.instance.program_name = $0

class Distribution
    def initialize
        @groups = [ "0.1", "0.2", "0.5", "1", "2", "5", "10", "100" ].map do | x |
            Paludis::VersionSpec.new x
        end
        @counts = Hash.new(0)
        @rest_count = 0
    end

    def add_version version
        group = @groups.find { | x | version < x }
        if group
            @counts[group] += 1
        else
            @rest_count += 1
        end
    end

    def add_repository repo
        repo.category_names.each do | cat |
            repo.package_names(cat).each do | pkg |
                pids = repo.package_ids(pkg)
                add_version pids.last.version unless pids.empty?
            end
        end
    end

    def text_graph
        biggest_group = [ @rest_count, @groups.inject(1) { | x, y | [ x, @counts[y] ].max } ].max
        unit_scale = 40.0 / biggest_group
        @groups.each do | group |
            printf "<%-10.10s %10d %s\n", group, @counts[group], "#" * (@counts[group] * unit_scale)
        end
        printf "%-11.11s %10d %s\n", "rest", @rest_count, "#" * (@rest_count * unit_scale)
    end

    def gruff_graph file_name
        require 'rubygems'
        require 'gruff'

        g = Gruff::Bar.new
        g.title = "Package versions distribution"

        g.data("Number of packages", (@groups.map { | group | @counts[group] }) << @rest_count)
        labels = Hash.new
        @groups.each_with_index do | group, idx |
            labels[idx] = "<" + group.to_s
        end
        labels[@groups.length] = "rest"
        g.labels = labels

        g.minimum_value = 0
        g.write file_name
    end
end

opts = GetoptLong.new(
    [ '--help',           '-h',  GetoptLong::NO_ARGUMENT ],
    [ '--version',        '-V',  GetoptLong::NO_ARGUMENT ],
    [ '--log-level',             GetoptLong::REQUIRED_ARGUMENT ],
    [ '--repository-dir', '-D',  GetoptLong::REQUIRED_ARGUMENT ],
    [ '--image',                 GetoptLong::REQUIRED_ARGUMENT ],
    [ '--write-cache-dir',       GetoptLong::REQUIRED_ARGUMENT ],
    [ '--master-repository-dir', GetoptLong::REQUIRED_ARGUMENT ])

output_image = nil
repository_dir = Dir.getwd
write_cache_dir = '/var/empty'
master_repository_dir = ''

opts.each do | opt, arg |
    case opt
    when '--help'
        puts "Usage: " + $0 + " [options]"
        puts
        puts "Options:"
        puts "  --help                        Display a help message"
        puts "  --version                     Display program version"
        puts
        puts "  --log-level level             Set log level (debug, qa, warning, silent)"
        puts "  --repository-dir dir          Set repository directory (default: cwd)"
        puts "  --write-cache-dir dir         Use a subdirectory named for the repository name under the specified directory for repository write cache"
        puts "  --master-repository-dir dir   Use a subdirectory named for the repository name under the specified directory for repository write cache"
        puts
        puts "  --image foo.png               Output as the specified image rather than as text"
        exit 0

    when '--version'
        puts $0.to_s.split(/\//).last + " " + Paludis::Version.to_s
        exit 0

    when '--log-level'
        case arg
        when 'debug'
            Paludis::Log.instance.log_level = Paludis::LogLevel::Debug
        when 'qa'
            Paludis::Log.instance.log_level = Paludis::LogLevel::Qa
        when 'warning'
            Paludis::Log.instance.log_level = Paludis::LogLevel::Warning
        when 'silent'
            Paludis::Log.instance.log_level = Paludis::LogLevel::Silent
        else
            puts "Bad --log-level value " + arg
            exit 1
        end

    when '--repository-dir'
        repository_dir = arg

    when '--write-cache-dir'
        write_cache_dir = arg

    when '--master-repository-dir'
        master_repository_dir = arg

    when '--image'
        output_image = arg

    end
end

distribution = Distribution.new
env = Paludis::NoConfigEnvironment.new repository_dir, write_cache_dir, master_repository_dir
env.repositories.each do | repo |
    distribution.add_repository repo
end
if output_image
    distribution.gruff_graph output_image
else
    distribution.text_graph
end

