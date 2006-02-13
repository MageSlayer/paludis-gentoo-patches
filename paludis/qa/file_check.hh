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

#ifndef PALUDIS_GUARD_PALUDIS_QA_FILE_CHECK_HH
#define PALUDIS_GUARD_PALUDIS_QA_FILE_CHECK_HH 1

#include <paludis/exception.hh>
#include <paludis/fs_entry.hh>
#include <paludis/counted_ptr.hh>
#include <paludis/virtual_constructor.hh>
#include <paludis/qa/check.hh>
#include <paludis/qa/check_result.hh>

/** \file
 * Declarations for the FIleCheck class.
 *
 * \ingroup QA
 */

namespace paludis
{
    namespace qa
    {
        /**
         * A QA check that operates upon a file.
         *
         * \ingroup QA
         */
        class FileCheck :
            public Check,
            public InternalCounted<FileCheck>
        {
            protected:
                FileCheck();

            public:
                virtual CheckResult operator() (const FSEntry &) const = 0;
        };

        class NoSuchFileCheckTypeError :
            public Exception
        {
            public:
                NoSuchFileCheckTypeError(const std::string &) throw ();
        };

        template <typename T_>
        FileCheck::Pointer
        make_file_check()
        {
            return FileCheck::Pointer(new T_);
        }

        typedef VirtualConstructor<
            std::string,
            FileCheck::Pointer (*) (),
            virtual_constructor_not_found::ThrowException<NoSuchFileCheckTypeError> > FileCheckMaker;
    }
}

#endif
