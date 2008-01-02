/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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
#include <paludis/util/remove_shared_ptr.hh>
#include <paludis/util/tr1_type_traits.hh>
#include <string>

/** \file
 * Declarations for metadata key classes.
 *
 * \ingroup g_metadata_key
 *
 * \section Examples
 *
 * - \ref example_metadata_key.cc "example_metadata_key.cc" (for metadata keys)
 */

namespace paludis
{
    /**
     * Types for a visitor that can visit a MetadataKey subclass.
     *
     * \ingroup g_metadata_key
     * \since 0.26
     * \nosubgrouping
     */
    struct MetadataKeyVisitorTypes :
        VisitorTypes<
            MetadataKeyVisitorTypes,
            MetadataKey,
            MetadataPackageIDKey,
            MetadataCollectionKey<UseFlagNameSet>,
            MetadataCollectionKey<IUseFlagSet>,
            MetadataCollectionKey<KeywordNameSet>,
            MetadataCollectionKey<Set<std::string> >,
            MetadataCollectionKey<PackageIDSequence>,
            MetadataCollectionKey<FSEntrySequence>,
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
            MetadataFSEntryKey,
            MetadataSectionKey
            >
    {
    };

    /**
     * A MetadataKey is a generic key that contains a particular piece of
     * information about a PackageID or Repository instance.
     *
     * A basic MetadataKey has:
     *
     * - A raw name. This is in a repository-defined format designed to closely
     *   represent the internal name. For example, ebuilds and VDB IDs use
     *   raw names like 'DESCRIPTION' and 'KEYWORDS', whereas CRAN uses names
     *   like 'Title' and 'BundleDescription'. The raw name is unique in a
     *   PackageID or Repository.
     *
     * - A human name. This is the name that should be used when outputting
     *   normally for a human to read.
     *
     * - A MetadataKeyType. This is a hint to clients as to whether the key
     *   should be displayed when outputting information about a package ID
     *   or Repository.
     *
     * Subclasses provide additional information, including the 'value' of the
     * key. A ConstVisitor using MetadataKeyVisitorTypes can be used to get more
     * detail.
     *
     * The header \ref paludis/literal_metadata_key.hh "literal_metadata_key.hh"
     * contains various concrete implementations of MetadataKey subclasses.
     *
     * \ingroup g_metadata_key
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE MetadataKey :
        private InstantiationPolicy<MetadataKey, instantiation_method::NonCopyableTag>,
        private PrivateImplementationPattern<MetadataKey>,
        public virtual ConstAcceptInterface<MetadataKeyVisitorTypes>
    {
        protected:
            ///\name Basic operations
            ///\{

            MetadataKey(const std::string & raw_name, const std::string & human_name, const MetadataKeyType);

        public:
            virtual ~MetadataKey() = 0;

            ///\}

            /**
             * Fetch our raw name.
             */
            virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Fetch our human name.
             */
            virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Fetch our key type.
             */
            virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * A MetadataPackageIDKey is a MetadataKey that has a PackageID as its
     * value.
     *
     * \ingroup g_metadata_key
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE MetadataPackageIDKey :
        public MetadataKey,
        public ConstAcceptInterfaceVisitsThis<MetadataKeyVisitorTypes, MetadataPackageIDKey>
    {
        protected:
            ///\name Basic operations
            ///\{

            MetadataPackageIDKey(const std::string &, const std::string &, const MetadataKeyType);

            ///\}

        public:
            /**
             * Fetch our value.
             */
            virtual const tr1::shared_ptr<const PackageID> value() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * A MetadataStringKey is a MetadataKey that has a std::string as its
     * value.
     *
     * \ingroup g_metadata_key
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE MetadataStringKey :
        public MetadataKey,
        public ConstAcceptInterfaceVisitsThis<MetadataKeyVisitorTypes, MetadataStringKey>
    {
        protected:
            ///\name Basic operations
            ///\{

            MetadataStringKey(const std::string &, const std::string &, const MetadataKeyType);

            ///\}

        public:
            /**
             * Fetch our value.
             */
            virtual const std::string value() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * A MetadataFSEntryKey is a MetadataKey that has an FSEntry as its value.
     *
     * \ingroup g_metadata_key
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE MetadataFSEntryKey :
        public MetadataKey,
        public ConstAcceptInterfaceVisitsThis<MetadataKeyVisitorTypes, MetadataFSEntryKey>
    {
        protected:
            ///\name Basic operations
            ///\{

            MetadataFSEntryKey(const std::string &, const std::string &, const MetadataKeyType);

            ///\}

        public:
            /**
             * Fetch our value.
             */
            virtual const FSEntry value() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * A MetadataTimeKey is a MetadataKey that has a time_t as its value.
     *
     * \ingroup g_metadata_key
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE MetadataTimeKey :
        public MetadataKey,
        public ConstAcceptInterfaceVisitsThis<MetadataKeyVisitorTypes, MetadataTimeKey>
    {
        protected:
            ///\name Basic operations
            ///\{

            MetadataTimeKey(const std::string &, const std::string &, const MetadataKeyType);

            ///\}

        public:
            /**
             * Fetch our value.
             */
            virtual time_t value() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * A MetadataContentsKey is a MetadataKey that holds a Contents heirarchy.
     *
     * \ingroup g_metadata_key
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE MetadataContentsKey :
        public MetadataKey,
        public ConstAcceptInterfaceVisitsThis<MetadataKeyVisitorTypes, MetadataContentsKey>
    {
        protected:
            ///\name Basic operations
            ///\{

            MetadataContentsKey(const std::string &, const std::string &, const MetadataKeyType);

            ///\}

        public:
            /**
             * Fetch our value.
             */
            virtual const tr1::shared_ptr<const Contents> value() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * A MetadataRepositoryMaskInfoKey is a MetadataKey that holds
     * RepositoryMaskInfo as its value.
     *
     * \ingroup g_metadata_key
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE MetadataRepositoryMaskInfoKey :
        public MetadataKey,
        public ConstAcceptInterfaceVisitsThis<MetadataKeyVisitorTypes, MetadataRepositoryMaskInfoKey>
    {
        protected:
            ///\name Basic operations
            ///\{

            MetadataRepositoryMaskInfoKey(const std::string &, const std::string &, const MetadataKeyType);

            ///\}

        public:
            /**
             * Fetch our value.
             */
            virtual const tr1::shared_ptr<const RepositoryMaskInfo> value() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * A MetadataSectionKey is a MetadataKey that holds a number of other
     * MetadataKey instances.
     *
     * \ingroup g_metadata_key
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE MetadataSectionKey :
        public MetadataKey,
        public ConstAcceptInterfaceVisitsThis<MetadataKeyVisitorTypes, MetadataSectionKey>,
        private PrivateImplementationPattern<MetadataSectionKey>
    {
        private:
            PrivateImplementationPattern<MetadataSectionKey>::ImpPtr & _imp;

        protected:
            ///\name Basic operations
            ///\{

            MetadataSectionKey(const std::string &, const std::string &, const MetadataKeyType);

            ///\}

            /**
             * Add a new MetadataKey, which must not use the same raw name as
             * any previous MetadataKey added to this key.
             */
            virtual void add_metadata_key(const tr1::shared_ptr<const MetadataKey> &) const;

            /**
             * This method will be called before any of the metadata key
             * iteration methods does its work. It can be used by subclasses to
             * implement as-needed loading of keys.
             */
            virtual void need_keys_added() const = 0;

        public:
            ///\name Basic operations
            ///\{

            virtual ~MetadataSectionKey();

            ///\}

            ///\name Finding and iterating over metadata keys
            ///\{

            struct MetadataConstIteratorTag;
            typedef WrappedForwardIterator<MetadataConstIteratorTag, tr1::shared_ptr<const MetadataKey> > MetadataConstIterator;

            MetadataConstIterator begin_metadata() const PALUDIS_ATTRIBUTE((warn_unused_result));
            MetadataConstIterator end_metadata() const PALUDIS_ATTRIBUTE((warn_unused_result));
            MetadataConstIterator find_metadata(const std::string &) const PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}
    };

    /**
     * A MetadataCollectionKey is a MetadataKey that holds a Set of some kind of item
     * as its value.
     *
     * \ingroup g_metadata_key
     * \since 0.26
     * \nosubgrouping
     */
    template <typename C_>
    class PALUDIS_VISIBLE MetadataCollectionKey :
        public MetadataKey,
        public ConstAcceptInterfaceVisitsThis<MetadataKeyVisitorTypes, MetadataCollectionKey<C_> >
    {
        protected:
            ///\name Basic operations
            ///\{

            MetadataCollectionKey(const std::string &, const std::string &, const MetadataKeyType);

            ///\}

        public:
            /**
             * Fetch our value.
             */
            virtual const tr1::shared_ptr<const C_> value() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a single-line formatted version of our value, using the
             * supplied Formatter to format individual items.
             */
            virtual std::string pretty_print_flat(const Formatter<
                    typename tr1::remove_const<typename RemoveSharedPtr<typename C_::value_type>::Type>::type> &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * A MetadataCollectionKey<IUseFlagSet> is a MetadataKey that holds an IUseFlagSet
     * as its value.
     *
     * This specialisation of MetadataCollectionKey provides an additional
     * pretty_print_flat_with_comparison method.
     *
     * \ingroup g_metadata_key
     * \since 0.26
     * \nosubgrouping
     */
    template <>
    class PALUDIS_VISIBLE MetadataCollectionKey<IUseFlagSet> :
        public MetadataKey,
        public ConstAcceptInterfaceVisitsThis<MetadataKeyVisitorTypes, MetadataCollectionKey<IUseFlagSet> >
    {
        protected:
            ///\name Basic operations
            ///\{

            MetadataCollectionKey(const std::string &, const std::string &, const MetadataKeyType);

            ///\}

        public:
            /**
             * Fetch our value.
             */
            virtual const tr1::shared_ptr<const IUseFlagSet> value() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a single-line formatted version of our value, using the
             * supplied Formatter to format individual items.
             */
            virtual std::string pretty_print_flat(const Formatter<IUseFlag> &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a single-line formatted version of our value, using the
             * supplied Formatter to format individual items, and the supplied
             * PackageID to decorate using format::Added and format::Changed as
             * appropriate.
             */
            virtual std::string pretty_print_flat_with_comparison(
                    const Environment * const,
                    const tr1::shared_ptr<const PackageID> &,
                    const Formatter<IUseFlag> &
                    ) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * A MetadataSpecTreeKey<> is a MetadataKey that holds a spec tree of some
     * kind as its value.
     *
     * \ingroup g_metadata_key
     * \since 0.26
     * \nosubgrouping
     */
    template <typename C_>
    class PALUDIS_VISIBLE MetadataSpecTreeKey :
        public MetadataKey,
        public ConstAcceptInterfaceVisitsThis<MetadataKeyVisitorTypes, MetadataSpecTreeKey<C_> >
    {
        protected:
            ///\name Basic operations
            ///\{

            MetadataSpecTreeKey(const std::string &, const std::string &, const MetadataKeyType);

            ///\}

        public:
            /**
             * Fetch our value.
             */
            virtual const tr1::shared_ptr<const typename C_::ConstItem> value() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a multiline-line indented and formatted version of our
             * value, using the supplied Formatter to format individual items.
             */
            virtual std::string pretty_print(const typename C_::ItemFormatter &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a single-line formatted version of our value, using the
             * supplied Formatter to format individual items.
             */
            virtual std::string pretty_print_flat(const typename C_::ItemFormatter &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * A MetadataSpecTreeKey<FetchableURISpecTree> is a MetadataKey that holds
     * a FetchableURISpecTree as its value.
     *
     * This specialisation of MetadataSpecTreeKey provides an additional
     * initial_label method.
     *
     * \ingroup g_metadata_key
     * \since 0.26
     * \nosubgrouping
     */
    template <>
    class PALUDIS_VISIBLE MetadataSpecTreeKey<FetchableURISpecTree> :
        public MetadataKey,
        public ConstAcceptInterfaceVisitsThis<MetadataKeyVisitorTypes, MetadataSpecTreeKey<FetchableURISpecTree> >
    {
        protected:
            ///\name Basic operations
            ///\{

            MetadataSpecTreeKey(const std::string &, const std::string &, const MetadataKeyType);

            ///\}

        public:
            /**
             * Fetch our value.
             */
            virtual const tr1::shared_ptr<const FetchableURISpecTree::ConstItem> value() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a multiline-line indented and formatted version of our
             * value, using the supplied Formatter to format individual items.
             */
            virtual std::string pretty_print(const FetchableURISpecTree::ItemFormatter &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a single-line formatted version of our value, using the
             * supplied Formatter to format individual items.
             */
            virtual std::string pretty_print_flat(const FetchableURISpecTree::ItemFormatter &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a URILabel that represents the initial label to use when
             * deciding the behaviour of individual items in the heirarchy.
             */
            virtual const tr1::shared_ptr<const URILabel> initial_label() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * A MetadataSpecTreeKey<DependencySpecTree> is a MetadataKey that holds
     * a FetchableURISpecTree as its value.
     *
     * This specialisation of MetadataSpecTreeKey provides an additional
     * initial_label method.
     *
     * \ingroup g_metadata_key
     * \since 0.26
     * \nosubgrouping
     */
    template <>
    class PALUDIS_VISIBLE MetadataSpecTreeKey<DependencySpecTree> :
        public MetadataKey,
        public ConstAcceptInterfaceVisitsThis<MetadataKeyVisitorTypes, MetadataSpecTreeKey<DependencySpecTree> >
    {
        protected:
            ///\name Basic operations
            ///\{

            MetadataSpecTreeKey(const std::string &, const std::string &, const MetadataKeyType);

            ///\}

        public:
            /**
             * Fetch our value.
             */
            virtual const tr1::shared_ptr<const DependencySpecTree::ConstItem> value() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a multiline-line indented and formatted version of our
             * value, using the supplied Formatter to format individual items.
             */
            virtual std::string pretty_print(const DependencySpecTree::ItemFormatter &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a single-line formatted version of our value, using the
             * supplied Formatter to format individual items.
             */
            virtual std::string pretty_print_flat(const DependencySpecTree::ItemFormatter &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a DependencyLabelSequence that represents the initial labels to use when
             * deciding the behaviour of individual items in the heirarchy.
             */
            virtual const tr1::shared_ptr<const DependencyLabelSequence> initial_labels() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };
}

#endif
