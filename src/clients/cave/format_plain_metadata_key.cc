/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include "format_plain_metadata_key.hh"
#include "format_string.hh"
#include <paludis/util/join.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/map.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/metadata_key.hh>
#include <paludis/mask.hh>
#include <paludis/unformatted_pretty_printer.hh>
#include <paludis/name.hh>
#include <sstream>

using namespace paludis;
using namespace cave;

namespace
{
    struct ValueGetter
    {
        std::stringstream s;

        void visit(const MetadataTimeKey & k)
        {
            s << k.value().seconds();
        }

        void visit(const MetadataValueKey<std::shared_ptr<const Contents> > &)
        {
            s << "<unprintable>";
        }

        void visit(const MetadataValueKey<std::shared_ptr<const Choices> > &)
        {
            s << "<unprintable>";
        }

        void visit(const MetadataValueKey<std::shared_ptr<const PackageID> > & k)
        {
            s << *k.value();
        }

        void visit(const MetadataValueKey<FSPath> & k)
        {
            s << k.value();
        }

        void visit(const MetadataValueKey<bool> & k)
        {
            s << (k.value() ? "true" : "false");
        }

        void visit(const MetadataValueKey<long> & k)
        {
            s << k.value();
        }

        void visit(const MetadataValueKey<std::string> & k)
        {
            s << k.value();
        }

        void visit(const MetadataValueKey<SlotName> & k)
        {
            s << k.value();
        }

        void visit(const MetadataSpecTreeKey<DependencySpecTree> & k)
        {
            s << k.pretty_print_value(UnformattedPrettyPrinter(), { });
        }

        void visit(const MetadataSpecTreeKey<ProvideSpecTree> & k)
        {
            s << k.pretty_print_value(UnformattedPrettyPrinter(), { });
        }

        void visit(const MetadataSpecTreeKey<FetchableURISpecTree> & k)
        {
            s << k.pretty_print_value(UnformattedPrettyPrinter(), { });
        }

        void visit(const MetadataSpecTreeKey<PlainTextSpecTree> & k)
        {
            s << k.pretty_print_value(UnformattedPrettyPrinter(), { });
        }

        void visit(const MetadataSpecTreeKey<RequiredUseSpecTree> & k)
        {
            s << k.pretty_print_value(UnformattedPrettyPrinter(), { });
        }

        void visit(const MetadataSpecTreeKey<SimpleURISpecTree> & k)
        {
            s << k.pretty_print_value(UnformattedPrettyPrinter(), { });
        }

        void visit(const MetadataSpecTreeKey<LicenseSpecTree> & k)
        {
            s << k.pretty_print_value(UnformattedPrettyPrinter(), { });
        }

        void visit(const MetadataCollectionKey<FSPathSequence> & k)
        {
            s << k.pretty_print_value(UnformattedPrettyPrinter(), { });
        }

        void visit(const MetadataCollectionKey<PackageIDSequence> & k)
        {
            s << k.pretty_print_value(UnformattedPrettyPrinter(), { });
        }

        void visit(const MetadataCollectionKey<Sequence<std::string> > & k)
        {
            s << k.pretty_print_value(UnformattedPrettyPrinter(), { });
        }

        void visit(const MetadataCollectionKey<Set<std::string> > & k)
        {
            s << k.pretty_print_value(UnformattedPrettyPrinter(), { });
        }

        void visit(const MetadataCollectionKey<Map<std::string, std::string> > & k)
        {
            s << k.pretty_print_value(UnformattedPrettyPrinter(), { });
        }

        void visit(const MetadataCollectionKey<KeywordNameSet> & k)
        {
            s << k.pretty_print_value(UnformattedPrettyPrinter(), { });
        }

        void visit(const MetadataSectionKey &)
        {
            s << "<unprintable>";
        }
    };
}

std::string
paludis::cave::format_plain_metadata_key(
        const std::shared_ptr<const MetadataKey> & k,
        const std::string & i,
        const std::string & f)
{
    std::shared_ptr<Map<char, std::string> > m(std::make_shared<Map<char, std::string>>());
    m->insert('r', k->raw_name());
    m->insert('h', k->human_name());
    m->insert('v', format_plain_metadata_key_value(k));
    m->insert('i', i);

    return format_string(f, m);
}

std::string
paludis::cave::format_plain_metadata_key_value(
        const std::shared_ptr<const MetadataKey> & k)
{
    ValueGetter v;
    k->accept(v);
    return v.s.str();
}

