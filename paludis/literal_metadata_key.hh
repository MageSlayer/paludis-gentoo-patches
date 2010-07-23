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

#ifndef PALUDIS_GUARD_PALUDIS_LITERAL_METADATA_KEY_HH
#define PALUDIS_GUARD_PALUDIS_LITERAL_METADATA_KEY_HH 1

#include <paludis/metadata_key.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/set.hh>

/** \file
 * Declarations for literal metadata key classes.
 *
 * \ingroup g_literal_metadata_key
 *
 * \section Examples
 *
 * - \ref example_metadata_key.cc "example_metadata_key.cc" (for metadata keys)
 */


namespace paludis
{
    /**
     * Implement extra methods for LiteralMetadataValueKey.
     *
     * \ingroup g_metadata_key
     * \since 0.26
     */
    template <typename T_>
    class ExtraLiteralMetadataValueKeyMethods
    {
    };

    /**
     * Implement extra methods for LiteralMetadataValueKey for PackageID.
     *
     * \ingroup g_metadata_key
     * \since 0.26
     */
    template <>
    class PALUDIS_VISIBLE ExtraLiteralMetadataValueKeyMethods<std::shared_ptr<const PackageID> > :
        public virtual ExtraMetadataValueKeyMethods<std::shared_ptr<const PackageID> >
    {
        public:
            virtual ~ExtraLiteralMetadataValueKeyMethods() = 0;

            virtual std::string pretty_print(const Formatter<PackageID> &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * Implement extra methods for LiteralMetadataValueKey for long.
     *
     * \ingroup g_metadata_key
     * \since 0.28
     */
    template <>
    class PALUDIS_VISIBLE ExtraLiteralMetadataValueKeyMethods<long> :
        public virtual ExtraMetadataValueKeyMethods<long>
    {
        public:
            virtual ~ExtraLiteralMetadataValueKeyMethods() = 0;

            virtual std::string pretty_print() const
                PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * Implement extra methods for LiteralMetadataValueKey for bool.
     *
     * \ingroup g_metadata_key
     * \since 0.34.1
     */
    template <>
    class PALUDIS_VISIBLE ExtraLiteralMetadataValueKeyMethods<bool> :
        public virtual ExtraMetadataValueKeyMethods<bool>
    {
        public:
            virtual ~ExtraLiteralMetadataValueKeyMethods() = 0;

            virtual std::string pretty_print() const
                PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * A LiteralMetadataValueKey is a MetadataValueKey whose value is a
     * copyable literal that is known at construction time.
     *
     * \ingroup g_literal_metadata_key
     * \since 0.26
     */
    template <typename T_>
    class PALUDIS_VISIBLE LiteralMetadataValueKey :
        public MetadataValueKey<T_>,
        private Pimp<LiteralMetadataValueKey<T_> >,
        public ExtraLiteralMetadataValueKeyMethods<T_>
    {
        private:
            typename Pimp<LiteralMetadataValueKey<T_> >::ImpPtr & _imp;

        public:
            ///\name Basic operations
            ///\{

            LiteralMetadataValueKey(const std::string &, const std::string &, const MetadataKeyType,
                    const T_ &);
            ~LiteralMetadataValueKey();

            ///\}

            virtual const T_ value() const PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * \since 0.36
             */
            void change_value(const T_ &);
    };

    /**
     * A LiteralMetadataFSEntrySequenceKey is a MetadataCollectionKey<FSEntrySequence>
     * whose value is known at construction time.
     *
     * \ingroup g_literal_metadata_key
     * \since 0.26
     */
    class PALUDIS_VISIBLE LiteralMetadataFSEntrySequenceKey :
        public MetadataCollectionKey<FSEntrySequence>,
        private Pimp<LiteralMetadataFSEntrySequenceKey>
    {
        private:
            Pimp<LiteralMetadataFSEntrySequenceKey>::ImpPtr & _imp;

        public:
            ///\name Basic operations
            ///\{

            LiteralMetadataFSEntrySequenceKey(const std::string &, const std::string &, const MetadataKeyType,
                    const std::shared_ptr<const FSEntrySequence> &);
            ~LiteralMetadataFSEntrySequenceKey();

            ///\}

            virtual const std::shared_ptr<const FSEntrySequence> value() const PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::string pretty_print_flat(const Formatter<FSEntry> &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * A LiteralMetadataStringSetKey is a MetadataCollectionKey<Set<std::string> >
     * whose value is known at construction time.
     *
     * \ingroup g_literal_metadata_key
     * \since 0.26
     */
    class PALUDIS_VISIBLE LiteralMetadataStringSetKey :
        public MetadataCollectionKey<Set<std::string> >,
        private Pimp<LiteralMetadataStringSetKey>
    {
        private:
            Pimp<LiteralMetadataStringSetKey>::ImpPtr & _imp;

        public:
            ///\name Basic operations
            ///\{

            LiteralMetadataStringSetKey(const std::string &, const std::string &, const MetadataKeyType,
                    const std::shared_ptr<const Set<std::string> > &);
            ~LiteralMetadataStringSetKey();

            ///\}

            virtual const std::shared_ptr<const Set<std::string> > value() const PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::string pretty_print_flat(const Formatter<std::string> &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * A LiteralMetadataStringSequenceKey is a MetadataCollectionKey<Sequence<std::string> >
     * whose value is known at construction time.
     *
     * \ingroup g_literal_metadata_key
     * \since 0.30
     */
    class PALUDIS_VISIBLE LiteralMetadataStringSequenceKey :
        public MetadataCollectionKey<Sequence<std::string> >,
        private Pimp<LiteralMetadataStringSequenceKey>
    {
        private:
            Pimp<LiteralMetadataStringSequenceKey>::ImpPtr & _imp;

        public:
            ///\name Basic operations
            ///\{

            LiteralMetadataStringSequenceKey(const std::string &, const std::string &, const MetadataKeyType,
                    const std::shared_ptr<const Sequence<std::string> > &);
            ~LiteralMetadataStringSequenceKey();

            ///\}

            virtual const std::shared_ptr<const Sequence<std::string> > value() const PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::string pretty_print_flat(const Formatter<std::string> &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * A LiteralMetadataTimeKey is a MetadataTimeKey whose value is known at
     * construction time.
     *
     * \ingroup g_literal_metadata_key
     * \since 0.36
     * \since 0.44 Timestamp instead of time_t
     */
    class PALUDIS_VISIBLE LiteralMetadataTimeKey :
        public MetadataTimeKey,
        private Pimp<LiteralMetadataTimeKey>
    {
        private:
            Pimp<LiteralMetadataTimeKey>::ImpPtr & _imp;

        public:
            ///\name Basic operations
            ///\{

            LiteralMetadataTimeKey(const std::string &, const std::string &, const MetadataKeyType, const Timestamp);
            ~LiteralMetadataTimeKey();

            ///\}

            virtual Timestamp value() const PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };
}

#endif
