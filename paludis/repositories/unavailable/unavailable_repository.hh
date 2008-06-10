/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNAVAILABLE_UNAVAILABLE_REPOSITORY_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNAVAILABLE_UNAVAILABLE_REPOSITORY_HH 1

#include <paludis/repositories/unavailable/unavailable_repository-fwd.hh>
#include <paludis/repository.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/attributes.hh>

namespace paludis
{
    namespace unavailable_repository
    {
        class PALUDIS_VISIBLE UnavailableRepositoryConfigurationError :
            public ConfigurationError
        {
            public:
                UnavailableRepositoryConfigurationError(const std::string &) throw ();
        };

        typedef kc::KeyedClass<
            kc::Field<k::environment, Environment *>,
            kc::Field<k::location, FSEntry>,
            kc::Field<k::sync, std::string>,
            kc::Field<k::sync_options, std::string>
                > UnavailableRepositoryParams;

        class PALUDIS_VISIBLE UnavailableRepository :
            private PrivateImplementationPattern<UnavailableRepository>,
            public Repository,
            public RepositorySyncableInterface,
            public std::tr1::enable_shared_from_this<UnavailableRepository>
        {
            private:
                PrivateImplementationPattern<UnavailableRepository>::ImpPtr & _imp;

                void _add_metadata_keys();

            protected:
                virtual void need_keys_added() const;

            public:
                UnavailableRepository(const UnavailableRepositoryParams &);
                ~UnavailableRepository();

                virtual const std::tr1::shared_ptr<const MetadataValueKey<std::string> > format_key() const;
                virtual const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > installed_root_key() const;

                virtual bool has_category_named(const CategoryNamePart & c) const;
                virtual bool has_package_named(const QualifiedPackageName & q) const;
                virtual std::tr1::shared_ptr<const CategoryNamePartSet> category_names() const;
                virtual std::tr1::shared_ptr<const CategoryNamePartSet> unimportant_category_names() const;
                virtual std::tr1::shared_ptr<const CategoryNamePartSet> category_names_containing_package(
                        const PackageNamePart & p) const;
                virtual std::tr1::shared_ptr<const QualifiedPackageNameSet> package_names(
                        const CategoryNamePart & c) const;
                virtual std::tr1::shared_ptr<const PackageIDSequence> package_ids(const QualifiedPackageName & p) const;

                virtual bool some_ids_might_support_action(const SupportsActionTestBase &) const;
                virtual void invalidate();
                virtual void invalidate_masks();

                virtual bool sync() const;
        };
    }

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class PrivateImplementationPattern<unavailable_repository::UnavailableRepository>;
#endif
}

#endif
