#!/usr/bin/env python

import os
from dogtail.procedural import *
import dogtail.utils

os.spawnv(os.P_NOWAIT, "./gtkpaludis", ["gtkpaludis", "-c", "doesnotexist"])
for x in range(10):
    try:
        focus.application('lt-gtkpaludis')
        break
    except FocusError:
        dogtail.utils.doDelay()

try:
    focus.application('lt-gtkpaludis')
    click('OK')
except:
    True

