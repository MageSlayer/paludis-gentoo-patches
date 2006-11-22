#!/usr/bin/env python
# vim: set ft=python sw=4 sts=4 et :

import os, sys, dogtail.utils
from dogtail import tree, predicate
from dogtail.procedural import *

os.environ["PALUDIS_HOME"] = os.getcwd() + "/sets_list_TEST_dir/home"

dogtail.utils.run('./gtkpaludis', appName = 'lt-gtkpaludis')
focus.application('lt-gtkpaludis')

focus.widget(roleName = 'page tab', name = 'Sets')
focus.widget.node.select()

sets_list = tree.root.findChild(predicate.IsNamed("SetsList"))

monkey_cell = sets_list.findChild(predicate.IsNamed("monkey"), requireResult = False, retry = False)
if monkey_cell:
    raise "there should be no monkey"

system_cell = sets_list.findChild(predicate.IsNamed("system"))
system_cell.select()

for x in range(10):
    dogtail.utils.doDelay()

click('Quit')



