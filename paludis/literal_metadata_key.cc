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

#include <paludis/literal_metadata_key.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/join.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/package_id.hh>
#include <paludis/action.hh>
#include <paludis/repository.hh>
#include <paludis/name.hh>
#include <paludis/pretty_printer.hh>
#include <paludis/call_pretty_printer.hh>
#include <paludis/slot.hh>
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

    template <>
    struct Imp<LiteralMetadataMaintainersKey>
    {
        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;
        const std::shared_ptr<const Maintainers> value;

        Imp(const std::string & r, const std::string & h, const MetadataKeyType t,
                const std::shared_ptr<const Maintainers> & v) :
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
    _imp(r, h, t, v)
{
}

LiteralMetadataFSPathSequenceKey::~LiteralMetadataFSPathSequenceKey()
{
}

const std::shared_ptr<const FSPathSequence>
LiteralMetadataFSPathSequenceKey::parse_value() const
{
    return _imp->value;
}

const std::string
LiteralMetadataFSPathSequenceKey::pretty_print_value(
        const PrettyPrinter & p, const PrettyPrintOptions &) const
{
    return join(_imp->value->begin(), _imp->value->end(), " ", CallPrettyPrinter(p));
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
    _imp(r, h, t, v)
{
}

LiteralMetadataStringSetKey::~LiteralMetadataStringSetKey()
{
}

const std::shared_ptr<const Set<std::string> >
LiteralMetadataStringSetKey::parse_value() const
{
    return _imp->value;
}

LiteralMetadataStringStringMapKey::LiteralMetadataStringStringMapKey(const std::string & r, const std::string & h,
        const MetadataKeyType t, const std::shared_ptr<const Map<std::string, std::string> > & v) :
    _imp(r, h, t, v)
{
}

LiteralMetadataStringStringMapKey::~LiteralMetadataStringStringMapKey()
{
}

const std::shared_ptr<const Map<std::string, std::string> >
LiteralMetadataStringStringMapKey::parse_value() const
{
    return _imp->value;
}

LiteralMetadataStringSequenceKey::LiteralMetadataStringSequenceKey(const std::string & r, const std::string & h,
        const MetadataKeyType t, const std::shared_ptr<const Sequence<std::string> > & v) :
    _imp(r, h, t, v)
{
}

LiteralMetadataStringSequenceKey::~LiteralMetadataStringSequenceKey()
{
}

const std::shared_ptr<const Sequence<std::string> >
LiteralMetadataStringSequenceKey::parse_value() const
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

const std::string
LiteralMetadataStringSetKey::pretty_print_value(
        const PrettyPrinter & p, const PrettyPrintOptions &) const
{
    return join(_imp->value->begin(), _imp->value->end(), " ", CallPrettyPrinter(p));
}

const std::string
LiteralMetadataStringStringMapKey::pretty_print_value(
        const PrettyPrinter & p, const PrettyPrintOptions &) const
{
    return join(_imp->value->begin(), _imp->value->end(), " ", CallPrettyPrinter(p));
}

const std::string
LiteralMetadataStringSequenceKey::pretty_print_value(
        const PrettyPrinter & p, const PrettyPrintOptions &) const
{
    return join(_imp->value->begin(), _imp->value->end(), " ", CallPrettyPrinter(p));
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
    _imp(r, h, t, v)
{
}

template <typename T_>
LiteralMetadataValueKey<T_>::~LiteralMetadataValueKey()
{
}

template <typename T_>
const T_
LiteralMetadataValueKey<T_>::parse_value() const
{
    return _imp->value;
}

template <typename T_>
const std::string
PrettyPrintableLiteralMetadataValueKey<T_>::pretty_print_value(
        const PrettyPrinter & printer,
        const PrettyPrintOptions &) const
{
    return printer.prettify(this->parse_value());
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
    _imp(r, h, k, v)
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
LiteralMetadataTimeKey::parse_value() const
{
    return _imp->value;
}

LiteralMetadataMaintainersKey::LiteralMetadataMaintainersKey(const std::string & r, const std::string & h,
        const MetadataKeyType t, const std::shared_ptr<const Maintainers> & v) :
    _imp(r, h, t, v)
{
}

LiteralMetadataMaintainersKey::~LiteralMetadataMaintainersKey()
{
}

const std::shared_ptr<const Maintainers>
LiteralMetadataMaintainersKey::parse_value() const
{
    return _imp->value;
}

const std::string
LiteralMetadataMaintainersKey::pretty_print_value(
        const PrettyPrinter & p, const PrettyPrintOptions &) const
{
    return join(_imp->value->begin(), _imp->value->end(), " ", CallPrettyPrinter(p));
}

const std::string
LiteralMetadataMaintainersKey::human_name() const
{
    return _imp->human_name;
}

const std::string
LiteralMetadataMaintainersKey::raw_name() const
{
    return _imp->raw_name;
}

MetadataKeyType
LiteralMetadataMaintainersKey::type() const
{
    return _imp->type;
}

namespace paludis
{
    template class LiteralMetadataValueKey<FSPath>;
    template class LiteralMetadataValueKey<std::string>;
    template class LiteralMetadataValueKey<Slot>;
    template class LiteralMetadataValueKey<bool>;
    template class LiteralMetadataValueKey<long>;
    template class LiteralMetadataValueKey<std::shared_ptr<const PackageID> >;
}

