/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008 Ciaran McCreesh
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

#include <paludis/environment_implementation.hh>
#include <paludis/version_spec-fwd.hh>
#include <paludis/util/private_implementation_pattern.hh>

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
    class PALUDIS_VISIBLE TestEnvironment :
        private PrivateImplementationPattern<TestEnvironment>,
        public EnvironmentImplementation
    {
        protected:
            virtual tr1::shared_ptr<SetSpecTree::ConstItem> local_set(const SetName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<SetSpecTree::ConstItem> world_set() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

        public:
            ///\name Basic operations
            ///\{

            TestEnvironment();

            ~TestEnvironment();

            ///\}

            virtual bool query_use(const UseFlagName &, const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<PackageDatabase> package_database()
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const PackageDatabase> package_database() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::string paludis_command() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual void set_paludis_command(const std::string &);

            /**
             * Convenience way of getting a package id.
             */
            const tr1::shared_ptr<const PackageID> fetch_package_id(const QualifiedPackageName &,
                    const VersionSpec &, const RepositoryName &) const PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const UseFlagNameSet> known_use_expand_names(
                    const UseFlagName &, const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool accept_license(const std::string &, const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool accept_keywords(tr1::shared_ptr<const KeywordNameSet>, const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const tr1::shared_ptr<const Mask> mask_for_breakage(const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const tr1::shared_ptr<const Mask> mask_for_user(const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool unmasked_by_user(const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const FSEntrySequence> hook_dirs() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const FSEntry root() const;

            virtual uid_t reduced_uid() const;

            virtual gid_t reduced_gid() const;

            virtual tr1::shared_ptr<const MirrorsSequence> mirrors(const std::string &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual HookResult perform_hook(const Hook &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual void add_to_world(const QualifiedPackageName &) const;

            virtual void add_to_world(const SetName &) const;

            virtual void remove_from_world(const QualifiedPackageName &) const;

            virtual void remove_from_world(const SetName &) const;
    };
}

#endif
