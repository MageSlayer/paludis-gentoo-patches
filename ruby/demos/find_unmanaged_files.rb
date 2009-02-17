#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :
#
['Paludis','find','getoptlong'].each {|x| require x}

include Paludis

Log.instance.log_level = LogLevel::Warning
Log.instance.program_name = $0

def get_contents(repo, files, root)
    in_contents= []
    repo.category_names do |cat|
        repo.package_names(cat) do |pkg|
            repo.package_ids(pkg) do |pid|
                next unless pid.supports_action(SupportsActionTest.new(InstalledAction))
                next if pid.contents_key.nil?
                contents = pid.contents_key.value
                contents.each do |entry|
                    next if entry.kind_of? ContentsOtherEntry
                    files.each do |file|
                        if (root + entry.location_key.value)[0,file.length] == file
                            in_contents << root + entry.location_key.value
                            break;
                        end
                    end
                end
            end
        end
    end
    return in_contents
end

opts = GetoptLong.new(
    [ '--help',          '-h',  GetoptLong::NO_ARGUMENT ],
    [ '--version',       '-V',  GetoptLong::NO_ARGUMENT ],
    [ '--log-level',            GetoptLong::REQUIRED_ARGUMENT ],
    [ '--environment',   '-E',  GetoptLong::REQUIRED_ARGUMENT ])

env_spec = ""
opts.each do | opt, arg |
    case opt
    when '--help'
        puts "Usage: " + $0 + " [options] directory1 [directory2 ...]"
        puts
        puts "Options:"
        puts "  --help                  Display a help message"
        puts "  --version               Display program version"
        puts
        puts "  --log-level level       Set log level (debug, qa, warning, silent)"
        puts "  --environment env       Environment specification (class:suffix, both parts optional)"
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

    end
end

env = Paludis::EnvironmentFactory.instance.create env_spec
db = env.package_database
root = env.root[-1] == ?/ ? env.root.chop : env.root

files = []

if ARGV.empty?
    puts "No directory to check"
    exit 1
else
    ARGV.each do |file|
        unless File.directory? file
            puts "#{file} is not a directory."
            exit 1
        end
        unless file == root or file[0,root.length + 1] == root + "/"
            puts "#{file} is not under ${ROOT} (#{root}/)"
            exit 1
        end
        files << (file[-1] == ?/ ? file.chop : file)
    end
end

in_fs = []
Find.find(*files) {|file| in_fs << file}

db.repositories do |repo|
    next unless repo.some_ids_might_support_action(SupportsActionTest.new(InstalledAction))
    in_fs-= get_contents(repo, files, root)
end

puts in_fs
