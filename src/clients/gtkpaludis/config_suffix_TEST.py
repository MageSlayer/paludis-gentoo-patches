#!/usr/bin/env python
# vim: set ft=python sw=4 sts=4 et :

import os
import sys
from dogtail.procedural import *
import dogtail.utils

os.environ["PALUDIS_HOME"] = os.getcwd() + "/config_suffix_TEST_dir/home"
os.spawnv(os.P_NOWAIT, "./gtkpaludis", ["gtkpaludis", "-c", "doesnotexist"])
for x in range(10):
    try:
        focus.application('lt-gtkpaludis')
        break
    except FocusError:
        dogtail.utils.doDelay()

focus.application('lt-gtkpaludis')
click('OK')

