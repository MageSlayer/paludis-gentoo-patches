#!/usr/bin/env python
# vim: set fileencoding=utf-8 sw=4 sts=4 et :

"""Basic command line handling for most examples"""

import optparse
import paludis

class ExampleCommandLine:
    def __init__(self):
        parser = optparse.OptionParser()
        parser.add_option("", "--log-level", type = "choice",
                choices = ["debug", "qa", "warning", "silence"],
                action = "store", dest = "log_level", help = "Specify the log level")
        parser.add_option("-E", "--environment", action = "store", dest = "environment", default = "",
                help = "Environment specification")

        (options, args) = parser.parse_args()

        print options.log_level
        if options.log_level:
            paludis.Log.instance.log_level = getattr(paludis.LogLevel, options.log_level.upper())

if __name__ == "__main__":
    ExampleCommandLine()
