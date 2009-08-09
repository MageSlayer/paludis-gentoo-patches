/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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

#include "cmd_resolve_display_resolution.hh"
#include "formats.hh"
#include <paludis/resolver/resolver.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/destinations.hh>
#include <paludis/util/stringify.hh>
#include <paludis/package_id.hh>
#include <paludis/version_spec.hh>
#include <iostream>

using namespace paludis;
using namespace cave;
using namespace paludis::resolver;

void
paludis::cave::display_resolution(
        const std::tr1::shared_ptr<Environment> &,
        const std::tr1::shared_ptr<Resolver> & resolver,
        const ResolveCommandLine &)
{
    Context context("When displaying chosen resolution:");

    std::cout << "These are the actions I will take, in order:" << std::endl << std::endl;

    for (Resolver::ConstIterator c(resolver->begin()), c_end(resolver->end()) ;
            c != c_end ; ++c)
    {
        const std::tr1::shared_ptr<const PackageID> id((*c)->decision()->if_package_id());
        if (! id)
            throw InternalError(PALUDIS_HERE, "why did that happen?");

        bool is_new(false), is_upgrade(false), is_downgrade(false), is_reinstall(false);

        if ((*c)->destinations()->slash())
        {
            if ((*c)->destinations()->slash()->replacing()->empty())
                is_new = true;
            else
                for (PackageIDSequence::ConstIterator x((*c)->destinations()->slash()->replacing()->begin()),
                        x_end((*c)->destinations()->slash()->replacing()->end()) ;
                        x != x_end ; ++x)
                    if ((*x)->version() == id->version())
                        is_reinstall = true;
                    else if ((*x)->version() < id->version())
                        is_upgrade = true;
                    else if ((*x)->version() > id->version())
                        is_downgrade = true;

            /* pick the worst of what it is */
            is_upgrade = is_upgrade && (! is_reinstall) && (! is_downgrade);
            is_reinstall = is_reinstall && (! is_downgrade);
        }

        if (is_new)
            std::cout << "[n] " << c::bold_green() << id->canonical_form(idcf_no_version);
        else if (is_upgrade)
            std::cout << "[u] " << c::green() << id->canonical_form(idcf_no_version);
        else if (is_reinstall)
            std::cout << "[r] " << c::yellow() << id->canonical_form(idcf_no_version);
        else if (is_downgrade)
            std::cout << "[d] " << c::bold_yellow() << id->canonical_form(idcf_no_version);
        else
            throw InternalError(PALUDIS_HERE, "why did that happen?");

        std::cout << c::normal() << " " << id->canonical_form(idcf_version);

        if ((*c)->destinations()->slash())
        {
            std::cout << " to ::" << (*c)->destinations()->slash()->repository();
            if (! (*c)->destinations()->slash()->replacing()->empty())
            {
                std::cout << " replacing";
                for (PackageIDSequence::ConstIterator x((*c)->destinations()->slash()->replacing()->begin()),
                        x_end((*c)->destinations()->slash()->replacing()->end()) ;
                        x != x_end ; ++x)
                    std::cout << " " << (*x)->canonical_form(idcf_version);
            }
        }

        std::cout << std::endl;
    }

    std::cout << std::endl;
}

