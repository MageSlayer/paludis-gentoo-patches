/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_CONTENTS_HH
#define PALUDIS_GUARD_PALUDIS_CONTENTS_HH 1

#include <paludis/contents-fwd.hh>
#include <paludis/util/simple_visitor.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/type_list.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <paludis/metadata_key_holder.hh>
#include <tr1/memory>
#include <string>

/** \file
 * Declarations for the Contents classes.
 *
 * \ingroup g_contents
 *
 * \section Examples
 *
 * - \ref example_contents.cc "example_contents.cc"
 */

namespace paludis
{
    /**
     * Base class for a contents entry.
     *
     * \since 0.36 for MetadataKeyHolder methods.
     *
     * \ingroup g_contents
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE ContentsEntry :
        private PrivateImplementationPattern<ContentsEntry>,
        public MetadataKeyHolder,
        public virtual DeclareAbstractAcceptMethods<ContentsEntry, MakeTypeList<
            ContentsFileEntry, ContentsDirEntry, ContentsSymEntry, ContentsOtherEntry>::Type>
    {
        private:
            PrivateImplementationPattern<ContentsEntry>::ImpPtr & _imp;

        protected:
            virtual void need_keys_added() const;

        public:
            ///\name Basic operations
            ///\{

            ContentsEntry(const FSEntry & path);
            virtual ~ContentsEntry() = 0;

            ///\}

            ///\name Metadata key operations
            ///\{

            /**
             * Must be called straight after construction.
             *
             * \since 0.36
             */
            using MetadataKeyHolder::add_metadata_key;

            ///\}

            ///\name Specific metadata keys
            ///\{

            /**
             * Our path on disk. Must not be zero. Not modified for root.
             *
             * \since 0.36
             */
            const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > location_key() const;

            ///\}

            ///\}
    };

    /**
     * A file contents entry.
     *
     * \ingroup g_contents
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE ContentsFileEntry :
        public ContentsEntry,
        public ImplementAcceptMethods<ContentsEntry, ContentsFileEntry>
    {
        public:
            ///\name Basic operations
            ///\{

            ContentsFileEntry(const FSEntry &);

            ///\}
    };

    /**
     * A directory contents entry.
     *
     * \ingroup g_contents
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE ContentsDirEntry :
        public ContentsEntry,
        public ImplementAcceptMethods<ContentsEntry, ContentsDirEntry>
    {
        public:
            ///\name Basic operations
            ///\{

            ContentsDirEntry(const FSEntry &);

            ///\}
    };

    /**
     * An 'other' contents entry, which we can't handle.
     *
     * \since 0.36
     * \ingroup g_contents
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE ContentsOtherEntry :
        public ContentsEntry,
        public ImplementAcceptMethods<ContentsEntry, ContentsOtherEntry>
    {
        public:
            ///\name Basic operations
            ///\{

            ContentsOtherEntry(const FSEntry &);

            ///\}
    };

    /**
     * A sym contents entry.
     *
     * \ingroup g_contents
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE ContentsSymEntry :
        private PrivateImplementationPattern<ContentsSymEntry>,
        public ContentsEntry,
        public ImplementAcceptMethods<ContentsEntry, ContentsSymEntry>
    {
        private:
            PrivateImplementationPattern<ContentsSymEntry>::ImpPtr & _imp;

        public:
            ///\name Basic operations
            ///\{

            ContentsSymEntry(const FSEntry &, const std::string & target);
            ~ContentsSymEntry();

            ///\}

            ///\name Specific metadata keys
            ///\{

            /**
             * Our target, as per readlink. Must not be zero.
             *
             * \since 0.36
             */
            const std::tr1::shared_ptr<const MetadataValueKey<std::string> > target_key() const;

            ///\}
    };

    /**
     * A package's contents, obtainable by PackageID::contents_key.
     *
     * \ingroup g_contents
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE Contents :
        private InstantiationPolicy<Contents, instantiation_method::NonCopyableTag>,
        private PrivateImplementationPattern<Contents>
    {
        public:
            ///\name Basic operations
            ///\{

            Contents();
            ~Contents();

            ///\}

            /// Add a new entry.
            void add(const std::tr1::shared_ptr<const ContentsEntry> & c);

            ///\name Iterate over our entries
            ///\{

            struct ConstIteratorTag;
            typedef WrappedForwardIterator<ConstIteratorTag, const std::tr1::shared_ptr<const ContentsEntry> > ConstIterator;

            ConstIterator begin() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            ConstIterator end() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}
    };

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class InstantiationPolicy<Contents, instantiation_method::NonCopyableTag>;
    extern template class PrivateImplementationPattern<Contents>;
    extern template class PrivateImplementationPattern<ContentsEntry>;
    extern template class PrivateImplementationPattern<ContentsSymEntry>;

    extern template class WrappedForwardIterator<Contents::ConstIteratorTag, const std::tr1::shared_ptr<const ContentsEntry> >;
#endif
}

#endif
