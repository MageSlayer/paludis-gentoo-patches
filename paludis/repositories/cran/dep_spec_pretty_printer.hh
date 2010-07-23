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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_CRAN_DEP_SPEC_PRETTY_PRINTER_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_CRAN_DEP_SPEC_PRETTY_PRINTER_HH 1

#include <paludis/util/pimp.hh>
#include <paludis/spec_tree.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/formatter.hh>
#include <paludis/environment-fwd.hh>
#include <iosfwd>

namespace paludis
{
    namespace cranrepository
    {
        /**
         * Pretty printer for CRAN dep heirarchies.
         *
         * \ingroup grpcranrepository
         */
        class PALUDIS_VISIBLE DepSpecPrettyPrinter :
            private Pimp<DepSpecPrettyPrinter>
        {
            friend std::ostream & operator<< (std::ostream &, const DepSpecPrettyPrinter &);

            public:
                ///\name Basic operations
                ///\{

                /**
                 * Constructor.
                 *
                 * \param env An optional environment, to use for formatting PackageDepSpec items
                 *   as format::Installed() etc. May be null, in which case format::Plain() is
                 *   always used.
                 *
                 * \param formatter The formatter to use. If no fancy formatting is required, use
                 *   StringifyFormatter.
                 *
                 * \param initial_indent Amount of indenting to use. Should probably be 0 if
                 *   use_newlines is false.
                 *
                 * \param use_newlines Whether to format over multiple lines.
                 */
                DepSpecPrettyPrinter(
                        const Environment * const env,
                        const GenericSpecTree::ItemFormatter & formatter,
                        unsigned initial_indent,
                        bool use_newlines);

                ~DepSpecPrettyPrinter();

                ///\}

                void visit(const DependencySpecTree::NodeType<AllDepSpec>::Type & node);
                void visit(const DependencySpecTree::NodeType<AnyDepSpec>::Type & node);
                void visit(const DependencySpecTree::NodeType<ConditionalDepSpec>::Type & node);
                void visit(const DependencySpecTree::NodeType<BlockDepSpec>::Type & node);
                void visit(const DependencySpecTree::NodeType<PackageDepSpec>::Type & node);
                void visit(const DependencySpecTree::NodeType<DependenciesLabelsDepSpec>::Type & node);
                void visit(const DependencySpecTree::NodeType<NamedSetDepSpec>::Type & node);
        };

        std::ostream & operator<< (std::ostream & s, const DepSpecPrettyPrinter & p) PALUDIS_VISIBLE;
    }
}

#endif
