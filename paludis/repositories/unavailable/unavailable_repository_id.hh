/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010 Ciaran McCreesh
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
#include <tr1/memory>

namespace paludis
{
    namespace n
    {
        struct dependencies;
        struct description;
        struct environment;
        struct format;
        struct homepage;
        struct mask;
        struct name;
        struct repository;
        struct sync;
    }

    namespace unavailable_repository
    {
        struct UnavailableRepositoryIDParams
        {
            NamedValue<n::dependencies, std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > > dependencies;
            NamedValue<n::description, std::tr1::shared_ptr<const MetadataValueKey<std::string> > > description;
            NamedValue<n::environment, const Environment *> environment;
            NamedValue<n::format, std::tr1::shared_ptr<const MetadataValueKey<std::string> > > format;
            NamedValue<n::homepage, std::tr1::shared_ptr<const MetadataValueKey<std::string> > > homepage;
            NamedValue<n::mask, std::tr1::shared_ptr<const Mask> > mask;
            NamedValue<n::name, QualifiedPackageName> name;
            NamedValue<n::repository, const UnavailableRepository *> repository;
            NamedValue<n::sync, std::tr1::shared_ptr<const MetadataValueKey<std::string> > > sync;
        };

        class PALUDIS_VISIBLE UnavailableRepositoryID :
            public PackageID,
            private PrivateImplementationPattern<UnavailableRepositoryID>,
            public std::tr1::enable_shared_from_this<UnavailableRepositoryID>
        {
            private:
                PrivateImplementationPattern<UnavailableRepositoryID>::ImpPtr & _imp;

            protected:
                void need_keys_added() const;
                void need_masks_added() const;

            public:
                UnavailableRepositoryID(const UnavailableRepositoryIDParams &);
                ~UnavailableRepositoryID();

                 const std::string canonical_form(const PackageIDCanonicalForm) const;
                 const QualifiedPackageName name() const;
                 const VersionSpec version() const;
                 const std::tr1::shared_ptr<const Repository> repository() const;
                 virtual PackageDepSpec uniquely_identifying_spec() const;

                 const std::tr1::shared_ptr<const MetadataValueKey<SlotName> > slot_key() const;
                 const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >
                     virtual_for_key() const;
                 const std::tr1::shared_ptr<const MetadataCollectionKey<KeywordNameSet> > keywords_key() const;
                 const std::tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> > provide_key() const;
                 const std::tr1::shared_ptr<const MetadataCollectionKey<PackageIDSequence> > contains_key() const;
                 const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >
                     contained_in_key() const;
                 const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
                     dependencies_key() const;
                 const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
                     build_dependencies_key() const;
                 const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
                     run_dependencies_key() const;
                 const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
                     post_dependencies_key() const;
                 const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
                     suggested_dependencies_key() const;
                 const std::tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> > fetches_key() const;
                 const std::tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> > homepage_key() const;
                 const std::tr1::shared_ptr<const MetadataValueKey<std::string> > short_description_key() const;
                 const std::tr1::shared_ptr<const MetadataValueKey<std::string> > long_description_key() const;
                 const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Contents> > >
                     contents_key() const;
                 const std::tr1::shared_ptr<const MetadataTimeKey> installed_time_key() const;
                 const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > fs_location_key() const;
                 const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > > behaviours_key() const;
                 const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > > from_repositories_key() const;
                 const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Choices> > > choices_key() const;

                 bool supports_action(const SupportsActionTestBase &) const
                     PALUDIS_ATTRIBUTE((warn_unused_result));
                 void perform_action(Action &) const;

                 std::tr1::shared_ptr<const Set<std::string> > breaks_portage() const
                     PALUDIS_ATTRIBUTE((warn_unused_result));

                 bool arbitrary_less_than_comparison(const PackageID &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
                 std::size_t extra_hash_value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }
}

#endif
