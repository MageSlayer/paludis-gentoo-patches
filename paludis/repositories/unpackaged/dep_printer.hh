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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNPACKAGED_DEP_PRINTER_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNPACKAGED_DEP_PRINTER_HH 1

#include <paludis/util/visitor.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/dep_tree.hh>
#include <paludis/formatter.hh>

namespace paludis
{
    namespace unpackaged_repositories
    {
        class DepPrinter :
            public ConstVisitor<DependencySpecTree>,
            public ConstVisitor<DependencySpecTree>::VisitConstSequence<DepPrinter, AllDepSpec>,
            public ConstVisitor<DependencySpecTree>::VisitConstSequence<DepPrinter, AnyDepSpec>,
            public ConstVisitor<DependencySpecTree>::VisitConstSequence<DepPrinter, UseDepSpec>,
            private PrivateImplementationPattern<DepPrinter>
        {
            public:
                DepPrinter(const DependencySpecTree::ItemFormatter &, const bool);
                ~DepPrinter();

                const std::string result() const;

                void visit_leaf(const BlockDepSpec &);
                void visit_leaf(const PackageDepSpec &);
                void visit_leaf(const NamedSetDepSpec &);
                void visit_leaf(const DependencyLabelsDepSpec &);

                using ConstVisitor<DependencySpecTree>::VisitConstSequence<DepPrinter, AllDepSpec>::visit_sequence;
                using ConstVisitor<DependencySpecTree>::VisitConstSequence<DepPrinter, AnyDepSpec>::visit_sequence;
                using ConstVisitor<DependencySpecTree>::VisitConstSequence<DepPrinter, UseDepSpec>::visit_sequence;
        };
    }
}

#endif
