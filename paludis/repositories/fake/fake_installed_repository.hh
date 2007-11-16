/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_FAKE_FAKE_INSTALLED_REPOSITORY_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_FAKE_FAKE_INSTALLED_REPOSITORY_HH 1

#include <paludis/repositories/fake/fake_repository_base.hh>

namespace paludis
{
    /**
     * A fake repository for test cases, for installed packages.
     *
     * \ingroup grpfakerepository
     */
    class PALUDIS_VISIBLE FakeInstalledRepository :
        public FakeRepositoryBase,
        public RepositoryDestinationInterface,
        public RepositoryProvidesInterface,
        private PrivateImplementationPattern<FakeInstalledRepository>
    {
        private:
            PrivateImplementationPattern<FakeInstalledRepository>::ImpPtr & _imp;

        protected:
            /* RepositoryDestinationInterface */

            virtual bool is_suitable_destination_for(const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool is_default_destination() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool want_pre_post_phases() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual void merge(const MergeOptions &);

            /* RepositoryProvidesInterface */

            virtual tr1::shared_ptr<const ProvidesSequence> provided_packages() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

        public:
            ///\name Basic operations
            ///\{

            FakeInstalledRepository(const Environment * const, const RepositoryName &);
            ~FakeInstalledRepository();

            ///\}

            virtual bool some_ids_might_support_action(const SupportsActionTestBase &) const;

            /* Keys */
            virtual const tr1::shared_ptr<const MetadataStringKey> format_key() const;
            virtual const tr1::shared_ptr<const MetadataFSEntryKey> installed_root_key() const;

    };
}

#endif
