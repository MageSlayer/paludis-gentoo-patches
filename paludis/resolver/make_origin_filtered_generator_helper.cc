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

#include <paludis/resolver/make_origin_filtered_generator_helper.hh>
#include <paludis/resolver/destination_utils.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/set.hh>
#include <paludis/filter.hh>
#include <paludis/filter_handler.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <>
    struct Imp<MakeOriginFilteredGeneratorHelper>
    {
        const Environment * const env;
        bool making_binaries;

        Imp(const Environment * const e) :
            env(e),
            making_binaries(false)
        {
        }
    };
}

MakeOriginFilteredGeneratorHelper::MakeOriginFilteredGeneratorHelper(const Environment * const e) :
    _imp(e)
{
}

MakeOriginFilteredGeneratorHelper::~MakeOriginFilteredGeneratorHelper() = default;

void
MakeOriginFilteredGeneratorHelper::set_making_binaries(const bool v)
{
    _imp->making_binaries = v;
}

namespace
{
    struct BinaryableFilterHandler :
        AllFilterHandlerBase
    {
        std::shared_ptr<const PackageIDSet> ids(
                const Environment * const,
                const std::shared_ptr<const PackageIDSet> & id) const override
        {
            std::shared_ptr<PackageIDSet> result(std::make_shared<PackageIDSet>());

            for (const auto & i : *id)
                if (! is_already_binary(i))
                    result->insert(i);

            return result;
        }

        std::string as_string() const override
        {
            return "binaryable";
        }
    };

    struct BinaryableFilter :
        Filter
    {
        BinaryableFilter() :
            Filter(std::make_shared<BinaryableFilterHandler>())
        {
        }
    };
}

FilteredGenerator
MakeOriginFilteredGeneratorHelper::operator() (const Generator & g) const
{
    if (_imp->making_binaries)
        return g | BinaryableFilter();
    else
        return g;
}

namespace paludis
{
    template class Pimp<MakeOriginFilteredGeneratorHelper>;
}
