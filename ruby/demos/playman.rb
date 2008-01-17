#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 ft=ruby :
# $Id$

# Copyright (c) 2007 Mike Kelly <pioto@gentoo.org>
#
# This file is part of the Paludis package manager. Paludis is free software;
# you can redistribute it and/or modify it under the terms of the GNU General
# Public License, version 2, as published by the Free Software Foundation.
#
# Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place, Suite 330, Boston, MA  02111-1307  USA

require 'Paludis'

require 'getoptlong'

require 'rexml/document'
require 'uri'
require 'net/http'

include Paludis

Log.instance.log_level = LogLevel::Warning
Log.instance.program_name = $0

version = "0.1.4"
laymanglobal_url = URI.parse('http://www.gentoo.org/proj/en/overlays/layman-global.txt')

opts = GetoptLong.new(
    [ '--help',          '-h', GetoptLong::NO_ARGUMENT ],
    [ '--version',       '-V', GetoptLong::NO_ARGUMENT ],
    [ '--log-level',           GetoptLong::REQUIRED_ARGUMENT ],
    [ '--environment',   '-E', GetoptLong::REQUIRED_ARGUMENT ],
    [ '--config-suffix', '-c', GetoptLong::REQUIRED_ARGUMENT ],
    [ '--list',          '-l', GetoptLong::NO_ARGUMENT ],
    [ '--add',           '-a', GetoptLong::NO_ARGUMENT ],
    [ '--no-names-cache',      GetoptLong::NO_ARGUMENT ],
    [ '--no-write-cache',      GetoptLong::NO_ARGUMENT ])

$mode = ""
$config_suffix = ""
$environment = ""
$names_cache = true
$write_cache = true

opts.each do | opt, arg |
    case opt
    when '--help'
        puts <<HELP
Usage: #{$0} [options] [overlay1] [overlay2] [...]

Options:
  --help, -h             Display a help message
  --version, -V          Display program version
  --log-level            Set log level (debug, qa, warning, slient)
  --environment, -E      Environment specification (class:suffix, both parts
                         optional, class must be 'paludis' if specified)
  --config-suffix, -c    Set configuration suffix (deprecated, use --environment)

  --list, -l             List available overlays.
  --add, -a              Add the given overlays.

  --no-names-cache       Disable the names cache for the added repos.
  --no-write-cache       Disable the write cache for the added repos.

Manages paludis configuration for layman overlays. Can add new overlays, and
list the currently available overlays from the global layman list.
HELP
        exit 0
    when '--version'
        puts $0.to_s.split(/\//).last + " " + version + " (Paludis Version: " + Version + ")"
        exit 0

    when '--log-level'
        case arg
        when 'debug'
            Log.instance.log_level = LogLevel::Debug
        when 'qa'
            Log.instance.log_level = LogLevel::Qa
        when 'warning'
            Log.instance.log_level = LogLevel::Warning
        when 'silent'
            Log.instance.log_level = LogLevel::Silent
        else
            puts "Bad --log-level value " + arg
            exit 1
        end
    when '--environment'
        $environment = arg
        if Paludis.const_defined?(:EnvironmentMaker) then
            $envspec = arg
        else
            $stderr.puts "#$0: --environment needs >= Paludis 0.21"
            exit 1
        end
    when '--config-suffix'
        $config_suffix = arg
        if Paludis.const_defined?(:EnvironmentMaker) then
            $envspec = ":#{arg}"
        else
            Paludis::DefaultConfig.config_suffix = arg
        end

    when '--list'
        $mode="list"
    when '--add'
        $mode="add"

    when '--no-names-cache'
        $names_cache = false
    when '--no-write-cache'
        $write_cache = false
    end
end

if Paludis.const_defined?(:EnvironmentMaker) then
    if ($envspec || "") =~ /^(?:paludis)?(?::(.*))?$/ then
        $env = Paludis::PaludisEnvironment.new($1 || "")
        $config_dir = $env.config_dir
    else
        $stderr.puts "#$0: --environment must specifiy class 'paludis'"
        exit 1
    end
else
    $env = DefaultEnvironment.instance
    $config_dir = Paludis::DefaultConfig.instance.config_dir
end

if $mode.empty?
    $stderr.puts "You must choose a mode of operation."
    exit 1
end
if ARGV.empty?
    if $mode != "list"
        puts "No repository names supplied"
        exit 1
    end
end

repositories = $env.package_database.repositories

begin
    req = Net::HTTP::Get.new(laymanglobal_url.path)
    res = Net::HTTP.start(laymanglobal_url.host, laymanglobal_url.port) {|http|
        http.request(req)
    }
    laymanxml = REXML::Document.new res.body
rescue Exception => e
    puts "Couldn't open layman-global.txt."
    puts e.message
    puts e.backtrace.inspect
    exit 1
end

layman_srcs = Hash.new
laymanxml.elements.each("layman/overlay") do | element |
    name = element.attribute("name").to_s
    src = element.attribute("src").to_s
    type = element.attribute("type").to_s
    case type
    when 'svn'
        if src.include? "http://" or src.include? "https://"
            src = "svn+" + src
        end
    when 'darcs'
        src = "darcs+" + src
    when 'git'
        if src.include? "http://"
            src = "git+" + src
        end
    when 'tar'
        src = "tar+" + src
    end
    layman_srcs[name]=src
end

case $mode
when 'list'
    pad = layman_srcs.keys.collect { | key | key.length }.max
    layman_srcs.sort.each { | key, value | printf "%*s | %s\n", pad, key, value }
when 'add'
    ARGV.each do | overlay_name |
        unless layman_srcs.key? overlay_name
            puts "We don't know anything about #{overlay_name}"
            exit 1
        end
 
        filename = $config_dir + "/repositories/#{overlay_name}.conf"
        File.open(filename,'w') do |f|
            f.puts "# Config generated by: " + $0 + ", version " + version

            f.puts "location = /var/paludis/repositories/#{overlay_name}"
            f.puts "format = ebuild"
            f.puts "sync = #{layman_srcs[overlay_name]}"
            f.puts
            f.puts "master_repository = gentoo"

            if $names_cache
                f.puts "names_cache = ${location}/.cache/names"
            else
                f.puts "names_cache = /var/empty"
            end
            if $write_cache
                f.puts "write_cache = /var/cache/paludis/metadata"
            else
                f.puts "write_cache = /var/empty"
            end
        end
        puts
        puts "You should now run:"
        print "  paludis"
        print " -c #{$config_suffix}" unless $config_suffix.empty?
        print " -E #{$environment}" unless $environment.empty?
        print " -s x-#{overlay_name} \n"
    end
end
