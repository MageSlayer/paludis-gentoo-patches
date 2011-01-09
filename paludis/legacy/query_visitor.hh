/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_QUERY_VISITOR_HH
#define PALUDIS_GUARD_PALUDIS_QUERY_VISITOR_HH 1

#include <paludis/legacy/dep_list-fwd.hh>
#include <paludis/util/pimp.hh>
#include <paludis/dep_spec-fwd.hh>

/** \file
 * Declarations for QueryVisitor, which is used internally by DepList.
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
     * Used by DepList to check for existing deps.
     *
     * \ingroup g_dep_list
     * \nosubgrouping
     */
    class QueryVisitor
    {
        private:
            Pimp<QueryVisitor> _imp;

        public:
            ///\name Basic operations
            ///\{

            QueryVisitor(const DepList * const, const std::shared_ptr<const DestinationsSet> &,
                    const Environment * const, const std::shared_ptr<const PackageID> &);

            ~QueryVisitor();

            ///\}

            ///\name Visitor operations
            ///\{

            void visit(const DependencySpecTree::NodeType<AllDepSpec>::Type & node);
            void visit(const DependencySpecTree::NodeType<AnyDepSpec>::Type & node);
            void visit(const DependencySpecTree::NodeType<ConditionalDepSpec>::Type & node);
            void visit(const DependencySpecTree::NodeType<PackageDepSpec>::Type & node);
            void visit(const DependencySpecTree::NodeType<BlockDepSpec>::Type & node);
            void visit(const DependencySpecTree::NodeType<DependenciesLabelsDepSpec>::Type & node);
            void visit(const DependencySpecTree::NodeType<NamedSetDepSpec>::Type & node);

            ///\}

            /**
             * Are we matched?
             */
            bool result() const;
    };
}

#endif
