#!/usr/bin/ruby
# vim: set sw=4 sts=4 et tw=80 :

require 'Paludis'
require 'getoptlong'

Paludis::Log.instance.log_level = Paludis::LogLevel::Warning
Paludis::Log.instance.program_name = $0

opts = GetoptLong.new(
    [ '--help',          '-h',  GetoptLong::NO_ARGUMENT ],
    [ '--version',       '-V',  GetoptLong::NO_ARGUMENT ],
    [ '--log-level',            GetoptLong::REQUIRED_ARGUMENT ],
    [ '--config-suffix', '-c',  GetoptLong::REQUIRED_ARGUMENT ])

config_suffix = ""
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
        puts "  --config-suffix suffix  Set configuration suffix)"
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

    when '--config-suffix'
        config_suffix = arg

    end
end

Paludis::DefaultConfig::config_suffix = config_suffix
env = Paludis::DefaultEnvironment.instance

# build up a list of all src_uri things that're used by installed packages
parts = Hash.new
env.package_database.repositories.each do | repo |
    repo.installed_interface or next
    repo.category_names.each do | cat |
        repo.package_names(cat).each do | pkg |
            repo.version_specs(pkg).each do | ver |
                src_uri = repo.version_metadata(pkg, ver).src_uri
                src_uri or next
                src_uri.split(/\s+/).each do | part |
                    part =~ %r~/~ or next
                    parts[part.sub(%r~^.*/~, "")] = true
                end
            end
        end
    end
end

# figure out a list of places where distfiles can be found
distdirs = []
env.package_database.repositories.each do | repo |
    repo.info(false).sections.each do | info_section |
        info_section.kvs.each do | key, value |
            key == "distdir" or next
            distdirs << value unless distdirs.member? value
        end
    end
end

# display each unused distfile
distdirs.each do | dir |
    Dir.new(dir).sort.each do | file |
        File.new(dir + "/" + file).stat.file? or next
        puts dir + "/" + file unless parts[file]
    end
end

