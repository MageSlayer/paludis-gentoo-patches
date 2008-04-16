/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNPACKAGED_UNPACKAGED_REPOSITORY_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNPACKAGED_UNPACKAGED_REPOSITORY_HH 1

#include <paludis/repository.hh>
#include <paludis/util/map.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/kc-fwd.hh>
#include <paludis/util/keys.hh>

namespace paludis
{
    namespace unpackaged_repositories
    {
        typedef kc::KeyedClass<
            kc::Field<k::environment, Environment *>,
            kc::Field<k::location, FSEntry>,
            kc::Field<k::install_under, FSEntry>,
            kc::Field<k::name, QualifiedPackageName>,
            kc::Field<k::version, VersionSpec>,
            kc::Field<k::slot, SlotName>,
            kc::Field<k::build_dependencies, std::string>,
            kc::Field<k::run_dependencies, std::string>,
            kc::Field<k::description, std::string>
                > UnpackagedRepositoryParams;
    }

    class PALUDIS_VISIBLE UnpackagedRepository :
        private PrivateImplementationPattern<UnpackagedRepository>,
        public Repository
    {
        private:
            PrivateImplementationPattern<UnpackagedRepository>::ImpPtr & _imp;
            void _add_metadata_keys() const;

        protected:
            virtual void need_keys_added() const;

        public:
            UnpackagedRepository(
                    const RepositoryName &,
                    const unpackaged_repositories::UnpackagedRepositoryParams &);

            ~UnpackagedRepository();

            virtual void invalidate();
            virtual void invalidate_masks();

            virtual tr1::shared_ptr<const PackageIDSequence> package_ids(
                    const QualifiedPackageName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const QualifiedPackageNameSet> package_names(
                    const CategoryNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const CategoryNamePartSet> category_names() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const CategoryNamePartSet> category_names_containing_package(
                    const PackageNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool has_package_named(const QualifiedPackageName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool has_category_named(const CategoryNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool some_ids_might_support_action(const SupportsActionTestBase &) const;

            /* Keys */

            virtual const tr1::shared_ptr<const MetadataValueKey<std::string> > format_key() const;
            virtual const tr1::shared_ptr<const MetadataValueKey<FSEntry> > installed_root_key() const;
    };
}

#endif
