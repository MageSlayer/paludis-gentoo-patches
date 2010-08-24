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

#ifndef PALUDIS_GUARD_PALUDIS_BROKEN_LINKAGE_FINDER_LINKAGE_CHECKER_HH
#define PALUDIS_GUARD_PALUDIS_BROKEN_LINKAGE_FINDER_LINKAGE_CHECKER_HH

#include <paludis/broken_linkage_finder.hh>
#include <paludis/util/fs_path-fwd.hh>
#include <paludis/util/set-fwd.hh>

#include <paludis/package_id-fwd.hh>
#include <memory>
#include <functional>
#include <string>

namespace paludis
{
    class LinkageChecker
    {
        public:
            LinkageChecker();
            virtual ~LinkageChecker();

            LinkageChecker(const LinkageChecker &) = delete;
            LinkageChecker & operator= (const LinkageChecker &) = delete;

            virtual bool check_file(const FSPath &) PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
            virtual void note_symlink(const FSPath &, const FSPath &) = 0;

            virtual void add_extra_lib_dir(const FSPath &) = 0;
            virtual void need_breakage_added(
                const std::function<void (const FSPath &, const std::string &)> &) = 0;

    };
}

#endif

