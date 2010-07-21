/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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
#include <paludis/util/named_value.hh>

namespace paludis
{
    namespace n
    {
        typedef Name<struct environment_name> environment;
        typedef Name<struct name_name> name;
    }

    /**
     * Options for FakeRepository.
     *
     * \ingroup grpfakerepository
     * \see FakeRepository
     * \since 0.26
     * \nosubgrouping
     */
    struct FakeRepositoryParams
    {
        NamedValue<n::environment, const Environment *> environment;
        NamedValue<n::name, RepositoryName> name;
    };

    /**
     * Fake repository for use in test cases.
     *
     * \ingroup grpfakerepository
     */
    class PALUDIS_VISIBLE FakeRepository :
        private PrivateImplementationPattern<FakeRepository>,
        public FakeRepositoryBase,
        public RepositoryVirtualsInterface
    {
        private:
            PrivateImplementationPattern<FakeRepository>::ImpPtr & _imp;

        public:
            ///\name Basic operations
            ///\{

            ///\since 0.26
            FakeRepository(const FakeRepositoryParams &);

            ~FakeRepository();

            ///\}

            /**
             * Add a virtual package.
             */
            void add_virtual_package(const QualifiedPackageName &, const std::tr1::shared_ptr<const PackageDepSpec> &);

            /* RepositoryVirtualsInterface */

            virtual std::tr1::shared_ptr<const VirtualsSequence> virtual_packages() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool some_ids_might_support_action(const SupportsActionTestBase &) const;

            virtual bool some_ids_might_not_be_masked() const;

            virtual const bool is_unimportant() const;

            /* Keys */

            virtual const std::tr1::shared_ptr<const MetadataValueKey<std::string> > format_key() const;
            virtual const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > location_key() const;
            virtual const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > installed_root_key() const;
            virtual const std::tr1::shared_ptr<const MetadataValueKey<std::string> > accept_keywords_key() const;
            virtual const std::tr1::shared_ptr<const MetadataValueKey<std::string> > sync_host_key() const;
    };
}

#endif
