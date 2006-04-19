/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#ifndef PALUDIS_GUARD_PALUDIS_TEST_ENVIRONMENT_HH
#define PALUDIS_GUARD_PALUDIS_TEST_ENVIRONMENT_HH 1

#include <paludis/package_database.hh>
#include <paludis/qa/environment.hh>

/** \file
 * Declarations for the TestEnvironment class.
 *
 * \ingroup grptestenvironment
 */

namespace paludis
{
    /**
     * A TestEnvironment is an environment used during testing that lets us
     * control all the options rather than reading them from configuration
     * files.
     *
     * \ingroup grptestenvironment
     */
    class TestEnvironment : public Environment
    {
        public:
            /**
             * Constructor.
             */
            TestEnvironment();

            virtual bool query_use(const UseFlagName &, const PackageDatabaseEntry *) const;

            virtual bool accept_keyword(const KeywordName &, const PackageDatabaseEntry * const) const;

            virtual bool query_user_masks(const PackageDatabaseEntry &) const;

            virtual bool query_user_unmasks(const PackageDatabaseEntry &) const;

            virtual bool accept_license(const std::string &, const PackageDatabaseEntry * const) const;

            virtual std::string bashrc_files() const
            {
                return "";
            }

            virtual std::string paludis_command() const
            {
                return "false";
            }

            virtual UseFlagNameCollection::Pointer query_enabled_use_matching(
                    const std::string &, const PackageDatabaseEntry *) const
            {
                return UseFlagNameCollection::Pointer(new UseFlagNameCollection);
            }
    };
}

#endif
