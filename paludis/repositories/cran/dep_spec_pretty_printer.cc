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

#include <paludis/repositories/cran/dep_spec_pretty_printer.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/options.hh>
#include <paludis/environment.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/action-fwd.hh>
#include <ostream>
#include <sstream>

using namespace paludis;
using namespace paludis::cranrepository;

namespace paludis
{
    template <>
    struct Imp<DepSpecPrettyPrinter>
    {
        std::stringstream s;
        const Environment * const env;
        GenericSpecTree::ItemFormatter formatter;
        const unsigned indent;
        const bool multiline;
        bool need_comma;

        Imp(
                const Environment * const e,
                const GenericSpecTree::ItemFormatter & f,
                const unsigned u,
                const bool m) :
            env(e),
            formatter(f),
            indent(u),
            multiline(m),
            need_comma(false)
        {
        }
    };
}

DepSpecPrettyPrinter::DepSpecPrettyPrinter(const Environment * const e,
        const GenericSpecTree::ItemFormatter & f, const unsigned initial_indent, const bool multiline) :
    Pimp<DepSpecPrettyPrinter>(e, f, initial_indent, multiline)
{
}

DepSpecPrettyPrinter::~DepSpecPrettyPrinter()
{
}

void
DepSpecPrettyPrinter::visit(const DependencySpecTree::NodeType<PackageDepSpec>::Type & node)
{
    if (_imp->multiline)
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

    if (_imp->multiline)
        _imp->s << _imp->formatter.newline();
}

void
DepSpecPrettyPrinter::visit(const DependencySpecTree::NodeType<AllDepSpec>::Type & node)
{
    std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
}

void
DepSpecPrettyPrinter::visit(const DependencySpecTree::NodeType<AnyDepSpec>::Type & node)
{
    std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
}

void
DepSpecPrettyPrinter::visit(const DependencySpecTree::NodeType<ConditionalDepSpec>::Type & node)
{
    std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
}

void
DepSpecPrettyPrinter::visit(const DependencySpecTree::NodeType<BlockDepSpec>::Type &)
{
}

void
DepSpecPrettyPrinter::visit(const DependencySpecTree::NodeType<DependenciesLabelsDepSpec>::Type &)
{
}

void
DepSpecPrettyPrinter::visit(const DependencySpecTree::NodeType<NamedSetDepSpec>::Type & node)
{
    if (_imp->multiline)
        _imp->s << _imp->formatter.indent(_imp->indent);
    else if (_imp->need_comma)
        _imp->s << ", ";
    else
        _imp->need_comma = true;

    _imp->s << _imp->formatter.format(*node.spec(), format::Plain());

    if (_imp->multiline)
        _imp->s << _imp->formatter.newline();
}

std::ostream &
paludis::cranrepository::operator<< (std::ostream & s, const DepSpecPrettyPrinter & p)
{
    s << p._imp->s.str();
    return s;
}

