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

#include <paludis/repositories/cran/spec_tree_pretty_printer.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/options.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/pretty_printer.hh>
#include <algorithm>
#include <ostream>
#include <sstream>

using namespace paludis;
using namespace paludis::cranrepository;

namespace paludis
{
    template <>
    struct Imp<SpecTreePrettyPrinter>
    {
        std::stringstream s;
        const PrettyPrinter & printer;
        const PrettyPrintOptions options;
        const unsigned indent;
        const bool multiline;
        bool need_comma;

        Imp(
                const PrettyPrinter & p, const PrettyPrintOptions & o) :
            printer(p),
            options(o),
            indent(0),
            multiline(o[ppo_multiline_allowed]),
            need_comma(false)
        {
        }
    };
}

SpecTreePrettyPrinter::SpecTreePrettyPrinter(
        const PrettyPrinter & p, const PrettyPrintOptions & o) :
    Pimp<SpecTreePrettyPrinter>(p, o)
{
}

SpecTreePrettyPrinter::~SpecTreePrettyPrinter()
{
}

void
SpecTreePrettyPrinter::visit(const DependencySpecTree::NodeType<PackageDepSpec>::Type & node)
{
    if (_imp->multiline)
        _imp->s << _imp->printer.indentify(_imp->indent);
    else if (_imp->need_comma)
        _imp->s << ", ";
    else
        _imp->need_comma = true;

    _imp->s << _imp->printer.prettify(*node.spec());

    if (_imp->multiline)
        _imp->s << _imp->printer.newline();
}

void
SpecTreePrettyPrinter::visit(const DependencySpecTree::NodeType<AllDepSpec>::Type & node)
{
    std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
}

void
SpecTreePrettyPrinter::visit(const DependencySpecTree::NodeType<AnyDepSpec>::Type & node)
{
    std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
}

void
SpecTreePrettyPrinter::visit(const DependencySpecTree::NodeType<ConditionalDepSpec>::Type & node)
{
    std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
}

void
SpecTreePrettyPrinter::visit(const DependencySpecTree::NodeType<BlockDepSpec>::Type &)
{
}

void
SpecTreePrettyPrinter::visit(const DependencySpecTree::NodeType<DependenciesLabelsDepSpec>::Type &)
{
}

void
SpecTreePrettyPrinter::visit(const DependencySpecTree::NodeType<NamedSetDepSpec>::Type & node)
{
    if (_imp->multiline)
        _imp->s << _imp->printer.indentify(_imp->indent);
    else if (_imp->need_comma)
        _imp->s << ", ";
    else
        _imp->need_comma = true;

    _imp->s << _imp->printer.prettify(*node.spec());

    if (_imp->multiline)
        _imp->s << _imp->printer.newline();
}

std::ostream &
paludis::cranrepository::operator<< (std::ostream & s, const SpecTreePrettyPrinter & p)
{
    s << p._imp->s.str();
    return s;
}

