/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/util/tr1_functional.hh>

namespace paludis
{
    class FakeRepositoryBase;

    template <typename C_>
    class PALUDIS_VISIBLE FakeMetadataSetKey :
        public MetadataSetKey<C_>,
        private PrivateImplementationPattern<FakeMetadataSetKey<C_> >
    {
        protected:
            Implementation<FakeMetadataSetKey> * const _imp;

            FakeMetadataSetKey(const std::string &, const std::string &, const MetadataKeyType);

        public:
            ~FakeMetadataSetKey();

            virtual const tr1::shared_ptr<const C_> value() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE FakeMetadataKeywordSetKey :
        public FakeMetadataSetKey<KeywordNameSet>
    {
        public:
            FakeMetadataKeywordSetKey(const std::string &, const std::string &, const std::string &, const MetadataKeyType);

            void set_from_string(const std::string &);
    };

    class PALUDIS_VISIBLE FakeMetadataIUseSetKey :
        public FakeMetadataSetKey<IUseFlagSet>
    {
        public:
            FakeMetadataIUseSetKey(const std::string &, const std::string &, const std::string &, const IUseFlagParseMode,
                    const MetadataKeyType);

            void set_from_string(const std::string &, const IUseFlagParseMode);
    };

    template <typename C_>
    class PALUDIS_VISIBLE FakeMetadataSpecTreeKey :
        public MetadataSpecTreeKey<C_>,
        private PrivateImplementationPattern<FakeMetadataSpecTreeKey<C_> >
    {
        private:
            Implementation<FakeMetadataSpecTreeKey<C_> > * const _imp;

        public:
            FakeMetadataSpecTreeKey(const std::string &, const std::string &, const std::string &,
                    const tr1::function<const tr1::shared_ptr<const typename C_::ConstItem> (const std::string &)> &, const MetadataKeyType);
            ~FakeMetadataSpecTreeKey();

            virtual const tr1::shared_ptr<const typename C_::ConstItem> value() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            void set_from_string(const std::string &);
    };

    class PALUDIS_VISIBLE FakeMetadataPackageIDKey :
        public MetadataPackageIDKey,
        private PrivateImplementationPattern<FakeMetadataPackageIDKey>
    {
        private:
            Implementation<FakeMetadataPackageIDKey> * const _imp;

        public:
            FakeMetadataPackageIDKey(const std::string &, const std::string &,
                    const tr1::shared_ptr<const PackageID> &, const MetadataKeyType);
            ~FakeMetadataPackageIDKey();

            virtual const tr1::shared_ptr<const PackageID> value() const
                PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE FakeUnacceptedMask :
        public UnacceptedMask,
        private PrivateImplementationPattern<FakeUnacceptedMask>
    {
        public:
            FakeUnacceptedMask(const char, const std::string &, const tr1::shared_ptr<const MetadataKey> &);
            ~FakeUnacceptedMask();

            const char key() const;
            const std::string description() const;
            const tr1::shared_ptr<const MetadataKey> unaccepted_key() const;
    };

    class PALUDIS_VISIBLE FakeUnsupportedMask :
        public UnsupportedMask,
        private PrivateImplementationPattern<FakeUnsupportedMask>
    {
        public:
            FakeUnsupportedMask(const char, const std::string &, const std::string &);
            ~FakeUnsupportedMask();

            const char key() const;
            const std::string description() const;
            const std::string explanation() const;
    };

    class PALUDIS_VISIBLE FakePackageID :
        public PackageID,
        private PrivateImplementationPattern<FakePackageID>
    {
        private:
            Implementation<FakePackageID> * const _imp;

        protected:
            virtual void need_keys_added() const;
            virtual void need_masks_added() const;

        public:
            FakePackageID(const Environment * const e,
                    const tr1::shared_ptr<const FakeRepositoryBase> &,
                    const QualifiedPackageName &, const VersionSpec &);
            ~FakePackageID();

            virtual const std::string canonical_form(const PackageIDCanonicalForm) const;

            virtual const QualifiedPackageName name() const;
            virtual const VersionSpec version() const;
            virtual const SlotName slot() const;
            virtual const tr1::shared_ptr<const Repository> repository() const;
            virtual const tr1::shared_ptr<const EAPI> eapi() const;

            virtual const tr1::shared_ptr<const MetadataPackageIDKey> virtual_for_key() const;
            virtual const tr1::shared_ptr<const MetadataSetKey<KeywordNameSet> > keywords_key() const;
            virtual const tr1::shared_ptr<const MetadataSetKey<UseFlagNameSet> > use_key() const;
            virtual const tr1::shared_ptr<const MetadataSetKey<IUseFlagSet> > iuse_key() const;
            virtual const tr1::shared_ptr<const MetadataSetKey<InheritedSet> > inherited_key() const;
            virtual const tr1::shared_ptr<const MetadataSpecTreeKey<LicenseSpecTree> > license_key() const;
            virtual const tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> > provide_key() const;
            virtual const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > build_dependencies_key() const;
            virtual const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > run_dependencies_key() const;
            virtual const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > post_dependencies_key() const;
            virtual const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > suggested_dependencies_key() const;
            virtual const tr1::shared_ptr<const MetadataSpecTreeKey<URISpecTree> > src_uri_key() const;
            virtual const tr1::shared_ptr<const MetadataSpecTreeKey<URISpecTree> > bin_uri_key() const;
            virtual const tr1::shared_ptr<const MetadataSpecTreeKey<URISpecTree> > homepage_key() const;
            virtual const tr1::shared_ptr<const MetadataSpecTreeKey<RestrictSpecTree> > restrict_key() const;
            virtual const tr1::shared_ptr<const MetadataStringKey> short_description_key() const;
            virtual const tr1::shared_ptr<const MetadataStringKey> long_description_key() const;
            virtual const tr1::shared_ptr<const MetadataContentsKey> contents_key() const;
            virtual const tr1::shared_ptr<const MetadataTimeKey> installed_time_key() const;
            virtual const tr1::shared_ptr<const MetadataStringKey> source_origin_key() const;
            virtual const tr1::shared_ptr<const MetadataStringKey> binary_origin_key() const;

            const tr1::shared_ptr<FakeMetadataKeywordSetKey> keywords_key();
            const tr1::shared_ptr<FakeMetadataIUseSetKey> iuse_key();

            const tr1::shared_ptr<FakeMetadataSpecTreeKey<ProvideSpecTree> > provide_key();
            const tr1::shared_ptr<FakeMetadataSpecTreeKey<DependencySpecTree> > build_dependencies_key();
            const tr1::shared_ptr<FakeMetadataSpecTreeKey<DependencySpecTree> > run_dependencies_key();
            const tr1::shared_ptr<FakeMetadataSpecTreeKey<DependencySpecTree> > post_dependencies_key();
            const tr1::shared_ptr<FakeMetadataSpecTreeKey<DependencySpecTree> > suggested_dependencies_key();

            void set_slot(const SlotName &);

            virtual bool arbitrary_less_than_comparison(const PackageID &) const;
            virtual std::size_t extra_hash_value() const;

            virtual bool supports_action(const SupportsActionTestBase &) const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual void perform_action(Action &) const;

    };
}

#endif
