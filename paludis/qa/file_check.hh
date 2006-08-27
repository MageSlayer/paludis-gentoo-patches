/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/qa/check.hh>
#include <paludis/qa/check_result.hh>
#include <paludis/util/counted_ptr.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/virtual_constructor.hh>

/** \file
 * Declarations for the FileCheck class.
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

        /* Do not turn this into a straight function. It makes icc cry. */
        template <typename T_>
        struct MakeFileCheck
        {
            static FileCheck::Pointer make_file_check();
        };

        typedef VirtualConstructor<
            std::string,
            FileCheck::Pointer (*) (),
            virtual_constructor_not_found::ThrowException<NoSuchFileCheckTypeError> > FileCheckMaker;
    }

}

template <typename T_>
paludis::qa::FileCheck::Pointer
paludis::qa::MakeFileCheck<T_>::make_file_check()
{
    return paludis::qa::FileCheck::Pointer(new T_);
}


#endif
