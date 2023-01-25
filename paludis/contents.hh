/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/util/visitor.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/type_list.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/fs_path-fwd.hh>
#include <paludis/metadata_key_holder.hh>
#include <memory>
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
        public MetadataKeyHolder,
        public virtual DeclareAbstractAcceptMethods<ContentsEntry, MakeTypeList<
            ContentsFileEntry, ContentsDirEntry, ContentsSymEntry, ContentsOtherEntry>::Type>
    {
        private:
            Pimp<ContentsEntry> _imp;

        protected:
            void need_keys_added() const override;

        public:
            ///\name Basic operations
            ///\{

            ContentsEntry(const FSPath & path);
            ~ContentsEntry() override = 0;

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
            const std::shared_ptr<const MetadataValueKey<FSPath> > location_key() const;

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
        private:
            Pimp<ContentsFileEntry> _imp;

        public:
            ///\name Basic operations
            ///\{

            ContentsFileEntry(const FSPath &, const std::string &);
            ~ContentsFileEntry() override;

            ///\}

            ///\name Specific metadata keys
            ///\{

            const std::shared_ptr<const MetadataValueKey<std::string> >
                part_key() const;

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

            ContentsDirEntry(const FSPath &);

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

            ContentsOtherEntry(const FSPath &);

            ///\}
    };

    /**
     * A sym contents entry.
     *
     * \ingroup g_contents
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE ContentsSymEntry :
        public ContentsEntry,
        public ImplementAcceptMethods<ContentsEntry, ContentsSymEntry>
    {
        private:
            Pimp<ContentsSymEntry> _imp;

        public:
            ///\name Basic operations
            ///\{

            ContentsSymEntry(const FSPath &, const std::string &,
                             const std::string &);
            ~ContentsSymEntry() override;

            ///\}

            ///\name Specific metadata keys
            ///\{

            /**
             * Our target, as per readlink. Must not be zero.
             *
             * \since 0.36
             */
            const std::shared_ptr<const MetadataValueKey<std::string> > target_key() const;

            const std::shared_ptr<const MetadataValueKey<std::string> >
                part_key() const;

            ///\}
    };

    /**
     * A package's contents, obtainable by PackageID::contents.
     *
     * \ingroup g_contents
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE Contents
    {
        private:
            Pimp<Contents> _imp;

        public:
            ///\name Basic operations
            ///\{

            Contents();
            ~Contents();

            Contents(const Contents &) = delete;
            Contents & operator= (const Contents &) = delete;

            ///\}

            /// Add a new entry.
            void add(const std::shared_ptr<const ContentsEntry> & c);

            ///\name Iterate over our entries
            ///\{

            struct ConstIteratorTag;
            typedef WrappedForwardIterator<ConstIteratorTag, const std::shared_ptr<const ContentsEntry> > ConstIterator;

            ConstIterator begin() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            ConstIterator end() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}
    };

    extern template class Pimp<Contents>;
    extern template class Pimp<ContentsEntry>;
    extern template class Pimp<ContentsSymEntry>;
    extern template class Pimp<ContentsFileEntry>;

    extern template class PALUDIS_VISIBLE WrappedForwardIterator<Contents::ConstIteratorTag, const std::shared_ptr<const ContentsEntry> >;
}

#endif
