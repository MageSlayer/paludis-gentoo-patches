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

#include <paludis/repositories/e/qa/qa_checks_group.hh>
#include <paludis/repositories/e/qa/qa_checks.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/graph.hh>
#include <paludis/util/graph-impl.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/hashed_containers.hh>

#include <list>

using namespace paludis;
using namespace paludis::erepository;

namespace paludis
{
    template <>
    template <typename T_>
    struct Implementation<QAChecksGroup<T_> >
    {
        mutable Mutex mutex;
        DirectedGraph<std::string, int> deps;
        mutable typename MakeHashedMap<std::string, T_>::Type unordered;
        mutable tr1::shared_ptr<std::list<T_> > ordered;
    };
}

template <typename T_>
QAChecksGroup<T_>::QAChecksGroup() :
    PrivateImplementationPattern<QAChecksGroup>(new Implementation<QAChecksGroup>)
{
}

template <typename T_>
QAChecksGroup<T_>::~QAChecksGroup()
{
}


template <typename T_>
typename QAChecksGroup<T_>::ConstIterator
QAChecksGroup<T_>::begin() const
{
    need_ordering();
    return ConstIterator(_imp->ordered->begin());
}

template <typename T_>
typename QAChecksGroup<T_>::ConstIterator
QAChecksGroup<T_>::end() const
{
    need_ordering();
    return ConstIterator(_imp->ordered->end());
}

template <typename T_>
void
QAChecksGroup<T_>::add_check(const std::string & s, const T_ & f)
{
    Lock l(_imp->mutex);

    _imp->deps.add_node(s);
    _imp->ordered.reset();
    _imp->unordered.insert(std::make_pair(s, f));
}

template <typename T_>
void
QAChecksGroup<T_>::add_prerequirement(const std::string & f, const std::string & t)
{
    Lock l(_imp->mutex);

    _imp->deps.add_edge(f, t, 0);
    _imp->ordered.reset();
}

template <typename T_>
void
QAChecksGroup<T_>::need_ordering() const
{
    Lock l(_imp->mutex);

    using namespace tr1::placeholders;

    if (_imp->ordered)
        return;

    std::list<std::string> o;
    _imp->deps.topological_sort(std::back_inserter(o));
    _imp->ordered.reset(new std::list<T_>);
    std::transform(o.begin(), o.end(), std::back_inserter(*_imp->ordered),
            tr1::bind(tr1::mem_fn(&MakeHashedMap<std::string, T_>::Type::operator []), &_imp->unordered, _1));
}

template class QAChecksGroup<TreeCheckFunction>;
template class QAChecksGroup<EclassFileContentsCheckFunction>;
template class QAChecksGroup<CategoryDirCheckFunction>;
template class QAChecksGroup<PackageDirCheckFunction>;
template class QAChecksGroup<PackageIDCheckFunction>;
template class QAChecksGroup<PackageIDFileContentsCheckFunction>;

template class WrappedForwardIterator<QAChecksGroup<TreeCheckFunction>::ConstIteratorTag, TreeCheckFunction>;
template class WrappedForwardIterator<QAChecksGroup<EclassFileContentsCheckFunction>::ConstIteratorTag, EclassFileContentsCheckFunction>;
template class WrappedForwardIterator<QAChecksGroup<CategoryDirCheckFunction>::ConstIteratorTag, CategoryDirCheckFunction>;
template class WrappedForwardIterator<QAChecksGroup<PackageDirCheckFunction>::ConstIteratorTag, PackageDirCheckFunction>;
template class WrappedForwardIterator<QAChecksGroup<PackageIDCheckFunction>::ConstIteratorTag, PackageIDCheckFunction>;
template class WrappedForwardIterator<QAChecksGroup<PackageIDFileContentsCheckFunction>::ConstIteratorTag, PackageIDFileContentsCheckFunction>;

