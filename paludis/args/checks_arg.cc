/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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

#include "checks_arg.hh"
#include "do_help.hh"

using namespace paludis;
using namespace paludis::args;

ChecksArg::ChecksArg(ArgsGroup * const grp, const std::string & ln, char sh) :
    EnumArg(grp, ln, sh, "Whether to run post-build checks",
            EnumArgOptions
            ("none",              "Don't run checks, even if they should be run")
            ("default",           "Run checks if they should be run by default")
            ("always",            "Always run checks"),
            "default")
{
}

ChecksArg::~ChecksArg()
{
}

InstallActionChecksOption
ChecksArg::option() const
{
    if ("none" == argument())
        return iaco_none;
    if ("default" == argument())
        return iaco_default;
    if ("always" == argument())
        return iaco_always;

    throw DoHelp("Bad value for --" + long_name());
}

