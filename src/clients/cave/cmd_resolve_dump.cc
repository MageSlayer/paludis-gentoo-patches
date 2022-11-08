/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/sanitised_dependencies.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/destination.hh>
#include <paludis/resolver/resolutions_by_resolvent.hh>
#include <paludis/resolver/change_by_resolvent.hh>
#include <paludis/resolver/collect_depped_upon.hh>

#include <paludis/util/enum_iterator.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/join.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/wrapped_forward_iterator.hh>

#include <iostream>
#include <sstream>

using namespace paludis;
using namespace cave;
using namespace paludis::resolver;

namespace
{
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

    std::string stringify_if_not_null(const std::shared_ptr<const Destination> & d)
    {
        if (d)
        {
            std::stringstream ss;
            ss << *d;
            return ss.str();
        }
        else
            return "null";
    }

    std::string stringify_change_by_resolvent(const ChangeByResolvent & r)
    {
        return stringify(*r.package_id());
    }

    struct DecisionStringifier
    {
        const std::string visit(const UnableToMakeDecision & d) const
        {
            return "UnableToMakeDecision(taken: " + stringify(d.taken()) + ")";
        }

        const std::string visit(const RemoveDecision & d) const
        {
            return "RemoveDecision(ids: " + join(indirect_iterator(d.ids()->begin()),
                        indirect_iterator(d.ids()->end()), ", ") + " taken: " + stringify(d.taken()) + ")";
        }

        const std::string visit(const NothingNoChangeDecision & d) const
        {
            return "NothingNoChangeDecision(taken: " + stringify(d.taken()) + ")";
        }

        const std::string visit(const ExistingNoChangeDecision & d) const
        {
            std::string attrs;
            for (EnumIterator<ExistingPackageIDAttribute> t, t_end(last_epia) ; t != t_end ; ++t)
                if (d.attributes()[*t])
                {
                    if (! attrs.empty())
                        attrs += ", ";
                    attrs += stringify(*t);
                }

            return "ExistingNoChangeDecision(" + stringify(*d.existing_id()) + " " +
                attrs + " taken: " + stringify(d.taken()) + ")";
        }

        const std::string visit(const ChangesToMakeDecision & d) const
        {
            return "ChangesToMakeDecision(" + stringify(*d.origin_id()) + " best: "
                + stringify(d.best()) + " taken: " + stringify(d.taken())
                + " destination: " + stringify_if_not_null(d.destination())
                + ")";
        }

        const std::string visit(const BreakDecision & d) const
        {
            return "BreakDecision(taken: " + stringify(d.taken()) + " existing_id: " + stringify(*d.existing_id()) + ")";
        }
    };

    std::ostream &
    operator<< (std::ostream & s, const Decision & d)
    {
        s << d.accept_returning<std::string>(DecisionStringifier());
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

        ss << " { " << join(indirect_iterator(d.active_dependency_labels()->begin()),
                indirect_iterator(d.active_dependency_labels()->end()), ", ") << " }";

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

        void visit(const TargetReason & r)
        {
            str = "Target(" + r.extra_information() + ")";
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

        void visit(const LikeOtherDestinationTypeReason & r)
        {
            ReasonFinder f;
            if (r.reason_for_other())
                r.reason_for_other()->accept(f);
            str = "LikeOtherDestinationTypeReason(" + stringify(r.other_resolvent()) + " " + f.str + ")";
        }

        void visit(const ViaBinaryReason & r)
        {
            str = "ViaBinaryReason(" + stringify(r.other_resolvent()) + ")";
        }

        void visit(const DependentReason & r)
        {
            str = "Dependent(" + stringify(*r.dependent_upon().package_id()) + ", " +
                r.dependent_upon().active_dependency_labels_as_string() + ")";
        }

        void visit(const WasUsedByReason & r)
        {
            str = "WasUsedBy(" + join(r.ids_and_resolvents_being_removed()->begin(),
                        r.ids_and_resolvents_being_removed()->end(), ", ", stringify_change_by_resolvent) + ")";
        }

        void visit(const DependencyReason & r)
        {
            std::stringstream s;
            s << r.sanitised_dependency();
            str = "Dependency(" + s.str() + " from " + stringify(r.from_resolvent()) + ")";
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

    std::string stringify_constraint(const Constraint & c)
    {
        std::stringstream s;
        s << c;
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
        ss << ")";
        s << ss.str();
        return s;
    }

    void dump(
            const std::shared_ptr<Environment> &,
            const std::shared_ptr<Resolver> & resolver,
            const ResolveCommandLineResolutionOptions &)
    {
        std::cout << "Dumping resolutions by QPN:S:" << std::endl << std::endl;

        for (const auto & c : *resolver->resolved()->resolutions_by_resolvent())
        {
            std::cout << c->resolvent() << std::endl;
            std::cout << "  = " << *c << std::endl;
        }

        std::cout << std::endl;
    }
}

void
paludis::cave::dump_if_requested(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<Resolver> & resolver,
        const ResolveCommandLineResolutionOptions & resolution_options)
{
    Context context("When dumping the resolver:");

    if (! resolution_options.a_dump.specified())
        return;

    dump(env, resolver, resolution_options);
}

