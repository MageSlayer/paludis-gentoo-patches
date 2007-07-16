/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

/** \file
 * Declarations for the DepSpecPrettyPrinter class.
 *
 * \ingroup grpdepspecprettyprinter
 */

namespace paludis
{
    /**
     * Pretty print dependency specs.
     *
     * \ingroup grpdepspecprettyprinter
     */
    class PALUDIS_VISIBLE DepSpecPrettyPrinter :
        public ConstVisitor<GenericSpecTree>,
        private PrivateImplementationPattern<DepSpecPrettyPrinter>
    {
        friend std::ostream & operator<< (std::ostream &, const DepSpecPrettyPrinter &);

        private:
            std::string newline();
            std::string indent();

        public:
            ///\name Basic operations
            ///\{

            DepSpecPrettyPrinter(unsigned initial_indent, bool use_newlines = true);

            ~DepSpecPrettyPrinter();

            ///\}

            /// \name Visit functions
            ///{

            void visit_sequence(const AllDepSpec &,
                    GenericSpecTree::ConstSequenceIterator,
                    GenericSpecTree::ConstSequenceIterator);

            void visit_sequence(const AnyDepSpec &,
                    GenericSpecTree::ConstSequenceIterator,
                    GenericSpecTree::ConstSequenceIterator);

            void visit_sequence(const UseDepSpec &,
                    GenericSpecTree::ConstSequenceIterator,
                    GenericSpecTree::ConstSequenceIterator);

            void visit_leaf(const PackageDepSpec &);

            void visit_leaf(const PlainTextDepSpec &);

            void visit_leaf(const BlockDepSpec &);

            void visit_leaf(const URIDepSpec &);

            ///}
    };

    /**
     * Output a DepSpecPrettyPrinter to an ostream.
     *
     * \ingroup grpdepspecprettyprinter
     */
    std::ostream & operator<< (std::ostream & s, const DepSpecPrettyPrinter & p) PALUDIS_VISIBLE;
}

#endif
