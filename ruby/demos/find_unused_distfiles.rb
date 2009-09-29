#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :

require 'Paludis'
require 'getoptlong'

Paludis::Log.instance.log_level = Paludis::LogLevel::Warning
Paludis::Log.instance.program_name = $0

opts = GetoptLong.new(
    [ '--help',          '-h',   GetoptLong::NO_ARGUMENT ],
    [ '--version',       '-V',   GetoptLong::NO_ARGUMENT ],
    [ '--log-level',             GetoptLong::REQUIRED_ARGUMENT ],
    [ '--environment',   '-E',   GetoptLong::REQUIRED_ARGUMENT ],
    [ '--size-limit',    '-s',   GetoptLong::REQUIRED_ARGUMENT ],
    [ '--time-limit',    '-t',   GetoptLong::REQUIRED_ARGUMENT ],
    [ '--mirror-repository',     GetoptLong::REQUIRED_ARGUMENT ],
    [ '--mirror-distdir',        GetoptLong::REQUIRED_ARGUMENT ],
    [ '--write-cache-dir',       GetoptLong::REQUIRED_ARGUMENT ],
    [ '--extra-repository-dir',  GetoptLong::REQUIRED_ARGUMENT ],
    [ '--master-repository-dir', GetoptLong::REQUIRED_ARGUMENT ])

env_spec                                  = nil
size_limit        = time_limit            = nil
mirror_repository = mirror_distdir        = nil
write_cache_dir                           = "/var/empty"
master_repository_name                    = ''
extra_repository_dirs                     = []

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
        puts
        puts "  --mirror-repository     In mirror mode, the location of the ebuild repository"
        puts "  --mirror-distdir        In mirror mode, the location of the downloaded files"
        puts "  --write-cache-dir       Use a subdirectory named for the repository name under the specified directory for repository write cache"
        puts "  --extra-repository-dir  Also include the repository at this location. May be specified multiple times, in creation order."
        puts "  --master-repository     The name of the master repository. Specify the location using --extra-repository-dir."
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

    when '--environment'
        env_spec = arg

    when '--size-limit'
        if arg =~ /^[0-9]+[gmkb]$/i then
            size_limit = arg[0..-2].to_i *
                case arg.downcase[-1]
                when ?b then 1
                when ?k then 1024
                when ?m then 1024 * 1024
                when ?g then 1024 * 1024 * 1024
                end
        else
            puts "Bad --size-limit value " + arg
            exit 1
        end

    when '--time-limit'
        if arg =~ /^[0-9]+[ymwdh]/ then
            time_limit = Time.now - arg[0..-2].to_i *
                case arg.downcase[-1]
                when ?h then 60 * 60
                when ?d then 60 * 60 * 24
                when ?w then 60 * 60 * 24 * 7
                when ?m then 60 * 60 * 24 * 30
                when ?y then 60 * 60 * 24 * 365
                end
        else
            puts "Bad --time-limit value " + arg
            exit 1
        end

    when '--mirror-repository'
        mirror_repository = arg
    when '--mirror-distdir'
        mirror_distdir = arg

    when '--write-cache-dir'
        write_cache_dir = arg
    when '--extra-repository-dir'
        extra_repository_dirs << arg
    when '--master-repository'
        master_repository_name = arg

    end
end

if mirror_repository.nil? ^ mirror_distdir.nil? then
    puts "Must specify neither or both of --mirror-repository and --mirror-distdir"
    exit 1
end
if mirror_repository && env_spec then
    puts "Can't use --environment in mirror mode"
    exit 1
end

if mirror_repository then
    env = Paludis::NoConfigEnvironment.new(mirror_repository, write_cache_dir, master_repository_name, extra_repository_dirs)
    relevant_packages = Paludis::Generator::InRepository.new(env.main_repository.name)
    $check_condition = lambda { true }
    $banned_labels = {
        Paludis::URIListedOnlyLabel       => true,
        Paludis::URILocalMirrorsOnlyLabel => true,
        Paludis::URIManualOnlyLabel       => true,
    }
else
    env = Paludis::EnvironmentFactory.instance.create(env_spec || "")
    relevant_packages = Paludis::Generator::All.new | Paludis::Filter::InstalledAtRoot.new("/")
    $check_condition = lambda { | spec | spec.condition_met? }
    $banned_labels = { }
end

def collect_filenames(parts, id, label, spec)
    case spec

    when Paludis::AllDepSpec
        new_label = [label[0]]
        spec.each do | item |
            collect_filenames(parts, id, new_label, item)
        end

    when Paludis::ConditionalDepSpec
        if $check_condition[spec] then
            new_label = label.dup
            spec.each do | item |
                collect_filenames(parts, id, new_label, item)
            end
        end

    when Paludis::FetchableURIDepSpec
        parts[spec.filename] = true unless $banned_labels[label[0]]

    when Paludis::URILabelsDepSpec
        label[0] = spec.labels.last.class

    else
        raise "Unexpected DepSpec class #{spec.class} in #{id}"
    end
end

# build up a list of all src_uri things that're used by installed packages
parts = Hash.new
env[Paludis::Selection::AllVersionsUnsorted.new(relevant_packages)].each do | id |
    key = id.fetches_key
    collect_filenames(parts, id, [key.initial_label.class], key.value) if key && key.value
end

# figure out a list of places where distfiles can be found
distdirs = []
if mirror_distdir then
    distdirs << mirror_distdir
else
    env.package_database.repositories.each do | repo |
        key = repo['distdir'] or next
        distdirs << key.value unless distdirs.member? key.value
    end
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

