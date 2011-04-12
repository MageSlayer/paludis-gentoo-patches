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

#include <paludis/dep_spec_annotations.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <ostream>
#include <vector>
#include <algorithm>
#include <functional>

using namespace paludis;

#include <paludis/dep_spec_annotations-se.cc>

typedef std::vector<DepSpecAnnotation> Annotations;

namespace paludis
{
    template <>
    struct Imp<DepSpecAnnotations>
    {
        Annotations annotations;
    };

    template <>
    struct WrappedForwardIteratorTraits<DepSpecAnnotations::ConstIteratorTag>
    {
        typedef Annotations::const_iterator UnderlyingIterator;
    };
}

DepSpecAnnotations::DepSpecAnnotations() :
    _imp()
{
}

DepSpecAnnotations::~DepSpecAnnotations() = default;

DepSpecAnnotations::ConstIterator
DepSpecAnnotations::begin() const
{
    return ConstIterator(_imp->annotations.begin());
}

DepSpecAnnotations::ConstIterator
DepSpecAnnotations::end() const
{
    return ConstIterator(_imp->annotations.end());
}

namespace
{
    bool key_is(const DepSpecAnnotation & a, const std::string & k)
    {
        return a.key() == k;
    }

    bool role_is(const DepSpecAnnotation & a, const DepSpecAnnotationRole k)
    {
        return a.role() == k;
    }
}

DepSpecAnnotations::ConstIterator
DepSpecAnnotations::find(const std::string & s) const
{
    return ConstIterator(std::find_if(_imp->annotations.begin(), _imp->annotations.end(),
                std::bind(&key_is, std::placeholders::_1, s)));
}

DepSpecAnnotations::ConstIterator
DepSpecAnnotations::find(const DepSpecAnnotationRole r) const
{
    return ConstIterator(std::find_if(_imp->annotations.begin(), _imp->annotations.end(),
                std::bind(&role_is, std::placeholders::_1, r)));
}

void
DepSpecAnnotations::add(const DepSpecAnnotation & a)
{
    _imp->annotations.push_back(a);
}

DepSpecAnnotationRole
paludis::find_blocker_role_in_annotations(
        const std::shared_ptr<const DepSpecAnnotations> & maybe_annotations)
{
    if (! maybe_annotations)
        return dsar_none;

    for (auto i(maybe_annotations->begin()), i_end(maybe_annotations->end()) ;
            i != i_end ; ++i)
    {
        switch (i->role())
        {
            case dsar_blocker_manual:
            case dsar_blocker_uninstall_blocked_after:
            case dsar_blocker_uninstall_blocked_before:
            case dsar_blocker_upgrade_blocked_before:
            case dsar_blocker_weak:
            case dsar_blocker_strong:
                return i->role();

            case dsar_none:
            case dsar_general_description:
            case dsar_general_url:
            case dsar_general_note:
            case dsar_general_lang:
            case dsar_general_defined_in:
            case dsar_general_date:
            case dsar_general_author:
            case dsar_general_token:
            case dsar_myoptions_requires:
            case dsar_myoptions_n_at_least_one:
            case dsar_myoptions_n_at_most_one:
            case dsar_myoptions_n_exactly_one:
            case dsar_suggestions_group_name:
            case dsar_system_implicit:
                break;

            case last_dsar:
                throw InternalError(PALUDIS_HERE, "bad dsar");
        }
    }

    return dsar_none;
}

template class Pimp<DepSpecAnnotations>;
template class WrappedForwardIterator<DepSpecAnnotations::ConstIteratorTag, const DepSpecAnnotation>;

