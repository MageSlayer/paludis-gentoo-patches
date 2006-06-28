/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include "merge_common.hh"

#include <paludis/util/dir_iterator.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/log.hh>
#include <paludis/util/pstream.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/system.hh>
#include <paludis/util/tokeniser.hh>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include <cstdlib>
#include <errno.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <sys/stat.h>
#include <sys/types.h>

using namespace paludis;

std::vector<std::string>
merge::get_config_var(const std::string & var)
{
    std::vector<std::string> result;
    WhitespaceTokeniser::get_instance()->tokenise(getenv_with_default(var, ""),
            std::back_inserter(result));
    return result;
}

bool
merge::is_config_protected(const FSEntry & root, const FSEntry & file)
{
    static std::vector<std::string> cfg_pro(get_config_var("CONFIG_PROTECT")),
        cfg_pro_mask(get_config_var("CONFIG_PROTECT_MASK"));

    std::string file_str(stringify(file)), root_str(stringify(root));
    if (0 != file_str.compare(0, root_str.length(), root_str))
        throw Failure("is_config_protected confused: '" + root_str + "' '" + file_str + "'");
    file_str.erase(0, root_str.length());
    if (file_str.empty())
        file_str = "/";

    bool result(false);
    for (std::vector<std::string>::const_iterator c(cfg_pro.begin()),
            c_end(cfg_pro.end()) ; c != c_end && ! result ; ++c)
        if (0 == fnmatch((*c + "/*").c_str(), file_str.c_str(), 0))
            result = true;

    for (std::vector<std::string>::const_iterator c(cfg_pro_mask.begin()),
            c_end(cfg_pro_mask.end()) ; c != c_end && result ; ++c)
        if (0 == fnmatch((*c + "/*").c_str(), file_str.c_str(), 0))
            result = false;

    return result;
}

