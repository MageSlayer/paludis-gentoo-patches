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
        const unsigned indent;
        const bool multiline;
        bool need_comma;

        Implementation(const unsigned u, const bool m) :
            indent(u),
            multiline(m),
            need_comma(false)
        {
        }
    };
}

DepSpecPrettyPrinter::DepSpecPrettyPrinter(const unsigned initial_indent, const bool multiline) :
    PrivateImplementationPattern<DepSpecPrettyPrinter>(new Implementation<DepSpecPrettyPrinter>(initial_indent, multiline))
{
}

DepSpecPrettyPrinter::~DepSpecPrettyPrinter()
{
}

void
DepSpecPrettyPrinter::visit_leaf(const PackageDepSpec & p)
{
    if (_imp->multiline)
        _imp->s << std::string(_imp->indent, ' ');
    else if (_imp->need_comma)
        _imp->s << ", ";
    else
        _imp->need_comma = true;

    _imp->s << stringify(p);

    if (_imp->multiline)
        _imp->s << std::endl;
}

void
DepSpecPrettyPrinter::visit_leaf(const BlockDepSpec &)
{
}

std::ostream &
paludis::cranrepository::operator<< (std::ostream & s, const DepSpecPrettyPrinter & p)
{
    s << p._imp->s.str();
    return s;
}

