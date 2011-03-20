/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2011 Ciaran McCreesh
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

#include <paludis/repositories/e/file_suffixes.hh>

#include <paludis/util/pimp-impl.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/system.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/log.hh>
#include <paludis/util/options.hh>
#include <paludis/util/env_var_names.hh>

#include <paludis/name.hh>

using namespace paludis;
using namespace paludis::erepository;

namespace paludis
{
    template <>
    struct Imp<FileSuffixes>
    {
        KeyValueConfigFile file;

        Imp() :
            file(FSPath(getenv_with_default(env_vars::suffixes_file, DATADIR "/paludis/ebuild_entries_suffixes.conf")),
                    { }, &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation)
        {
        }
    };
}

FileSuffixes::FileSuffixes() :
    _imp()
{
}

FileSuffixes::~FileSuffixes() = default;

bool
FileSuffixes::is_known_suffix(const std::string & s) const
{
    return ! _imp->file.get("suffix_" + s + "_known").empty();
}

const std::string
FileSuffixes::guess_eapi_without_hint(const std::string & s) const
{
    return _imp->file.get("guess_eapi_" + s);
}

const std::string
FileSuffixes::guess_eapi_from_filename(const QualifiedPackageName &, const FSPath & e) const
{
    std::string::size_type p(e.basename().rfind('.'));
    if (std::string::npos == p)
        return "";

    std::string suffix(e.basename().substr(p + 1));
    return guess_eapi_without_hint(suffix);
}

const std::string
FileSuffixes::manifest_key(const std::string & s) const
{
    std::string result(_imp->file.get("manifest_key_" + s));
    if (result.empty())
    {
        Log::get_instance()->message("e.ebuild.unknown_manifest_key", ll_warning, lc_context)
            << "Don't know what the manifest key for files with suffix '" << s << "' is, guessing 'MISC'";
        return "MISC";
    }
    else
        return result;
}

bool
FileSuffixes::is_package_file(const QualifiedPackageName & n, const FSPath & e) const
{
    Context context("When working out whether '" + stringify(e) + "' is a package file for '" + stringify(n) + "':");

    if (0 != e.basename().compare(0, stringify(n.package()).length() + 1, stringify(n.package()) + "-"))
        return false;

    std::string::size_type p(e.basename().rfind('.'));
    if (std::string::npos == p)
        return false;

    std::string suffix(e.basename().substr(p + 1));
    return is_known_suffix(suffix);
}

const std::string
FileSuffixes::get_package_file_manifest_key(const FSPath & e, const QualifiedPackageName & q) const
{
    if (! is_package_file(q, e))
        return "";

    std::string::size_type p(e.basename().rfind('.'));
    if (std::string::npos == p)
        return "EBUILD";

    std::string suffix(e.basename().substr(p + 1));
    return manifest_key(suffix);
}

template class Singleton<FileSuffixes>;

