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

#ifndef PALUDIS_GUARD_PALUDIS_RANGE_REWRITER_HH
#define PALUDIS_GUARD_PALUDIS_RANGE_REWRITER_HH 1

#include <paludis/spec_tree.hh>
#include <paludis/util/pimp.hh>

/** \file
 * Declarations for RangeRewriter, which is used internally by Deplist.
 *
 * \ingroup g_dep_list
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    /**
     * Rewrite a DepSpec heirarchy to replace AllDepSpec and AnyDepSpec
     * collections of PackageDepSpec with a single PackageDepSpec using ranged
     * dependencies.
     *
     * \ingroup g_dep_list
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE RangeRewriter :
        private Pimp<RangeRewriter>
    {
        public:
            ///\name Basic operations
            ///\{

            RangeRewriter();
            virtual ~RangeRewriter();

            ///\}

            /**
             * Our rewritten spec, or a zero pointer if we couldn't do any
             * rewriting.
             */
            std::shared_ptr<PackageDepSpec> spec() const;

            ///\name Visit methods
            ///\{

            void visit(const DependencySpecTree::NodeType<AnyDepSpec>::Type & node);
            void visit(const DependencySpecTree::NodeType<AllDepSpec>::Type & node);
            void visit(const DependencySpecTree::NodeType<ConditionalDepSpec>::Type & node);
            void visit(const DependencySpecTree::NodeType<PackageDepSpec>::Type & node);
            void visit(const DependencySpecTree::NodeType<BlockDepSpec>::Type & node);
            void visit(const DependencySpecTree::NodeType<DependenciesLabelsDepSpec>::Type & node);
            void visit(const DependencySpecTree::NodeType<NamedSetDepSpec>::Type & node);

            ///\}
    };
}

#endif
