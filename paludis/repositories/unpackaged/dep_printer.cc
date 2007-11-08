/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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

#include <paludis/repositories/unpackaged/dep_printer.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <sstream>

using namespace paludis;
using namespace paludis::unpackaged_repositories;

namespace paludis
{
    template <>
    struct Implementation<DepPrinter>
    {
        const DependencySpecTree::ItemFormatter & formatter;
        const bool flat;
        std::stringstream result;

        Implementation(const DependencySpecTree::ItemFormatter & f,
                const bool b) :
            formatter(f),
            flat(b)
        {
        }
    };
}

DepPrinter::DepPrinter(const DependencySpecTree::ItemFormatter & f, const bool b) :
    PrivateImplementationPattern<DepPrinter>(new Implementation<DepPrinter>(f, b))
{
}

DepPrinter::~DepPrinter()
{
}

const std::string
DepPrinter::result() const
{
    return _imp->result.str();
}

void
DepPrinter::visit_leaf(const PackageDepSpec & s)
{
    if (! _imp->result.str().empty())
    {
        if (! _imp->flat)
            _imp->result << _imp->formatter.indent(1);
        else
            _imp->result << ", ";
    }

    _imp->result << s;
}

void
DepPrinter::visit_leaf(const NamedSetDepSpec & s)
{
    if (! _imp->result.str().empty())
    {
        if (! _imp->flat)
            _imp->result << _imp->formatter.indent(1);
        else
            _imp->result << ", ";
    }

    _imp->result << s;
}

void
DepPrinter::visit_leaf(const BlockDepSpec & s)
{
    if (! _imp->result.str().empty())
    {
        if (! _imp->flat)
            _imp->result << _imp->formatter.indent(1);
        else
            _imp->result << ", ";
    }

    _imp->result << s;
}

void
DepPrinter::visit_leaf(const DependencyLabelsDepSpec & s)
{
    if (! _imp->result.str().empty())
    {
        if (! _imp->flat)
            _imp->result << _imp->formatter.indent(1);
        else
            _imp->result << ", ";
    }

    _imp->result << s;
}

