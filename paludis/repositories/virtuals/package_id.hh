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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_VIRTUALS_PACKAGE_ID_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_VIRTUALS_PACKAGE_ID_HH 1

#include <paludis/util/pimp.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>

namespace paludis
{
    namespace virtuals
    {
        class VirtualsDepKey :
            public MetadataSpecTreeKey<DependencySpecTree>
        {
            private:
                Pimp<VirtualsDepKey> _imp;

            public:
                VirtualsDepKey(const Environment * const, const std::string &, const std::string &,
                        const std::shared_ptr<const PackageID> &,
                        const std::shared_ptr<const DependenciesLabelSequence> &,
                        const bool);
                ~VirtualsDepKey();

                virtual const std::shared_ptr<const DependencySpecTree> parse_value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::shared_ptr<const DependenciesLabelSequence> initial_labels() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::string pretty_print_value(
                        const PrettyPrinter &,
                        const PrettyPrintOptions &) const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class VirtualsPackageID :
            public PackageID
        {
            private:
                Pimp<VirtualsPackageID> _imp;

            protected:
                virtual void need_keys_added() const;
                virtual void need_masks_added() const;

            public:
                VirtualsPackageID(
                        const Environment * const,
                        const RepositoryName &,
                        const QualifiedPackageName & virtual_name,
                        const std::shared_ptr<const PackageID> & virtual_for,
                        const bool exact);

                virtual ~VirtualsPackageID();

                virtual const std::string canonical_form(const PackageIDCanonicalForm f) const;

                virtual const QualifiedPackageName name() const;
                virtual const VersionSpec version() const;
                virtual const RepositoryName repository_name() const;
                virtual PackageDepSpec uniquely_identifying_spec() const;

                virtual const std::shared_ptr<const MetadataValueKey<SlotName> > slot_key() const;
                virtual const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const PackageID> > > virtual_for_key() const;
                virtual const std::shared_ptr<const MetadataCollectionKey<KeywordNameSet> > keywords_key() const;
                virtual const std::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> > provide_key() const;
                virtual const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > dependencies_key() const;
                virtual const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > build_dependencies_key() const;
                virtual const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > run_dependencies_key() const;
                virtual const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > post_dependencies_key() const;
                virtual const std::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> > fetches_key() const;
                virtual const std::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> > homepage_key() const;
                virtual const std::shared_ptr<const MetadataValueKey<std::string> > short_description_key() const;
                virtual const std::shared_ptr<const MetadataValueKey<std::string> > long_description_key() const;
                virtual const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const Contents> > > contents_key() const;
                virtual const std::shared_ptr<const MetadataTimeKey> installed_time_key() const;
                virtual const std::shared_ptr<const MetadataCollectionKey<PackageIDSequence> > contains_key() const;
                virtual const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const PackageID> > > contained_in_key() const;
                virtual const std::shared_ptr<const MetadataValueKey<FSPath> > fs_location_key() const;
                virtual const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > behaviours_key() const;
                virtual const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > from_repositories_key() const;
                virtual const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const Choices> > > choices_key() const;

                virtual bool supports_action(const SupportsActionTestBase &) const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual void perform_action(Action &) const;

                virtual bool arbitrary_less_than_comparison(const PackageID &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::size_t extra_hash_value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::shared_ptr<const Set<std::string> > breaks_portage() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }
}

#endif
