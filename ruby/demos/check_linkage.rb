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
    [ '--environment',   '-E',  GetoptLong::REQUIRED_ARGUMENT ],
    [ '--pretend',       '-p',  GetoptLong::NO_ARGUMENT ] )

env_spec = ""
override_directory = []
pretend = false
opts.each do | opt, arg |
    case opt
    when '--help'
        puts "Usage: " + $0 + " [options]"
        puts
        puts "Options:"
        puts "  --help                  Display a help message"
        puts "  --version               Display program version"
        puts
        puts "  --pretend               Stop before reinstalling"
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

    when '--directory'
        override_directory << arg

    when '--pretend'
        pretend = true

    end
end

module Enumerable
    def uniq_using
        first = true
        result = []
        old_x_t = nil
        each do | x |
            x_t = yield x
            if first or x_t != old_x_t
                old_x_t = x_t
                result << x
                first = false
            end
        end
        result
    end
end

def status x
    $stderr << "\n" << x << "\n"
end

def executable x
    begin
        s = File.lstat x
        s.executable?
    rescue
        false
    end
end

def check_file file
    %x{ldd "#{file}" 2>/dev/null}.split(%r/\n/).map do | line |
        line.sub(/^\s*/, "").sub(/\s*$/, "")
    end.detect do | line |
        line =~ /=> not found/
    end
end

env = Paludis::EnvironmentMaker.instance.make_from_spec env_spec

status "Checking linkage for package-manager installed files"

broken = [ ]
env.package_database.repositories.each do | repo |
    (repo.installed_interface and repo.contents_interface) or next

    repo.category_names.each do | cat |
        repo.package_names(cat).each do | pkg |
            repo.version_specs(pkg).each do | ver |
                repo.contents(pkg, ver).entries.each do | entry |
                    entry.kind_of? Paludis::ContentsFileEntry or next
                    (entry.name =~ /\.(la|so|so\..*)$/ or executable(entry.name)) or next
                    check_file entry.name or next
                    broken << Paludis::PackageDatabaseEntry.new(pkg, ver, repo.name)
                    break
                end
            end
        end
    end
end

broken = broken.find_all { | x | x }.sort { | x, y | x.to_s <=> y.to_s }.uniq_using { | x | x.to_s }

if broken.empty?
    status "No broken packages found"
    exit 0
end

broken.each do | x |
    puts "  * " + x.to_s
end

status "Finding merge targets"

atoms = broken.map do | owner |
    slot = env.package_database.fetch_repository(owner.repository).version_metadata(owner.name, owner.version).slot
    Paludis::PackageDepAtom.new("=" + owner.name + "-" + owner.version.to_s + ":" + slot)
end

if atoms.empty?
    status "Couldn't find any owners"
    exit 0
end

status "Building dependency list"

system("paludis --pretend --install #{atoms.join ' '} --dl-upgrade as-needed --preserve-world") or exit 1

exit 0 if pretend

status "Rebuilding broken packages"

system("paludis --install #{atoms.join ' '} --dl-upgrade as-needed --preserve-world")

