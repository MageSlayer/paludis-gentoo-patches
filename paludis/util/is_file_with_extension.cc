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

#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/options.hh>
#include <ostream>

using namespace paludis;

#include <paludis/util/is_file_with_extension-se.cc>

bool
paludis::is_file_with_extension(const FSPath & f, const std::string & s, const IsFileWithOptions & o)
{
    return is_file_with_prefix_extension(f, "", s, o);
}

bool
paludis::is_file_with_prefix_extension(const FSPath & f, const std::string & prefix,
        const std::string & ext, const IsFileWithOptions & o)
{
    const std::string filename(f.basename());

    if (filename.length() < ext.length() + prefix.length())
        return false;

    if (0 != filename.compare(filename.length() - ext.length(), ext.length(), ext))
        return false;
    if (0 != filename.compare(0, prefix.length(), prefix))
        return false;

    FSStat f_stat(f);
    return f_stat.is_regular_file() || ((! o[ifwo_no_follow_symlinks]) && f_stat.exists() && f.realpath().stat().is_regular_file());
}

