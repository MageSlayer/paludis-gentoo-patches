#!/usr/bin/ruby
# vim: set sw=4 sts=4 et tw=80 :

require 'Paludis'

packages = Paludis::EnvironmentMaker.instance.make_from_spec('').package_database.query(
    "app-editors/vim", Paludis::InstallState::InstalledOnly, Paludis::QueryOrder::OrderByVersion)

if packages.empty?
    puts "Vim is not installed"
else
    puts "Vim " + packages.last.version.to_s + " is installed"
end

