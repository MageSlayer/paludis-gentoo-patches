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

#ifndef PALUDIS_GUARD_PALUDIS_BROKEN_LINKAGE_FINDER_LIBTOOL_LINKAGE_CHECKER_HH
#define PALUDIS_GUARD_PALUDIS_BROKEN_LINKAGE_FINDER_LIBTOOL_LINKAGE_CHECKER_HH

#include <paludis/linkage_checker.hh>
#include <paludis/util/pimp.hh>

namespace paludis
{
    class LibtoolLinkageChecker :
        public LinkageChecker,
        private paludis::Pimp<LibtoolLinkageChecker>
    {
        public:
            LibtoolLinkageChecker(const paludis::FSEntry &);
            virtual ~LibtoolLinkageChecker();

            virtual bool check_file(const paludis::FSEntry &) PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual void note_symlink(const paludis::FSEntry &, const paludis::FSEntry &);

            virtual void add_extra_lib_dir(const paludis::FSEntry &);
            virtual void need_breakage_added(
                const std::function<void (const paludis::FSEntry &, const std::string &)> &);
    };
}

#endif

