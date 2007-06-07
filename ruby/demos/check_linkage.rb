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
    [ '--pretend',       '-p',  GetoptLong::NO_ARGUMENT ],
    [ '--verbose',       '-v',  GetoptLong::NO_ARGUMENT ] )

env_spec = ""
override_directory = []
pretend = verbose = false
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
        puts "  --verbose               Print the name of each broken binary"
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

    when '--verbose'
        verbose = true

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

def read_shell_vars filename
    vars = {}
    IO.foreach(filename) do | line |
        line.chomp!
        line    =~ /^\s*(?:export\s+)?(\w+)=(["']?)((?:[^\\]|\\.)*?)\2$/ or next
        var     = $1
        content = $3.gsub(/\\(.)/, '\1')
        content.gsub!(/\$(?:\{([^}]+)\}|(\w+))/) { | var | vars[var] }
        vars[var] = content
    end
    vars
end

def eligible? filename
    dirname = filename
    while dirname != "/"
        dirname = File.dirname(dirname)
        return false if @search_dirs_mask[dirname]
    end

    dirname = filename
    while dirname != "/"
        dirname = File.dirname(dirname)
        return true if @search_dirs[dirname]
    end

    false
end

def check_file file
    %x{ldd "#{file}" 2>/dev/null}.split(%r/\n/).map do | line |
        line.sub(/^\s*/, "").sub(/\s*$/, "")
    end.detect do | line |
        line =~ /(\S+) => not found/ and not @ld_library_mask[$1]
    end
end

# configuration var logic from revdep-rebuild, gentoolkit-0.2.3
prelim_ld_library_mask  = (ENV["LD_LIBRARY_MASK"]  || "").split
prelim_search_dirs      = (ENV["SEARCH_DIRS"]      || "").split
prelim_search_dirs_mask = (ENV["SEARCH_DIRS_MASK"] || "").split

# XXX make.conf / paludis/bashrc ?

if File.stat("/etc/revdep-rebuild").directory? then
    Dir["/etc/revdep-rebuild/[^.#]*[^~]"].sort.each do | filename |
        vars = read_shell_vars filename
        prelim_ld_library_mask  += (vars["LD_LIBRARY_MASK"]  || "").split
        prelim_search_dirs      += (vars["SEARCH_DIRS"]      || "").split
        prelim_search_dirs_mask += (vars["SEARCH_DIRS_MASK"] || "").split
    end
else
    prelim_ld_library_mask  += %w(libodbcinst.so libodbc.so libjava.so libjvm.so)
    prelim_search_dirs      += %w(/bin /sbin /usr/bin /usr/sbin /lib* /usr/lib*)
    prelim_search_dirs_mask += %w(/opt/OpenOffice /usr/lib/openoffice)
end

vars = read_shell_vars "/etc/profile.env"
prelim_search_dirs += (vars["PATH"]     || "").split(/:/)
prelim_search_dirs += (vars["ROOTPATH"] || "").split(/:/)

ld_so_conf = []
IO.foreach("/etc/ld.so.conf") do | line |
    line.chomp!
    line =~ /^[^\s#]/ and ld_so_conf << line
end
prelim_search_dirs += ld_so_conf

@ld_library_mask = {}
prelim_ld_library_mask.each do | mask |
    mask == "-*" and break
    @ld_library_mask[mask] = true
end

@search_dirs = {}
prelim_search_dirs.each do | dir |
    dir == "-*" and break
    Dir[dir].each do | expanded |
        @search_dirs[expanded.sub(/\/+$/, "").gsub(/\/\//, "/")] = true
    end
end

@search_dirs_mask = {}
prelim_search_dirs_mask.each do | dir |
    dir == "-*" and break
    Dir[dir].each do | expanded |
        @search_dirs_mask[expanded.sub(/\/+$/, "").gsub(/\/\//, "/")] = true
    end
end

env = Paludis::EnvironmentMaker.instance.make_from_spec env_spec

status "Checking linkage for package-manager installed files"


files = { }
directories = { }
env.package_database.repositories.each do | repo |
    (repo.installed_interface and repo.contents_interface) or next

    repo.category_names.each do | cat |
        repo.package_names(cat).each do | pkg |
            repo.version_specs(pkg).each do | ver |
                package = Paludis::PackageDatabaseEntry.new(pkg, ver, repo.name)
                repo.contents(pkg, ver).entries.each do | entry |
                    entry.kind_of? Paludis::ContentsFileEntry or next
                    eligible?(entry.name) or next
                    (entry.name =~ /\.(la|so|so\..*)$/ or executable(entry.name)) or next
                    files[package] ||= []
                    files[package] << entry.name
                    directories[File.dirname(entry.name)] = true
                end
            end
        end
    end
end

ENV["LD_LIBRARY_PATH"] = (Dir["/lib*"] + Dir["/usr/lib*"] + ld_so_conf + directories.keys).join(":")

broken = [ ]
files.each_pair do | package, files |
    files.each do | filename |
        check_file filename or next
        puts "  * #{filename} is broken" if verbose
        broken << package
        break
    end
end

broken = broken.find_all { | x | x }.sort { | x, y | x.to_s <=> y.to_s }.uniq_using { | x | x.to_s }

if broken.empty?
    status "No broken packages found"
    exit 0
end

puts if verbose
broken.each do | x |
    puts "  * " + x.to_s
end

status "Finding merge targets"

specs = broken.map do | owner |
    slot = env.package_database.fetch_repository(owner.repository).version_metadata(owner.name, owner.version).slot
    Paludis::PackageDepSpec.new("=" + owner.name + "-" + owner.version.to_s + ":" + slot, Paludis::PackageDepSpecParseMode::Permissive)
end

if specs.empty?
    status "Couldn't find any owners"
    exit 0
end

status "Building dependency list"

system("paludis --pretend --install #{specs.join ' '} --dl-upgrade as-needed --preserve-world") or exit 1

exit 0 if pretend

status "Rebuilding broken packages"

system("paludis --install #{specs.join ' '} --dl-upgrade as-needed --preserve-world")

