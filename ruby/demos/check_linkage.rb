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
    [ '--config-suffix', '-c',  GetoptLong::REQUIRED_ARGUMENT ],
    [ '--pretend',       '-p',  GetoptLong::NO_ARGUMENT ],
    [ '--directory',     '-d',  GetoptLong::REQUIRED_ARGUMENT ] )

config_suffix = ""
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
        puts "  --config-suffix suffix  Set configuration suffix"
        puts
        puts "  --directory             Check the specified directory only"
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

def get_dirs override_directory
    override_directory.empty? or return override_directory

    result = %w{/bin /sbin /usr/bin /usr/sbin /lib* /usr/lib*}

    File.open "/etc/ld.so.conf" do | f |
        f.find_all do | line |
            line =~ /^\//
        end.each do | line |
            result << line.chomp
        end
    end

    result
end

def get_files dir
    %w{/lib/modules}.include? dir and return []

    result = []
    Dir.glob(dir + "/*").each do | entry |
        begin
            stat = File.stat(entry)
            stat.directory? and result.push(*(get_files entry))

            if stat.executable? or entry =~ /\.(so|so\..*|la)$/
                result << entry
            end
        rescue Errno::ENOENT
        end
    end
    result
end

def check_file file
    %x{ldd "#{file}" 2>/dev/null}.split(%r/\n/).map do | line |
        line.sub(/^\s*/, "").sub(/\s*$/, "")
    end.detect do | line |
        line =~ /=> not found/
    end
end

Paludis::DefaultConfig::config_suffix = config_suffix
env = Paludis::DefaultEnvironment.instance

status "Finding candidate search directories"
dirs = get_dirs(override_directory).map { | c | Dir.glob c }.flatten.map { | x | x.squeeze('/') }.sort.uniq

status "Finding libraries and executables"
files = dirs.map { | dir | get_files dir }.flatten.sort.uniq

status "Checking dynamic links"
broken = files.find_all { | file | check_file file }

if broken.empty?
    status "Nothing broken found"
    exit 0
else
    broken.each do | b |
        puts "  * broken: " + b
    end
end

status "Finding owners for broken files"

all_owners = Hash.new
env.package_database.repositories.each do | repo |
    (repo.installed_interface and repo.contents_interface) or next

    repo.category_names.each do | cat |
        repo.package_names(cat).each do | pkg |
            repo.version_specs(pkg).each do | ver |
                repo.contents(pkg, ver).entries.each do | entry |
                    all_owners[entry.name] = Paludis::PackageDatabaseEntry.new(pkg, ver, repo.name)
                end
            end
        end
    end
end

owners = broken.map do | file |
    all_owners[file]
end.find_all { | owner | owner }.sort { | x, y | x.to_s <=> y.to_s }.uniq_using { | x | x.to_s }

owners.each do | owner |
    puts "  * " + owner.to_s
end

status "Finding merge targets"

atoms = owners.map do | owner |
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

