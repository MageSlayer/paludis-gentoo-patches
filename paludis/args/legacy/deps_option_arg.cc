/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2010 Ciaran McCreesh
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

#include "deps_option_arg.hh"
#include <paludis/args/do_help.hh>
#include <paludis/util/stringify.hh>

using namespace paludis;
using namespace paludis::args;

namespace
{
    std::string def_to_string(const DepListDepsOption d)
    {
        if (-1 == d)
            return "none";

        switch (d)
        {
            case dl_deps_discard:
                return "discard";
            case dl_deps_pre:
                return "pre";
            case dl_deps_pre_or_post:
                return "pre-or-post";
            case dl_deps_post:
                return "post";
            case dl_deps_try_post:
                return "try-post";
            case last_dl_deps:
                ;
        };

        throw InternalError(PALUDIS_HERE, "Unexpected DepListDepsOption value");
    }
}

DepsOptionArg::DepsOptionArg(ArgsGroup * const grp, const std::string & ln,
        char sh, const std::string & desc, const DepListDepsOption def) :
    EnumArg(grp, ln, sh, desc,
            EnumArgOptions
            ("pre",                  "As pre dependencies")
            ("pre-or-post",          "As pre dependencies, or post dependencies where needed")
            ("post",                 "As post dependencies")
            ("try-post",             "As post dependencies, with no error for failures")
            ("discard",              "Discard"),
            def_to_string(def))
{
}

DepsOptionArg::~DepsOptionArg()
{
}

DepListDepsOption
DepsOptionArg::option() const
{
    if ("pre" == argument())
        return dl_deps_pre;
    if ("pre-or-post" == argument())
        return dl_deps_pre_or_post;
    if ("post" == argument())
        return dl_deps_post;
    if ("try-post" == argument())
        return dl_deps_try_post;
    if ("discard" == argument())
        return dl_deps_discard;

    throw DoHelp("Bad value for --" + long_name());
}



