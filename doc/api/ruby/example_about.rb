#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :

=begin description
Simple example showing how to use Paludis version constants.
=end

require 'Paludis'

print "Built using Paludis ", Paludis::VersionMajor, ".", Paludis::VersionMinor,
    ".", Paludis::VersionMicro, Paludis::VersionSuffix

if not Paludis::SubversionRevision.empty?
    print ' ', Paludis::SubversionRevision
end

print "\n"

