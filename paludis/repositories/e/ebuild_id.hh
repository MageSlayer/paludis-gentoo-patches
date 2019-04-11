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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_GENTOO_EBUILD_ID_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_GENTOO_EBUILD_ID_HH 1

#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/repositories/e/eapi-fwd.hh>
#include <paludis/repositories/e/e_repository_id.hh>
#include <paludis/util/exception.hh>
#include <time.h>

namespace paludis
{
    class EclassMtimes;

    namespace erepository
    {
        class EbuildID :
            public ERepositoryID,
            public std::enable_shared_from_this<EbuildID>
        {
            private:
                Pimp<EbuildID> _imp;

            protected:
                void need_keys_added() const override;
                void need_non_xml_keys_added() const;
                const std::shared_ptr<const EAPI> presource_eapi() const;
                void need_xml_keys_added() const;

                void need_masks_added() const override;

                void need_behaviours() const;

                const std::shared_ptr<const Map<ChoiceNameWithPrefix, std::string> > choice_descriptions() const;

            public:
                EbuildID(const QualifiedPackageName &, const VersionSpec &,
                        const Environment * const e,
                        const RepositoryName &,
                        const FSPath & file,
                        const std::string & guessed_eapi,
                        const bool eapi_from_suffix,
                        const time_t master_mtime,
                        const std::shared_ptr<const EclassMtimes> & eclass_mtimes);

                ~EbuildID() override;

                const std::string canonical_form(const PackageIDCanonicalForm) const override;
                PackageDepSpec uniquely_identifying_spec() const override;

                const QualifiedPackageName name() const override;
                const VersionSpec version() const override;
                const RepositoryName repository_name() const override;
                bool is_installed() const override PALUDIS_ATTRIBUTE((warn_unused_result));

                const std::shared_ptr<const MetadataValueKey<Slot> > slot_key() const override;
                const std::shared_ptr<const MetadataCollectionKey<KeywordNameSet> > keywords_key() const override;
                const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > build_dependencies_target_key() const override;
                const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > build_dependencies_host_key() const override;
                const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > run_dependencies_key() const override;
                const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > post_dependencies_key() const override;
                const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > dependencies_key() const override;
                const std::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> > fetches_key() const override;
                const std::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> > homepage_key() const override;
                const std::shared_ptr<const MetadataValueKey<std::string> > short_description_key() const override;
                const std::shared_ptr<const MetadataValueKey<std::string> > long_description_key() const override;
                const std::shared_ptr<const MetadataTimeKey> installed_time_key() const override;
                const std::shared_ptr<const MetadataValueKey<FSPath> > fs_location_key() const override;
                const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const Choices> > > choices_key() const override;
                const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > from_repositories_key() const override;

                const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > inherited_key() const override;
                const std::shared_ptr<const MetadataSpecTreeKey<LicenseSpecTree> > license_key() const override;
                const std::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> > restrict_key() const override;
                const std::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> > properties_key() const override;

                const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > raw_use_key() const override;
                const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > raw_iuse_key() const override;
                const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > raw_iuse_effective_key() const override;
                const std::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> > raw_myoptions_key() const override;
                const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > raw_use_expand_key() const override;
                const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > raw_use_expand_hidden_key() const override;
                const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > defined_phases_key() const override;
                const std::shared_ptr<const MetadataSpecTreeKey<RequiredUseSpecTree> > required_use_key() const override;
                const std::shared_ptr<const MetadataValueKey<std::string> > scm_revision_key() const override;

                const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > behaviours_key() const override;

                const std::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> > remote_ids_key() const;
                const std::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> > bugs_to_key() const;
                const std::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> > upstream_changelog_key() const;
                const std::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> > upstream_documentation_key() const;
                const std::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> > upstream_release_notes_key() const;
                const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > generated_from_key() const;
                const std::shared_ptr<const MetadataTimeKey> generated_time_key() const;
                const std::shared_ptr<const MetadataValueKey<std::string> > generated_using_key() const;

                bool arbitrary_less_than_comparison(const PackageID &) const
                    override PALUDIS_ATTRIBUTE((warn_unused_result));

                std::size_t extra_hash_value() const
                    override PALUDIS_ATTRIBUTE((warn_unused_result));

                void set_eapi(const std::string &) const;
                std::string guessed_eapi_name() const;

                void load_short_description(const std::string &, const std::string &, const std::string &) const;
                void load_captured_stdout(const std::string &, const std::string &, const MetadataKeyType, const std::string &) const;
                void load_captured_stderr(const std::string &, const std::string &, const MetadataKeyType, const std::string &) const;
                void load_dependencies(const std::string &, const std::string &, const std::string &) const;
                void load_build_depend_target(const std::string &, const std::string &, const std::string &,
                        const bool rewritten) const;
                void load_build_depend_host(const std::string &, const std::string &, const std::string &,
                        const bool rewritten) const;
                void load_run_depend(const std::string &, const std::string &, const std::string &,
                        const bool rewritten) const;
                void load_post_depend(const std::string &, const std::string &, const std::string &,
                        const bool rewritten) const;
                void load_src_uri(const std::shared_ptr<const EAPIMetadataVariable> &, const std::string &) const;
                void load_homepage(const std::shared_ptr<const EAPIMetadataVariable> &, const std::string &) const;
                void load_license(const std::shared_ptr<const EAPIMetadataVariable> &, const std::string &) const;
                void load_iuse(const std::shared_ptr<const EAPIMetadataVariable> &, const std::string &) const;
                void load_myoptions(const std::shared_ptr<const EAPIMetadataVariable> &, const std::string &) const;
                void load_required_use(const std::shared_ptr<const EAPIMetadataVariable> &, const std::string &) const;
                void load_use(const std::shared_ptr<const EAPIMetadataVariable> &, const std::string &) const;
                void load_inherited(const std::shared_ptr<const EAPIMetadataVariable> &, const std::string &) const;
                void load_keywords(const std::shared_ptr<const EAPIMetadataVariable> &, const std::string &) const;
                void load_restrict(const std::shared_ptr<const EAPIMetadataVariable> & m, const std::string & v) const;
                void load_properties(const std::shared_ptr<const EAPIMetadataVariable> & m, const std::string &) const;
                void load_long_description(const std::string &, const std::string &, const std::string &) const;
                void load_upstream_changelog(const std::shared_ptr<const EAPIMetadataVariable> &, const std::string &) const;
                void load_upstream_documentation(const std::shared_ptr<const EAPIMetadataVariable> &, const std::string &) const;
                void load_upstream_release_notes(const std::shared_ptr<const EAPIMetadataVariable> &, const std::string &) const;
                void load_bugs_to(const std::shared_ptr<const EAPIMetadataVariable> &, const std::string &) const;
                void load_remote_ids(const std::shared_ptr<const EAPIMetadataVariable> &, const std::string &) const;
                void load_defined_phases(const std::shared_ptr<const EAPIMetadataVariable> &, const std::string &) const;
                void load_slot(const std::shared_ptr<const EAPIMetadataVariable> &, const std::string &) const;
                void load_generated_from(const std::shared_ptr<const EAPIMetadataVariable> &, const std::string &) const;
                void load_generated_using(const std::string &, const std::string &, const std::string &) const;
                void load_generated_time(const std::string &, const std::string &, const std::string &) const;
                void load_scm_revision(const std::string &, const std::string &, const std::string &) const;

                bool supports_action(const SupportsActionTestBase &) const override PALUDIS_ATTRIBUTE((warn_unused_result));
                void perform_action(Action &) const override;

                const std::shared_ptr<const EAPI> eapi() const override PALUDIS_ATTRIBUTE((warn_unused_result));

                const std::shared_ptr<const ChoiceValue> make_choice_value(
                        const std::shared_ptr<const Choice> &, const UnprefixedChoiceName &, const Tribool,
                        const bool, const ChoiceOrigin, const std::string &, const bool, const bool) const override;

                void add_build_options(const std::shared_ptr<Choices> &) const override;

                void purge_invalid_cache() const override;

                bool might_be_binary() const;
                bool is_stable() const;

                void set_scm_revision(const std::string &) const override;

                const std::shared_ptr<const Contents> contents() const override;
        };
    }
}

#endif
