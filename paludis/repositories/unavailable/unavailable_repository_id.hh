/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNAVAILABLE_UNAVAILABLE_REPOSITORY_ID_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNAVAILABLE_UNAVAILABLE_REPOSITORY_ID_HH 1

#include <paludis/util/named_value.hh>
#include <paludis/package_id.hh>
#include <paludis/repositories/unavailable/unavailable_repository_file-fwd.hh>
#include <paludis/repositories/unavailable/unavailable_repository-fwd.hh>
#include <memory>

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_dependencies> dependencies;
        typedef Name<struct name_description> description;
        typedef Name<struct name_environment> environment;
        typedef Name<struct name_format> format;
        typedef Name<struct name_homepage> homepage;
        typedef Name<struct name_mask> mask;
        typedef Name<struct name_name> name;
        typedef Name<struct name_repository> repository;
        typedef Name<struct name_sync> sync;
    }

    namespace unavailable_repository
    {
        struct UnavailableRepositoryIDParams
        {
            NamedValue<n::dependencies, std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > > dependencies;
            NamedValue<n::description, std::shared_ptr<const MetadataValueKey<std::string> > > description;
            NamedValue<n::environment, const Environment *> environment;
            NamedValue<n::format, std::shared_ptr<const MetadataValueKey<std::string> > > format;
            NamedValue<n::homepage, std::shared_ptr<const MetadataValueKey<std::string> > > homepage;
            NamedValue<n::mask, std::shared_ptr<const Mask> > mask;
            NamedValue<n::name, QualifiedPackageName> name;
            NamedValue<n::repository, RepositoryName> repository;
            NamedValue<n::sync, std::shared_ptr<const MetadataValueKey<std::string> > > sync;
        };

        class PALUDIS_VISIBLE UnavailableRepositoryID :
            public PackageID,
            public std::enable_shared_from_this<UnavailableRepositoryID>
        {
            private:
                Pimp<UnavailableRepositoryID> _imp;

            protected:
                void need_keys_added() const;
                void need_masks_added() const;

            public:
                UnavailableRepositoryID(const UnavailableRepositoryIDParams &);
                ~UnavailableRepositoryID();

                 const std::string canonical_form(const PackageIDCanonicalForm) const;
                 const QualifiedPackageName name() const;
                 const VersionSpec version() const;
                 const RepositoryName repository_name() const;
                 virtual PackageDepSpec uniquely_identifying_spec() const;

                 const std::shared_ptr<const MetadataValueKey<SlotName> > slot_key() const;
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
                 const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const Contents> > >
                     contents_key() const;
                 const std::shared_ptr<const MetadataTimeKey> installed_time_key() const;
                 const std::shared_ptr<const MetadataValueKey<FSPath> > fs_location_key() const;
                 const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > behaviours_key() const;
                 const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > from_repositories_key() const;
                 const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const Choices> > > choices_key() const;

                 bool supports_action(const SupportsActionTestBase &) const
                     PALUDIS_ATTRIBUTE((warn_unused_result));
                 void perform_action(Action &) const;

                 std::shared_ptr<const Set<std::string> > breaks_portage() const
                     PALUDIS_ATTRIBUTE((warn_unused_result));

                 bool arbitrary_less_than_comparison(const PackageID &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
                 std::size_t extra_hash_value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                 virtual const std::shared_ptr<const Contents> contents() const;
        };
    }
}

#endif
