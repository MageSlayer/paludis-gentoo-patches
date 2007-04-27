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

#include <paludis/dep_spec.hh>
#include <paludis/dep_spec_flattener.hh>
#include <list>

/** \file
 * Implementation of dep_spec_flattener.hh.
 *
 * \ingroup grpdepspecflattener
 */

using namespace paludis;

namespace paludis
{
    /**
     * Implementation data for DepSpecFlattener.
     *
     * \ingroup grpdepspecflattener
     */
    template<>
    struct Implementation<DepSpecFlattener>
    {
        const Environment * const env;

        const PackageDatabaseEntry * const pkg;

        std::tr1::shared_ptr<const DepSpec> a;

        mutable std::list<const StringDepSpec *> specs;

        mutable bool done;

        Implementation(const Environment * const e,
                const PackageDatabaseEntry * const p,
                std::tr1::shared_ptr<const DepSpec> aa) :
            env(e),
            pkg(p),
            a(aa),
            done(false)
        {
        }
    };
}

DepSpecFlattener::DepSpecFlattener(
        const Environment * const env,
        const PackageDatabaseEntry * const pkg,
        std::tr1::shared_ptr<const DepSpec> a) :
    PrivateImplementationPattern<DepSpecFlattener>(new Implementation<DepSpecFlattener>(
                env, pkg, a))
{
}

DepSpecFlattener::~DepSpecFlattener()
{
}

DepSpecFlattener::Iterator
DepSpecFlattener::begin()
{
    if (! _imp->done)
    {
        _imp->a->accept(static_cast<DepSpecVisitorTypes::ConstVisitor *>(this));
        _imp->done = true;
    }

    return Iterator(_imp->specs.begin());
}

DepSpecFlattener::Iterator
DepSpecFlattener::end() const
{
    return Iterator(_imp->specs.end());
}

void DepSpecFlattener::visit(const AllDepSpec * a)
{
    std::for_each(a->begin(), a->end(), accept_visitor(
                static_cast<DepSpecVisitorTypes::ConstVisitor *>(this)));
}

void DepSpecFlattener::visit(const AnyDepSpec *)
{
    throw InternalError(PALUDIS_HERE, "Found unexpected AnyDepSpec");
}

void DepSpecFlattener::visit(const UseDepSpec * u)
{
    if (_imp->env->query_use(u->flag(), *_imp->pkg) ^ u->inverse())
        std::for_each(u->begin(), u->end(), accept_visitor(
                    static_cast<DepSpecVisitorTypes::ConstVisitor *>(this)));
}

void DepSpecFlattener::visit(const PlainTextDepSpec * p)
{
    _imp->specs.push_back(p);
}

void DepSpecFlattener::visit(const PackageDepSpec * p)
{
    _imp->specs.push_back(p);
}

void DepSpecFlattener::visit(const BlockDepSpec * p)
{
    _imp->specs.push_back(p);
}

