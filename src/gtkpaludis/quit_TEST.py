#!/usr/bin/env python

import os
import dogtail.procedural
import dogtail.utils

os.environ["PALUDIS_HOME"] = os.getcwd() + "/quit_TEST_dir/home"

dogtail.utils.run('./gtkpaludis', appName = 'lt-gtkpaludis')
dogtail.procedural.focus.application('lt-gtkpaludis')
dogtail.procedural.click('Quit')


