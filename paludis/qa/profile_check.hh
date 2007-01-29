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

#ifndef PALUDIS_GUARD_PALUDIS_QA_PROFILE_CHECK_HH
#define PALUDIS_GUARD_PALUDIS_QA_PROFILE_CHECK_HH 1

#include <paludis/qa/check.hh>
#include <paludis/qa/check_result.hh>
#include <paludis/repositories/portage/portage_repository.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/virtual_constructor.hh>

/** \file
 * Declarations for the ProfileCheck class.
 *
 * \ingroup grpqa
 */

namespace paludis
{
    namespace qa
    {

#include <paludis/qa/profile_check-sr.hh>
        /**
         * A QA check that operates upon a profiles.desc entry directory.
         *
         * \ingroup grpqa
         */
        class PALUDIS_VISIBLE ProfileCheck :
            public Check
        {
            protected:
                ProfileCheck();

            public:
                virtual CheckResult operator() (const ProfileCheckData &) const = 0;
        };

        /**
         * Thrown if a bad profile check is requested.
         *
         * \ingroup grpexceptions
         */
        class PALUDIS_VISIBLE NoSuchProfileCheckTypeError :
            public Exception
        {
            public:
                NoSuchProfileCheckTypeError(const std::string &) throw ();
        };

        /**
         * Make a ProfileCheck class.
         *
         * We're implementing things this way to avoid breaking icc70. Icky.
         *
         * \ingroup grpqa
         */
        template <typename T_>
        struct MakeProfileCheck
        {
            static std::tr1::shared_ptr<ProfileCheck> make_profile_check();
        };

        /**
         * Virtual constructor for profile checks.
         *
         * \ingroup grpqa
         */
        class ProfileCheckMaker :
            public VirtualConstructor<std::string, std::tr1::shared_ptr<ProfileCheck> (*) (),
                virtual_constructor_not_found::ThrowException<NoSuchProfileCheckTypeError> >,
            public InstantiationPolicy<ProfileCheckMaker, instantiation_method::SingletonTag>
        {
            friend class InstantiationPolicy<ProfileCheckMaker, instantiation_method::SingletonTag>;

            private:
                ProfileCheckMaker();
        };
    }

}

template <typename T_>
std::tr1::shared_ptr<paludis::qa::ProfileCheck>
paludis::qa::MakeProfileCheck<T_>::make_profile_check()
{
    return std::tr1::shared_ptr<paludis::qa::ProfileCheck>(new T_);
}

#endif
