/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 David Leverton
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

#include <paludis/libtool_linkage_checker.hh>

#include <paludis/util/realpath.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/log.hh>
#include <paludis/util/options.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/fs_error.hh>

#include <algorithm>
#include <cstring>
#include <cerrno>
#include <functional>
#include <vector>
#include <mutex>

using namespace paludis;

typedef std::vector<std::pair<FSPath, std::string> > Breakage;

namespace paludis
{
    template <>
    struct Imp<LibtoolLinkageChecker>
    {
        FSPath root;

        std::mutex mutex;

        Breakage breakage;

        Imp(const FSPath & the_root) :
            root(the_root)
        {
        }
    };
}

LibtoolLinkageChecker::LibtoolLinkageChecker(const FSPath & root) :
    _imp(root)
{
}

LibtoolLinkageChecker::~LibtoolLinkageChecker() = default;

bool
LibtoolLinkageChecker::check_file(const FSPath & file)
{
    std::string basename(file.basename());
    if (! (3 <= basename.length() &&
           ".la" == basename.substr(basename.length() - 3)))
        return false;

    Context ctx("When checking '" + stringify(file) + "' as a libtool library:");

    SafeIFStream stream(file);

    KeyValueConfigFileOptions opts;
    opts += kvcfo_disallow_space_around_equals;
    opts += kvcfo_disallow_space_inside_unquoted_values;
    opts += kvcfo_ignore_single_quotes_inside_strings;

    std::vector<std::string> deps;

    try
    {
        KeyValueConfigFile kvs(stream, opts,
                &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation);
        tokenise_whitespace(kvs.get("dependency_libs"), std::back_inserter(deps));
    }
    catch (const ConfigFileError & ex)
    {
        Log::get_instance()->message("broken_linkage_finder.failure", ll_warning, lc_context) << ex.message();
        return true;
    }

    const auto is_not_absolute_path = [](const std::string & str) {
        return str.empty() || '/' != str[0];
    };
    deps.erase(std::remove_if(deps.begin(), deps.end(), is_not_absolute_path), deps.end());
    if (deps.empty())
    {
        Log::get_instance()->message("broken_linkage_finder.no_libtool", ll_debug, lc_context)
            << "No libtool library dependencies found";
        return true;
    }

    for (const auto & it : deps)
    {
        try
        {
            FSPath dep(_imp->root / it);
            if (! dereference_with_root(dep, _imp->root).stat().is_regular_file())
            {
                Log::get_instance()->message("broken_linkage_finder.dependency_missing",
                    ll_debug, lc_context) << "Dependency '" << it <<
                    "' is missing or not a regular file in '" << _imp->root << "'";

                std::unique_lock<std::mutex> l(_imp->mutex);
                _imp->breakage.push_back(std::make_pair(file, it));
            }

            else
                Log::get_instance()->message("broken_linkage_finder.dependency_exists", ll_debug, lc_context)
                    << "Dependency '" << it << "' exists in '" << _imp->root << "'";
        }

        catch (const FSError & ex)
        {
            Log::get_instance()->message("broken_linkage_finder.failure", ll_warning, lc_no_context) << ex.message();
        }
    }

    return true;
}

void
LibtoolLinkageChecker::note_symlink(const FSPath &, const FSPath &)
{
}

void
LibtoolLinkageChecker::add_extra_lib_dir(const FSPath &)
{
}

void
LibtoolLinkageChecker::need_breakage_added(
    const std::function<void (const FSPath &, const std::string &)> & callback)
{
    for (const auto & it : _imp->breakage)
        callback(it.first, it.second);
}
