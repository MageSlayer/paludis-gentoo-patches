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

#include "cmd_resolve_display_explanations.hh"
#include <paludis/dep_spec.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/match_package.hh>
#include <paludis/resolver/resolver.hh>
#include <paludis/resolver/qpn_s.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/destinations.hh>
#include <paludis/args/do_help.hh>
#include <iostream>

using namespace paludis;
using namespace cave;
using namespace paludis::resolver;

namespace
{
    struct ReasonDisplayer
    {
        void visit(const TargetReason &)
        {
            std::cout << "it is a target";
        }

        void visit(const DependencyReason & reason)
        {
            std::cout << "of dependency " << reason.sanitised_dependency() << " from " << *reason.from_id();
        }

        void visit(const PresetReason &)
        {
            std::cout << "of a preset";
        }

        void visit(const SetReason & reason)
        {
            std::cout << "of set " << reason.set_name() << ", which is because ";
            reason.reason_for_set()->accept(*this);
        }
    };
}

void
cave::display_explanations(
        const std::tr1::shared_ptr<Environment> & env,
        const std::tr1::shared_ptr<Resolver> & resolver,
        const ResolveCommandLine & cmdline)
{
    Context context("When displaying explanations:");

    if (cmdline.display_options.a_explain.begin_args() == cmdline.display_options.a_explain.end_args())
        return;

    std::cout << "Explaining requested decisions:" << std::endl << std::endl;

    for (args::StringSetArg::ConstIterator i(cmdline.display_options.a_explain.begin_args()),
            i_end(cmdline.display_options.a_explain.end_args()) ;
            i != i_end ; ++i)
    {
        bool any(false);
        PackageDepSpec spec(parse_user_package_dep_spec(*i, env.get(), UserPackageDepSpecOptions() + updso_allow_wildcards));
        for (Resolver::ResolutionsByQPN_SConstIterator r(resolver->begin_resolutions_by_qpn_s()),
                r_end(resolver->end_resolutions_by_qpn_s()) ;
                r != r_end ; ++r)
        {
            if (! r->second->decision()->if_package_id())
            {
                /* really we want this to work for simple cat/pkg and
                 * cat/pkg:slot specs anyway, even if we chose nothing */
                continue;
            }

            if (! match_package(*env, spec, *r->second->decision()->if_package_id(), MatchPackageOptions()))
                continue;

            any = true;

            std::cout << "For " << r->first << ":" << std::endl;
            std::cout << "    The following constraints were in action:" << std::endl;
            for (Constraints::ConstIterator c(r->second->constraints()->begin()),
                    c_end(r->second->constraints()->end()) ;
                    c != c_end ; ++c)
            {
                std::cout << "      * " << (*c)->spec();

                switch ((*c)->use_installed())
                {
                    case ui_if_same:
                        std::cout << ", use installed if same";
                        break;
                    case ui_never:
                        std::cout << ", never using installed";
                        break;
                    case ui_only_if_transient:
                        std::cout << ", using installed only if transient";
                        break;
                    case ui_if_same_version:
                        std::cout << ", use installed if same version";
                        break;
                    case ui_if_possible:
                        std::cout << ", use installed if possible";
                        break;

                    case last_ui:
                        break;
                }

                if ((*c)->to_destination_slash())
                    std::cout << ", installing to /";

                std::cout << std::endl;
                std::cout << "        because ";
                ReasonDisplayer v;
                (*c)->reason()->accept(v);
                std::cout << std::endl;
            }
            std::cout << "    The decision made was:" << std::endl;
            std::cout << "        Use " << *r->second->decision()->if_package_id() << std::endl;
            if (r->second->destinations()->slash())
            {
                std::cout << "        Install to / using repository " << r->second->destinations()->slash()->repository() << std::endl;
                if (! r->second->destinations()->slash()->replacing()->empty())
                    for (PackageIDSequence::ConstIterator x(r->second->destinations()->slash()->replacing()->begin()),
                            x_end(r->second->destinations()->slash()->replacing()->end()) ;
                            x != x_end ; ++x)
                        std::cout << "            Replacing " << **x << std::endl;
            }
            std::cout << std::endl;
        }

        if (! any)
            throw args::DoHelp("There is nothing matching '" + *i + "' in the resolution set.");
    }
}

