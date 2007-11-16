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

#ifndef PALUDIS_GUARD_PALUDIS_LITERAL_METADATA_KEY_HH
#define PALUDIS_GUARD_PALUDIS_LITERAL_METADATA_KEY_HH 1

#include <paludis/metadata_key.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/private_implementation_pattern.hh>
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
     * A LiteralMetadataStringKey is a MetadataStringKey whose value is a
     * literal string that is known at construction time.
     *
     * \ingroup g_literal_metadata_key
     * \since 0.26
     */
    class PALUDIS_VISIBLE LiteralMetadataStringKey :
        public MetadataStringKey,
        private PrivateImplementationPattern<LiteralMetadataStringKey>
    {
        private:
            PrivateImplementationPattern<LiteralMetadataStringKey>::ImpPtr & _imp;

        public:
            ///\name Basic operations
            ///\{

            LiteralMetadataStringKey(const std::string &, const std::string &, const MetadataKeyType,
                    const std::string &);
            ~LiteralMetadataStringKey();

            ///\}

            virtual const std::string value() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * A LiteralMetadataFSEntryKey is a MetadataFSEntryKey whose value is a
     * literal FSEntry that is known at construction time.
     *
     * \ingroup g_literal_metadata_key
     * \since 0.26
     */
    class PALUDIS_VISIBLE LiteralMetadataFSEntryKey :
        public MetadataFSEntryKey,
        private PrivateImplementationPattern<LiteralMetadataFSEntryKey>
    {
        private:
            PrivateImplementationPattern<LiteralMetadataFSEntryKey>::ImpPtr & _imp;

        public:
            ///\name Basic operations
            ///\{

            LiteralMetadataFSEntryKey(const std::string &, const std::string &, const MetadataKeyType,
                    const FSEntry &);
            ~LiteralMetadataFSEntryKey();

            ///\}

            virtual const FSEntry value() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * A LiteralMetadataPackageIDKey is a MetadataPackageIDKey whose value is
     * known at construction time.
     *
     * \ingroup g_literal_metadata_key
     * \since 0.26
     */
    class PALUDIS_VISIBLE LiteralMetadataPackageIDKey :
        public MetadataPackageIDKey,
        private PrivateImplementationPattern<LiteralMetadataPackageIDKey>
    {
        private:
            PrivateImplementationPattern<LiteralMetadataPackageIDKey>::ImpPtr & _imp;

        public:
            ///\name Basic operations
            ///\{

            LiteralMetadataPackageIDKey(const std::string &, const std::string &, const MetadataKeyType,
                    const tr1::shared_ptr<const PackageID> &);
            ~LiteralMetadataPackageIDKey();

            ///\}

            virtual const tr1::shared_ptr<const PackageID> value() const PALUDIS_ATTRIBUTE((warn_unused_result));
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
        private PrivateImplementationPattern<LiteralMetadataFSEntrySequenceKey>
    {
        private:
            PrivateImplementationPattern<LiteralMetadataFSEntrySequenceKey>::ImpPtr & _imp;

        public:
            ///\name Basic operations
            ///\{

            LiteralMetadataFSEntrySequenceKey(const std::string &, const std::string &, const MetadataKeyType,
                    const tr1::shared_ptr<const FSEntrySequence> &);
            ~LiteralMetadataFSEntrySequenceKey();

            ///\}

            virtual const tr1::shared_ptr<const FSEntrySequence> value() const PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::string pretty_print_flat(const Formatter<FSEntry> &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));
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
        private PrivateImplementationPattern<LiteralMetadataStringSetKey>
    {
        private:
            PrivateImplementationPattern<LiteralMetadataStringSetKey>::ImpPtr & _imp;

        public:
            ///\name Basic operations
            ///\{

            LiteralMetadataStringSetKey(const std::string &, const std::string &, const MetadataKeyType,
                    const tr1::shared_ptr<const Set<std::string> > &);
            ~LiteralMetadataStringSetKey();

            ///\}

            virtual const tr1::shared_ptr<const Set<std::string> > value() const PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::string pretty_print_flat(const Formatter<std::string> &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));
    };
}

#endif
