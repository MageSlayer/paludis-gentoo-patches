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
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/join.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/formatter.hh>
#include <paludis/package_id.hh>
#include <paludis/action.hh>
#include <paludis/repository.hh>
#include <paludis/name.hh>
#include <functional>

using namespace paludis;

namespace paludis
{
    template <>
    struct Implementation<LiteralMetadataFSEntrySequenceKey>
    {
        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;
        const std::shared_ptr<const FSEntrySequence> value;

        Implementation(const std::string & r, const std::string & h, const MetadataKeyType t,
                const std::shared_ptr<const FSEntrySequence> & v) :
            raw_name(r),
            human_name(h),
            type(t),
            value(v)
        {
        }
    };

    template <>
    struct Implementation<LiteralMetadataStringSetKey>
    {
        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;
        const std::shared_ptr<const Set<std::string> > value;

        Implementation(const std::string & r, const std::string & h, const MetadataKeyType t,
                const std::shared_ptr<const Set<std::string> > & v) :
            raw_name(r),
            human_name(h),
            type(t),
            value(v)
        {
        }
    };

    template <>
    struct Implementation<LiteralMetadataStringSequenceKey>
    {
        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;
        const std::shared_ptr<const Sequence<std::string> > value;

        Implementation(const std::string & r, const std::string & h, const MetadataKeyType t,
                const std::shared_ptr<const Sequence<std::string> > & v) :
            raw_name(r),
            human_name(h),
            type(t),
            value(v)
        {
        }
    };

    template <typename T_>
    struct Implementation<LiteralMetadataValueKey<T_> >
    {
        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;
        T_ value;

        Implementation(const std::string & r, const std::string & h, const MetadataKeyType t, const T_ & v) :
            raw_name(r),
            human_name(h),
            type(t),
            value(v)
        {
        }
    };
}

LiteralMetadataFSEntrySequenceKey::LiteralMetadataFSEntrySequenceKey(const std::string & r, const std::string & h,
        const MetadataKeyType t, const std::shared_ptr<const FSEntrySequence> & v) :
    PrivateImplementationPattern<LiteralMetadataFSEntrySequenceKey>(r, h, t, v),
    _imp(PrivateImplementationPattern<LiteralMetadataFSEntrySequenceKey>::_imp)
{
}

LiteralMetadataFSEntrySequenceKey::~LiteralMetadataFSEntrySequenceKey()
{
}

const std::shared_ptr<const FSEntrySequence>
LiteralMetadataFSEntrySequenceKey::value() const
{
    return _imp->value;
}

namespace
{
    std::string format_fsentry(const FSEntry & i, const Formatter<FSEntry> & f)
    {
        return f.format(i, format::Plain());
    }
}

std::string
LiteralMetadataFSEntrySequenceKey::pretty_print_flat(const Formatter<FSEntry> & f) const
{
    using namespace std::placeholders;
    return join(value()->begin(), value()->end(), " ", std::bind(&format_fsentry, _1, f));
}

const std::string
LiteralMetadataFSEntrySequenceKey::human_name() const
{
    return _imp->human_name;
}

const std::string
LiteralMetadataFSEntrySequenceKey::raw_name() const
{
    return _imp->raw_name;
}

MetadataKeyType
LiteralMetadataFSEntrySequenceKey::type() const
{
    return _imp->type;
}

LiteralMetadataStringSetKey::LiteralMetadataStringSetKey(const std::string & r, const std::string & h,
        const MetadataKeyType t, const std::shared_ptr<const Set<std::string> > & v) :
    PrivateImplementationPattern<LiteralMetadataStringSetKey>(r, h, t, v),
    _imp(PrivateImplementationPattern<LiteralMetadataStringSetKey>::_imp)
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

LiteralMetadataStringSequenceKey::LiteralMetadataStringSequenceKey(const std::string & r, const std::string & h,
        const MetadataKeyType t, const std::shared_ptr<const Sequence<std::string> > & v) :
    PrivateImplementationPattern<LiteralMetadataStringSequenceKey>(r, h, t, v),
    _imp(PrivateImplementationPattern<LiteralMetadataStringSequenceKey>::_imp)
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
}

std::string
LiteralMetadataStringSetKey::pretty_print_flat(const Formatter<std::string> & f) const
{
    using namespace std::placeholders;
    return join(value()->begin(), value()->end(), " ", std::bind(&format_string, _1, f));
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

ExtraLiteralMetadataValueKeyMethods<long>::~ExtraLiteralMetadataValueKeyMethods()
{
}

std::string
ExtraLiteralMetadataValueKeyMethods<long>::pretty_print() const
{
    long v(static_cast<const LiteralMetadataValueKey<long> *>(this)->value());
    return stringify(v);
}

ExtraLiteralMetadataValueKeyMethods<bool>::~ExtraLiteralMetadataValueKeyMethods()
{
}

std::string
ExtraLiteralMetadataValueKeyMethods<bool>::pretty_print() const
{
    bool v(static_cast<const LiteralMetadataValueKey<bool> *>(this)->value());
    return stringify(v);
}

ExtraLiteralMetadataValueKeyMethods<std::shared_ptr<const PackageID> >::~ExtraLiteralMetadataValueKeyMethods()
{
}

std::string
ExtraLiteralMetadataValueKeyMethods<std::shared_ptr<const PackageID> >::pretty_print(const Formatter<PackageID> & f) const
{
    std::shared_ptr<const PackageID> v(static_cast<const LiteralMetadataValueKey<std::shared_ptr<const PackageID> > *>(this)->value());
    if (v->repository()->installed_root_key())
        return f.format(*v, format::Installed());
    else if (v->supports_action(SupportsActionTest<InstallAction>()))
    {
        if (v->masked())
            return f.format(*v, format::Plain());
        else
            return f.format(*v, format::Installable());
    }
    else
        return f.format(*v, format::Plain());
}

template <typename T_>
LiteralMetadataValueKey<T_>::LiteralMetadataValueKey(const std::string & r, const std::string & h,
        const MetadataKeyType t, const T_ & v) :
    PrivateImplementationPattern<LiteralMetadataValueKey<T_> >(r, h, t, v),
    _imp(PrivateImplementationPattern<LiteralMetadataValueKey<T_ > >::_imp)
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

namespace paludis
{
    template <>
    struct Implementation<LiteralMetadataTimeKey>
    {
        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;
        const Timestamp value;

        Implementation(const std::string & r, const std::string & h, const MetadataKeyType t, const Timestamp v) :
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
    PrivateImplementationPattern<LiteralMetadataTimeKey>(r, h, k, v),
    _imp(PrivateImplementationPattern<LiteralMetadataTimeKey>::_imp)
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

template class LiteralMetadataValueKey<FSEntry>;
template class LiteralMetadataValueKey<std::string>;
template class LiteralMetadataValueKey<SlotName>;
template class LiteralMetadataValueKey<bool>;
template class LiteralMetadataValueKey<long>;
template class LiteralMetadataValueKey<std::shared_ptr<const PackageID> >;

