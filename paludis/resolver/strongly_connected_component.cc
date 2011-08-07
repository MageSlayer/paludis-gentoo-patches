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

#include <paludis/resolver/strongly_connected_component.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/util/set-impl.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/wrapped_output_iterator-impl.hh>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template class Set<Resolvent>;
    template class WrappedForwardIterator<Set<Resolvent>::ConstIteratorTag, const Resolvent>;
    template class WrappedOutputIterator<Set<Resolvent>::InserterTag, Resolvent>;

    template class Sequence<StronglyConnectedComponent>;
    template class WrappedForwardIterator<Sequence<StronglyConnectedComponent>::ConstIteratorTag, const StronglyConnectedComponent>;
}
