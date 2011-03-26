/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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

#include <paludis/resolver/make_destination_filtered_generator_helper.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/destination_utils.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/set.hh>
#include <paludis/filter.hh>
#include <paludis/filter_handler.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/generator.hh>
#include <paludis/repository.hh>
#include <paludis/metadata_key.hh>
#include <paludis/environment.hh>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <>
    struct Imp<MakeDestinationFilteredGeneratorHelper>
    {
        const Environment * const env;

        Imp(const Environment * const e) :
            env(e)
        {
        }
    };
}

MakeDestinationFilteredGeneratorHelper::MakeDestinationFilteredGeneratorHelper(const Environment * const e) :
    _imp(e)
{
}

MakeDestinationFilteredGeneratorHelper::~MakeDestinationFilteredGeneratorHelper() = default;

FilteredGenerator
MakeDestinationFilteredGeneratorHelper::operator() (
        const Generator & g,
        const std::shared_ptr<const Resolution> & r) const
{
    return destination_filtered_generator(_imp->env, r->resolvent().destination_type(), g);
}

template class Pimp<MakeDestinationFilteredGeneratorHelper>;

