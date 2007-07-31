/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_CRAN_DEP_SPEC_PRETTY_PRINTER_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_CRAN_DEP_SPEC_PRETTY_PRINTER_HH 1

#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/dep_spec.hh>
#include <iosfwd>

namespace paludis
{
    namespace cranrepository
    {
        class DepSpecPrettyPrinter :
            private PrivateImplementationPattern<DepSpecPrettyPrinter>,
            public ConstVisitor<DependencySpecTree>,
            public ConstVisitor<DependencySpecTree>::VisitConstSequence<DepSpecPrettyPrinter, AllDepSpec>,
            public ConstVisitor<DependencySpecTree>::VisitConstSequence<DepSpecPrettyPrinter, UseDepSpec>,
            public ConstVisitor<DependencySpecTree>::VisitConstSequence<DepSpecPrettyPrinter, AnyDepSpec>
        {
            friend std::ostream & operator<< (std::ostream &, const DepSpecPrettyPrinter &);

            public:
                DepSpecPrettyPrinter(const unsigned initial_indent, const bool multiline);
                ~DepSpecPrettyPrinter();

                void visit_leaf(const PackageDepSpec &);

                void visit_leaf(const BlockDepSpec &);
        };

        std::ostream & operator<< (std::ostream & s, const DepSpecPrettyPrinter & p);
    }
}

#endif
