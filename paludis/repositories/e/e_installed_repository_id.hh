/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_E_INSTALLED_REPOSITORY_ID_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_E_INSTALLED_REPOSITORY_ID_HH 1

#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/repositories/e/e_repository_id.hh>

namespace paludis
{
    namespace erepository
    {
        class EInstalledRepositoryID :
            public ERepositoryID,
            public std::enable_shared_from_this<EInstalledRepositoryID>
        {
            private:
                Pimp<EInstalledRepositoryID> _imp;

            protected:
                virtual void need_keys_added() const;
                virtual void need_masks_added() const;

                EInstalledRepositoryID(const QualifiedPackageName &, const VersionSpec &,
                        const Environment * const,
                        const RepositoryName &,
                        const FSPath & file);

            public:
                ~EInstalledRepositoryID();

                virtual const std::string canonical_form(const PackageIDCanonicalForm) const;
                virtual PackageDepSpec uniquely_identifying_spec() const;

                virtual const QualifiedPackageName name() const;
                virtual const VersionSpec version() const;
                virtual const RepositoryName repository_name() const;
                virtual const std::shared_ptr<const EAPI> eapi() const;

                virtual const std::shared_ptr<const MetadataValueKey<SlotName> > slot_key() const;
                virtual const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const PackageID> > > virtual_for_key() const;
                virtual const std::shared_ptr<const MetadataCollectionKey<KeywordNameSet> > keywords_key() const;
                virtual const std::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> > provide_key() const;
                virtual const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > build_dependencies_key() const;
                virtual const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > run_dependencies_key() const;
                virtual const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > post_dependencies_key() const;
                virtual const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > suggested_dependencies_key() const;
                virtual const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > dependencies_key() const;
                virtual const std::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> > restrict_key() const;
                virtual const std::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> > properties_key() const;
                virtual const std::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> > fetches_key() const;
                virtual const std::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> > homepage_key() const;
                virtual const std::shared_ptr<const MetadataValueKey<std::string> > short_description_key() const;
                virtual const std::shared_ptr<const MetadataValueKey<std::string> > long_description_key() const;
                virtual const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const Contents> > > contents_key() const;
                virtual const std::shared_ptr<const MetadataTimeKey> installed_time_key() const;
                virtual const std::shared_ptr<const MetadataCollectionKey<PackageIDSequence> > contains_key() const;
                virtual const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const PackageID> > > contained_in_key() const;
                virtual const std::shared_ptr<const MetadataValueKey<FSPath> > fs_location_key() const;
                virtual const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > from_repositories_key() const;

                virtual const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > raw_use_key() const;
                virtual const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > raw_iuse_key() const;
                virtual const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > raw_iuse_effective_key() const;
                virtual const std::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> > raw_myoptions_key() const;
                virtual const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > raw_use_expand_key() const;
                virtual const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > raw_use_expand_hidden_key() const;
                virtual const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const Choices> > > choices_key() const;
                virtual const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > inherited_key() const;
                virtual const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > defined_phases_key() const;
                virtual const std::shared_ptr<const MetadataSpecTreeKey<LicenseSpecTree> > license_key() const;
                virtual const std::shared_ptr<const MetadataSpecTreeKey<RequiredUseSpecTree> > required_use_key() const;

                virtual const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > behaviours_key() const;

                virtual bool supports_action(const SupportsActionTestBase &) const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual void perform_action(Action &) const;

                virtual bool arbitrary_less_than_comparison(const PackageID &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::size_t extra_hash_value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string fs_location_raw_name() const = 0;
                virtual std::string fs_location_human_name() const = 0;
                virtual std::string contents_filename() const = 0;
                virtual std::shared_ptr<MetadataValueKey<std::shared_ptr<const Contents> > > make_contents_key() const = 0;

                virtual const std::shared_ptr<const ChoiceValue> make_choice_value(
                        const std::shared_ptr<const Choice> &, const UnprefixedChoiceName &, const Tribool,
                        const bool, const bool, const std::string &, const bool) const;

                virtual void add_build_options(const std::shared_ptr<Choices> &) const;

                virtual void purge_invalid_cache() const;
                virtual void can_drop_in_memory_cache() const;
        };
    }
}

#endif
