/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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
        typedef Name<struct name_environment> environment;
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
        public FakeRepositoryBase
    {
        private:
            Pimp<FakeRepository> _imp;

        public:
            ///\name Basic operations
            ///\{

            ///\since 0.26
            FakeRepository(const FakeRepositoryParams &);

            ~FakeRepository() override;

            ///\}

            bool some_ids_might_support_action(const SupportsActionTestBase &) const override;

            bool some_ids_might_not_be_masked() const override;

            const bool is_unimportant() const override;

            const std::shared_ptr<const Set<std::string> > maybe_expand_licence_nonrecursively(
                    const std::string &) const override;

            /* Keys */

            const std::shared_ptr<const MetadataValueKey<std::string>> cross_compile_host_key() const override;
            const std::shared_ptr<const MetadataValueKey<std::string> > format_key() const override;
            const std::shared_ptr<const MetadataValueKey<FSPath> > location_key() const override;
            const std::shared_ptr<const MetadataValueKey<FSPath> > installed_root_key() const override;
            const std::shared_ptr<const MetadataCollectionKey<Map<std::string, std::string> > > sync_host_key() const override;
            const std::shared_ptr<const MetadataValueKey<std::string>> tool_prefix_key() const override;
    };
}

#endif
