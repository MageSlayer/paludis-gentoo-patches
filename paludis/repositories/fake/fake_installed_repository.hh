/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
        public RepositoryInstalledInterface,
        public RepositoryDestinationInterface,
        public RepositoryProvidesInterface
    {
        protected:
            virtual Contents::ConstPointer do_contents(
                    const QualifiedPackageName &,
                    const VersionSpec &) const;

            virtual ProvidesCollection::ConstPointer provided_packages() const;

            virtual VersionMetadata::ConstPointer provided_package_version_metadata(
                    const RepositoryProvidesEntry &) const;

        public:
            FakeInstalledRepository(const Environment * const, const RepositoryName &);

            bool is_suitable_destination_for(const PackageDatabaseEntry &) const;
    };
}

#endif
