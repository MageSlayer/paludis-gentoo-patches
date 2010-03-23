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

#include <paludis/util/realpath.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/stringify.hh>

using namespace paludis;

namespace
{
    FSEntry
    do_realpath_with_current_and_root(
        const FSEntry & file, const FSEntry & current, const FSEntry & root,
        const FSEntry & orig_file, const FSEntry & orig_current, unsigned & symlinks)
    {
        if (symlinks >= 40)
            throw FSError("Too many symlinks encountered while canonicalising '" + stringify(orig_file) +
                          "' relative to '" + stringify(orig_current) + "' with root '" + stringify(root) + "'");

        FSEntry cur(current);

        std::string file_str(stringify(file));
        if ('/' == file_str[0])
        {
            file_str.erase(0, 1);
            cur = FSEntry("/");
        }

        while (! file_str.empty())
        {
            std::string::size_type slash(file_str.find('/'));
            if (0 == slash)
            {
                file_str.erase(0, 1);
                continue;
            }

            std::string component(std::string::npos == slash
                                  ? file_str : file_str.substr(0, slash));
            if (std::string::npos == slash)
                file_str.erase();
            else
                file_str.erase(0, slash + 1);

            if ("." == component)
                continue;
            if (".." == component)
            {
                cur = cur.dirname();
                continue;
            }

            FSEntry full(root / cur / component);
            if (full.is_symbolic_link())
                cur = do_realpath_with_current_and_root(FSEntry(full.readlink()), cur, root, orig_file, orig_current, ++symlinks);
            else
                cur = cur / component;
        }

        return cur;
    }
}

FSEntry
paludis::realpath_with_current_and_root(const FSEntry & file, const FSEntry & current, const FSEntry & root)
{
    unsigned symlinks(0);
    return do_realpath_with_current_and_root(file, current, root, file, current, symlinks);
}

FSEntry
paludis::dereference_with_root(const FSEntry & file, const FSEntry & root)
{
    if (file.is_symbolic_link())
        return root / realpath_with_current_and_root(FSEntry(file.readlink()), file.dirname().strip_leading(root), root);
    else
        return file;
}

