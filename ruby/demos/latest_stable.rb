#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :

require 'Paludis'
require 'getoptlong'

include Paludis

Log.instance.log_level = Paludis::LogLevel::Warning
Log.instance.program_name = $0

opts = GetoptLong.new(
    [ '--help',           '-h', GetoptLong::NO_ARGUMENT ],
    [ '--version',        '-V', GetoptLong::NO_ARGUMENT ],
    [ '--log-level',            GetoptLong::REQUIRED_ARGUMENT ],
    [ '--repository-dir', '-D', GetoptLong::REQUIRED_ARGUMENT ],
    [ '--write-cache-dir',       GetoptLong::REQUIRED_ARGUMENT ],
    [ '--master-repository-dir', GetoptLong::REQUIRED_ARGUMENT ])

repository_dir = Dir.pwd
write_cache_dir = '/var/empty'
master_repository_dir = ''

opts.each do | opt, arg |
    case opt
    when '--help'
        puts "Usage: " + $0 + " [options] keyword1 keyword2 ...."
        puts
        puts "Options:"
        puts "  --help                         Display a help message"
        puts "  --version                      Display program version"
        puts
        puts "  --log-level level              Set log level (debug, qa, warning, silent)"
        puts "  --repository-dir dir           Repository directory to use (defaults to .)"
        puts "  --write-cache-dir dir          Use a subdirectory named for the repository name under the specified directory for repository write cache"
        puts "  --master-repository-dir dir    Use a subdirectory named for the repository name under the specified directory for repository write cache"
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

    end
end

if ARGV.empty?
    puts "No keywords supplied"
    exit 1
end
keywords = ARGV;

env = EnvironmentFactory.instance.create("")

def check_one_package(env, search_keywords, repo, pkg)
    results = {}
    repo.package_ids(pkg).each do |pid|
        next if pid.keywords_key.nil?
        keywords = pid.keywords_key.value
        search_keywords.each do |keyword|
            if keywords.include? keyword
                results[keyword] ||= {}
                results[keyword][pid.slot] = pid.version
            end
        end
    end
    unless results.empty?
        seen_ver = []
        results.each_value do |slot|
            slot.each do |ver|
                unless seen_ver.include? ver
                    seen_ver << ver
                    puts "#{pkg}-#{ver}"
                end
            end
        end
    end
end

env.repositories.each do |repo|
    repo.category_names.each do |cat|
        repo.package_names(cat).each do |pkg|
            check_one_package(env, keywords, repo, pkg)
        end
    end
end

