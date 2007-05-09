/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Fernando J. Pereda <ferdy@gentoo.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_QA_GPG_CHECK_HH
#define PALUDIS_GUARD_PALUDIS_QA_GPG_CHECK_HH 1

#include <paludis/qa/package_dir_check.hh>
#include <string>

namespace paludis
{
    namespace qa
    {
        /**
         * QA check: is Manifest gpg signed?
         *
         * \ingroup grpqacheck
         */
        class GPGCheck :
            public PackageDirCheck
        {
            public:
                GPGCheck();

                CheckResult operator() (const FSEntry &) const;

                static const PALUDIS_VISIBLE std::string & identifier();

                virtual std::string describe() const
                {
                    return "Checks whether the Manifest is signed";
                }
        };
    }
}

#endif
