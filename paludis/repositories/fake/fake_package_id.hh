/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_FAKE_FAKE_PACKAGE_ID_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_FAKE_FAKE_PACKAGE_ID_HH 1

#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/mask.hh>
#include <paludis/util/set.hh>
#include <tr1/functional>

namespace paludis
{
    class FakeRepositoryBase;

    template <typename C_>
    class PALUDIS_VISIBLE FakeMetadataValueKey :
        public MetadataValueKey<C_>,
        private PrivateImplementationPattern<FakeMetadataValueKey<C_> >
    {
        protected:
            typename PrivateImplementationPattern<FakeMetadataValueKey<C_> >::ImpPtr & _imp;

        public:
            FakeMetadataValueKey(const std::string &, const std::string &, const MetadataKeyType,
                    const C_ &);

            ~FakeMetadataValueKey();

            virtual const C_ value() const PALUDIS_ATTRIBUTE((warn_unused_result));
            void set_value(const C_ &);

            virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::string pretty_print() const
                PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    template <typename C_>
    class PALUDIS_VISIBLE FakeMetadataCollectionKey :
        public MetadataCollectionKey<C_>,
        private PrivateImplementationPattern<FakeMetadataCollectionKey<C_> >
    {
        protected:
            typename PrivateImplementationPattern<FakeMetadataCollectionKey<C_> >::ImpPtr & _imp;

            FakeMetadataCollectionKey(const std::string &, const std::string &, const MetadataKeyType,
                    const PackageID * const, const Environment * const);

        public:
            ~FakeMetadataCollectionKey();

            virtual const std::tr1::shared_ptr<const C_> value() const PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE FakeMetadataKeywordSetKey :
        public FakeMetadataCollectionKey<KeywordNameSet>
    {
        public:
            FakeMetadataKeywordSetKey(const std::string &, const std::string &, const std::string &, const MetadataKeyType,
                    const PackageID * const, const Environment * const);

            void set_from_string(const std::string &);

            virtual std::string pretty_print_flat(const Formatter<KeywordName> &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    template <typename C_>
    class PALUDIS_VISIBLE FakeMetadataSpecTreeKey :
        public MetadataSpecTreeKey<C_>,
        private PrivateImplementationPattern<FakeMetadataSpecTreeKey<C_> >
    {
        private:
            typename PrivateImplementationPattern<FakeMetadataSpecTreeKey<C_> >::ImpPtr & _imp;

        public:
            FakeMetadataSpecTreeKey(const std::string &, const std::string &, const std::string &,
                    const std::tr1::function<const std::tr1::shared_ptr<const C_> (const std::string &)> &, const MetadataKeyType);
            ~FakeMetadataSpecTreeKey();

            virtual const std::tr1::shared_ptr<const C_> value() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            void set_from_string(const std::string &);

            virtual std::string pretty_print(const typename C_::ItemFormatter &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::string pretty_print_flat(const typename C_::ItemFormatter &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    template <>
    class PALUDIS_VISIBLE FakeMetadataSpecTreeKey<FetchableURISpecTree> :
        public MetadataSpecTreeKey<FetchableURISpecTree>,
        private PrivateImplementationPattern<FakeMetadataSpecTreeKey<FetchableURISpecTree> >
    {
        private:
            PrivateImplementationPattern<FakeMetadataSpecTreeKey<FetchableURISpecTree> >::ImpPtr & _imp;

        public:
            FakeMetadataSpecTreeKey(const std::string &, const std::string &, const std::string &,
                    const std::tr1::function<const std::tr1::shared_ptr<const FetchableURISpecTree> (const std::string &)> &,
                    const MetadataKeyType);
            ~FakeMetadataSpecTreeKey();

            virtual const std::tr1::shared_ptr<const FetchableURISpecTree> value() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            void set_from_string(const std::string &);

            virtual std::string pretty_print(const FetchableURISpecTree::ItemFormatter &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::string pretty_print_flat(const FetchableURISpecTree::ItemFormatter &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const std::tr1::shared_ptr<const URILabel> initial_label() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    template <>
    class PALUDIS_VISIBLE FakeMetadataSpecTreeKey<DependencySpecTree> :
        public MetadataSpecTreeKey<DependencySpecTree>,
        private PrivateImplementationPattern<FakeMetadataSpecTreeKey<DependencySpecTree> >
    {
        private:
            PrivateImplementationPattern<FakeMetadataSpecTreeKey<DependencySpecTree> >::ImpPtr & _imp;

        public:
            FakeMetadataSpecTreeKey(const std::string &, const std::string &, const std::string &,
                    const std::tr1::function<const std::tr1::shared_ptr<const DependencySpecTree> (const std::string &)> &,
                    const std::tr1::shared_ptr<const DependenciesLabelSequence> &,
                    const MetadataKeyType);
            ~FakeMetadataSpecTreeKey();

            virtual const std::tr1::shared_ptr<const DependencySpecTree> value() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            void set_from_string(const std::string &);

            virtual std::string pretty_print(const DependencySpecTree::ItemFormatter &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::string pretty_print_flat(const DependencySpecTree::ItemFormatter &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const std::tr1::shared_ptr<const DependenciesLabelSequence> initial_labels() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE FakeMetadataChoicesKey :
        public MetadataValueKey<std::tr1::shared_ptr<const Choices> >,
        private PrivateImplementationPattern<FakeMetadataChoicesKey>
    {
        private:
            PrivateImplementationPattern<FakeMetadataChoicesKey>::ImpPtr & _imp;

        public:
            FakeMetadataChoicesKey(
                    const Environment * const,
                    const std::tr1::shared_ptr<const PackageID> &);
            ~FakeMetadataChoicesKey();

            void add(const std::string &, const std::string &);
            const std::tr1::shared_ptr<const Choices> value() const PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE FakeUnacceptedMask :
        public UnacceptedMask,
        private PrivateImplementationPattern<FakeUnacceptedMask>
    {
        public:
            FakeUnacceptedMask(const char, const std::string &, const std::tr1::shared_ptr<const MetadataKey> &);
            ~FakeUnacceptedMask();

            char key() const;
            const std::string description() const;
            const std::tr1::shared_ptr<const MetadataKey> unaccepted_key() const;
    };

    class PALUDIS_VISIBLE FakeUnsupportedMask :
        public UnsupportedMask
    {
        public:
            FakeUnsupportedMask();
            ~FakeUnsupportedMask();

            char key() const;
            const std::string description() const;
            const std::string explanation() const;
    };

    /**
     * A PackageID in a FakeRepository or a FakeInstalledRepository.
     *
     * Various keys can be modified.
     *
     * \ingroup g_fake_repository
     * \since 0.26
     */
    class PALUDIS_VISIBLE FakePackageID :
        public PackageID,
        private PrivateImplementationPattern<FakePackageID>,
        public std::tr1::enable_shared_from_this<FakePackageID>
    {
        private:
            PrivateImplementationPattern<FakePackageID>::ImpPtr & _imp;

        protected:
            virtual void need_keys_added() const;
            virtual void need_masks_added() const;

        public:
            ///\name Basic operations
            ///\{

            FakePackageID(const Environment * const e,
                    const std::tr1::shared_ptr<const FakeRepositoryBase> &,
                    const QualifiedPackageName &, const VersionSpec &);
            ~FakePackageID();

            ///\}

            virtual const std::string canonical_form(const PackageIDCanonicalForm) const;

            virtual const QualifiedPackageName name() const;
            virtual const VersionSpec version() const;
            virtual const std::tr1::shared_ptr<const Repository> repository() const;
            virtual PackageDepSpec uniquely_identifying_spec() const;

            virtual const std::tr1::shared_ptr<const MetadataValueKey<SlotName> > slot_key() const;
            virtual const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > > virtual_for_key() const;
            virtual const std::tr1::shared_ptr<const MetadataCollectionKey<KeywordNameSet> > keywords_key() const;
            virtual const std::tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> > provide_key() const;
            virtual const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > dependencies_key() const;
            virtual const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > build_dependencies_key() const;
            virtual const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > run_dependencies_key() const;
            virtual const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > post_dependencies_key() const;
            virtual const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > suggested_dependencies_key() const;
            virtual const std::tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> > fetches_key() const;
            virtual const std::tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> > homepage_key() const;
            virtual const std::tr1::shared_ptr<const MetadataValueKey<std::string> > short_description_key() const;
            virtual const std::tr1::shared_ptr<const MetadataValueKey<std::string> > long_description_key() const;
            virtual const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Contents> > > contents_key() const;
            virtual const std::tr1::shared_ptr<const MetadataTimeKey> installed_time_key() const;
            virtual const std::tr1::shared_ptr<const MetadataCollectionKey<PackageIDSequence> > contains_key() const;
            virtual const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > > contained_in_key() const;
            virtual const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > fs_location_key() const;
            virtual const std::tr1::shared_ptr<const MetadataValueKey<long> > size_of_download_required_key() const;
            virtual const std::tr1::shared_ptr<const MetadataValueKey<long> > size_of_all_distfiles_key() const;
            virtual const std::tr1::shared_ptr<const MetadataValueKey<bool> > transient_key() const;
            virtual const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > > from_repositories_key() const;
            virtual const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Choices> > > choices_key() const;

            const std::tr1::shared_ptr<const MetadataSpecTreeKey<LicenseSpecTree> > license_key() const;

            ///\name Modifiable keys
            ///\{

            const std::tr1::shared_ptr<FakeMetadataKeywordSetKey> keywords_key();
            const std::tr1::shared_ptr<FakeMetadataSpecTreeKey<ProvideSpecTree> > provide_key();
            const std::tr1::shared_ptr<FakeMetadataSpecTreeKey<DependencySpecTree> > build_dependencies_key();
            const std::tr1::shared_ptr<FakeMetadataSpecTreeKey<DependencySpecTree> > run_dependencies_key();
            const std::tr1::shared_ptr<FakeMetadataSpecTreeKey<DependencySpecTree> > post_dependencies_key();
            const std::tr1::shared_ptr<FakeMetadataSpecTreeKey<DependencySpecTree> > suggested_dependencies_key();
            const std::tr1::shared_ptr<FakeMetadataSpecTreeKey<FetchableURISpecTree> > fetches_key();
            const std::tr1::shared_ptr<FakeMetadataSpecTreeKey<SimpleURISpecTree> > homepage_key();
            const std::tr1::shared_ptr<FakeMetadataChoicesKey> choices_key();
            const std::tr1::shared_ptr<FakeMetadataValueKey<bool> > transient_key();
            const std::tr1::shared_ptr<FakeMetadataValueKey<long> > hitchhiker_key();

            void set_slot(const SlotName &);

            ///\}

            char use_expand_separator() const;

            virtual bool arbitrary_less_than_comparison(const PackageID &) const;
            virtual std::size_t extra_hash_value() const;

            virtual bool supports_action(const SupportsActionTestBase &) const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual void perform_action(Action &) const;

            virtual std::tr1::shared_ptr<const Set<std::string> > breaks_portage() const PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual void invalidate_masks() const;

            void make_unsupported();

    };
}

#endif
