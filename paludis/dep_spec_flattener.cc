/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009 Ciaran McCreesh
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

#include <paludis/dep_spec_flattener.hh>
#include <paludis/dep_spec.hh>
#include <paludis/environment.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/log.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/repository.hh>
#include <list>
#include <algorithm>
#include <set>

using namespace paludis;

namespace paludis
{
#ifndef PALUDIS_NO_DOUBLE_TEMPLATE
    template <>
#endif
    template <typename Heirarchy_, typename Item_>
    struct Implementation<DepSpecFlattener<Heirarchy_, Item_> >
    {
        const Environment * const env;

        std::list<std::tr1::shared_ptr<const Item_> > specs;
        std::set<SetName> recursing_sets;

        Implementation(const Environment * const e) :
            env(e)
        {
        }
    };

    template <>
    struct WrappedForwardIteratorTraits<DepSpecFlattener<ProvideSpecTree, PackageDepSpec>::ConstIteratorTag>
    {
        typedef std::list<std::tr1::shared_ptr<const PackageDepSpec> >::const_iterator UnderlyingIterator;
    };

    template <>
    struct WrappedForwardIteratorTraits<DepSpecFlattener<PlainTextSpecTree, PlainTextDepSpec>::ConstIteratorTag>
    {
        typedef std::list<std::tr1::shared_ptr<const PlainTextDepSpec> >::const_iterator UnderlyingIterator;
    };

    template <>
    struct WrappedForwardIteratorTraits<DepSpecFlattener<SetSpecTree, PackageDepSpec>::ConstIteratorTag>
    {
        typedef std::list<std::tr1::shared_ptr<const PackageDepSpec> >::const_iterator UnderlyingIterator;
    };

    template <>
    struct WrappedForwardIteratorTraits<DepSpecFlattener<SimpleURISpecTree, SimpleURIDepSpec>::ConstIteratorTag>
    {
        typedef std::list<std::tr1::shared_ptr<const SimpleURIDepSpec> >::const_iterator UnderlyingIterator;
    };
}

template <typename Heirarchy_, typename Item_>
DepSpecFlattener<Heirarchy_, Item_>::DepSpecFlattener(
        const Environment * const env) :
    PrivateImplementationPattern<DepSpecFlattener<Heirarchy_, Item_> >(
            new Implementation<DepSpecFlattener<Heirarchy_, Item_> >(env)),
    _imp(PrivateImplementationPattern<DepSpecFlattener<Heirarchy_, Item_> >::_imp)
{
}

template <typename Heirarchy_, typename Item_>
DepSpecFlattener<Heirarchy_, Item_>::~DepSpecFlattener()
{
}

template <typename Heirarchy_, typename Item_>
typename DepSpecFlattener<Heirarchy_, Item_>::ConstIterator
DepSpecFlattener<Heirarchy_, Item_>::begin() const
{
    return ConstIterator(_imp->specs.begin());
}

template <typename Heirarchy_, typename Item_>
typename DepSpecFlattener<Heirarchy_, Item_>::ConstIterator
DepSpecFlattener<Heirarchy_, Item_>::end() const
{
    return ConstIterator(_imp->specs.end());
}

namespace
{
    template <typename Heirarchy_, typename Item_, bool>
    struct HandleNamedSet
    {
        inline static void handle(const typename Heirarchy_::template NodeType<NamedSetDepSpec>::Type &, DepSpecFlattener<Heirarchy_, Item_> &)
        {
        }
    };

    template <typename Heirarchy_, typename Item_>
    struct HandleNamedSet<Heirarchy_, Item_, true>
    {
        inline static void handle(const typename Heirarchy_::template NodeType<NamedSetDepSpec>::Type & node, DepSpecFlattener<Heirarchy_, Item_> & f)
        {
            f.template handle_named_set<true>(*node.spec());
        }
    };
}

template <typename Heirarchy_, typename Item_>
void
DepSpecFlattener<Heirarchy_, Item_>::visit(const typename Heirarchy_::template NodeType<NamedSetDepSpec>::Type & node)
{
    HandleNamedSet<Heirarchy_, Item_, TypeListContains<
        typename Heirarchy_::VisitableTypeList, typename Heirarchy_::template NodeType<NamedSetDepSpec>::Type>::value>::handle(node, *this);
}

namespace
{
    template <typename Heirarchy_, typename Item_, typename MaybeItem_, bool>
    struct HandleItem
    {
        inline static void handle(const typename Heirarchy_::template NodeType<MaybeItem_>::Type &, DepSpecFlattener<Heirarchy_, Item_> &)
        {
        }
    };

    template <typename Heirarchy_, typename Item_, typename MaybeItem_>
    struct HandleItem<Heirarchy_, Item_, MaybeItem_, true>
    {
        inline static void handle(const typename Heirarchy_::template NodeType<MaybeItem_>::Type & node, DepSpecFlattener<Heirarchy_, Item_> & f)
        {
            f.handle_item(*node.spec());
        }
    };
}

template <typename Heirarchy_, typename Item_>
void
DepSpecFlattener<Heirarchy_, Item_>::visit(const typename Heirarchy_::template NodeType<PlainTextDepSpec>::Type & node)
{
    HandleItem<Heirarchy_, Item_, PlainTextDepSpec, TypeListContains<
        typename Heirarchy_::VisitableTypeList, typename Heirarchy_::template NodeType<PlainTextDepSpec>::Type>::value>::handle(node, *this);
}

template <typename Heirarchy_, typename Item_>
void
DepSpecFlattener<Heirarchy_, Item_>::visit(const typename Heirarchy_::template NodeType<PackageDepSpec>::Type & node)
{
    HandleItem<Heirarchy_, Item_, PackageDepSpec, TypeListContains<
        typename Heirarchy_::VisitableTypeList, typename Heirarchy_::template NodeType<PackageDepSpec>::Type>::value>::handle(node, *this);
}

template <typename Heirarchy_, typename Item_>
void
DepSpecFlattener<Heirarchy_, Item_>::visit(const typename Heirarchy_::template NodeType<SimpleURIDepSpec>::Type & node)
{
    HandleItem<Heirarchy_, Item_, SimpleURIDepSpec, TypeListContains<
        typename Heirarchy_::VisitableTypeList, typename Heirarchy_::template NodeType<SimpleURIDepSpec>::Type>::value>::handle(node, *this);
}

template <typename Heirarchy_, typename Item_>
void
DepSpecFlattener<Heirarchy_, Item_>::visit(const typename Heirarchy_::template NodeType<PlainTextLabelDepSpec>::Type &)
{
}

template <typename Heirarchy_, typename Item_>
void
DepSpecFlattener<Heirarchy_, Item_>::visit(const typename Heirarchy_::template NodeType<AllDepSpec>::Type & node)
{
    std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
}

namespace
{
    template <typename Heirarchy_, typename Item_, bool>
    struct HandleAny
    {
        inline static void handle(const typename Heirarchy_::template NodeType<AnyDepSpec>::Type &, DepSpecFlattener<Heirarchy_, Item_> &)
        {
        }
    };

    template <typename Heirarchy_, typename Item_>
    struct HandleAny<Heirarchy_, Item_, true>
    {
        inline static void handle(const typename Heirarchy_::template NodeType<AnyDepSpec>::Type & node, DepSpecFlattener<Heirarchy_, Item_> & f)
        {
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(f));
        }
    };
}

template <typename Heirarchy_, typename Item_>
void
DepSpecFlattener<Heirarchy_, Item_>::visit(const typename Heirarchy_::template NodeType<AnyDepSpec>::Type & node)
{
    HandleAny<Heirarchy_, Item_, TypeListContains<
        typename Heirarchy_::VisitableTypeList, typename Heirarchy_::template NodeType<AnyDepSpec>::Type>::value>::handle(node, *this);
}

namespace
{
    template <typename Heirarchy_, typename Item_, bool>
    struct HandleConditional
    {
        inline static void handle(const typename Heirarchy_::template NodeType<ConditionalDepSpec>::Type &, DepSpecFlattener<Heirarchy_, Item_> &)
        {
        }
    };

    template <typename Heirarchy_, typename Item_>
    struct HandleConditional<Heirarchy_, Item_, true>
    {
        inline static void handle(const typename Heirarchy_::template NodeType<ConditionalDepSpec>::Type & node, DepSpecFlattener<Heirarchy_, Item_> & f)
        {
            if (node.spec()->condition_met())
                std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(f));
        }
    };
}

template <typename Heirarchy_, typename Item_>
void
DepSpecFlattener<Heirarchy_, Item_>::visit(const typename Heirarchy_::template NodeType<ConditionalDepSpec>::Type & node)
{
    HandleConditional<Heirarchy_, Item_, TypeListContains<
        typename Heirarchy_::VisitableTypeList, typename Heirarchy_::template NodeType<ConditionalDepSpec>::Type>::value>::handle(node, *this);
}

template <typename Heirarchy_, typename Item_>
template <bool>
void
DepSpecFlattener<Heirarchy_, Item_>::handle_named_set(const NamedSetDepSpec & spec)
{
    if (! _imp->recursing_sets.insert(spec.name()).second)
        throw RecursivelyDefinedSetError(stringify(spec.name()));

    std::tr1::shared_ptr<const SetSpecTree> set(_imp->env->set(spec.name()));
    if (! set)
        throw NoSuchSetError(stringify(spec.name()));

    set->root()->accept(*this);

    _imp->recursing_sets.erase(spec.name());
}

template <typename Heirarchy_, typename Item_>
void
DepSpecFlattener<Heirarchy_, Item_>::handle_item(const Item_ & spec)
{
    _imp->specs.push_back(std::tr1::static_pointer_cast<const Item_>(spec.clone()));
}

template class DepSpecFlattener<ProvideSpecTree, PackageDepSpec>;
template class DepSpecFlattener<PlainTextSpecTree, PlainTextDepSpec>;
template class DepSpecFlattener<SetSpecTree, PackageDepSpec>;
template class DepSpecFlattener<SimpleURISpecTree, SimpleURIDepSpec>;

template class WrappedForwardIterator<DepSpecFlattener<ProvideSpecTree, PackageDepSpec>::ConstIteratorTag,
         const std::tr1::shared_ptr<const PackageDepSpec> >;
template class WrappedForwardIterator<DepSpecFlattener<PlainTextSpecTree, PlainTextDepSpec>::ConstIteratorTag,
         const std::tr1::shared_ptr<const PlainTextDepSpec> >;
template class WrappedForwardIterator<DepSpecFlattener<SetSpecTree, PackageDepSpec>::ConstIteratorTag,
         const std::tr1::shared_ptr<const PackageDepSpec> >;
template class WrappedForwardIterator<DepSpecFlattener<SimpleURISpecTree, SimpleURIDepSpec>::ConstIteratorTag,
         const std::tr1::shared_ptr<const SimpleURIDepSpec> >;

