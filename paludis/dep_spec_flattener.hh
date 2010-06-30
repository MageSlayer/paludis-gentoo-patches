/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_DEP_ATOM_FLATTENER_HH
#define PALUDIS_GUARD_PALUDIS_DEP_ATOM_FLATTENER_HH 1

#include <paludis/environment-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/spec_tree.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/no_type.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <tr1/memory>

/** \file
 * Declarations for DepSpecFlattener.
 *
 * \ingroup g_dep_spec
 *
 * \section Examples
 *
 * - \ref example_dep_spec_flattener.cc "example_dep_spec_flattener.cc"
 */

namespace paludis
{
    /**
     * Extract the enabled components of a dep heirarchy for a particular
     * package. Sets, via NamedSetDepSpec, are automatically expanded.
     *
     * This template can be instantiated as:
     *
     * - DepSpecFlattener<ProvideSpecTree, PackageDepSpec>
     * - DepSpecFlattener<PlainTextSpecTree, PlainTextDepSpec>
     * - DepSpecFlattener<SetSpecTree, PackageDepSpec>
     * - DepSpecFlattener<SimpleURISpecTree, SimpleURIDepSpec>
     *
     * It is <b>not</b> suitable for heirarchies that can contain AnyDepSpec.
     * Any labels are discarded.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     * \nosubgrouping
     */
    template <typename Heirarchy_, typename Item_>
    class PALUDIS_VISIBLE DepSpecFlattener :
        private InstantiationPolicy<DepSpecFlattener<Heirarchy_, Item_>, instantiation_method::NonCopyableTag>,
        private PrivateImplementationPattern<DepSpecFlattener<Heirarchy_, Item_> >
    {
        private:
            typename PrivateImplementationPattern<DepSpecFlattener<Heirarchy_, Item_> >::ImpPtr & _imp;

        public:
            ///\name Visit operations
            ///\{

            void visit(const typename Heirarchy_::template NodeType<NamedSetDepSpec>::Type & node);
            void visit(const typename Heirarchy_::template NodeType<PlainTextDepSpec>::Type & node);
            void visit(const typename Heirarchy_::template NodeType<PackageDepSpec>::Type & node);
            void visit(const typename Heirarchy_::template NodeType<SimpleURIDepSpec>::Type & node);
            void visit(const typename Heirarchy_::template NodeType<PlainTextLabelDepSpec>::Type & node);

            void visit(const typename Heirarchy_::template NodeType<AllDepSpec>::Type & node);
            void visit(const typename Heirarchy_::template NodeType<AnyDepSpec>::Type & node);
            void visit(const typename Heirarchy_::template NodeType<ConditionalDepSpec>::Type & node);

            ///\}

            ///\name Visit implementation operations

            template <bool b_> void handle_named_set(const NamedSetDepSpec & spec);
            void handle_item(const Item_ & spec);

            ///\}

            ///\name Basic operations
            ///\{

            DepSpecFlattener(const Environment * const);

            ~DepSpecFlattener();

            ///\}

            ///\name Iterate over our dep specs
            ///\{

            struct ConstIteratorTag;
            typedef WrappedForwardIterator<ConstIteratorTag, const std::tr1::shared_ptr<const Item_> > ConstIterator;

            ConstIterator begin() const;
            ConstIterator end() const;

            ///\}
    };

    extern template class DepSpecFlattener<ProvideSpecTree, PackageDepSpec>;
    extern template class DepSpecFlattener<PlainTextSpecTree, PlainTextDepSpec>;
    extern template class DepSpecFlattener<SetSpecTree, PackageDepSpec>;
    extern template class DepSpecFlattener<SimpleURISpecTree, SimpleURIDepSpec>;

    extern template class WrappedForwardIterator<DepSpecFlattener<ProvideSpecTree, PackageDepSpec>::ConstIteratorTag,
           const std::tr1::shared_ptr<const PackageDepSpec> >;
    extern template class WrappedForwardIterator<DepSpecFlattener<PlainTextSpecTree, PlainTextDepSpec>::ConstIteratorTag,
           const std::tr1::shared_ptr<const PlainTextDepSpec> >;
    extern template class WrappedForwardIterator<DepSpecFlattener<SetSpecTree, PackageDepSpec>::ConstIteratorTag,
           const std::tr1::shared_ptr<const PackageDepSpec> >;
    extern template class WrappedForwardIterator<DepSpecFlattener<SimpleURISpecTree, SimpleURIDepSpec>::ConstIteratorTag,
           const std::tr1::shared_ptr<const SimpleURIDepSpec> >;
}

#endif
