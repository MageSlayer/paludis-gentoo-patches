/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/util/exception.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/virtual_constructor.hh>

/** \file
 * Declarations for the PackageDirCheck class.
 *
 * \ingroup grpqa
 */

namespace paludis
{
    namespace qa
    {
        /**
         * A QA check that operates upon a package directory.
         *
         * \ingroup grpqa
         */
        class PackageDirCheck :
            public Check
        {
            protected:
                PackageDirCheck();

            public:
                virtual CheckResult operator() (const FSEntry &) const = 0;
        };

        /**
         * Thrown if a bad package dir check is requested.
         *
         * \ingroup grpexceptions
         */
        class NoSuchPackageDirCheckTypeError :
            public Exception
        {
            public:
                NoSuchPackageDirCheckTypeError(const std::string &) throw ();
        };

        /**
         * Make a PackageDirCheck class.
         *
         * We're implementing things this way to avoid breaking icc70. Icky.
         *
         * \ingroup grpqa
         */
        template <typename T_>
        struct MakePackageDirCheck
        {
            static std::tr1::shared_ptr<PackageDirCheck>
            make_package_dir_check()
            {
                return std::tr1::shared_ptr<PackageDirCheck>(new T_);
            }
        };

        /**
         * Virtual constructor for package dir checks.
         *
         * \ingroup grpqa
         */
        class PackageDirCheckMaker :
            public VirtualConstructor<std::string, std::tr1::shared_ptr<PackageDirCheck> (*) (),
                virtual_constructor_not_found::ThrowException<NoSuchPackageDirCheckTypeError> >,
            public InstantiationPolicy<PackageDirCheckMaker, instantiation_method::SingletonTag>
        {
            friend class InstantiationPolicy<PackageDirCheckMaker, instantiation_method::SingletonTag>;

            private:
                PackageDirCheckMaker();
        };
    }
}
#endif
