/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008 Ciaran McCreesh
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
#include <paludis/util/virtual_constructor-impl.hh>
#include <paludis/util/instantiation_policy-impl.hh>
#include <paludis/util/map-impl.hh>

using namespace paludis;
using namespace paludis::erepository;

template class VirtualConstructor<std::string,
         std::tr1::shared_ptr<Layout> (*) (const ERepository * const, const FSEntry &,
                 std::tr1::shared_ptr<const ERepositoryEntries>,
                 std::tr1::shared_ptr<const FSEntry>),
         virtual_constructor_not_found::ThrowException<NoSuchLayoutType> >;

template class InstantiationPolicy<LayoutMaker, instantiation_method::SingletonTag>;

template class Map<FSEntry, std::string>;

Layout::Layout(std::tr1::shared_ptr<const FSEntry> l) :
    _master_repository_location(l)
{
}

Layout::~Layout()
{
}

std::tr1::shared_ptr<const FSEntry>
Layout::master_repository_location() const
{
    return _master_repository_location;
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
            std::tr1::shared_ptr<const ERepositoryEntries> e,
            std::tr1::shared_ptr<const FSEntry> f)
    {
        return std::tr1::shared_ptr<Layout>(new T_(n, b, e, f));
    }
}

LayoutMaker::LayoutMaker()
{
    register_maker("traditional", &make_layout<TraditionalLayout>);
    register_maker("exheres", &make_layout<ExheresLayout>);
}

NoSuchLayoutType::NoSuchLayoutType(const std::string & format) throw () :
    ConfigurationError("No available maker for E repository layout type '" + format + "'")
{
}

