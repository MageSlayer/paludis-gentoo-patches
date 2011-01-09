/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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

#include <paludis/comma_separated_dep_pretty_printer.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/options.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/metadata_key.hh>
#include <paludis/pretty_printer.hh>
#include <algorithm>
#include <sstream>

using namespace paludis;

namespace paludis
{
    template <>
    struct Imp<CommaSeparatedDepPrettyPrinter>
    {
        std::stringstream s;

        const PrettyPrinter & pretty_printer;
        const PrettyPrintOptions options;

        bool need_comma;

        Imp(
                const PrettyPrinter & p,
                const PrettyPrintOptions & o) :
            pretty_printer(p),
            options(o),
            need_comma(false)
        {
        }
    };
}

CommaSeparatedDepPrettyPrinter::CommaSeparatedDepPrettyPrinter(
        const PrettyPrinter & p, const PrettyPrintOptions & o) :
    _imp(p, o)
{
}

CommaSeparatedDepPrettyPrinter::~CommaSeparatedDepPrettyPrinter()
{
}

void
CommaSeparatedDepPrettyPrinter::visit(const DependencySpecTree::NodeType<PackageDepSpec>::Type & node)
{
    if (_imp->options[ppo_multiline_allowed])
        _imp->s << _imp->pretty_printer.indentify(0);
    else if (_imp->need_comma)
        _imp->s << ", ";
    else
        _imp->need_comma = true;

    _imp->s << _imp->pretty_printer.prettify(*node.spec());

    if (_imp->options[ppo_multiline_allowed])
        _imp->s << _imp->pretty_printer.newline();
}

void
CommaSeparatedDepPrettyPrinter::visit(const DependencySpecTree::NodeType<BlockDepSpec>::Type &)
{
}

void
CommaSeparatedDepPrettyPrinter::visit(const DependencySpecTree::NodeType<DependenciesLabelsDepSpec>::Type &)
{
}

void
CommaSeparatedDepPrettyPrinter::visit(const DependencySpecTree::NodeType<AllDepSpec>::Type & node)
{
    std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
}

void
CommaSeparatedDepPrettyPrinter::visit(const DependencySpecTree::NodeType<AnyDepSpec>::Type & node)
{
    std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
}

void
CommaSeparatedDepPrettyPrinter::visit(const DependencySpecTree::NodeType<ConditionalDepSpec>::Type & node)
{
    std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
}

void
CommaSeparatedDepPrettyPrinter::visit(const DependencySpecTree::NodeType<NamedSetDepSpec>::Type & node)
{
    if (_imp->options[ppo_multiline_allowed])
        _imp->s << _imp->pretty_printer.indentify(0);
    else if (_imp->need_comma)
        _imp->s << ", ";
    else
        _imp->need_comma = true;

    _imp->s << _imp->pretty_printer.prettify(*node.spec());

    if (_imp->options[ppo_multiline_allowed])
        _imp->s << _imp->pretty_printer.newline();
}

const std::string
CommaSeparatedDepPrettyPrinter::result() const
{
    return _imp->s.str();
}

