/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010 Ciaran McCreesh
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
#include <paludis/util/stringify.hh>

#include <paludis/util/sequence-impl.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/wrapped_output_iterator-impl.hh>

#include <ostream>
#include <algorithm>

using namespace paludis;

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
SpecificURILabel<T_>::SpecificURILabel(const std::string & t) :
    _text(t)
{
}

template <typename T_>
SpecificURILabel<T_>::~SpecificURILabel()
{
}

template <typename T_>
const std::string
SpecificURILabel<T_>::text() const
{
    return _text;
}

DependenciesLabel::~DependenciesLabel()
{
}

template <typename T_>
SpecificDependenciesLabel<T_>::SpecificDependenciesLabel(const std::string & t,
        const std::function<bool ()> & e) :
    _text(t),
    _enabled(e)
{
}

template <typename T_>
SpecificDependenciesLabel<T_>::~SpecificDependenciesLabel()
{
}

template <typename T_>
const std::string
SpecificDependenciesLabel<T_>::text() const
{
    return _text;
}

template <typename T_>
bool
SpecificDependenciesLabel<T_>::enabled() const
{
    return _enabled();
}

template class SpecificURILabel<URIMirrorsThenListedLabelTag>;
template class SpecificURILabel<URIMirrorsOnlyLabelTag>;
template class SpecificURILabel<URIListedOnlyLabelTag>;
template class SpecificURILabel<URIListedThenMirrorsLabelTag>;
template class SpecificURILabel<URILocalMirrorsOnlyLabelTag>;
template class SpecificURILabel<URIManualOnlyLabelTag>;

template class SpecificDependenciesLabel<DependenciesBuildLabelTag>;
template class SpecificDependenciesLabel<DependenciesRunLabelTag>;
template class SpecificDependenciesLabel<DependenciesPostLabelTag>;
template class SpecificDependenciesLabel<DependenciesCompileAgainstLabelTag>;
template class SpecificDependenciesLabel<DependenciesFetchLabelTag>;
template class SpecificDependenciesLabel<DependenciesInstallLabelTag>;
template class SpecificDependenciesLabel<DependenciesSuggestionLabelTag>;
template class SpecificDependenciesLabel<DependenciesRecommendationLabelTag>;
template class SpecificDependenciesLabel<DependenciesTestLabelTag>;

template class Sequence<std::shared_ptr<const DependenciesLabel> >;
template class WrappedForwardIterator<Sequence<std::shared_ptr<const DependenciesLabel> >::ConstIteratorTag,
         const std::shared_ptr<const DependenciesLabel> >;
template class WrappedOutputIterator<Sequence<std::shared_ptr<const DependenciesLabel> >::InserterTag,
         std::shared_ptr<const DependenciesLabel> >;


