/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_SPEC_TREE_PRETTY_PRINTER_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_SPEC_TREE_PRETTY_PRINTER_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/dep_spec.hh>
#include <paludis/spec_tree.hh>
#include <paludis/metadata_key-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/pretty_print_options-fwd.hh>
#include <paludis/pretty_printer-fwd.hh>

namespace paludis
{
    namespace erepository
    {
        class PALUDIS_VISIBLE SpecTreePrettyPrinter :
            private Pimp<SpecTreePrettyPrinter>
        {
            friend std::ostream & operator<< (std::ostream &, const SpecTreePrettyPrinter &);

            private:
                void do_annotations(const DepSpec &);

            public:
                SpecTreePrettyPrinter(
                        const PrettyPrinter &,
                        const PrettyPrintOptions &
                        );

                ~SpecTreePrettyPrinter();

                void visit(const GenericSpecTree::NodeType<AllDepSpec>::Type & node);
                void visit(const GenericSpecTree::NodeType<AnyDepSpec>::Type & node);
                void visit(const GenericSpecTree::NodeType<ExactlyOneDepSpec>::Type & node);
                void visit(const GenericSpecTree::NodeType<ConditionalDepSpec>::Type & node);
                void visit(const GenericSpecTree::NodeType<PackageDepSpec>::Type & node);
                void visit(const GenericSpecTree::NodeType<BlockDepSpec>::Type & node);
                void visit(const GenericSpecTree::NodeType<DependenciesLabelsDepSpec>::Type & node);
                void visit(const GenericSpecTree::NodeType<NamedSetDepSpec>::Type & node);
                void visit(const GenericSpecTree::NodeType<SimpleURIDepSpec>::Type & node);
                void visit(const GenericSpecTree::NodeType<FetchableURIDepSpec>::Type & node);
                void visit(const GenericSpecTree::NodeType<URILabelsDepSpec>::Type & node);
                void visit(const GenericSpecTree::NodeType<PlainTextDepSpec>::Type & node);
                void visit(const GenericSpecTree::NodeType<PlainTextLabelDepSpec>::Type & node);
                void visit(const GenericSpecTree::NodeType<LicenseDepSpec>::Type & node);
        };

        std::ostream & operator<< (std::ostream & s, const SpecTreePrettyPrinter & p) PALUDIS_VISIBLE;
    }
}

#endif
