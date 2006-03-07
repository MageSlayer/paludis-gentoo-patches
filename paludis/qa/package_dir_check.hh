/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_QA_PACKAGE_DIR_CHECK_HH
#define PALUDIS_GUARD_PALUDIS_QA_PACKAGE_DIR_CHECK_HH 1

#include <paludis/qa/check.hh>
#include <paludis/qa/check_result.hh>
#include <paludis/util/counted_ptr.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/virtual_constructor.hh>

/** \file
 * Declarations for the PackageDirCheck class.
 *
 * \ingroup QA
 */

namespace paludis
{
    namespace qa
    {
        /**
         * A QA check that operates upon a package directory.
         *
         * \ingroup QA
         */
        class PackageDirCheck :
            public Check,
            public InternalCounted<PackageDirCheck>
        {
            protected:
                PackageDirCheck();

            public:
                virtual CheckResult operator() (const FSEntry &) const = 0;
        };

        class NoSuchPackageDirCheckTypeError :
            public Exception
        {
            public:
                NoSuchPackageDirCheckTypeError(const std::string &) throw ();
        };

        /* icc is dumb */
        template <typename T_>
        struct MakePackageDirCheck
        {
            static PackageDirCheck::Pointer
            make_package_dir_check()
            {
                return PackageDirCheck::Pointer(new T_);
            }
        };

        typedef VirtualConstructor<
            std::string,
            PackageDirCheck::Pointer (*) (),
            virtual_constructor_not_found::ThrowException<NoSuchPackageDirCheckTypeError> > PackageDirCheckMaker;
    }
}
#endif
