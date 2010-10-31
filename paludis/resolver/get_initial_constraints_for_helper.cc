/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#include <paludis/resolver/get_initial_constraints_for_helper.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/destination_utils.hh>
#include <paludis/resolver/match_qpns.hh>
#include <paludis/resolver/suggest_restart.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/enum_iterator.hh>
#include <paludis/dep_spec.hh>
#include <paludis/package_id.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>
#include <paludis/environment.hh>
#include <paludis/metadata_key.hh>
#include <paludis/version_spec.hh>
#include <paludis/partially_made_package_dep_spec.hh>
#include <list>
#include <unordered_map>

using namespace paludis;
using namespace paludis::resolver;

typedef std::unordered_map<Resolvent, std::shared_ptr<Constraints>, Hash<Resolvent> > InitialConstraints;

namespace paludis
{
    template <>
    struct Imp<GetInitialConstraintsForHelper>
    {
        const Environment * const env;

        std::list<PackageDepSpec> without_specs;
        int reinstall_scm_days;

        InitialConstraints initial_constraints;

        Imp(const Environment * const e) :
            env(e),
            reinstall_scm_days(-1)
        {
        }
    };
}

GetInitialConstraintsForHelper::GetInitialConstraintsForHelper(const Environment * const e) :
    Pimp<GetInitialConstraintsForHelper>(e)
{
}

GetInitialConstraintsForHelper::~GetInitialConstraintsForHelper() = default;

void
GetInitialConstraintsForHelper::add_without_spec(const PackageDepSpec & spec)
{
    _imp->without_specs.push_back(spec);
}

void
GetInitialConstraintsForHelper::add_preset_spec(const PackageDepSpec & spec)
{
    auto reason(std::make_shared<PresetReason>("preset", make_null_shared_ptr()));

    auto ids((*_imp->env)[selection::BestVersionInEachSlot(generator::Matches(spec, { }))]);
    for (auto i(ids->begin()), i_end(ids->end()) ;
            i != i_end ; ++i)
        for (EnumIterator<DestinationType> t, t_end(last_dt) ; t != t_end ; ++t)
        {
            Resolvent r(*i, *t);

            const std::shared_ptr<Constraint> constraint(std::make_shared<Constraint>(make_named_values<Constraint>(
                            n::destination_type() = r.destination_type(),
                            n::force_unable() = false,
                            n::nothing_is_fine_too() = true,
                            n::reason() = reason,
                            n::spec() = spec,
                            n::untaken() = false,
                            n::use_existing() = ue_if_possible
                            )));
            auto ic(_imp->initial_constraints.find(r));
            if (ic == _imp->initial_constraints.end())
                ic = _imp->initial_constraints.insert(std::make_pair(r, _make_initial_constraints_for(r))).first;

            ic->second->add(constraint);
        }
}

void
GetInitialConstraintsForHelper::add_suggested_restart(const SuggestRestart & e)
{
    auto ic(_imp->initial_constraints.find(e.resolvent()));
    if (ic == _imp->initial_constraints.end())
        ic = _imp->initial_constraints.insert(std::make_pair(e.resolvent(), _make_initial_constraints_for(e.resolvent()))).first;

    ic->second->add(e.suggested_preset());
}

void
GetInitialConstraintsForHelper::set_reinstall_scm_days(const int d)
{
    _imp->reinstall_scm_days = d;
}

const std::shared_ptr<Constraints>
GetInitialConstraintsForHelper::operator() (const Resolvent & resolvent) const
{
    auto i(_imp->initial_constraints.find(resolvent));
    if (i == _imp->initial_constraints.end())
        return _make_initial_constraints_for(resolvent);
    else
        return i->second;
}

namespace
{
    bool is_scm_name(const QualifiedPackageName & n)
    {
        std::string pkg(stringify(n.package()));
        switch (pkg.length())
        {
            case 0:
            case 1:
            case 2:
            case 3:
                return false;

            default:
                if (0 == pkg.compare(pkg.length() - 6, 6, "-darcs"))
                    return true;

            case 5:
                if (0 == pkg.compare(pkg.length() - 5, 5, "-live"))
                    return true;

            case 4:
                if (0 == pkg.compare(pkg.length() - 4, 4, "-cvs"))
                    return true;
                if (0 == pkg.compare(pkg.length() - 4, 4, "-svn"))
                    return true;
                return false;
        }
    }

    bool is_scm_older_than(const std::shared_ptr<const PackageID> & id, const int n)
    {
        if (id->version().is_scm() || is_scm_name(id->name()))
        {
            static Timestamp current_time(Timestamp::now()); /* static to avoid weirdness */
            time_t installed_time(current_time.seconds());
            if (id->installed_time_key())
                installed_time = id->installed_time_key()->value().seconds();

            return (current_time.seconds() - installed_time) > (24 * 60 * 60 * n);
        }
        else
            return false;
    }

    bool installed_is_scm_older_than(
            const Environment * const env,
            const Resolvent & q,
            const int n)
    {
        Context context("When working out whether '" + stringify(q) + "' has installed SCM packages:");

        const std::shared_ptr<const PackageIDSequence> ids((*env)[selection::AllVersionsUnsorted(
                    destination_filtered_generator(env, q.destination_type(), generator::Package(q.package())) |
                    make_slot_filter(q)
                    )]);

        for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                i != i_end ; ++i)
        {
            if (is_scm_older_than(*i, n))
                return true;
        }

        return false;
    }

    bool use_existing_from_withish(
            const Environment * const env,
            const QualifiedPackageName & name,
            const std::list<PackageDepSpec> & list)
    {
        for (auto l(list.begin()), l_end(list.end()) ;
                l != l_end ; ++l)
            if (match_qpns(*env, *l, name))
                return true;
        return false;
    }
}

const std::shared_ptr<Constraints>
GetInitialConstraintsForHelper::_make_initial_constraints_for(
        const Resolvent & resolvent) const
{
    auto result(std::make_shared<Constraints>());

    if ((-1 != _imp->reinstall_scm_days) &&
            installed_is_scm_older_than(_imp->env, resolvent, _imp->reinstall_scm_days) &&
            ! use_existing_from_withish(_imp->env, resolvent.package(), _imp->without_specs))
    {
        result->add(std::make_shared<Constraint>(make_named_values<Constraint>(
                        n::destination_type() = resolvent.destination_type(),
                        n::force_unable() = false,
                        n::nothing_is_fine_too() = true,
                        n::reason() = std::make_shared<PresetReason>("is scm", make_null_shared_ptr()),
                        n::spec() = make_package_dep_spec({ }).package(resolvent.package()),
                        n::untaken() = false,
                        n::use_existing() = ue_only_if_transient
                        )));
    }

    return result;
}

template class Pimp<GetInitialConstraintsForHelper>;

