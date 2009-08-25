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
#include "colour_formatter.hh"
#include "formats.hh"
#include <paludis/resolver/resolver.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/destinations.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/resolutions.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/join.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/package_id.hh>
#include <paludis/version_spec.hh>
#include <paludis/metadata_key.hh>
#include <paludis/choice.hh>
#include <iostream>
#include <set>

using namespace paludis;
using namespace cave;
using namespace paludis::resolver;

namespace
{
    struct ReasonNameGetter
    {
        std::string visit(const DependencyReason & r) const
        {
            return stringify(r.from_id()->name());
        }

        std::string visit(const TargetReason &) const
        {
            return "target";
        }

        std::string visit(const SetReason & r) const
        {
            return r.reason_for_set()->accept_returning<std::string>(*this) +
                " (" + stringify(r.set_name()) + ")";
        }

        std::string visit(const PresetReason &) const
        {
            return "";
        }
    };
}

void
paludis::cave::display_resolution(
        const std::tr1::shared_ptr<Environment> &,
        const std::tr1::shared_ptr<Resolver> & resolver,
        const ResolveCommandLine &)
{
    Context context("When displaying chosen resolution:");

    std::cout << "These are the actions I will take, in order:" << std::endl << std::endl;

    for (Resolutions::ConstIterator c(resolver->resolutions()->begin()), c_end(resolver->resolutions()->end()) ;
            c != c_end ; ++c)
    {
        const std::tr1::shared_ptr<const PackageID> id((*c)->decision()->if_package_id());
        if (! id)
            throw InternalError(PALUDIS_HERE, "why did that happen?");

        bool is_new(false), is_upgrade(false), is_downgrade(false), is_reinstall(false);
        std::tr1::shared_ptr<const PackageID> old_id;

        if ((*c)->destinations()->slash())
        {
            if ((*c)->destinations()->slash()->replacing()->empty())
                is_new = true;
            else
                for (PackageIDSequence::ConstIterator x((*c)->destinations()->slash()->replacing()->begin()),
                        x_end((*c)->destinations()->slash()->replacing()->end()) ;
                        x != x_end ; ++x)
                {
                    old_id = *x;
                    if ((*x)->version() == id->version())
                        is_reinstall = true;
                    else if ((*x)->version() < id->version())
                        is_upgrade = true;
                    else if ((*x)->version() > id->version())
                        is_downgrade = true;
                }

            /* pick the worst of what it is */
            is_upgrade = is_upgrade && (! is_reinstall) && (! is_downgrade);
            is_reinstall = is_reinstall && (! is_downgrade);
        }

        if (is_new)
            std::cout << "[n] " << c::bold_blue() << id->canonical_form(idcf_no_version);
        else if (is_upgrade)
            std::cout << "[u] " << c::blue() << id->canonical_form(idcf_no_version);
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

        if (id->choices_key())
        {
            ColourFormatter formatter(0);

            std::tr1::shared_ptr<const Choices> old_choices;
            if (old_id && old_id->choices_key())
                old_choices = old_id->choices_key()->value();

            bool non_blank_prefix(false);
            std::string s;
            for (Choices::ConstIterator k(id->choices_key()->value()->begin()),
                    k_end(id->choices_key()->value()->end()) ;
                    k != k_end ; ++k)
            {
                if ((*k)->hidden())
                    continue;

                bool shown_prefix(false);
                for (Choice::ConstIterator i((*k)->begin()), i_end((*k)->end()) ;
                        i != i_end ; ++i)
                {
                    if (! (*i)->explicitly_listed())
                        continue;

                    if (! shown_prefix)
                    {
                        if (non_blank_prefix || ! (*k)->show_with_no_prefix())
                        {
                            shown_prefix = true;
                            if (! s.empty())
                                s.append(" ");
                            s.append((*k)->raw_name() + ":");
                        }
                    }

                    if (! s.empty())
                        s.append(" ");

                    std::string t;
                    if ((*i)->enabled())
                    {
                        if ((*i)->locked())
                            t = formatter.format(**i, format::Forced());
                        else
                            t = formatter.format(**i, format::Enabled());
                    }
                    else
                    {
                        if ((*i)->locked())
                            t = formatter.format(**i, format::Masked());
                        else
                            t = formatter.format(**i, format::Disabled());
                    }

                    bool changed(false), added(false);
                    if ((*k)->consider_added_or_changed())
                    {
                        if (old_choices)
                        {
                            std::tr1::shared_ptr<const ChoiceValue> old_choice(
                                    old_choices->find_by_name_with_prefix((*i)->name_with_prefix()));
                            if (! old_choice)
                                added = true;
                            else if (old_choice->enabled() != (*i)->enabled())
                                changed = true;
                        }
                        else
                            added = true;
                    }

                    if (changed)
                    {
                        t = formatter.decorate(**i, t, format::Changed());
                    }
                    else if (added)
                    {
                        if (old_id)
                            t = formatter.decorate(**i, t, format::Added());
                    }

                    s.append(t);
                }
            }

            if (s.empty())
                break;

            std::cout << "    " << s << std::endl;
        }

        if (id->short_description_key())
            std::cout << "    \"" << id->short_description_key()->value() << "\"" << std::endl;

        std::set<std::string> reason_names;
        for (Constraints::ConstIterator r((*c)->constraints()->begin()),
                r_end((*c)->constraints()->end()) ;
                r != r_end ; ++r)
        {
            ReasonNameGetter g;
            std::string s((*r)->reason()->accept_returning<std::string>(g));
            if (! s.empty())
                reason_names.insert(s);
        }

        if (! reason_names.empty())
        {
            if (reason_names.size() > 4)
                std::cout << "    Because of " << join(reason_names.begin(), next(reason_names.begin(), 3), ", ")
                    << ", " << (reason_names.size() - 3) << " more" << std::endl;
            else
                std::cout << "    Because of " << join(reason_names.begin(), reason_names.end(), ", ") << std::endl;
        }
    }

    std::cout << std::endl;
}

