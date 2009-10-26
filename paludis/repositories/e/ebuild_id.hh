/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_GENTOO_EBUILD_ID_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_GENTOO_EBUILD_ID_HH 1

#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <paludis/repositories/e/eapi-fwd.hh>
#include <paludis/repositories/e/e_repository_id.hh>

namespace paludis
{
    class ERepository;
    class EclassMtimes;

    namespace erepository
    {
        class EbuildID :
            public ERepositoryID,
            public std::tr1::enable_shared_from_this<EbuildID>,
            private PrivateImplementationPattern<EbuildID>
        {
            private:
                PrivateImplementationPattern<EbuildID>::ImpPtr & _imp;

            protected:
                virtual void need_keys_added() const;
                virtual void need_masks_added() const;

            public:
                EbuildID(const QualifiedPackageName &, const VersionSpec &,
                        const Environment * const e,
                        const std::tr1::shared_ptr<const ERepository> &,
                        const FSEntry & file,
                        const std::string & guessed_eapi,
                        const time_t master_mtime,
                        const std::tr1::shared_ptr<const EclassMtimes> & eclass_mtimes);

                ~EbuildID();

                virtual const std::string canonical_form(const PackageIDCanonicalForm) const;
                virtual PackageDepSpec uniquely_identifying_spec() const;

                virtual const QualifiedPackageName name() const;
                virtual const VersionSpec version() const;
                virtual const std::tr1::shared_ptr<const Repository> repository() const;

                virtual const std::tr1::shared_ptr<const MetadataValueKey<SlotName> > slot_key() const;
                virtual const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > > virtual_for_key() const;
                virtual const std::tr1::shared_ptr<const MetadataCollectionKey<KeywordNameSet> > keywords_key() const;
                virtual const std::tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> > provide_key() const;
                virtual const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > build_dependencies_key() const;
                virtual const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > run_dependencies_key() const;
                virtual const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > post_dependencies_key() const;
                virtual const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > suggested_dependencies_key() const;
                virtual const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > dependencies_key() const;
                virtual const std::tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> > fetches_key() const;
                virtual const std::tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> > homepage_key() const;
                virtual const std::tr1::shared_ptr<const MetadataValueKey<std::string> > short_description_key() const;
                virtual const std::tr1::shared_ptr<const MetadataValueKey<std::string> > long_description_key() const;
                virtual const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Contents> > > contents_key() const;
                virtual const std::tr1::shared_ptr<const MetadataTimeKey> installed_time_key() const;
                virtual const std::tr1::shared_ptr<const MetadataCollectionKey<PackageIDSequence> > contains_key() const;
                virtual const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > > contained_in_key() const;
                virtual const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > fs_location_key() const;
                virtual const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Choices> > > choices_key() const;
                const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > > from_repositories_key() const;

                const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > > inherited_key() const;
                const std::tr1::shared_ptr<const MetadataSpecTreeKey<LicenseSpecTree> > license_key() const;
                virtual const std::tr1::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> > restrict_key() const;
                virtual const std::tr1::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> > properties_key() const;

                virtual const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > > raw_use_key() const;
                virtual const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > > raw_iuse_key() const;
                virtual const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > > raw_iuse_effective_key() const;
                virtual const std::tr1::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> > raw_myoptions_key() const;
                virtual const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > > raw_use_expand_key() const;
                virtual const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > > raw_use_expand_hidden_key() const;
                virtual const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > > defined_phases_key() const;

                virtual const std::tr1::shared_ptr<const MetadataValueKey<bool> > transient_key() const;

                const std::tr1::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> > remote_ids_key() const;
                const std::tr1::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> > bugs_to_key() const;
                const std::tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> > upstream_changelog_key() const;
                const std::tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> > upstream_documentation_key() const;
                const std::tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> > upstream_release_notes_key() const;

                virtual bool arbitrary_less_than_comparison(const PackageID &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::size_t extra_hash_value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::tr1::shared_ptr<const ERepository> e_repository() const;

                void set_eapi(const std::string &) const;
                std::string guessed_eapi_name() const;

                void load_short_description(const std::string &, const std::string &, const std::string &) const;
                void load_captured_stderr(const std::string &, const std::string &, const MetadataKeyType, const std::string &) const;
                void load_raw_depend(const std::string &, const std::string &, const std::string &) const;
                void load_build_depend(const std::string &, const std::string &, const std::string &,
                        const bool rewritten) const;
                void load_run_depend(const std::string &, const std::string &, const std::string &,
                        const bool rewritten) const;
                void load_post_depend(const std::string &, const std::string &, const std::string &,
                        const bool rewritten) const;
                void load_src_uri(const std::tr1::shared_ptr<const EAPIMetadataVariable> &, const std::string &) const;
                void load_homepage(const std::string &, const std::string &, const std::string &) const;
                void load_license(const std::tr1::shared_ptr<const EAPIMetadataVariable> &, const std::string &) const;
                void load_provide(const std::string &, const std::string &, const std::string &) const;
                void load_iuse(const std::string &, const std::string &, const std::string &) const;
                void load_myoptions(const std::string &, const std::string &, const std::string &) const;
                void load_use(const std::string &, const std::string &, const std::string &) const;
                void load_inherited(const std::string &, const std::string &, const std::string &) const;
                void load_keywords(const std::string &, const std::string &, const std::string &) const;
                void load_restrict(const std::tr1::shared_ptr<const EAPIMetadataVariable> & m, const std::string & v) const;
                void load_properties(const std::tr1::shared_ptr<const EAPIMetadataVariable> & m, const std::string &) const;
                void load_long_description(const std::string &, const std::string &, const std::string &) const;
                void load_upstream_changelog(const std::string &, const std::string &, const std::string &) const;
                void load_upstream_documentation(const std::string &, const std::string &, const std::string &) const;
                void load_upstream_release_notes(const std::string &, const std::string &, const std::string &) const;
                void load_bugs_to(const std::tr1::shared_ptr<const EAPIMetadataVariable> &, const std::string &) const;
                void load_remote_ids(const std::tr1::shared_ptr<const EAPIMetadataVariable> &, const std::string &) const;
                void load_defined_phases(const std::string &, const std::string &, const std::string &) const;
                void load_slot(const std::tr1::shared_ptr<const EAPIMetadataVariable> &, const std::string &) const;

                virtual bool supports_action(const SupportsActionTestBase &) const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual void perform_action(Action &) const;

                virtual const std::tr1::shared_ptr<const EAPI> eapi() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual void invalidate_masks() const;

                virtual std::tr1::shared_ptr<ChoiceValue> make_choice_value(
                        const std::tr1::shared_ptr<const Choice> &, const UnprefixedChoiceName &, const Tribool,
                        const bool, const std::string &, const bool) const;

                virtual void add_build_options(const std::tr1::shared_ptr<Choices> &) const;

                virtual void purge_invalid_cache() const;
                virtual void can_drop_in_memory_cache() const;
        };
    }
}

#endif
