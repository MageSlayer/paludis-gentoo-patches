/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
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

#include "dep_atom_flattener.hh"
#include "package_dep_atom.hh"
#include "all_dep_atom.hh"
#include "any_dep_atom.hh"
#include "block_dep_atom.hh"
#include "use_dep_atom.hh"

using namespace paludis;

DepAtomFlattener::DepAtomFlattener(
        const Environment * const env,
        const PackageDatabaseEntry * const pkg,
        DepAtom::ConstPointer a) :
    _env(env),
    _pkg(pkg),
    _a(a),
    _done(false)
{
}

DepAtomFlattener::~DepAtomFlattener()
{
}

DepAtomFlattener::Iterator
DepAtomFlattener::begin()
{
    if (! _done)
    {
        _a->accept(static_cast<DepAtomVisitorTypes::ConstVisitor *>(this));
        _done = true;
    }

    return _atoms.begin();
}

void DepAtomFlattener::visit(const AllDepAtom * a)
{
    std::for_each(a->begin(), a->end(), accept_visitor(
                static_cast<DepAtomVisitorTypes::ConstVisitor *>(this)));
}

void DepAtomFlattener::visit(const AnyDepAtom *)
{
    throw paludis::InternalError(PALUDIS_HERE, "Not allowed an AnyDepAtom");
}

void DepAtomFlattener::visit(const UseDepAtom * u)
{
    if (_env->query_use(u->flag(), _pkg) ^ u->inverse())
        std::for_each(u->begin(), u->end(), accept_visitor(
                    static_cast<DepAtomVisitorTypes::ConstVisitor *>(this)));
}

void DepAtomFlattener::visit(const BlockDepAtom *)
{
    throw paludis::InternalError(PALUDIS_HERE, "Not allowed a BlockDepAtom");
}

void DepAtomFlattener::visit(const PackageDepAtom * p)
{
    _atoms.push_back(p);
}

