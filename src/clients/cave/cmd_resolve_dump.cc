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

#include "cmd_resolve_dump.hh"
#include <paludis/resolver/resolver.hh>
#include <paludis/resolver/qpn_s.hh>
#include <paludis/resolver/sanitised_dependencies.hh>
#include <paludis/resolver/resolution.hh>
#include <iostream>

using namespace paludis;
using namespace cave;
using namespace paludis::resolver;

void
paludis::cave::dump_if_requested(
        const std::tr1::shared_ptr<Environment> &,
        const std::tr1::shared_ptr<Resolver> & resolver,
        const ResolveCommandLine & cmdline)
{
    Context context("When dumping the resolver:");

    if (! cmdline.display_options.a_dump.specified())
        return;

    std::cout << "Dumping resolutions by QPN:S:" << std::endl << std::endl;

    for (Resolver::ResolutionsByQPN_SConstIterator c(resolver->begin_resolutions_by_qpn_s()),
            c_end(resolver->end_resolutions_by_qpn_s()) ;
            c != c_end ; ++c)
    {
        std::cout << c->first << std::endl;
        std::cout << "  = " << *c->second << std::endl;
        if (cmdline.display_options.a_dump_dependencies.specified() && c->second->sanitised_dependencies())
            for (SanitisedDependencies::ConstIterator d(c->second->sanitised_dependencies()->begin()),
                    d_end(c->second->sanitised_dependencies()->end()) ;
                    d != d_end ; ++d)
                std::cout << "  -> " << *d << std::endl;
    }

    std::cout << std::endl;
}

