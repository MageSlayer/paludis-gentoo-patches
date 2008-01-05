#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :

require 'Paludis'
require 'getoptlong'

Paludis::Log.instance.log_level = Paludis::LogLevel::Warning
Paludis::Log.instance.program_name = $0

opts = GetoptLong.new(
    [ '--help',          '-h',  GetoptLong::NO_ARGUMENT ],
    [ '--version',       '-V',  GetoptLong::NO_ARGUMENT ],
    [ '--log-level',            GetoptLong::REQUIRED_ARGUMENT ],
    [ '--environment',   '-E',  GetoptLong::REQUIRED_ARGUMENT ],
    [ '--size-limit',    '-s',  GetoptLong::REQUIRED_ARGUMENT ],
    [ '--time-limit',    '-t',  GetoptLong::REQUIRED_ARGUMENT ])

env_spec = ""
size_limit = time_limit = nil
opts.each do | opt, arg |
    case opt
    when '--help'
        puts "Usage: " + $0 + " [options]"
        puts
        puts "Options:"
        puts "  --help                  Display a help message"
        puts "  --version               Display program version"
        puts
        puts "  --log-level level       Set log level (debug, qa, warning, silent)"
        puts "  --environment env       Environment specification (class:suffix, both parts optional)"
        puts
        puts "  --size-limit limit      Don't delete anything bigger than limit (number followed by one of g, m, k, b)"
        puts "  --time-limit limit      Don't delete anything newer than limit (number followed by one of h, d, w, m, y)"
        exit 0

    when '--version'
        puts $0.to_s.split(/\//).last + " " + Paludis::Version
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

    when '--environment'
        env_spec = arg

    when '--size-limit'
        if arg =~ /^[0-9]+[gmkb]$/i then
            size_limit = arg[0..-2].to_i *
                case arg.downcase[-1]
                when ?b: 1
                when ?k: 1024
                when ?m: 1024 * 1024
                when ?g: 1024 * 1024 * 1024
                end
        else
            puts "Bad --size-limit value " + arg
            exit 1
        end

    when '--time-limit'
        if arg =~ /^[0-9]+[ymwdh]/ then
            time_limit = Time.now - arg[0..-2].to_i *
                case arg.downcase[-1]
                when ?h: 60 * 60
                when ?d: 60 * 60 * 24
                when ?w: 60 * 60 * 24 * 7
                when ?m: 60 * 60 * 24 * 30
                when ?y: 60 * 60 * 24 * 365
                end
        else
            puts "Bad --time-limit value " + arg
            exit 1
        end

    end
end

env = Paludis::EnvironmentMaker.instance.make_from_spec env_spec

def collect_filenames(env, parts, id, spec)
    case spec
    when Paludis::AllDepSpec
        spec.each { | item | collect_filenames(env, parts, id, item) }
    when Paludis::UseDepSpec
        spec.each { | item | collect_filenames(env, parts, id, item) } if
            env.query_use(spec.flag, id) ^ spec.inverse?
    when Paludis::FetchableURIDepSpec
        parts[spec.filename] = true
    when Paludis::URILabelsDepSpec
        # don't need to do anything
    else
        raise "Unexpected DepSpec class #{spec.class} in #{id}"
    end
end

# build up a list of all src_uri things that're used by installed packages
parts = Hash.new
env.package_database.repositories.each do | repo |
    repo.some_ids_might_support_action(Paludis::SupportsInstalledActionTest.new) or next
    repo.category_names.each do | cat |
        repo.package_names(cat).each do | pkg |
            repo.package_ids(pkg).each do | id |
                id.supports_action(Paludis::SupportsInstalledActionTest.new) or next
                collect_filenames(env, parts, id, id.fetches_key.value) if
                    id.fetches_key && id.fetches_key.value
            end
        end
    end
end

# figure out a list of places where distfiles can be found
distdirs = []
env.package_database.repositories.each do | repo |
    key = repo['distdir'] or next
    distdirs << key.value unless distdirs.member? key.value
end

# display each unused distfile
distdirs.each do | dir |
    Dir.new(dir).sort.each do | file |
        stat = File.new(dir + "/" + file).stat
        stat.file? or next
        stat.size  >= size_limit and next unless size_limit.nil?
        stat.mtime >= time_limit and next unless time_limit.nil?
        puts dir + "/" + file unless parts[file]
    end
end

