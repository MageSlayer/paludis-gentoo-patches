/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
 * Copyright (c) 2006 David Morgan <david.morgan@wadham.oxford.ac.uk>
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

#ifndef PALUDIS_GUARD_PALUDIS_QA_ENVIRONMENT_HH
#define PALUDIS_GUARD_PALUDIS_QA_ENVIRONMENT_HH 1

#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/util/fs_entry.hh>

/** \file
 * Declarations for the QAEnvironment class.
 *
 * \ingroup Environment
 * \ingroup QA
 */

namespace paludis
{
    namespace qa
    {
        /**
         * The QAEnvironment is an Environment that corresponds to the environment
         * used by Qualudis for QA checks.
         *
         * \ingroup Environment
         * \ingroup QA
         */
        class QAEnvironment :
            public Environment
        {
            private:
                std::multimap<std::string, std::string> _mirrors;

            public:
                QAEnvironment(const FSEntry & base);

                ~QAEnvironment();

                virtual bool query_use(const UseFlagName &, const PackageDatabaseEntry *) const;

                virtual bool accept_keyword(const KeywordName &, const PackageDatabaseEntry * const) const;

                virtual bool accept_license(const std::string &, const PackageDatabaseEntry * const) const;

                virtual bool query_user_masks(const PackageDatabaseEntry &) const;

                virtual bool query_user_unmasks(const PackageDatabaseEntry &) const;

                virtual std::string bashrc_files() const
                {
                    return "";
                }

                virtual std::string hook_dirs() const
                {
                    return "";
                }

                virtual std::string paludis_command() const PALUDIS_ATTRIBUTE((noreturn))
                {
                    throw InternalError(PALUDIS_HERE, "Cannot use paludis_command in a QA environment");
                }

                virtual UseFlagNameCollection::Pointer query_enabled_use_matching(
                        const std::string &, const PackageDatabaseEntry *) const
                {
                    return UseFlagNameCollection::Pointer(new UseFlagNameCollection);
                }

                virtual MirrorIterator begin_mirrors(const std::string & mirror) const
                {
                    return _mirrors.end();
                }

                virtual MirrorIterator end_mirrors(const std::string & mirror) const
                {
                    return _mirrors.end();
                }

                virtual void perform_hook (const Hook & hook) const
                {
                }
        };
    }
}

#endif
