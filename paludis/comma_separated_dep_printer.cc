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

#include <paludis/comma_separated_dep_printer.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/options.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/environment.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/action.hh>
#include <sstream>

using namespace paludis;

namespace paludis
{
    template <>
    struct Imp<CommaSeparatedDepPrinter>
    {
        std::stringstream s;
        const Environment * const env;
        DependencySpecTree::ItemFormatter formatter;
        const unsigned indent;
        const bool flat;
        bool need_comma;

        Imp(
                const Environment * const e,
                const DependencySpecTree::ItemFormatter & f,
                const unsigned u,
                const bool b) :
            env(e),
            formatter(f),
            indent(u),
            flat(b),
            need_comma(false)
        {
        }
    };
}

CommaSeparatedDepPrinter::CommaSeparatedDepPrinter(const Environment * const e,
        const DependencySpecTree::ItemFormatter & f, const bool flat) :
    Pimp<CommaSeparatedDepPrinter>(e, f, 0, flat)
{
}

CommaSeparatedDepPrinter::~CommaSeparatedDepPrinter()
{
}

void
CommaSeparatedDepPrinter::visit(const DependencySpecTree::NodeType<PackageDepSpec>::Type & node)
{
    if (! _imp->flat)
        _imp->s << _imp->formatter.indent(_imp->indent);
    else if (_imp->need_comma)
        _imp->s << ", ";
    else
        _imp->need_comma = true;

    if (_imp->env)
    {
        if (! (*_imp->env)[selection::SomeArbitraryVersion(generator::Matches(*node.spec(), MatchPackageOptions()) |
                    filter::InstalledAtRoot(_imp->env->root()))]->empty())
            _imp->s << _imp->formatter.format(*node.spec(), format::Installed());
        else if (! (*_imp->env)[selection::SomeArbitraryVersion(generator::Matches(*node.spec(), MatchPackageOptions()) |
                    filter::SupportsAction<InstallAction>() | filter::NotMasked())]->empty())
            _imp->s << _imp->formatter.format(*node.spec(), format::Installable());
        else
            _imp->s << _imp->formatter.format(*node.spec(), format::Plain());
    }
    else
        _imp->s << _imp->formatter.format(*node.spec(), format::Plain());

    if (! _imp->flat)
        _imp->s << _imp->formatter.newline();
}

void
CommaSeparatedDepPrinter::visit(const DependencySpecTree::NodeType<BlockDepSpec>::Type &)
{
}

void
CommaSeparatedDepPrinter::visit(const DependencySpecTree::NodeType<DependenciesLabelsDepSpec>::Type &)
{
}

void
CommaSeparatedDepPrinter::visit(const DependencySpecTree::NodeType<AllDepSpec>::Type & node)
{
    std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
}

void
CommaSeparatedDepPrinter::visit(const DependencySpecTree::NodeType<AnyDepSpec>::Type & node)
{
    std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
}

void
CommaSeparatedDepPrinter::visit(const DependencySpecTree::NodeType<ConditionalDepSpec>::Type & node)
{
    std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
}

void
CommaSeparatedDepPrinter::visit(const DependencySpecTree::NodeType<NamedSetDepSpec>::Type & node)
{
    if (! _imp->flat)
        _imp->s << _imp->formatter.indent(_imp->indent);
    else if (_imp->need_comma)
        _imp->s << ", ";
    else
        _imp->need_comma = true;

    _imp->s << _imp->formatter.format(*node.spec(), format::Plain());

    if (! _imp->flat)
        _imp->s << _imp->formatter.newline();
}

const std::string
CommaSeparatedDepPrinter::result() const
{
    return _imp->s.str();
}

