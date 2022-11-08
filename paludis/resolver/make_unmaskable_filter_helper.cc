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

#include <paludis/resolver/make_unmaskable_filter_helper.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/set.hh>
#include <paludis/filter.hh>
#include <paludis/filter_handler.hh>
#include <paludis/mask.hh>
#include <paludis/mask_utils.hh>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <>
    struct Imp<MakeUnmaskableFilterHelper>
    {
        const Environment * const env;
        bool override_masks;

        Imp(const Environment * const e) :
            env(e),
            override_masks(true)
        {
        }
    };
}

MakeUnmaskableFilterHelper::MakeUnmaskableFilterHelper(const Environment * const e) :
    _imp(e)
{
}

MakeUnmaskableFilterHelper::~MakeUnmaskableFilterHelper() = default;

void
MakeUnmaskableFilterHelper::set_override_masks(const bool v)
{
    _imp->override_masks = v;
}

namespace
{
    struct UnmaskableFilterHandler :
        AllFilterHandlerBase
    {
        std::shared_ptr<const PackageIDSet> ids(
                const Environment * const,
                const std::shared_ptr<const PackageIDSet> & ids) const override
        {
            std::shared_ptr<PackageIDSet> result(std::make_shared<PackageIDSet>());

            for (const auto & id : *ids)
                if (not_strongly_masked(id))
                    result->insert(id);

            return result;
        }

        std::string as_string() const override
        {
            return "unmaskable";
        }
    };

    struct UnmaskableFilter :
        Filter
    {
        UnmaskableFilter() :
            Filter(std::make_shared<UnmaskableFilterHandler>())
        {
        }
    };
}

Filter
MakeUnmaskableFilterHelper::operator() (
        const QualifiedPackageName &) const
{
    if (_imp->override_masks)
        return UnmaskableFilter();
    else
        return filter::NotMasked();
}

namespace paludis
{
    template class Pimp<MakeUnmaskableFilterHelper>;
}
