#!/usr/bin/env python
# vim: set fileencoding=utf-8 sw=4 sts=4 et :

"""A simple example showing how to use Paludis version constants"""

import paludis

print("Built using Paludis " + str(paludis.VERSION_MAJOR) + "." + str(paludis.VERSION_MINOR) +
        "." + str(paludis.VERSION_MICRO) + paludis.VERSION_SUFFIX),

if paludis.GIT_HEAD:
    print 'git-' + paludis.GIT_HEAD
