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

#include "nest_atom_flattener.hh"
#include "nest_atom.hh"

using namespace paludis;

NestAtomFlattener::NestAtomFlattener(
        const Environment * const env,
        const PackageDatabaseEntry * const pkg,
        NestAtom::ConstPointer a) :
    _env(env),
    _pkg(pkg),
    _a(a),
    _done(false)
{
}

NestAtomFlattener::~NestAtomFlattener()
{
}

NestAtomFlattener::Iterator
NestAtomFlattener::begin()
{
    if (! _done)
    {
        _a->accept(static_cast<NestAtomVisitorTypes::ConstVisitor *>(this));
        _done = true;
    }

    return _atoms.begin();
}

void NestAtomFlattener::visit(const AllNestAtom * a)
{
    std::for_each(a->begin(), a->end(), accept_visitor(
                static_cast<NestAtomVisitorTypes::ConstVisitor *>(this)));
}

void NestAtomFlattener::visit(const UseNestAtom * u)
{
    if (_env->query_use(u->flag(), _pkg) ^ u->inverse())
        std::for_each(u->begin(), u->end(), accept_visitor(
                    static_cast<NestAtomVisitorTypes::ConstVisitor *>(this)));
}

void NestAtomFlattener::visit(const TextNestAtom * p)
{
    _atoms.push_back(p);
}

