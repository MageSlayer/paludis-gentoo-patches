/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_VIRTUALS_INSTALLED_VIRTUALS_REPOSITORY_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_VIRTUALS_INSTALLED_VIRTUALS_REPOSITORY_HH 1

#include <paludis/repository.hh>
#include <paludis/util/map-fwd.hh>
#include <paludis/util/sequence-fwd.hh>
#include <tr1/memory>

namespace paludis
{
    /**
     * Repository representing installed virtual packages.
     *
     * \ingroup grpvirtualsrepository
     */
    class PALUDIS_VISIBLE InstalledVirtualsRepository :
        public Repository,
        public RepositoryDestinationInterface,
        public std::tr1::enable_shared_from_this<InstalledVirtualsRepository>,
        private PrivateImplementationPattern<InstalledVirtualsRepository>
    {
        private:
            PrivateImplementationPattern<InstalledVirtualsRepository>::ImpPtr & _imp;

            void need_ids() const;

        protected:
            virtual void need_keys_added() const;

        public:
            ///\name Basic operations
            //\{

            InstalledVirtualsRepository(const Environment * const env,
                    const FSEntry & root);

            virtual ~InstalledVirtualsRepository();

            ///\}

            ///\name RepositoryFactory functions
            ///\{

            static RepositoryName repository_factory_name(
                    const Environment * const env,
                    const std::tr1::function<std::string (const std::string &)> &);

            static std::tr1::shared_ptr<Repository> repository_factory_create(
                    const Environment * const env,
                    const std::tr1::function<std::string (const std::string &)> &);

            static std::tr1::shared_ptr<const RepositoryNameSet> repository_factory_dependencies(
                    const Environment * const env,
                    const std::tr1::function<std::string (const std::string &)> &);

            ///\}

            virtual std::tr1::shared_ptr<const CategoryNamePartSet> unimportant_category_names() const;

            virtual void invalidate();

            virtual void invalidate_masks();

            virtual bool can_be_favourite_repository() const;

            HookResult perform_hook(const Hook &)
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /* Repository */

            virtual std::tr1::shared_ptr<const PackageIDSequence> package_ids(
                    const QualifiedPackageName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<const QualifiedPackageNameSet> package_names(
                    const CategoryNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<const CategoryNamePartSet> category_names() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool has_package_named(const QualifiedPackageName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool has_category_named(const CategoryNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool some_ids_might_support_action(const SupportsActionTestBase &) const;

            virtual bool sync(const std::tr1::shared_ptr<OutputManager> &) const;

            /* Keys */

            virtual const std::tr1::shared_ptr<const MetadataValueKey<std::string> > format_key() const;
            virtual const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > location_key() const;
            virtual const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > installed_root_key() const;
            virtual const std::tr1::shared_ptr<const MetadataValueKey<std::string> > accept_keywords_key() const;
            virtual const std::tr1::shared_ptr<const MetadataValueKey<std::string> > sync_host_key() const;

            /* RepositoryDestinationInterface */

            virtual bool is_suitable_destination_for(const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool is_default_destination() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool want_pre_post_phases() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual void merge(const MergeParams &) PALUDIS_ATTRIBUTE((noreturn));

            ///\name Set methods
            ///\{

            virtual void populate_sets() const;

            ///\}
    };
}

#endif
