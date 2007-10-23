/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_FAKE_FAKE_REPOSITORY_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_FAKE_FAKE_REPOSITORY_HH 1

#include <paludis/repositories/fake/fake_repository_base.hh>

namespace paludis
{
    /**
     * Fake repository for use in test cases.
     *
     * \ingroup grpfakerepository
     */
    class PALUDIS_VISIBLE FakeRepository :
        private PrivateImplementationPattern<FakeRepository>,
        public FakeRepositoryBase,
        public RepositoryVirtualsInterface,
        public RepositoryMirrorsInterface
    {
        private:
            Implementation<FakeRepository> * const _imp;

        protected:
            virtual bool do_some_ids_might_support_action(const SupportsActionTestBase &) const;

        public:
            ///\name Basic operations
            ///\{

            FakeRepository(const Environment * const, const RepositoryName &);
            ~FakeRepository();

            ///\}

            /**
             * Add a virtual package.
             */
            void add_virtual_package(const QualifiedPackageName &, tr1::shared_ptr<const PackageDepSpec>);

            /* RepositoryVirtualsInterface */

            virtual tr1::shared_ptr<const VirtualsSequence> virtual_packages() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /* RepositoryMirrorsInterface */

            virtual MirrorsConstIterator begin_mirrors(const std::string & s) const
                PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual MirrorsConstIterator end_mirrors(const std::string & s) const
                PALUDIS_ATTRIBUTE((warn_unused_result));
    };
}

#endif
