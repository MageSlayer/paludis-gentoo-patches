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
#include <paludis/util/mutex.hh>
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

using namespace paludis;

typedef std::vector<std::pair<FSPath, std::string> > Breakage;

namespace paludis
{
    template <>
    struct Imp<LibtoolLinkageChecker>
    {
        FSPath root;

        Mutex mutex;

        Breakage breakage;

        Imp(const FSPath & the_root) :
            root(the_root)
        {
        }
    };
}

namespace
{
    struct IsNotAbsolutePath : std::unary_function<std::string, bool>
    {
        bool operator() (const std::string & str)
        {
            return str.empty() || '/' != str[0];
        }
    };
}

LibtoolLinkageChecker::LibtoolLinkageChecker(const FSPath & root) :
    _imp(root)
{
}

LibtoolLinkageChecker::~LibtoolLinkageChecker()
{
}

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

    std::vector<std::string> deps;

    try
    {
        KeyValueConfigFile kvs(stream, opts,
                &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation);
        tokenise_whitespace(kvs.get("dependency_libs"), std::back_inserter(deps));
    }
    catch (const ConfigFileError & ex)
    {
        Log::get_instance()->message("reconcilio.broken_linkage_finder.failure", ll_warning, lc_context) << ex.message();
        return true;
    }

    deps.erase(std::remove_if(deps.begin(), deps.end(), IsNotAbsolutePath()), deps.end());
    if (deps.empty())
    {
        Log::get_instance()->message("reconcilio.broken_linkage_finder.no_libtool", ll_debug, lc_context)
            << "No libtool library dependencies found";
        return true;
    }

    for (std::vector<std::string>::const_iterator it(deps.begin()),
             it_end(deps.end()); it_end != it; ++it)
    {
        try
        {
            FSPath dep(_imp->root / *it);
            if (! dereference_with_root(dep, _imp->root).stat().is_regular_file())
            {
                Log::get_instance()->message("reconcilio.broken_linkage_finder.dependency_missing",
                    ll_debug, lc_context) << "Dependency '" << *it <<
                    "' is missing or not a regular file in '" << _imp->root << "'";

                Lock l(_imp->mutex);
                _imp->breakage.push_back(std::make_pair(file, *it));
            }

            else
                Log::get_instance()->message("reconcilio.broken_linkage_finder.dependency_exists", ll_debug, lc_context)
                    << "Dependency '" << *it << "' exists in '" << _imp->root << "'";
        }

        catch (const FSError & ex)
        {
            Log::get_instance()->message("reconcilio.broken_linkage_finder.failure", ll_warning, lc_no_context) << ex.message();
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
    for (Breakage::const_iterator it(_imp->breakage.begin()), it_end(_imp->breakage.end()); it_end != it; ++it)
        callback(it->first, it->second);
}

