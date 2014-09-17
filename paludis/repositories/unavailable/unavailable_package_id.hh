/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011, 2014 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNAVAILABLE_UNAVAILABLE_PACKAGE_ID_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNAVAILABLE_UNAVAILABLE_PACKAGE_ID_HH 1

#include <paludis/util/named_value.hh>
#include <paludis/package_id.hh>
#include <paludis/repositories/unavailable/unavailable_repository_file-fwd.hh>
#include <paludis/repositories/unavailable/unavailable_repository-fwd.hh>

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_description> description;
        typedef Name<struct name_environment> environment;
        typedef Name<struct name_from_repositories> from_repositories;
        typedef Name<struct name_mask> mask;
        typedef Name<struct name_name> name;
        typedef Name<struct name_repository> repository;
        typedef Name<struct name_repository_description> repository_description;
        typedef Name<struct name_repository_homepage> repository_homepage;
        typedef Name<struct name_slot> slot;
        typedef Name<struct name_version> version;
    }

    namespace unavailable_repository
    {
        struct UnavailablePackageIDParams
        {
            NamedValue<n::description, std::shared_ptr<const MetadataValueKey<std::string> > > description;
            NamedValue<n::environment, const Environment *> environment;
            NamedValue<n::from_repositories, std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > > from_repositories;
            NamedValue<n::mask, std::shared_ptr<const Mask> > mask;
            NamedValue<n::name, QualifiedPackageName> name;
            NamedValue<n::repository, RepositoryName> repository;
            NamedValue<n::repository_description, std::shared_ptr<const MetadataValueKey<std::string> > > repository_description;
            NamedValue<n::repository_homepage, std::shared_ptr<const MetadataValueKey<std::string> > > repository_homepage;
            NamedValue<n::slot, Slot> slot;
            NamedValue<n::version, VersionSpec> version;
        };

        class PALUDIS_VISIBLE UnavailablePackageID :
            public PackageID
        {
            private:
                Pimp<UnavailablePackageID> _imp;

            protected:
                void need_keys_added() const;
                void need_masks_added() const;

            public:
                UnavailablePackageID(const UnavailablePackageIDParams &);
                ~UnavailablePackageID();

                 const std::string canonical_form(const PackageIDCanonicalForm) const;
                 const QualifiedPackageName name() const;
                 const VersionSpec version() const;
                 const RepositoryName repository_name() const;
                 virtual PackageDepSpec uniquely_identifying_spec() const;

                 const std::shared_ptr<const MetadataValueKey<Slot> > slot_key() const;
                 const std::shared_ptr<const MetadataCollectionKey<KeywordNameSet> > keywords_key() const;
                 const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
                     dependencies_key() const;
                 const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
                     build_dependencies_key() const;
                 const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
                     run_dependencies_key() const;
                 const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
                     post_dependencies_key() const;
                 const std::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> > fetches_key() const;
                 const std::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> > homepage_key() const;
                 const std::shared_ptr<const MetadataValueKey<std::string> > short_description_key() const;
                 const std::shared_ptr<const MetadataValueKey<std::string> > long_description_key() const;
                 const std::shared_ptr<const MetadataTimeKey> installed_time_key() const;
                 const std::shared_ptr<const MetadataValueKey<FSPath> > fs_location_key() const;
                 const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > behaviours_key() const;
                 const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > from_repositories_key() const;
                 const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const Choices> > > choices_key() const;

                 bool supports_action(const SupportsActionTestBase &) const
                     PALUDIS_ATTRIBUTE((warn_unused_result));
                 void perform_action(Action &) const PALUDIS_ATTRIBUTE((noreturn));

                 bool arbitrary_less_than_comparison(const PackageID &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
                 std::size_t extra_hash_value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                 virtual const std::shared_ptr<const Contents> contents() const;
        };
    }
}

#endif
