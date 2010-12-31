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

#include <paludis/literal_metadata_key.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/join.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/formatter.hh>
#include <paludis/package_id.hh>
#include <paludis/action.hh>
#include <paludis/repository.hh>
#include <paludis/name.hh>
#include <paludis/pretty_printer.hh>
#include <paludis/call_pretty_printer.hh>
#include <functional>

using namespace paludis;

namespace paludis
{
    template <>
    struct Imp<LiteralMetadataFSPathSequenceKey>
    {
        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;
        const std::shared_ptr<const FSPathSequence> value;

        Imp(const std::string & r, const std::string & h, const MetadataKeyType t,
                const std::shared_ptr<const FSPathSequence> & v) :
            raw_name(r),
            human_name(h),
            type(t),
            value(v)
        {
        }
    };

    template <>
    struct Imp<LiteralMetadataStringSetKey>
    {
        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;
        const std::shared_ptr<const Set<std::string> > value;

        Imp(const std::string & r, const std::string & h, const MetadataKeyType t,
                const std::shared_ptr<const Set<std::string> > & v) :
            raw_name(r),
            human_name(h),
            type(t),
            value(v)
        {
        }
    };

    template <>
    struct Imp<LiteralMetadataStringStringMapKey>
    {
        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;
        const std::shared_ptr<const Map<std::string, std::string> > value;

        Imp(const std::string & r, const std::string & h, const MetadataKeyType t,
                const std::shared_ptr<const Map<std::string, std::string> > & v) :
            raw_name(r),
            human_name(h),
            type(t),
            value(v)
        {
        }
    };

    template <>
    struct Imp<LiteralMetadataStringSequenceKey>
    {
        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;
        const std::shared_ptr<const Sequence<std::string> > value;

        Imp(const std::string & r, const std::string & h, const MetadataKeyType t,
                const std::shared_ptr<const Sequence<std::string> > & v) :
            raw_name(r),
            human_name(h),
            type(t),
            value(v)
        {
        }
    };

    template <typename T_>
    struct Imp<LiteralMetadataValueKey<T_> >
    {
        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;
        T_ value;

        Imp(const std::string & r, const std::string & h, const MetadataKeyType t, const T_ & v) :
            raw_name(r),
            human_name(h),
            type(t),
            value(v)
        {
        }
    };
}

LiteralMetadataFSPathSequenceKey::LiteralMetadataFSPathSequenceKey(const std::string & r, const std::string & h,
        const MetadataKeyType t, const std::shared_ptr<const FSPathSequence> & v) :
    Pimp<LiteralMetadataFSPathSequenceKey>(r, h, t, v),
    _imp(Pimp<LiteralMetadataFSPathSequenceKey>::_imp)
{
}

LiteralMetadataFSPathSequenceKey::~LiteralMetadataFSPathSequenceKey()
{
}

const std::shared_ptr<const FSPathSequence>
LiteralMetadataFSPathSequenceKey::value() const
{
    return _imp->value;
}

namespace
{
    std::string format_fsentry(const FSPath & i, const Formatter<FSPath> & f)
    {
        return f.format(i, format::Plain());
    }
}

std::string
LiteralMetadataFSPathSequenceKey::pretty_print_flat(const Formatter<FSPath> & f) const
{
    using namespace std::placeholders;
    return join(value()->begin(), value()->end(), " ", std::bind(&format_fsentry, _1, f));
}

const std::string
LiteralMetadataFSPathSequenceKey::pretty_print_value(
        const PrettyPrinter & p, const PrettyPrintOptions &) const
{
    return join(value()->begin(), value()->end(), " ", CallPrettyPrinter(p));
}

const std::string
LiteralMetadataFSPathSequenceKey::human_name() const
{
    return _imp->human_name;
}

const std::string
LiteralMetadataFSPathSequenceKey::raw_name() const
{
    return _imp->raw_name;
}

MetadataKeyType
LiteralMetadataFSPathSequenceKey::type() const
{
    return _imp->type;
}

LiteralMetadataStringSetKey::LiteralMetadataStringSetKey(const std::string & r, const std::string & h,
        const MetadataKeyType t, const std::shared_ptr<const Set<std::string> > & v) :
    Pimp<LiteralMetadataStringSetKey>(r, h, t, v),
    _imp(Pimp<LiteralMetadataStringSetKey>::_imp)
{
}

LiteralMetadataStringSetKey::~LiteralMetadataStringSetKey()
{
}

const std::shared_ptr<const Set<std::string> >
LiteralMetadataStringSetKey::value() const
{
    return _imp->value;
}

LiteralMetadataStringStringMapKey::LiteralMetadataStringStringMapKey(const std::string & r, const std::string & h,
        const MetadataKeyType t, const std::shared_ptr<const Map<std::string, std::string> > & v) :
    Pimp<LiteralMetadataStringStringMapKey>(r, h, t, v),
    _imp(Pimp<LiteralMetadataStringStringMapKey>::_imp)
{
}

LiteralMetadataStringStringMapKey::~LiteralMetadataStringStringMapKey()
{
}

const std::shared_ptr<const Map<std::string, std::string> >
LiteralMetadataStringStringMapKey::value() const
{
    return _imp->value;
}

LiteralMetadataStringSequenceKey::LiteralMetadataStringSequenceKey(const std::string & r, const std::string & h,
        const MetadataKeyType t, const std::shared_ptr<const Sequence<std::string> > & v) :
    Pimp<LiteralMetadataStringSequenceKey>(r, h, t, v),
    _imp(Pimp<LiteralMetadataStringSequenceKey>::_imp)
{
}

LiteralMetadataStringSequenceKey::~LiteralMetadataStringSequenceKey()
{
}

const std::shared_ptr<const Sequence<std::string> >
LiteralMetadataStringSequenceKey::value() const
{
    return _imp->value;
}

const std::string
LiteralMetadataStringSequenceKey::human_name() const
{
    return _imp->human_name;
}

const std::string
LiteralMetadataStringSequenceKey::raw_name() const
{
    return _imp->raw_name;
}

MetadataKeyType
LiteralMetadataStringSequenceKey::type() const
{
    return _imp->type;
}

namespace
{
    std::string format_string(const std::string & i, const Formatter<std::string> & f)
    {
        return f.format(i, format::Plain());
    }

    std::string format_string_string(const std::pair<const std::string, std::string> & i, const Formatter<std::pair<const std::string, std::string> > & f)
    {
        return f.format(i, format::Plain());
    }
}

const std::string
LiteralMetadataStringSetKey::pretty_print_value(
        const PrettyPrinter & p, const PrettyPrintOptions &) const
{
    return join(value()->begin(), value()->end(), " ", CallPrettyPrinter(p));
}

std::string
LiteralMetadataStringSetKey::pretty_print_flat(const Formatter<std::string> & f) const
{
    using namespace std::placeholders;
    return join(value()->begin(), value()->end(), " ", std::bind(&format_string, _1, f));
}

const std::string
LiteralMetadataStringStringMapKey::pretty_print_value(
        const PrettyPrinter & p, const PrettyPrintOptions &) const
{
    return join(value()->begin(), value()->end(), " ", CallPrettyPrinter(p));
}

std::string
LiteralMetadataStringStringMapKey::pretty_print_flat(const Formatter<std::pair<const std::string, std::string> > & f) const
{
    using namespace std::placeholders;
    return join(value()->begin(), value()->end(), " ", std::bind(&format_string_string, _1, f));
}

const std::string
LiteralMetadataStringSequenceKey::pretty_print_value(
        const PrettyPrinter & p, const PrettyPrintOptions &) const
{
    return join(value()->begin(), value()->end(), " ", CallPrettyPrinter(p));
}

std::string
LiteralMetadataStringSequenceKey::pretty_print_flat(const Formatter<std::string> & f) const
{
    using namespace std::placeholders;
    return join(value()->begin(), value()->end(), " ", std::bind(&format_string, _1, f));
}

const std::string
LiteralMetadataStringSetKey::human_name() const
{
    return _imp->human_name;
}

const std::string
LiteralMetadataStringSetKey::raw_name() const
{
    return _imp->raw_name;
}

MetadataKeyType
LiteralMetadataStringSetKey::type() const
{
    return _imp->type;
}

const std::string
LiteralMetadataStringStringMapKey::human_name() const
{
    return _imp->human_name;
}

const std::string
LiteralMetadataStringStringMapKey::raw_name() const
{
    return _imp->raw_name;
}

MetadataKeyType
LiteralMetadataStringStringMapKey::type() const
{
    return _imp->type;
}

template <typename T_>
const std::string
LiteralMetadataValueKey<T_>::human_name() const
{
    return _imp->human_name;
}

template <typename T_>
const std::string
LiteralMetadataValueKey<T_>::raw_name() const
{
    return _imp->raw_name;
}

template <typename T_>
void
LiteralMetadataValueKey<T_>::change_value(const T_ & v)
{
    _imp->value = v;
}

template <typename T_>
MetadataKeyType
LiteralMetadataValueKey<T_>::type() const
{
    return _imp->type;
}

template <typename T_>
LiteralMetadataValueKey<T_>::LiteralMetadataValueKey(const std::string & r, const std::string & h,
        const MetadataKeyType t, const T_ & v) :
    Pimp<LiteralMetadataValueKey<T_> >(r, h, t, v),
    _imp(Pimp<LiteralMetadataValueKey<T_ > >::_imp)
{
}

template <typename T_>
LiteralMetadataValueKey<T_>::~LiteralMetadataValueKey()
{
}

template <typename T_>
const T_
LiteralMetadataValueKey<T_>::value() const
{
    return _imp->value;
}

template <typename T_>
const std::string
PrettyPrintableLiteralMetadataValueKey<T_>::pretty_print_value(
        const PrettyPrinter & printer,
        const PrettyPrintOptions &) const
{
    return printer.prettify(this->value());
}

namespace paludis
{
    template <>
    struct Imp<LiteralMetadataTimeKey>
    {
        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;
        const Timestamp value;

        Imp(const std::string & r, const std::string & h, const MetadataKeyType t, const Timestamp v) :
            raw_name(r),
            human_name(h),
            type(t),
            value(v)
        {
        }
    };
}

LiteralMetadataTimeKey::LiteralMetadataTimeKey(
        const std::string & r, const std::string & h, const MetadataKeyType k, const Timestamp v) :
    Pimp<LiteralMetadataTimeKey>(r, h, k, v),
    _imp(Pimp<LiteralMetadataTimeKey>::_imp)
{
}

LiteralMetadataTimeKey::~LiteralMetadataTimeKey()
{
}

const std::string
LiteralMetadataTimeKey::human_name() const
{
    return _imp->human_name;
}

const std::string
LiteralMetadataTimeKey::raw_name() const
{
    return _imp->raw_name;
}

MetadataKeyType
LiteralMetadataTimeKey::type() const
{
    return _imp->type;
}

Timestamp
LiteralMetadataTimeKey::value() const
{
    return _imp->value;
}

template class LiteralMetadataValueKey<FSPath>;
template class LiteralMetadataValueKey<std::string>;
template class LiteralMetadataValueKey<SlotName>;
template class LiteralMetadataValueKey<bool>;
template class LiteralMetadataValueKey<long>;
template class LiteralMetadataValueKey<std::shared_ptr<const PackageID> >;

