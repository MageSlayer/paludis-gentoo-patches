/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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

#include <paludis/dep_label.hh>
#include <paludis/util/visitor-impl.hh>
#include <ostream>

using namespace paludis;

std::ostream &
paludis::operator<< (std::ostream & s, const URILabel & l)
{
    s << l.text();
    return s;
}

std::ostream &
paludis::operator<< (std::ostream & s, const DependencyLabel & l)
{
    s << l.text();
    return s;
}

URILabel::~URILabel()
{
}

namespace paludis
{
    template <>
    template <typename T_>
    struct Implementation<ConcreteURILabel<T_> >
    {
        const std::string text;

        Implementation(const std::string & t) :
            text(t)
        {
        }
    };
}

template <typename T_>
ConcreteURILabel<T_>::ConcreteURILabel(const std::string & t) :
    PrivateImplementationPattern<ConcreteURILabel<T_> >(new Implementation<ConcreteURILabel<T_> >(t))
{
}

template <typename T_>
ConcreteURILabel<T_>::~ConcreteURILabel()
{
}

template <typename T_>
const std::string
ConcreteURILabel<T_>::text() const
{
    return _imp->text;
}

template class ConcreteURILabel<URIMirrorsThenListedLabel::Tag>;
template class ConcreteURILabel<URIMirrorsOnlyLabel::Tag>;
template class ConcreteURILabel<URIListedOnlyLabel::Tag>;
template class ConcreteURILabel<URIListedThenMirrorsLabel::Tag>;
template class ConcreteURILabel<URILocalMirrorsOnlyLabel::Tag>;
template class ConcreteURILabel<URIManualOnlyLabel::Tag>;

std::ostream & operator<<(std::ostream & s, const DependencyLabel & l)
{
    s << l.text();
    return s;
}

DependencyLabel::~DependencyLabel()
{
}

namespace paludis
{
    template <>
    template <typename T_, typename C_>
    struct Implementation<ConcreteDependencyLabel<T_, C_> >
    {
        const std::string text;

        Implementation(const std::string & t) :
            text(t)
        {
        }
    };
}

template <typename T_, typename C_>
ConcreteDependencyLabel<T_, C_>::ConcreteDependencyLabel(const std::string & t) :
    PrivateImplementationPattern<ConcreteDependencyLabel<T_, C_> >(new Implementation<ConcreteDependencyLabel<T_, C_> >(t))
{
}

template <typename T_, typename C_>
ConcreteDependencyLabel<T_, C_>::~ConcreteDependencyLabel()
{
}

template <typename T_, typename C_>
const std::string
ConcreteDependencyLabel<T_, C_>::text() const
{
    return _imp->text;
}

template class ConcreteDependencyLabel<DependencyHostLabel::Tag, DependencySystemLabel>;
template class ConcreteDependencyLabel<DependencyTargetLabel::Tag, DependencySystemLabel>;
template class ConcreteDependencyLabel<DependencyBuildLabel::Tag, DependencyTypeLabel>;
template class ConcreteDependencyLabel<DependencyRunLabel::Tag, DependencyTypeLabel>;
template class ConcreteDependencyLabel<DependencyInstallLabel::Tag, DependencyTypeLabel>;
template class ConcreteDependencyLabel<DependencyCompileLabel::Tag, DependencyTypeLabel>;
template class ConcreteDependencyLabel<DependencySuggestedLabel::Tag, DependencySuggestLabel>;
template class ConcreteDependencyLabel<DependencyRecommendedLabel::Tag, DependencySuggestLabel>;
template class ConcreteDependencyLabel<DependencyRequiredLabel::Tag, DependencySuggestLabel>;
template class ConcreteDependencyLabel<DependencyAnyLabel::Tag, DependencyABIsLabel>;
template class ConcreteDependencyLabel<DependencyMineLabel::Tag, DependencyABIsLabel>;
template class ConcreteDependencyLabel<DependencyPrimaryLabel::Tag, DependencyABIsLabel>;
template class ConcreteDependencyLabel<DependencyABILabel::Tag, DependencyABIsLabel>;

