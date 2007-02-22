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

#include <algorithm>
#include <paludis/dep_spec.hh>
#include <paludis/dep_spec_pretty_printer.hh>
#include <paludis/util/save.hh>

/** \file
 * Implementation of dep_spec_pretty_printer.hh.
 *
 * \ingroup grpdepspecprettyprinter
 */

using namespace paludis;

std::ostream &
paludis::operator<< (std::ostream & s, const DepSpecPrettyPrinter & p)
{
    s << p._s.str();
    return s;
}

void
DepSpecPrettyPrinter::visit(const AllDepSpec * const a)
{
    _s << indent() << "(" << newline();
    {
        Save<unsigned> old_indent(&_indent, _indent + 4);
        std::for_each(a->begin(), a->end(), accept_visitor(this));
    }
    _s << indent() << ")" << newline();
}

void
DepSpecPrettyPrinter::visit(const AnyDepSpec * const a)
{
    _s << indent() << "|| (" << newline();
    {
        Save<unsigned> old_indent(&_indent, _indent + 4);
        std::for_each(a->begin(), a->end(), accept_visitor(this));
    }
    _s << indent() << ")" << newline();
}

void
DepSpecPrettyPrinter::visit(const UseDepSpec * const a)
{
    _s << indent() << (a->inverse() ? "!" : "") <<
        a->flag() << "? (" << newline();
    {
        Save<unsigned> old_indent(&_indent, _indent + 4);
        std::for_each(a->begin(), a->end(), accept_visitor(this));
    }
    _s << indent() << ")" << newline();
}

void
DepSpecPrettyPrinter::visit(const PackageDepSpec * const p)
{
    _s << indent() << *p << newline();
}

void
DepSpecPrettyPrinter::visit(const PlainTextDepSpec * const p)
{
    _s << indent() << p->text() << newline();
}

void
DepSpecPrettyPrinter::visit(const BlockDepSpec * const b)
{
    _s << indent() << "!" << *b->blocked_spec() << newline();
}

std::string
DepSpecPrettyPrinter::newline() const
{
    return _use_newlines ? "\n" : " ";
}

std::string
DepSpecPrettyPrinter::indent() const
{
    return _use_newlines ? std::string(_indent, ' ') : "";
}

