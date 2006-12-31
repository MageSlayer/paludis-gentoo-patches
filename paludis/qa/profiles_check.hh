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

#ifndef PALUDIS_GUARD_PALUDIS_QA_PROFILES_CHECK_HH
#define PALUDIS_GUARD_PALUDIS_QA_PROFILES_CHECK_HH 1

#include <paludis/qa/check.hh>
#include <paludis/qa/check_result.hh>
#include <paludis/util/counted_ptr.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/virtual_constructor.hh>

/** \file
 * Declarations for the ProfilesCheck class.
 *
 * \ingroup grpqa
 */

namespace paludis
{
    namespace qa
    {
        /**
         * A QA check that operates upon the top level profiles/ directory.
         *
         * \ingroup grpqa
         */
        class PALUDIS_VISIBLE ProfilesCheck :
            public Check,
            public InternalCounted<ProfilesCheck>
        {
            protected:
                ProfilesCheck();

            public:
                virtual CheckResult operator() (const FSEntry &) const = 0;
        };

        /**
         * Thrown if a bad profiles check is requested.
         *
         * \ingroup grpexceptions
         */
        class PALUDIS_VISIBLE NoSuchProfilesCheckTypeError :
            public Exception
        {
            public:
                NoSuchProfilesCheckTypeError(const std::string &) throw ();
        };

        /**
         * Make a ProfilesCheck class.
         *
         * We're implementing things this way to avoid breaking icc70. Icky.
         *
         * \ingroup grpqa
         */
        template <typename T_>
        struct MakeProfilesCheck
        {
            static ProfilesCheck::Pointer make_profiles_check();
        };

        /**
         * Virtual constructor for profiles checks.
         *
         * \ingroup grpqa
         */
        class ProfilesCheckMaker :
            public VirtualConstructor<std::string, ProfilesCheck::Pointer (*) (),
                virtual_constructor_not_found::ThrowException<NoSuchProfilesCheckTypeError> >,
            public InstantiationPolicy<ProfilesCheckMaker, instantiation_method::SingletonAsNeededTag>
        {
            friend class InstantiationPolicy<ProfilesCheckMaker, instantiation_method::SingletonAsNeededTag>;

            private:
                ProfilesCheckMaker();
        };
    }

}

template <typename T_>
paludis::qa::ProfilesCheck::Pointer
paludis::qa::MakeProfilesCheck<T_>::make_profiles_check()
{
    return paludis::qa::ProfilesCheck::Pointer(new T_);
}

#endif
