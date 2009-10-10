/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009 Ciaran McCreesh
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
#include <paludis/dep_spec.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/stringify.hh>
#include <ostream>
#include <algorithm>

using namespace paludis;

template class Sequence<std::tr1::shared_ptr<const DependenciesLabel> >;

std::ostream &
paludis::operator<< (std::ostream & s, const URILabel & l)
{
    s << l.text();
    return s;
}

std::ostream &
paludis::operator<< (std::ostream & s, const DependenciesLabel & l)
{
    s << l.text();
    return s;
}

URILabel::~URILabel()
{
}

template <typename T_>
ConcreteURILabel<T_>::ConcreteURILabel(const std::string & t) :
    _text(t)
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
    return _text;
}

DependenciesLabel::~DependenciesLabel()
{
}

template <typename T_>
ConcreteDependenciesLabel<T_>::ConcreteDependenciesLabel(const std::string & t) :
    _text(t)
{
}

template <typename T_>
ConcreteDependenciesLabel<T_>::~ConcreteDependenciesLabel()
{
}

template <typename T_>
const std::string
ConcreteDependenciesLabel<T_>::text() const
{
    return _text;
}

template class InstantiationPolicy<DependenciesLabel, instantiation_method::NonCopyableTag>;
template class InstantiationPolicy<URILabel, instantiation_method::NonCopyableTag>;

template class ConcreteURILabel<URIMirrorsThenListedLabelTag>;
template class ConcreteURILabel<URIMirrorsOnlyLabelTag>;
template class ConcreteURILabel<URIListedOnlyLabelTag>;
template class ConcreteURILabel<URIListedThenMirrorsLabelTag>;
template class ConcreteURILabel<URILocalMirrorsOnlyLabelTag>;
template class ConcreteURILabel<URIManualOnlyLabelTag>;

template class ConcreteDependenciesLabel<DependenciesBuildLabelTag>;
template class ConcreteDependenciesLabel<DependenciesRunLabelTag>;
template class ConcreteDependenciesLabel<DependenciesPostLabelTag>;
template class ConcreteDependenciesLabel<DependenciesCompileAgainstLabelTag>;
template class ConcreteDependenciesLabel<DependenciesFetchLabelTag>;
template class ConcreteDependenciesLabel<DependenciesInstallLabelTag>;
template class ConcreteDependenciesLabel<DependenciesSuggestionLabelTag>;
template class ConcreteDependenciesLabel<DependenciesRecommendationLabelTag>;
template class ConcreteDependenciesLabel<DependenciesTestLabelTag>;

