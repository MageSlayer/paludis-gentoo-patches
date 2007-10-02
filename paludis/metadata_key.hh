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

#ifndef PALUDIS_GUARD_PALUDIS_METADATA_KEY_HH
#define PALUDIS_GUARD_PALUDIS_METADATA_KEY_HH 1

#include <paludis/metadata_key-fwd.hh>
#include <paludis/mask-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/name-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/dep_tree.hh>
#include <paludis/contents-fwd.hh>
#include <paludis/repository-fwd.hh>
#include <paludis/formatter-fwd.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/visitor.hh>
#include <string>

namespace paludis
{
    struct MetadataKeyVisitorTypes :
        VisitorTypes<
            MetadataKeyVisitorTypes,
            MetadataKey,
            MetadataPackageIDKey,
            MetadataSetKey<UseFlagNameSet>,
            MetadataSetKey<IUseFlagSet>,
            MetadataSetKey<KeywordNameSet>,
            MetadataSetKey<Set<std::string> >,
            MetadataSetKey<PackageIDSequence>,
            MetadataSpecTreeKey<DependencySpecTree>,
            MetadataSpecTreeKey<LicenseSpecTree>,
            MetadataSpecTreeKey<FetchableURISpecTree>,
            MetadataSpecTreeKey<SimpleURISpecTree>,
            MetadataSpecTreeKey<ProvideSpecTree>,
            MetadataSpecTreeKey<RestrictSpecTree>,
            MetadataStringKey,
            MetadataContentsKey,
            MetadataTimeKey,
            MetadataRepositoryMaskInfoKey,
            MetadataFSEntryKey
            >
    {
    };

    class PALUDIS_VISIBLE MetadataKey :
        private InstantiationPolicy<MetadataKey, instantiation_method::NonCopyableTag>,
        private PrivateImplementationPattern<MetadataKey>,
        public virtual ConstAcceptInterface<MetadataKeyVisitorTypes>
    {
        protected:
            MetadataKey(const std::string &, const std::string &, const MetadataKeyType);

        public:
            virtual ~MetadataKey() = 0;

            virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE MetadataPackageIDKey :
        public MetadataKey,
        public ConstAcceptInterfaceVisitsThis<MetadataKeyVisitorTypes, MetadataPackageIDKey>
    {
        protected:
            MetadataPackageIDKey(const std::string &, const std::string &, const MetadataKeyType);

        public:
            virtual const tr1::shared_ptr<const PackageID> value() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    class PALUDIS_VISIBLE MetadataStringKey :
        public MetadataKey,
        public ConstAcceptInterfaceVisitsThis<MetadataKeyVisitorTypes, MetadataStringKey>
    {
        protected:
            MetadataStringKey(const std::string &, const std::string &, const MetadataKeyType);

        public:
            virtual const std::string value() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    class PALUDIS_VISIBLE MetadataFSEntryKey :
        public MetadataKey,
        public ConstAcceptInterfaceVisitsThis<MetadataKeyVisitorTypes, MetadataFSEntryKey>
    {
        protected:
            MetadataFSEntryKey(const std::string &, const std::string &, const MetadataKeyType);

        public:
            virtual const FSEntry value() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    class PALUDIS_VISIBLE MetadataTimeKey :
        public MetadataKey,
        public ConstAcceptInterfaceVisitsThis<MetadataKeyVisitorTypes, MetadataTimeKey>
    {
        protected:
            MetadataTimeKey(const std::string &, const std::string &, const MetadataKeyType);

        public:
            virtual const time_t value() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    class PALUDIS_VISIBLE MetadataContentsKey :
        public MetadataKey,
        public ConstAcceptInterfaceVisitsThis<MetadataKeyVisitorTypes, MetadataContentsKey>
    {
        protected:
            MetadataContentsKey(const std::string &, const std::string &, const MetadataKeyType);

        public:
            virtual const tr1::shared_ptr<const Contents> value() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    class PALUDIS_VISIBLE MetadataRepositoryMaskInfoKey :
        public MetadataKey,
        public ConstAcceptInterfaceVisitsThis<MetadataKeyVisitorTypes, MetadataRepositoryMaskInfoKey>
    {
        protected:
            MetadataRepositoryMaskInfoKey(const std::string &, const std::string &, const MetadataKeyType);

        public:
            virtual const tr1::shared_ptr<const RepositoryMaskInfo> value() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    template <typename C_>
    class PALUDIS_VISIBLE MetadataSetKey :
        public MetadataKey,
        public ConstAcceptInterfaceVisitsThis<MetadataKeyVisitorTypes, MetadataSetKey<C_> >
    {
        protected:
            MetadataSetKey(const std::string &, const std::string &, const MetadataKeyType);

        public:
            virtual const tr1::shared_ptr<const C_> value() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual std::string pretty_print_flat(const Formatter<typename C_::value_type> &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    template <>
    class PALUDIS_VISIBLE MetadataSetKey<IUseFlagSet> :
        public MetadataKey,
        public ConstAcceptInterfaceVisitsThis<MetadataKeyVisitorTypes, MetadataSetKey<IUseFlagSet> >
    {
        protected:
            MetadataSetKey(const std::string &, const std::string &, const MetadataKeyType);

        public:
            virtual const tr1::shared_ptr<const IUseFlagSet> value() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual std::string pretty_print_flat(const Formatter<IUseFlag> &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual std::string pretty_print_flat_with_comparison(
                    const Environment * const,
                    const tr1::shared_ptr<const PackageID> &,
                    const Formatter<IUseFlag> &
                    ) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    template <typename C_>
    class PALUDIS_VISIBLE MetadataSpecTreeKey :
        public MetadataKey,
        public ConstAcceptInterfaceVisitsThis<MetadataKeyVisitorTypes, MetadataSpecTreeKey<C_> >
    {
        protected:
            MetadataSpecTreeKey(const std::string &, const std::string &, const MetadataKeyType);

        public:
            virtual const tr1::shared_ptr<const typename C_::ConstItem> value() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual std::string pretty_print(const typename C_::Formatter &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual std::string pretty_print_flat(const typename C_::Formatter &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    template <>
    class PALUDIS_VISIBLE MetadataSpecTreeKey<FetchableURISpecTree> :
        public MetadataKey,
        public ConstAcceptInterfaceVisitsThis<MetadataKeyVisitorTypes, MetadataSpecTreeKey<FetchableURISpecTree> >
    {
        protected:
            MetadataSpecTreeKey(const std::string &, const std::string &, const MetadataKeyType);

        public:
            virtual const tr1::shared_ptr<const FetchableURISpecTree::ConstItem> value() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual std::string pretty_print(const FetchableURISpecTree::Formatter &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual std::string pretty_print_flat(const FetchableURISpecTree::Formatter &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual const tr1::shared_ptr<const URILabel> initial_label() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };
}

#endif
