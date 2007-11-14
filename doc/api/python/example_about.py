#!/usr/bin/env python
# vim: set fileencoding=utf-8 sw=4 sts=4 et :

"""A simple example showing how to use Paludis version constants"""

import paludis

print "Built using Paludis " + paludis.VERSION + paludis.VERSION_SUFFIX,

if paludis.SUBVERSION_REVISION:
    print 'r' + paludis.SUBVERSION_REVISION
