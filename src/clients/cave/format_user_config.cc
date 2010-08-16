/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#include "format_user_config.hh"
#include <paludis/util/config_file.hh>
#include <paludis/util/system.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/options.hh>
#include <paludis/util/stringify.hh>

using namespace paludis;
using namespace cave;

std::string
user_config_file_presets(
        const KeyValueConfigFile &,
        const std::string & s)
{
    if (s == "red")
        return "\033[0;31m";
    else if (s == "bold_red")
        return "\033[1;31m";
    else if (s == "green")
        return "\033[0;32m";
    else if (s == "bold_green")
        return "\033[1;32m";
    else if (s == "yellow")
        return "\033[0;33m";
    else if (s == "bold_yellow")
        return "\033[1;33m";
    else if (s == "blue")
        return "\033[0;34m";
    else if (s == "bold_blue")
        return "\033[1;34m";
    else if (s == "pink")
        return "\033[0;35m";
    else if (s == "bold_pink")
        return "\033[1;35m";
    else if (s == "normal")
        return "\033[0;0m";
    else if (s == "bold_normal")
        return "\033[1m";
    else
        return "";
}

namespace paludis
{
    template <>
    struct Imp<FormatUserConfigFile>
    {
        FSEntry path;
        KeyValueConfigFile conf;

        Imp() :
            path(getenv_with_default("CAVE_FORMATS_CONF", getenv_with_default("HOME", "/") + "/.cave/formats.conf")),
            conf(path.exists() ? ConfigFile::Source(path) : ConfigFile::Source(""),
                    { kvcfo_allow_sections, kvcfo_preserve_whitespace },
                    &user_config_file_presets,
                    &KeyValueConfigFile::no_transformation)
        {
        }
    };
}

FormatUserConfigFile::FormatUserConfigFile() :
    Pimp<FormatUserConfigFile>()
{
}

FormatUserConfigFile::~FormatUserConfigFile() = default;

std::string
FormatUserConfigFile::fetch(const std::string & v, int vi, const std::string & d) const
{
    std::string k(0 == vi ? v : v + "." + stringify(vi));
    std::string result(_imp->conf.get(k));
    if (result.empty())
        return d;
    else
        return result;
}

template class Singleton<cave::FormatUserConfigFile>;

