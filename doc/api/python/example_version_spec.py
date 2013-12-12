#!/usr/bin/env python
# vim: set fileencoding=utf-8 sw=4 sts=4 et :

"""This example demonstrates how to use VersionSpec"""

import paludis

# Make a list of versions
versions = [paludis.VersionSpec(v) for v in "1.0 1.1 1.2 1.2-r1 2.0 2.0-try1 2.0-scm 9999".split()]

# For each version...
for v in versions:
    print(str(v) + ":")

    # Show the output of various members.
    print("    Remove revision:             %s" % v.remove_revision())
    print("    Revision only:               %s" % v.revision_only())
    print("    Bump:                        %s" % v.bump())
    print("    Is SCM?                      %s" % v.is_scm)
    print("    Has -try?                    %s" % v.has_try_part)
    print("    Has -scm?                    %s" % v.has_scm_part)
