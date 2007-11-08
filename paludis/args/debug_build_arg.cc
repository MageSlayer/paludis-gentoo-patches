/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh
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

#include "debug_build_arg.hh"
#include "do_help.hh"

using namespace paludis;
using namespace paludis::args;

DebugBuildArg::DebugBuildArg(ArgsGroup * const grp, const std::string & ln,
        char sh) :
    EnumArg(grp, ln, sh, "What to do with debug information",
            EnumArgOptions
            ("none",              "Discard debug information")
            ("split",             "Split debug information")
            ("internal",          "Keep debug information"),
            "split")
{
}

DebugBuildArg::~DebugBuildArg()
{
}

InstallActionDebugOption
DebugBuildArg::option() const
{
    if ("none" == argument())
        return iado_none;
    if ("split" == argument())
        return iado_split;
    if ("internal" == argument())
        return iado_internal;

    throw DoHelp("Bad value for --" + long_name());
}

