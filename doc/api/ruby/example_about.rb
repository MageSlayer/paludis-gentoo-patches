#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :

require 'Paludis'

print "Built using Paludis ", Paludis::VersionMajor, ".", Paludis::VersionMinor,
    ".", Paludis::VersionMinor, Paludis::VersionSuffix

if not Paludis::SubversionRevision.empty?
    print Paludis::SubversionRevision
end

print "\n"

