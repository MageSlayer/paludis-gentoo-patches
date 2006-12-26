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

#ifndef PALUDIS_GUARD_PALUDIS_QA_EBUILD_CHECK_HH
#define PALUDIS_GUARD_PALUDIS_QA_EBUILD_CHECK_HH 1

#include <paludis/name.hh>
#include <paludis/qa/check.hh>
#include <paludis/qa/check_result.hh>
#include <paludis/util/counted_ptr.hh>
#include <paludis/util/virtual_constructor.hh>
#include <paludis/version_spec.hh>

namespace paludis
{
    class Environment;

    namespace qa
    {

#include <paludis/qa/ebuild_check-sr.hh>

        /**
         * Base class for QA checks that operate upon ebuilds.
         *
         * \ingroup grpqa
         */
        class PALUDIS_VISIBLE EbuildCheck :
            public Check,
            public InternalCounted<EbuildCheck>
        {
            protected:
                EbuildCheck();

            public:
                virtual CheckResult operator() (const EbuildCheckData &) const = 0;
        };

        /**
         * Thrown if a bad package ebuild check is requested.
         *
         * \ingroup grpexceptions
         */
        class PALUDIS_VISIBLE NoSuchEbuildCheckTypeError :
            public Exception
        {
            public:
                NoSuchEbuildCheckTypeError(const std::string &) throw ();
        };

        /**
         * Make an EbuildCheck class.
         *
         * We're implementing things this way to avoid breaking icc70. Icky.
         *
         * \ingroup grpqa
         */
        template <typename T_>
        struct MakeEbuildCheck
        {
            static EbuildCheck::Pointer
            make_ebuild_check()
            {
                return EbuildCheck::Pointer(new T_);
            }
        };

        class EbuildCheckMaker :
            public VirtualConstructor<std::string, EbuildCheck::Pointer (*) (),
                virtual_constructor_not_found::ThrowException<NoSuchEbuildCheckTypeError> >,
            public InstantiationPolicy<EbuildCheckMaker, instantiation_method::SingletonAsNeededTag>
        {
            friend class InstantiationPolicy<EbuildCheckMaker, instantiation_method::SingletonAsNeededTag>;

            private:
                EbuildCheckMaker();
        };
    }
}

#endif
