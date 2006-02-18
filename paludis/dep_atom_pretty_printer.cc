/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include "dep_atom_pretty_printer.hh"
#include "dep_atom.hh"
#include "save.hh"
#include <algorithm>

using namespace paludis;

std::ostream &
paludis::operator<< (std::ostream & s, const DepAtomPrettyPrinter & p)
{
    s << p._s.str();
    return s;
}

void
DepAtomPrettyPrinter::visit(const AllDepAtom * const a)
{
    _s << std::string(_indent, ' ') << "(" << std::endl;
    {
        Save<unsigned> old_indent(&_indent, _indent + 4);
        std::for_each(a->begin(), a->end(), accept_visitor(this));
    }
    _s << std::string(_indent, ' ') << ")" << std::endl;
}

void
DepAtomPrettyPrinter::visit(const AnyDepAtom * const a)
{
    _s << std::string(_indent, ' ') << "|| (" << std::endl;
    {
        Save<unsigned> old_indent(&_indent, _indent + 4);
        std::for_each(a->begin(), a->end(), accept_visitor(this));
    }
    _s << std::string(_indent, ' ') << ")" << std::endl;
}

void
DepAtomPrettyPrinter::visit(const UseDepAtom * const a)
{
    _s << std::string(_indent, ' ') << (a->inverse() ? "!" : "") <<
        a->flag() << "? (" << std::endl;
    {
        Save<unsigned> old_indent(&_indent, _indent + 4);
        std::for_each(a->begin(), a->end(), accept_visitor(this));
    }
    _s << std::string(_indent, ' ') << ")" << std::endl;
}

void
DepAtomPrettyPrinter::visit(const PackageDepAtom * const p)
{
    _s << std::string(_indent, ' ') << *p << std::endl;
}

void
DepAtomPrettyPrinter::visit(const BlockDepAtom * const b)
{
    _s << std::string(_indent, ' ') << "!" << *b->blocked_atom() << std::endl;
}

