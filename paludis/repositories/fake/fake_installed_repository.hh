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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_FAKE_FAKE_INSTALLED_REPOSITORY_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_FAKE_FAKE_INSTALLED_REPOSITORY_HH 1

#include <paludis/repositories/fake/fake_repository_base.hh>

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_environment> environment;
        typedef Name<struct name_name> name;
        typedef Name<struct name_suitable_destination> suitable_destination;
        typedef Name<struct name_supports_uninstall> supports_uninstall;
    }

    /**
     * Parameters for a FakeInstalledRepository.
     */
    struct FakeInstalledRepositoryParams
    {
        NamedValue<n::environment, const Environment *> environment;
        NamedValue<n::name, RepositoryName> name;
        NamedValue<n::suitable_destination, bool> suitable_destination;
        NamedValue<n::supports_uninstall, bool> supports_uninstall;
    };

    /**
     * A fake repository for test cases, for installed packages.
     *
     * \ingroup grpfakerepository
     */
    class PALUDIS_VISIBLE FakeInstalledRepository :
        public FakeRepositoryBase,
        public RepositoryDestinationInterface
    {
        private:
            Pimp<FakeInstalledRepository> _imp;

        protected:
            /* RepositoryDestinationInterface */

            bool is_suitable_destination_for(const std::shared_ptr<const PackageID> &) const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            bool want_pre_post_phases() const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            std::string split_debug_location() const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            void merge(const MergeParams &) override;

        public:
            ///\name Basic operations
            ///\{

            ///\since 0.42
            FakeInstalledRepository(const FakeInstalledRepositoryParams &);
            ~FakeInstalledRepository() override;

            ///\}

            bool some_ids_might_support_action(const SupportsActionTestBase &) const override;

            bool some_ids_might_not_be_masked() const override;

            const bool is_unimportant() const override;

            /* Keys */
            const std::shared_ptr<const MetadataValueKey<std::string>> cross_compile_host_key() const override;
            const std::shared_ptr<const MetadataValueKey<std::string> > format_key() const override;
            const std::shared_ptr<const MetadataValueKey<FSPath> > location_key() const override;
            const std::shared_ptr<const MetadataValueKey<FSPath> > installed_root_key() const override;
            const std::shared_ptr<const MetadataCollectionKey<Map<std::string, std::string> > > sync_host_key() const override;
            const std::shared_ptr<const MetadataValueKey<std::string>> tool_prefix_key() const override;

            const std::shared_ptr<const Set<std::string> > maybe_expand_licence_nonrecursively(
                    const std::string &) const override;

            ///\name RepositoryFactory functions
            ///\{

            static RepositoryName repository_factory_name(
                    const Environment * const env,
                    const std::function<std::string (const std::string &)> &);

            static std::shared_ptr<Repository> repository_factory_create(
                    Environment * const env,
                    const std::function<std::string (const std::string &)> &);

            static std::shared_ptr<const RepositoryNameSet> repository_factory_dependencies(
                    const Environment * const env,
                    const std::function<std::string (const std::string &)> &);

            ///\}
    };
}

#endif
