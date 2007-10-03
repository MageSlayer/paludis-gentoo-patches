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

#include <paludis/repositories/cran/dep_spec_pretty_printer.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/environment.hh>
#include <paludis/query.hh>
#include <paludis/package_database.hh>
#include <ostream>
#include <sstream>

using namespace paludis;
using namespace paludis::cranrepository;

namespace paludis
{
    template <>
    struct Implementation<DepSpecPrettyPrinter>
    {
        std::stringstream s;
        const Environment * const env;
        GenericSpecTree::Formatter formatter;
        const unsigned indent;
        const bool multiline;
        bool need_comma;

        Implementation(
                const Environment * const e,
                const GenericSpecTree::Formatter & f,
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
        const GenericSpecTree::Formatter & f, const unsigned initial_indent, const bool multiline) :
    PrivateImplementationPattern<DepSpecPrettyPrinter>(new Implementation<DepSpecPrettyPrinter>(e, f, initial_indent, multiline))
{
}

DepSpecPrettyPrinter::~DepSpecPrettyPrinter()
{
}

void
DepSpecPrettyPrinter::visit_leaf(const PackageDepSpec & p)
{
    if (_imp->multiline)
        _imp->s << _imp->formatter.indent(_imp->indent);
    else if (_imp->need_comma)
        _imp->s << ", ";
    else
        _imp->need_comma = true;

    if (_imp->env)
    {
        if (! _imp->env->package_database()->query(query::Matches(p) &
                    query::InstalledAtRoot(_imp->env->root()), qo_whatever)->empty())
            _imp->s << _imp->formatter.format(p, format::Installed());
        else if (! _imp->env->package_database()->query(query::Matches(p) &
                    query::SupportsAction<InstallAction>() & query::NotMasked(), qo_whatever)->empty())
            _imp->s << _imp->formatter.format(p, format::Installable());
        else
            _imp->s << _imp->formatter.format(p, format::Plain());
    }
    else
        _imp->s << _imp->formatter.format(p, format::Plain());

    if (_imp->multiline)
        _imp->s << _imp->formatter.newline();
}

void
DepSpecPrettyPrinter::visit_leaf(const BlockDepSpec &)
{
}

void
DepSpecPrettyPrinter::visit_leaf(const DependencyLabelsDepSpec &)
{
}

std::ostream &
paludis::cranrepository::operator<< (std::ostream & s, const DepSpecPrettyPrinter & p)
{
    s << p._imp->s.str();
    return s;
}

