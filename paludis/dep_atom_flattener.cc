/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include <paludis/dep_atom.hh>
#include <paludis/dep_atom_flattener.hh>

/** \file
 * Implementation of dep_atom_flattener.hh.
 *
 * \ingroup grpdepatomflattener
 */

using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<DepAtomFlattener> :
        InternalCounted<Implementation<DepAtomFlattener> >
    {
        const Environment * const env;

        const PackageDatabaseEntry * const pkg;

        DepAtom::ConstPointer a;

        mutable std::list<const StringDepAtom *> atoms;

        mutable bool done;

        Implementation(const Environment * const e,
                const PackageDatabaseEntry * const p,
                DepAtom::ConstPointer aa) :
            env(e),
            pkg(p),
            a(aa),
            done(false)
        {
        }
    };
}

DepAtomFlattener::DepAtomFlattener(
        const Environment * const env,
        const PackageDatabaseEntry * const pkg,
        DepAtom::ConstPointer a) :
    PrivateImplementationPattern<DepAtomFlattener>(new Implementation<DepAtomFlattener>(
                env, pkg, a))
{
}

DepAtomFlattener::~DepAtomFlattener()
{
}

DepAtomFlattener::Iterator
DepAtomFlattener::begin()
{
    if (! _imp->done)
    {
        _imp->a->accept(static_cast<DepAtomVisitorTypes::ConstVisitor *>(this));
        _imp->done = true;
    }

    return Iterator(_imp->atoms.begin());
}

DepAtomFlattener::Iterator
DepAtomFlattener::end() const
{
    return Iterator(_imp->atoms.end());
}

void DepAtomFlattener::visit(const AllDepAtom * a)
{
    std::for_each(a->begin(), a->end(), accept_visitor(
                static_cast<DepAtomVisitorTypes::ConstVisitor *>(this)));
}

void DepAtomFlattener::visit(const AnyDepAtom *)
{
    throw InternalError(PALUDIS_HERE, "Found unexpected AnyDepAtom");
}

void DepAtomFlattener::visit(const UseDepAtom * u)
{
    if (_imp->env->query_use(u->flag(), _imp->pkg) ^ u->inverse())
        std::for_each(u->begin(), u->end(), accept_visitor(
                    static_cast<DepAtomVisitorTypes::ConstVisitor *>(this)));
}

void DepAtomFlattener::visit(const PlainTextDepAtom * p)
{
    _imp->atoms.push_back(p);
}

void DepAtomFlattener::visit(const PackageDepAtom * p)
{
    _imp->atoms.push_back(p);
}

void DepAtomFlattener::visit(const BlockDepAtom * p)
{
    _imp->atoms.push_back(p);
}

