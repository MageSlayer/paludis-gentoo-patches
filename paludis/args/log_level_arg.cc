/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2010 Ciaran McCreesh
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "log_level_arg.hh"
#include "do_help.hh"

using namespace paludis;
using namespace paludis::args;

LogLevelArg::LogLevelArg(ArgsGroup * const grp, const std::string & ln,
        char sh) :
    EnumArg(grp, ln, sh, "Specify the log level",
            EnumArgOptions
            ("debug",   'd', "Show debug output (noisy)")
            ("qa",      'q', "Show QA messages and warnings only")
            ("warning", 'w', "Show warnings only")
            ("silent",  's', "Suppress all log messages (UNSAFE)"),
            "qa")
{
}

LogLevelArg::~LogLevelArg()
{
}

LogLevel
LogLevelArg::option() const
{
    if ("debug" == argument())
        return ll_debug;
    if ("qa" == argument())
        return ll_qa;
    if ("warning" == argument())
        return ll_warning;
    if ("silent" == argument())
        return ll_silent;

    throw DoHelp("Bad value for --" + long_name());
}


