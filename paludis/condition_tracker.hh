/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 David Leverton
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

#ifndef PALUDIS_GUARD_PALUDIS_CONDITION_TRACKER_HH
#define PALUDIS_GUARD_PALUDIS_CONDITION_TRACKER_HH 1

#include <paludis/util/visitor.hh>
#include <paludis/dep_tree.hh>
#include <paludis/dep_spec-fwd.hh>
#include <tr1/memory>
#include <tr1/functional>

/** \file
 * Declarations for ConditionTracker, which is used internally by DepList.
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
     * ConditionTracker is used by DepList to track the conditions under which a
     * dependency was pulled in.
     *
     * \ingroup g_dep_list
     * \nosubgrouping
     */
    class ConditionTracker :
        public ConstVisitor<DependencySpecTree>,
        public ConstVisitor<DependencySpecTree>::VisitConstSequence<ConditionTracker, AllDepSpec>
    {
        private:
            std::tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> > base;
            std::tr1::function<void (std::tr1::shared_ptr<ConstAcceptInterface<DependencySpecTree> >)> adder;

            template <typename T_>
            void do_visit_sequence(const T_ &,
                        DependencySpecTree::ConstSequenceIterator,
                        DependencySpecTree::ConstSequenceIterator);

            template <typename T_>
            std::tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> > do_add_sequence(const T_ &);
            template <typename T_>
            std::tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> > do_add_leaf(const T_ &);

        public:
            ///\name Basic operations
            ///\{

            ConditionTracker(std::tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> >);

            virtual ~ConditionTracker();

            ///\}

            ///\name Add a condition
            ///\{

            std::tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> > add_condition(const AnyDepSpec &);
            std::tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> > add_condition(const ConditionalDepSpec &);
            std::tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> > add_condition(const PackageDepSpec &);
            std::tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> > add_condition(const BlockDepSpec &);

            ///\}

            ///\name Visit methods
            ///\{

            using ConstVisitor<DependencySpecTree>::VisitConstSequence<ConditionTracker, AllDepSpec>::visit_sequence;

            void visit_sequence(const AnyDepSpec &, DependencySpecTree::ConstSequenceIterator, DependencySpecTree::ConstSequenceIterator);
            void visit_sequence(const ConditionalDepSpec &, DependencySpecTree::ConstSequenceIterator, DependencySpecTree::ConstSequenceIterator);

            void visit_leaf(const PackageDepSpec &) PALUDIS_ATTRIBUTE((noreturn));
            void visit_leaf(const BlockDepSpec &) PALUDIS_ATTRIBUTE((noreturn));
            void visit_leaf(const DependencyLabelsDepSpec &) PALUDIS_ATTRIBUTE((noreturn));
            void visit_leaf(const NamedSetDepSpec &) PALUDIS_ATTRIBUTE((noreturn));

            ///\}
    };
}

#endif
