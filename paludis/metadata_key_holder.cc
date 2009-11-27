/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009 Ciaran McCreesh
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

#include <paludis/metadata_key_holder.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/metadata_key.hh>
#include <tr1/functional>
#include <list>
#include <algorithm>

using namespace paludis;

namespace paludis
{
    template <>
    struct Implementation<MetadataKeyHolder>
    {
        mutable std::list<std::tr1::shared_ptr<const MetadataKey> > keys;
    };

    template <>
    struct WrappedForwardIteratorTraits<MetadataKeyHolder::MetadataConstIteratorTag>
    {
        typedef std::list<std::tr1::shared_ptr<const MetadataKey> >::const_iterator UnderlyingIterator;
    };
}

MetadataKeyHolder::MetadataKeyHolder() :
    PrivateImplementationPattern<MetadataKeyHolder>(new Implementation<MetadataKeyHolder>)
{
}

MetadataKeyHolder::~MetadataKeyHolder()
{
}

void
MetadataKeyHolder::add_metadata_key(const std::tr1::shared_ptr<const MetadataKey> & k) const
{
    using namespace std::tr1::placeholders;

    if (indirect_iterator(_imp->keys.end()) != std::find_if(indirect_iterator(_imp->keys.begin()), indirect_iterator(_imp->keys.end()),
                std::tr1::bind(std::equal_to<std::string>(), k->raw_name(), std::tr1::bind(std::tr1::mem_fn(&MetadataKey::raw_name), _1))))
        throw ConfigurationError("Tried to add duplicate key '" + k->raw_name() + "'");

    _imp->keys.push_back(k);
}

MetadataKeyHolder::MetadataConstIterator
MetadataKeyHolder::begin_metadata() const
{
    need_keys_added();
    return MetadataConstIterator(_imp->keys.begin());
}

MetadataKeyHolder::MetadataConstIterator
MetadataKeyHolder::end_metadata() const
{
    need_keys_added();
    return MetadataConstIterator(_imp->keys.end());
}

MetadataKeyHolder::MetadataConstIterator
MetadataKeyHolder::find_metadata(const std::string & s) const
{
    using namespace std::tr1::placeholders;

    need_keys_added();

    // std::tr1::mem_fn on a sptr doesn't work with boost
    // return std::find_if(begin_metadata(), end_metadata(),
    //        std::tr1::bind(std::equal_to<std::string>(), s, std::tr1::bind(std::tr1::mem_fn(&MetadataKey::raw_name), _1)));

    for (MetadataConstIterator i(begin_metadata()), i_end(end_metadata()) ;
            i != i_end ; ++i)
        if ((*i)->raw_name() == s)
            return i;
    return end_metadata();
}

void
MetadataKeyHolder::clear_metadata_keys() const
{
    _imp->keys.clear();
}

template class WrappedForwardIterator<MetadataKeyHolder::MetadataConstIteratorTag, const std::tr1::shared_ptr<const MetadataKey> >;

