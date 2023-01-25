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

#ifndef PALUDIS_GUARD_PALUDIS_LITERAL_METADATA_KEY_HH
#define PALUDIS_GUARD_PALUDIS_LITERAL_METADATA_KEY_HH 1

#include <paludis/metadata_key.hh>
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
    template <typename T_>
    class PALUDIS_VISIBLE PrettyPrintableLiteralMetadataValueKey :
        public MetadataValueKey<T_>
    {
        public:
            const std::string pretty_print_value(
                    const PrettyPrinter &,
                    const PrettyPrintOptions &) const override PALUDIS_ATTRIBUTE((warn_unused_result));
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
        public std::conditional<MetadataValueKeyIsPrettyPrintable<T_>::value, PrettyPrintableLiteralMetadataValueKey<T_>, MetadataValueKey<T_> >::type
    {
        private:
            Pimp<LiteralMetadataValueKey<T_> > _imp;

        public:
            ///\name Basic operations
            ///\{

            LiteralMetadataValueKey(const std::string &, const std::string &, const MetadataKeyType,
                    const T_ &);
            ~LiteralMetadataValueKey() override;

            ///\}

            const T_ parse_value() const override PALUDIS_ATTRIBUTE((warn_unused_result));

            const std::string raw_name() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::string human_name() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            MetadataKeyType type() const override PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * \since 0.36
             */
            void change_value(const T_ &);
    };

    /**
     * A LiteralMetadataFSPathSequenceKey is a MetadataCollectionKey<FSPathSequence>
     * whose value is known at construction time.
     *
     * \ingroup g_literal_metadata_key
     * \since 0.26
     */
    class PALUDIS_VISIBLE LiteralMetadataFSPathSequenceKey :
        public MetadataCollectionKey<FSPathSequence>
    {
        private:
            Pimp<LiteralMetadataFSPathSequenceKey> _imp;

        public:
            ///\name Basic operations
            ///\{

            LiteralMetadataFSPathSequenceKey(const std::string &, const std::string &, const MetadataKeyType,
                    const std::shared_ptr<const FSPathSequence> &);
            ~LiteralMetadataFSPathSequenceKey() override;

            ///\}

            const std::shared_ptr<const FSPathSequence> parse_value() const override PALUDIS_ATTRIBUTE((warn_unused_result));

            const std::string raw_name() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::string human_name() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            MetadataKeyType type() const override PALUDIS_ATTRIBUTE((warn_unused_result));

            const std::string pretty_print_value(
                    const PrettyPrinter &,
                    const PrettyPrintOptions &) const override PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * A LiteralMetadataStringSetKey is a MetadataCollectionKey<Set<std::string> >
     * whose value is known at construction time.
     *
     * \ingroup g_literal_metadata_key
     * \since 0.26
     */
    class PALUDIS_VISIBLE LiteralMetadataStringSetKey :
        public MetadataCollectionKey<Set<std::string> >
    {
        private:
            Pimp<LiteralMetadataStringSetKey> _imp;

        public:
            ///\name Basic operations
            ///\{

            LiteralMetadataStringSetKey(const std::string &, const std::string &, const MetadataKeyType,
                    const std::shared_ptr<const Set<std::string> > &);
            ~LiteralMetadataStringSetKey() override;

            ///\}

            const std::shared_ptr<const Set<std::string> > parse_value() const override PALUDIS_ATTRIBUTE((warn_unused_result));

            const std::string raw_name() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::string human_name() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            MetadataKeyType type() const override PALUDIS_ATTRIBUTE((warn_unused_result));

            const std::string pretty_print_value(
                    const PrettyPrinter &,
                    const PrettyPrintOptions &) const override PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * A LiteralMetadataStringSequenceKey is a MetadataCollectionKey<Sequence<std::string> >
     * whose value is known at construction time.
     *
     * \ingroup g_literal_metadata_key
     * \since 0.30
     */
    class PALUDIS_VISIBLE LiteralMetadataStringSequenceKey :
        public MetadataCollectionKey<Sequence<std::string> >
    {
        private:
            Pimp<LiteralMetadataStringSequenceKey> _imp;

        public:
            ///\name Basic operations
            ///\{

            LiteralMetadataStringSequenceKey(const std::string &, const std::string &, const MetadataKeyType,
                    const std::shared_ptr<const Sequence<std::string> > &);
            ~LiteralMetadataStringSequenceKey() override;

            ///\}

            const std::shared_ptr<const Sequence<std::string> > parse_value() const override PALUDIS_ATTRIBUTE((warn_unused_result));

            const std::string raw_name() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::string human_name() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            MetadataKeyType type() const override PALUDIS_ATTRIBUTE((warn_unused_result));

            const std::string pretty_print_value(
                    const PrettyPrinter &,
                    const PrettyPrintOptions &) const override PALUDIS_ATTRIBUTE((warn_unused_result));
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
        public MetadataTimeKey
    {
        private:
            Pimp<LiteralMetadataTimeKey> _imp;

        public:
            ///\name Basic operations
            ///\{

            LiteralMetadataTimeKey(const std::string &, const std::string &, const MetadataKeyType, const Timestamp);
            ~LiteralMetadataTimeKey() override;

            ///\}

            Timestamp parse_value() const override PALUDIS_ATTRIBUTE((warn_unused_result));

            const std::string raw_name() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::string human_name() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            MetadataKeyType type() const override PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * A LiteralMetadataStringStringMapKey is a MetadataCollectionKey<Set<std::string> >
     * whose value is known at construction time.
     *
     * \ingroup g_literal_metadata_key
     * \since 0.55
     */
    class PALUDIS_VISIBLE LiteralMetadataStringStringMapKey :
        public MetadataCollectionKey<Map<std::string, std::string> >
    {
        private:
            Pimp<LiteralMetadataStringStringMapKey> _imp;

        public:
            ///\name Basic operations
            ///\{

            LiteralMetadataStringStringMapKey(const std::string &, const std::string &, const MetadataKeyType,
                    const std::shared_ptr<const Map<std::string, std::string> > &);
            ~LiteralMetadataStringStringMapKey() override;

            ///\}

            const std::shared_ptr<const Map<std::string, std::string> > parse_value() const override PALUDIS_ATTRIBUTE((warn_unused_result));

            const std::string raw_name() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::string human_name() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            MetadataKeyType type() const override PALUDIS_ATTRIBUTE((warn_unused_result));

            const std::string pretty_print_value(
                    const PrettyPrinter &,
                    const PrettyPrintOptions &) const override PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * A LiteralMetadataMaintainersKey is a MetadataCollectionKey<Maintainers>
     * whose value is known at construction time.
     *
     * \ingroup g_literal_metadata_key
     * \since 0.68
     */
    class PALUDIS_VISIBLE LiteralMetadataMaintainersKey :
        public MetadataCollectionKey<Maintainers>
    {
        private:
            Pimp<LiteralMetadataMaintainersKey> _imp;

        public:
            ///\name Basic operations
            ///\{

            LiteralMetadataMaintainersKey(const std::string &, const std::string &, const MetadataKeyType,
                    const std::shared_ptr<const Maintainers> &);
            ~LiteralMetadataMaintainersKey() override;

            ///\}

            const std::shared_ptr<const Maintainers> parse_value() const override PALUDIS_ATTRIBUTE((warn_unused_result));

            const std::string raw_name() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::string human_name() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            MetadataKeyType type() const override PALUDIS_ATTRIBUTE((warn_unused_result));

            const std::string pretty_print_value(
                    const PrettyPrinter &,
                    const PrettyPrintOptions &) const override PALUDIS_ATTRIBUTE((warn_unused_result));
    };
}

#endif
