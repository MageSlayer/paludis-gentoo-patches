/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_COMMA_SEPARATED_DEP_PRETTY_PRINTER_HH
#define PALUDIS_GUARD_PALUDIS_COMMA_SEPARATED_DEP_PRETTY_PRINTER_HH 1

#include <paludis/util/pimp.hh>
#include <paludis/spec_tree.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/pretty_printer-fwd.hh>
#include <paludis/pretty_print_options-fwd.hh>

namespace paludis
{
    class PALUDIS_VISIBLE CommaSeparatedDepPrettyPrinter :
        private Pimp<CommaSeparatedDepPrettyPrinter>
    {
        public:
            CommaSeparatedDepPrettyPrinter(
                    const PrettyPrinter &,
                    const PrettyPrintOptions &);

            ~CommaSeparatedDepPrettyPrinter();

            const std::string result() const;

            void visit(const DependencySpecTree::NodeType<PackageDepSpec>::Type & node);
            void visit(const DependencySpecTree::NodeType<BlockDepSpec>::Type & node);
            void visit(const DependencySpecTree::NodeType<DependenciesLabelsDepSpec>::Type & node);
            void visit(const DependencySpecTree::NodeType<NamedSetDepSpec>::Type & node);
            void visit(const DependencySpecTree::NodeType<AllDepSpec>::Type & node);
            void visit(const DependencySpecTree::NodeType<ConditionalDepSpec>::Type & node);
            void visit(const DependencySpecTree::NodeType<AnyDepSpec>::Type & node);
    };
}

#endif
