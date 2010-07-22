/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_DEP_ATOM_PRETTY_PRINTER_HH
#define PALUDIS_GUARD_PALUDIS_DEP_ATOM_PRETTY_PRINTER_HH 1

#include <iosfwd>
#include <paludis/dep_spec.hh>
#include <paludis/spec_tree.hh>
#include <paludis/metadata_key-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/package_id-fwd.hh>

/** \file
 * Declarations for the paludis::erepository::DepSpecPrettyPrinter class.
 *
 * \ingroup grperepository
 */

namespace paludis
{
    namespace erepository
    {
        /**
         * Pretty print dependency specs.
         *
         * \ingroup grperepository
         */
        class PALUDIS_VISIBLE DepSpecPrettyPrinter :
            private PrivateImplementationPattern<DepSpecPrettyPrinter>
        {
            friend std::ostream & operator<< (std::ostream &, const DepSpecPrettyPrinter &);

            private:
                void do_annotations(const DepSpec &);

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
                 * \param id The ID to use for determining use flag formatting. May be null, in
                 *   which case format::Plain() is used.
                 *
                 * \param formatter The formatter to use. If no fancy formatting is required, use
                 *   StringifyFormatter.
                 *
                 * \param initial_indent Amount of indenting to use. Should probably be 0 if
                 *   use_newlines is false.
                 *
                 * \param use_newlines Whether to format over multiple lines.
                 *
                 * \param check_conditions Whether to check conditions and format as appropriate.
                 *   When writing cache files, formatting isn't useful and we don't have the choices
                 *   key ready yet.
                 */
                DepSpecPrettyPrinter(
                        const Environment * const env,
                        const std::shared_ptr<const PackageID> & id,
                        const GenericSpecTree::ItemFormatter & formatter,
                        unsigned initial_indent,
                        bool use_newlines,
                        bool check_conditions);

                ~DepSpecPrettyPrinter();

                ///\}

                /// \name Visit functions
                ///{

                void visit(const GenericSpecTree::NodeType<AllDepSpec>::Type & node);
                void visit(const GenericSpecTree::NodeType<AnyDepSpec>::Type & node);
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

                ///}
        };

        /**
         * Output a DepSpecPrettyPrinter to an ostream.
         *
         * \ingroup grperepository
         */
        std::ostream & operator<< (std::ostream & s, const DepSpecPrettyPrinter & p) PALUDIS_VISIBLE;
    }
}

#endif
