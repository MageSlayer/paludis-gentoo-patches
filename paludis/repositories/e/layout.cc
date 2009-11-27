/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009 Ciaran McCreesh
 * Copyright (c) 2006 Danny van Dyk
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

#include <paludis/repositories/e/layout.hh>
#include <paludis/repositories/e/traditional_layout.hh>
#include <paludis/repositories/e/exheres_layout.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/instantiation_policy-impl.hh>
#include <paludis/util/map-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>

using namespace paludis;
using namespace paludis::erepository;

template class InstantiationPolicy<LayoutFactory, instantiation_method::SingletonTag>;

Layout::Layout(const std::tr1::shared_ptr<const FSEntrySequence> & l) :
    _master_repositories_locations(l)
{
}

Layout::~Layout()
{
}

const std::tr1::shared_ptr<const FSEntrySequence>
Layout::master_repositories_locations() const
{
    return _master_repositories_locations;
}

FSEntry
Layout::sync_filter_file() const
{
    return FSEntry("/dev/null");
}

namespace
{
    template <typename T_>
    std::tr1::shared_ptr<Layout>
    make_layout(const ERepository * const n, const FSEntry & b,
            std::tr1::shared_ptr<const FSEntrySequence> f)
    {
        return std::tr1::shared_ptr<Layout>(new T_(n, b, f));
    }
}

LayoutFactory::LayoutFactory()
{
}

const std::tr1::shared_ptr<Layout>
LayoutFactory::create(
        const std::string & s,
        const ERepository * const r,
        const FSEntry & f,
        const std::tr1::shared_ptr<const FSEntrySequence> & ff) const
{
    if (s == "traditional")
        return make_layout<TraditionalLayout>(r, f, ff);
    if (s == "exheres")
        return make_layout<ExheresLayout>(r, f, ff);
    throw ConfigurationError("Unrecognised layout '" + s + "'");
}

template class Map<FSEntry, std::string>;
template class WrappedForwardIterator<Map<FSEntry, std::string>::ConstIteratorTag, const std::pair<const FSEntry, std::string> >;

