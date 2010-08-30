#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 ft=ruby :
# $Id$

# Copyright (c) 2007 Mike Kelly
# Copyright (c) 2007, 2008 David Leverton
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

version = "0.1.6"
$laymanglobal_url = URI.parse('http://www.gentoo.org/proj/en/overlays/repositories.xml')
$proxy_url = URI.parse(ENV['http_proxy']) if ENV['http_proxy']

opts = GetoptLong.new(
    [ '--help',          '-h', GetoptLong::NO_ARGUMENT ],
    [ '--version',       '-V', GetoptLong::NO_ARGUMENT ],
    [ '--log-level',           GetoptLong::REQUIRED_ARGUMENT ],
    [ '--environment',   '-E', GetoptLong::REQUIRED_ARGUMENT ],
    [ '--config-suffix', '-c', GetoptLong::REQUIRED_ARGUMENT ],
    [ '--list',          '-l', GetoptLong::NO_ARGUMENT ],
    [ '--add',           '-a', GetoptLong::NO_ARGUMENT ],
    [ '--no-names-cache',      GetoptLong::NO_ARGUMENT ],
    [ '--no-write-cache',      GetoptLong::NO_ARGUMENT ],
    [ '--layman-url',          GetoptLong::REQUIRED_ARGUMENT],
    [ '--http-proxy',          GetoptLong::REQUIRED_ARGUMENT])

$envspec = ""
$mode = ""
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
  --log-level            Set log level (debug, qa, warning, silent)
  --environment, -E      Environment specification (class:suffix, both parts
                         optional, class must be 'paludis' if specified)
  --config-suffix, -c    Set configuration suffix (deprecated, use --environment)

  --list, -l             List available overlays.
  --add, -a              Add the given overlays.

  --no-names-cache       Disable the names cache for the added repos.
  --no-write-cache       Disable the write cache for the added repos.

  --http-proxy           The url of the http proxy to use.

  --layman-url           The url to receive the global layman list from
                         (defaults to the Gentoo one).

Manages paludis configuration for layman overlays. Can add new overlays, and
list the currently available overlays from the global layman list.

If playman.tmpl exists in the Paludis repositories configuration directory, it
will be used as a template, with the strings @NAME@ and @SYNC@ replaced with
the name and sync URL of the overlay, respectively.  Otherwise, a builtin
default template will be used.
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
        $envspec = arg
    when '--config-suffix'
        $envspec = "paludis:#{arg}"

    when '--list'
        $mode="list"
    when '--add'
        $mode="add"

    when '--no-names-cache'
        $names_cache = false
    when '--no-write-cache'
        $write_cache = false

    when '--http-proxy'
        $proxy_url = URI.parse(arg)

    when '--layman-url'
        $laymanglobal_url = URI.parse(arg)
    end
end

$env = EnvironmentFactory.instance.create($envspec)
if $env.format_key.value != "paludis" then
    $stderr.puts "#$0: --environment must specify class 'paludis'"
    exit 1
end
$config_dir = $env.config_location_key.value

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
    req = Net::HTTP::Get.new($laymanglobal_url.request_uri)
    proxy_host = $proxy_url.host if $proxy_url and $proxy_url.host
    proxy_port = $proxy_url.port if $proxy_url and $proxy_url.port
    proxy_user, proxy_pass = $proxy_url.userinfo.split(/:/) if $proxy_url and $proxy_url.userinfo
    res = Net::HTTP::Proxy(proxy_host,proxy_port,
                           proxy_user,proxy_pass).start($laymanglobal_url.host,
                           $laymanglobal_url.port) {|http|
        http.request(req)
    }
    laymanxml = REXML::Document.new res.body
rescue Exception => e
    puts "Couldn't open repositories.xml."
    puts e.message
    puts e.backtrace.inspect
    exit 1
end

def munge_url(type, src, subpath = "")
    case type

    when 'bzr'
        case src
        when %r{^bzr(?:\+ssh)?://}
            return src
        when %r{^[a-z+]+://}
            return "bzr+#{src}"
        when %r{^lp:(.*)}
            return "bzr+lp://#$1"
        else
            return "bzr+file://#{src}"
        end

    when 'cvs'
        case src
        when /^:ext:(.*)$/
            return "cvs+ext://#$1:#{subpath}"
        when /^:pserver:(.*)$/
            return "cvs+pserver://#$1:#{subpath}"
        end

    when 'darcs'
        case src
        when %r{^[a-z+]+://}
            return "darcs+#{src}"
        when /..:/
            return "darcs+ssh://#{src}"
        else
            return "darcs+file://#{src}"
        end

    when 'git'
        case src
        when %r{^git(?:\+ssh)?://}
            return src
        when %r{^[a-z+]+://}
            return "git+#{src}"
        else
            return "git+file://#{src}"
        end

    when 'mercurial'
        return "hg+#{src}"

    when 'rsync'
        case src
        when %r{^rsync://}
            return src
        when /..:/
            return "rsync+ssh://#{src}"
        else
            return "file://#{src}"
        end

    when 'svn'
        case src
        when %r{^svn(?:\+ssh)?://}
            return src
        when %r{^[a-z+]+://}
            return "svn+#{src}"
        else
            return "svn+file://#{src}"
        end

    when 'tar'
        return "tar+#{src}" if subpath == ""

    end

    return src
end

layman_srcs = Hash.new
laymanxml.elements.each("repositories/repo") do | element |
    name    = element.elements["name"].text
    sources = []
    element.elements.each("source") do | source |
        src     = source.text
        type    = source.attribute("type").to_s
        subpath = ""
        sources << munge_url(type, src, subpath)
    end
    layman_srcs[name]=sources.join(" ");
end
laymanxml.elements.each("layman/overlay") do | element |
    name    = element.attribute("name").to_s
    src     = element.attribute("src").to_s
    type    = element.attribute("type").to_s
    subpath = element.attribute("subpath").to_s
    layman_srcs[name]=munge_url(type, src, subpath)
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
            template = $config_dir + "/repositories/playman.tmpl"
            if File.exist?(template) then
                substs = {
                    'NAME' => overlay_name,
                    'SYNC' => layman_srcs[overlay_name],
                }

                IO.foreach(template) do |line|
                    line.gsub!(/@([A-Z]+)@/) do
                        if substs.has_key?($1) then
                            substs[$1]
                        else
                            Log.instance.message("playman.template.unknown_var", LogLevel::Warning, "Unknown variable #$1 in #{template}")
                            ""
                        end
                    end
                    f.puts line
                end

            else
                f.puts "# Config generated by: " + $0 + ", version " + version

                f.puts "location = /var/paludis/repositories/#{overlay_name}"
                f.puts "format = e"
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
        end

        puts
        puts "You should now run:"
        print "  paludis"
        print " -E #{$envspec}" unless $envspec.empty?
        print " -s x-#{overlay_name} \n"
    end
end
