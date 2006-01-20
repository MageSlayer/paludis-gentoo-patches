/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include "all_dep_atom.hh"
#include "any_dep_atom.hh"
#include "use_dep_atom.hh"
#include "block_dep_atom.hh"
#include "package_dep_atom.hh"
#include "dep_atom_visitor.hh"
#include "dep_atom_dumper.hh"
#include <algorithm>

using namespace paludis;

DepAtomDumper::DepAtomDumper(std::ostream * const o) :
    _o(o)
{
}

void
DepAtomDumper::visit(const AllDepAtom * const a)
{
    *_o << "<all>";
    std::for_each(a->begin(), a->end(), accept_visitor(this));
    *_o << "</all>";
}

void
DepAtomDumper::visit(const AnyDepAtom * const a)
{
    *_o << "<any>";
    std::for_each(a->begin(), a->end(), accept_visitor(this));
    *_o << "</any>";
}

void
DepAtomDumper::visit(const UseDepAtom * const a)
{
    *_o << "<use flag=\"" << a->flag() << "\" inverse=\""
        << (a->inverse() ? "true" : "false") << "\">";
    std::for_each(a->begin(), a->end(), accept_visitor(this));
    *_o << "</use>";
}

void
DepAtomDumper::visit(const PackageDepAtom * const p)
{
    *_o << "<package";
    if (p->slot_ptr())
        *_o << " slot=\"" << *p->slot_ptr() << "\"";
    if (p->version_spec_ptr())
        *_o << " version=\"" << p->version_operator() << *p->version_spec_ptr() << "\"";
    *_o << ">" << p->package() << "</package>";
}

void
DepAtomDumper::visit(const BlockDepAtom * const b)
{
    *_o << "<block>";
    b->blocked_atom()->accept(this);
    *_o << "</block>";
}

