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
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/destinations.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/arrow.hh>
#include <paludis/util/enum_iterator.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/join.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <iostream>
#include <sstream>

using namespace paludis;
using namespace cave;
using namespace paludis::resolver;

namespace
{
    std::ostream &
    operator<< (std::ostream & s, const Decision & d)
    {
        std::stringstream ss;

        ss << "Decision(";

        ss << d.kind() << " ";

        if (d.if_package_id())
            ss << *d.if_package_id();
        else
            ss << "(nothing)";

        if (d.is_best())
            ss << ", is best";
        if (d.is_same())
            ss << ", is same";
        if (d.is_same_version())
            ss << ", is same version";

        ss << ")";

        s << ss.str();
        return s;
    }

    std::ostream &
    operator<< (std::ostream & s, const Destination & d)
    {
        std::stringstream ss;
        ss << "Destination(" << d.repository();
        if (! d.replacing()->empty())
            ss << " replacing " << join(indirect_iterator(d.replacing()->begin()),
                    indirect_iterator(d.replacing()->end()), ", ");
        ss << ")";

        s << ss.str();
        return s;
    }

    std::ostream &
    operator<< (std::ostream & s, const Destinations & d)
    {
        std::stringstream ss;
        ss << "Destinations(";
        for (EnumIterator<DestinationType> t, t_end(last_dt) ; t != t_end ; ++t)
            if (d.by_type(*t))
            {
                switch (*t)
                {
                    case dt_slash:
                        ss << "slash: " << *d.by_type(*t);
                        break;

                    case last_dt:
                        break;
                }
            }
        ss << ")";

        s << ss.str();
        return s;
    }

    std::ostream &
    operator<< (std::ostream & s, const SanitisedDependency & d)
    {
        std::stringstream ss;
        ss << "Dep(";
        if (! d.metadata_key_raw_name().empty())
            ss << d.metadata_key_raw_name() << " ";
        ss << d.spec();

        if (! d.active_dependency_labels()->system_labels()->empty())
            ss << " system { " << join(indirect_iterator(d.active_dependency_labels()->system_labels()->begin()),
                    indirect_iterator(d.active_dependency_labels()->system_labels()->end()), ", ") << " }";
        if (! d.active_dependency_labels()->type_labels()->empty())
            ss << " type { " << join(indirect_iterator(d.active_dependency_labels()->type_labels()->begin()),
                    indirect_iterator(d.active_dependency_labels()->type_labels()->end()), ", ") << " }";
        if (! d.active_dependency_labels()->abi_labels()->empty())
            ss << " abi { " << join(indirect_iterator(d.active_dependency_labels()->abi_labels()->begin()),
                    indirect_iterator(d.active_dependency_labels()->abi_labels()->end()), ", ") << " }";
        if (! d.active_dependency_labels()->suggest_labels()->empty())
            ss << " suggest { " << join(indirect_iterator(d.active_dependency_labels()->suggest_labels()->begin()),
                    indirect_iterator(d.active_dependency_labels()->suggest_labels()->end()), ", ") << " }";

        ss << ")";

        s << ss.str();
        return s;
    }

    struct ReasonFinder
    {
        std::string str;

        ReasonFinder() :
            str("none")
        {
        }

        void visit(const TargetReason &)
        {
            str = "Target";
        }

        void visit(const PresetReason &)
        {
            str = "Preset";
        }

        void visit(const SetReason & r)
        {
            ReasonFinder f;
            if (r.reason_for_set())
                r.reason_for_set()->accept(f);
            str = "Set(" + stringify(r.set_name()) + " " + f.str + ")";
        }

        void visit(const DependencyReason & r)
        {
            std::stringstream s;
            s << r.sanitised_dependency();
            str = "Dependency(" + s.str() + ")";
        }
    };

    std::ostream &
    operator<< (std::ostream & s, const Constraint & c)
    {
        std::stringstream ss;
        ss << "Constraint(spec: " << c.spec();
        if (c.nothing_is_fine_too())
            ss << "; nothing is fine too";
        ss
            << "; untaken: " << stringify(c.untaken())
            << "; use_existing: " << stringify(c.use_existing())
            << "; reason: ";

        ReasonFinder r;
        if (c.reason())
            c.reason()->accept(r);
        ss
            << r.str << ")";
        s << ss.str();

        return s;
    }

    std::ostream &
    operator<< (std::ostream & s, const Arrow & a)
    {
        s << "Arrow(-> " << a.to_qpn_s();
        if (0 != a.ignorable_pass())
            s << ", ignorable pass " << a.ignorable_pass();
        s << ")";
        return s;
    }

    std::string stringify_constraint(const Constraint & c)
    {
        std::stringstream s;
        s << c;
        return s.str();
    }

    std::string stringify_arrow(const Arrow & a)
    {
        std::stringstream s;
        s << a;
        return s.str();
    }

    std::ostream &
    operator<< (std::ostream & s, const Resolution & r)
    {
        std::stringstream ss;
        ss <<  "Resolution("
            << "constraints: " << join(indirect_iterator(r.constraints()->begin()),
                    indirect_iterator(r.constraints()->end()), ", ", stringify_constraint)
            << "; decision: ";
        if (r.decision())
            ss << *r.decision();
        else
            ss << "none";
        ss
            << "; arrows: " << join(indirect_iterator(r.arrows()->begin()),
                    indirect_iterator(r.arrows()->end()), ", ", stringify_arrow)
            << "; already_ordered: " << stringify(r.already_ordered()) << ")"
            << "; destinations: ";
        if (r.destinations())
            ss << *r.destinations();
        else
            ss << "unknown";
        ss
            << ")";
        s << ss.str();
        return s;
    }

    void dump(
            const std::tr1::shared_ptr<Environment> &,
            const std::tr1::shared_ptr<Resolver> & resolver,
            const ResolveCommandLine & cmdline)
    {
        std::cout << "Dumping resolutions by QPN:S:" << std::endl << std::endl;

        for (Resolver::ResolutionsByQPN_SConstIterator c(resolver->begin_resolutions_by_qpn_s()),
                c_end(resolver->end_resolutions_by_qpn_s()) ;
                c != c_end ; ++c)
        {
            std::cout << c->first << std::endl;
            std::cout << "  = " << *c->second << std::endl;
            if (cmdline.resolution_options.a_dump_dependencies.specified() && c->second->sanitised_dependencies())
                for (SanitisedDependencies::ConstIterator d(c->second->sanitised_dependencies()->begin()),
                        d_end(c->second->sanitised_dependencies()->end()) ;
                        d != d_end ; ++d)
                    std::cout << "  -> " << *d << std::endl;
        }

        std::cout << std::endl;
    }
}

void
paludis::cave::dump_if_requested(
        const std::tr1::shared_ptr<Environment> & env,
        const std::tr1::shared_ptr<Resolver> & resolver,
        const ResolveCommandLine & cmdline)
{
    Context context("When dumping the resolver:");

    if (! cmdline.resolution_options.a_dump.specified())
        return;

    dump(env, resolver, cmdline);
}

