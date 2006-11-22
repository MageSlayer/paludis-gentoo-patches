#!/usr/bin/env python

import os
from dogtail.procedural import *
import dogtail.utils

os.environ["PALUDIS_HOME"] = os.getcwd() + "/quit_TEST_dir/home"

dogtail.utils.run('./gtkpaludis', appName = 'lt-gtkpaludis')
focus.application('lt-gtkpaludis')
click('Quit')


