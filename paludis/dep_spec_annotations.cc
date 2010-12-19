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
    Pimp<DepSpecAnnotations>()
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
}

DepSpecAnnotations::ConstIterator
DepSpecAnnotations::find(const std::string & s) const
{
    return ConstIterator(std::find_if(_imp->annotations.begin(), _imp->annotations.end(),
                std::bind(&key_is, std::placeholders::_1, s)));
}

void
DepSpecAnnotations::add(const DepSpecAnnotation & a)
{
    _imp->annotations.push_back(a);
}

template class Pimp<DepSpecAnnotations>;
template class WrappedForwardIterator<DepSpecAnnotations::ConstIteratorTag, const DepSpecAnnotation>;

