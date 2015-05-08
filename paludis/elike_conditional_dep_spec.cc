/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2010, 2011 Ciaran McCreesh
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

#include <paludis/elike_conditional_dep_spec.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/tribool.hh>
#include <paludis/util/log.hh>
#include <paludis/dep_spec.hh>
#include <paludis/changed_choices.hh>
#include <paludis/name.hh>
#include <paludis/environment.hh>
#include <paludis/package_id.hh>
#include <paludis/repository.hh>
#include <paludis/choice.hh>
#include <paludis/dep_spec_data.hh>
#include <paludis/metadata_key.hh>
#include <ostream>
#include <string>

using namespace paludis;

ELikeConditionalDepSpecParseError::ELikeConditionalDepSpecParseError(const std::string & s, const std::string & m) noexcept :
    Exception("Error parsing conditional dep spec '" + s + "': " + m)
{
}

namespace
{
    bool icky_use_query(const ChoiceNameWithPrefix & f, const PackageID & id, const bool no_warning_for_unlisted)
    {
        if (! id.choices_key())
        {
            Log::get_instance()->message("elike_use_requirement.query", ll_warning, lc_context) <<
                "ID '" << id << "' has no choices, so couldn't get the state of flag '" << f << "'";
            return false;
        }

        auto choices(id.choices_key()->parse_value());
        auto v(choices->find_by_name_with_prefix(f));
        if (v)
        {
            if (co_special == v->origin())
                Log::get_instance()->message("elike_conditional_dep_spec.query", ll_warning, lc_context) <<
                    "ID '" << id << "' flag '" << f << "' should not be used as a conditional";
            return v->enabled();
        }

        if (! no_warning_for_unlisted)
            if (! choices->has_matching_contains_every_value_prefix(f))
                Log::get_instance()->message("elike_conditional_dep_spec.query", ll_warning, lc_context) <<
                    "ID '" << id << "' has no flag named '" << f << "'";
        return false;
    }

    bool icky_use_query_locked(const ChoiceNameWithPrefix & f, const PackageID & id, const bool no_warning_for_unlisted)
    {
        if (! id.choices_key())
        {
            Log::get_instance()->message("elike_use_requirement.query", ll_warning, lc_context) <<
                "ID '" << id << "' has no choices, so couldn't get the state of flag '" << f << "'";
            return false;
        }

        auto choices(id.choices_key()->parse_value());
        auto v(choices->find_by_name_with_prefix(f));
        if (v)
            return v->locked();

        if (! no_warning_for_unlisted)
            if (! choices->has_matching_contains_every_value_prefix(f))
                Log::get_instance()->message("elike_conditional_dep_spec.query", ll_warning, lc_context) <<
                    "ID '" << id << "' has no flag named '" << f << "'";
        return false;
    }

    struct EConditionalDepSpecData :
        ConditionalDepSpecData
    {
        bool inverse;
        ChoiceNameWithPrefix flag;
        bool no_warning_for_unlisted;

        EConditionalDepSpecData(const std::string & s, const bool n) :
            inverse(false),
            flag("x"),
            no_warning_for_unlisted(n)
        {
            if (s.empty())
                throw ELikeConditionalDepSpecParseError(s, "missing use flag name");

            if (s.at(s.length() - 1) != '?')
                throw ELikeConditionalDepSpecParseError(s, "missing ? on use conditional");

            inverse = '!' == s.at(0);
            if (s.length() < (inverse ? 3 : 2))
                throw ELikeConditionalDepSpecParseError(s, "missing flag name on use conditional");

            flag = ChoiceNameWithPrefix(s.substr(inverse ? 1 : 0, s.length() - (inverse ? 2 : 1)));
        }

        virtual std::string as_string() const
        {
            return (inverse ? "!" : "") + stringify(flag) + "?";
        }

        virtual bool condition_met(const Environment * const, const std::shared_ptr<const PackageID> & id) const
        {
            return icky_use_query(flag, *id, no_warning_for_unlisted) ^ inverse;
        }

        virtual bool condition_meetable(const Environment * const env, const std::shared_ptr<const PackageID> & id) const
        {
            return condition_met(env, id) || ! icky_use_query_locked(flag, *id, no_warning_for_unlisted);
        }

        virtual bool condition_would_be_met_when(const Environment * const env, const std::shared_ptr<const PackageID> & id,
                const ChangedChoices & changes) const
        {
            Tribool overridden(changes.overridden_value(flag));

            if (overridden.is_indeterminate())
                return condition_met(env, id);
            else if (! condition_meetable(env, id))
                return condition_met(env, id);
            else
                return overridden.is_true() ^ inverse;
        }
    };
}

ConditionalDepSpec
paludis::parse_elike_conditional_dep_spec(
        const std::string & s,
        const bool no_warning_for_unlisted)
{
    return ConditionalDepSpec(std::make_shared<EConditionalDepSpecData>(s, no_warning_for_unlisted));
}

