/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh
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
#include <paludis/util/attributes.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/tr1_memory.hh>
#include <paludis/util/visitor.hh>
#include <paludis/util/no_type.hh>
#include <paludis/util/wrapped_forward_iterator-fwd.hh>
#include <paludis/util/sr.hh>

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
    /** \namespace dep_spec_flattener_internals
     *
     * For internal use by DepSpecFlattener.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    namespace dep_spec_flattener_internals
    {
        /**
         * Implement visit for NamedSetDepSpec, if necessary.
         *
         * \ingroup g_dep_spec
         * \since 0.26
         * \nosubgrouping
         */
        template <typename H_, typename I_, bool b_>
        struct VisitNamedSetDepSpec
        {
            void visit_leaf(const NoType<0u> &);
        };

        /**
         * Implement visit for NamedSetDepSpec, if necessary.
         *
         * \ingroup g_dep_spec
         * \since 0.26
         * \nosubgrouping
         */
        template <typename H_, typename I_>
        class VisitNamedSetDepSpec<H_, I_, true> :
            public virtual visitor_internals::Visits<const TreeLeaf<H_, NamedSetDepSpec> >,
            private PrivateImplementationPattern<VisitNamedSetDepSpec<H_, I_, true> >
        {
            private:
                using PrivateImplementationPattern<VisitNamedSetDepSpec<H_, I_, true> >::_imp;

            protected:
                VisitNamedSetDepSpec();
                ~VisitNamedSetDepSpec();

            public:
                void visit_leaf(const NamedSetDepSpec &);
        };

        /**
         * Implement visit for UseDepSpec, if necessary.
         *
         * \ingroup g_dep_spec
         * \since 0.26
         * \nosubgrouping
         */
        template <typename H_, typename I_, bool b_>
        struct VisitUseDepSpec
        {
            void visit_sequence(const NoType<0u> &);
        };

        /**
         * Implement visit for UseDepSpec, if necessary.
         *
         * \ingroup g_dep_spec
         * \since 0.26
         * \nosubgrouping
         */
        template <typename H_, typename I_>
        struct VisitUseDepSpec<H_, I_, true> :
            virtual visitor_internals::Visits<const ConstTreeSequence<H_, UseDepSpec> >
        {
            void visit_sequence(
                    const UseDepSpec &,
                    typename H_::ConstSequenceIterator,
                    typename H_::ConstSequenceIterator);
        };
    }

    /**
     * Extract the enabled components of a dep heirarchy for a particular
     * package. Sets, via NamedSetDepSpec, are automatically expanded.
     *
     * This template can be instantiated as:
     *
     * - DepSpecFlattener<ProvideSpecTree, PlainTextDepSpec>
     * - DepSpecFlattener<RestrictSpecTree, PlainTextDepSpec>
     * - DepSpecFlattener<SetSpecTree, PackageDepSpec>
     * - DepSpecFlattener<SimpleURISpecTree, SimpleURIDepSpec>
     *
     * It is <b>not</b> suitable for heirarchies that can contain AnyDepSpec
     * or any kind of label.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     * \nosubgrouping
     */
    template <typename Heirarchy_, typename Item_>
    class PALUDIS_VISIBLE DepSpecFlattener :
        private InstantiationPolicy<DepSpecFlattener<Heirarchy_, Item_>, instantiation_method::NonCopyableTag>,
        private PrivateImplementationPattern<DepSpecFlattener<Heirarchy_, Item_> >,
        public ConstVisitor<Heirarchy_>,
        public ConstVisitor<Heirarchy_>::template VisitConstSequence<DepSpecFlattener<Heirarchy_, Item_>, AllDepSpec>,
        public dep_spec_flattener_internals::VisitNamedSetDepSpec<
            Heirarchy_, Item_, ConstVisitor<Heirarchy_>::template Contains<const TreeLeaf<Heirarchy_, NamedSetDepSpec> >::value>,
        public dep_spec_flattener_internals::VisitUseDepSpec<
            Heirarchy_, Item_, ConstVisitor<Heirarchy_>::template Contains<const ConstTreeSequence<Heirarchy_, UseDepSpec> >::value>
    {
        friend class dep_spec_flattener_internals::VisitNamedSetDepSpec<
            Heirarchy_, Item_, ConstVisitor<Heirarchy_>::template Contains<const TreeLeaf<Heirarchy_, NamedSetDepSpec> >::value>;
        friend class dep_spec_flattener_internals::VisitUseDepSpec<
            Heirarchy_, Item_, ConstVisitor<Heirarchy_>::template Contains<const ConstTreeSequence<Heirarchy_, UseDepSpec> >::value>;

        private:
            Implementation<DepSpecFlattener<Heirarchy_, Item_> > * const _imp;

        public:
            ///\name Visit methods
            ///{

            using ConstVisitor<Heirarchy_>::template VisitConstSequence<DepSpecFlattener<Heirarchy_, Item_>, AllDepSpec>::visit_sequence;

            using dep_spec_flattener_internals::VisitUseDepSpec<Heirarchy_, Item_,
                  ConstVisitor<Heirarchy_>::template Contains<const ConstTreeSequence<Heirarchy_, UseDepSpec> >::value>::visit_sequence;

            using dep_spec_flattener_internals::VisitNamedSetDepSpec<Heirarchy_, Item_,
                ConstVisitor<Heirarchy_>::template Contains<const TreeLeaf<Heirarchy_, NamedSetDepSpec> >::value>::visit_leaf;

            void visit_leaf(const Item_ &);

            ///}

            ///\name Basic operations
            ///\{

            /**
             * This constructor only works if we can't contain a UseDepSpec. The second
             * parameter is ignored.
             */
            DepSpecFlattener(const Environment * const,
                    const typename Select<ConstVisitor<Heirarchy_>::template
                    Contains<const ConstTreeSequence<Heirarchy_, UseDepSpec> >::value,
                    NoType<0u>, Empty>::Type & t = Empty::instance);

            DepSpecFlattener(const Environment * const, const PackageID &);

            ~DepSpecFlattener();

            ///\}

            ///\name Iterate over our dep specs
            ///\{

            typedef WrappedForwardIterator<enum ConstIteratorTag { }, const tr1::shared_ptr<const Item_> > ConstIterator;

            ConstIterator begin() const;
            ConstIterator end() const;

            ///\}
    };
}

#endif
