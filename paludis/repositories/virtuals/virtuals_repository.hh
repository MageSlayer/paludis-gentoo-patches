/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_VIRTUALS_VIRTUALS_REPOSITORY_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_VIRTUALS_VIRTUALS_REPOSITORY_HH 1

#include <paludis/repository.hh>
#include <paludis/util/map-fwd.hh>
#include <tr1/memory>

namespace paludis
{
    /**
     * A repository holding packages representing virtuals.
     *
     * \ingroup grpvirtualsrepository
     */
    class PALUDIS_VISIBLE VirtualsRepository :
        public Repository,
        public RepositoryMakeVirtualsInterface,
        private PrivateImplementationPattern<VirtualsRepository>,
        public std::tr1::enable_shared_from_this<VirtualsRepository>
    {
        private:
            PrivateImplementationPattern<VirtualsRepository>::ImpPtr & _imp;

            void need_names() const;
            void need_ids() const;

        protected:
            virtual void need_keys_added() const;

        public:
            ///\name Basic operations
            ///\{

            VirtualsRepository(const Environment * const env);

            virtual ~VirtualsRepository();

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

            virtual const bool is_unimportant() const;

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

            virtual bool some_ids_might_not_be_masked() const;

            virtual bool sync(const std::tr1::shared_ptr<OutputManager> &) const;

            /* RepositoryMakeVirtualsInterface */

            virtual const std::tr1::shared_ptr<const PackageID> make_virtual_package_id(
                    const QualifiedPackageName & virtual_name, const std::tr1::shared_ptr<const PackageID> & provider) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /* Keys */

            virtual const std::tr1::shared_ptr<const MetadataValueKey<std::string> > format_key() const;
            virtual const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > location_key() const;
            virtual const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > installed_root_key() const;
            virtual const std::tr1::shared_ptr<const MetadataValueKey<std::string> > accept_keywords_key() const;
            virtual const std::tr1::shared_ptr<const MetadataValueKey<std::string> > sync_host_key() const;

            ///\name Set methods
            ///\{

            virtual void populate_sets() const;

            ///\}

            virtual HookResult perform_hook(const Hook & hook)
                PALUDIS_ATTRIBUTE((warn_unused_result));
    };
}

#endif
