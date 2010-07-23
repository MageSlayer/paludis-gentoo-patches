/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#include <paludis/elike_annotations.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/map.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/literal_metadata_key.hh>

using namespace paludis;

namespace paludis
{
    template <>
    struct Imp<ELikeAnnotations>
    {
    };
}

ELikeAnnotations::ELikeAnnotations(const std::shared_ptr<const Map<std::string, std::string> > & m) :
    Pimp<ELikeAnnotations>()
{
    for (Map<std::string, std::string>::ConstIterator k(m->begin()), k_end(m->end()) ;
            k != k_end ; ++k)
        add_metadata_key(std::make_shared<LiteralMetadataValueKey<std::string>>(
                        k->first, k->first, mkt_normal, k->second));
}

ELikeAnnotations::~ELikeAnnotations()
{
}


void
ELikeAnnotations::need_keys_added() const
{
}

const std::string
ELikeAnnotations::human_name() const
{
    return "Annotations";
}

const std::string
ELikeAnnotations::raw_name() const
{
    return "Annotations";
}

MetadataKeyType
ELikeAnnotations::type() const
{
    return mkt_normal;
}

template class Pimp<ELikeAnnotations>;

