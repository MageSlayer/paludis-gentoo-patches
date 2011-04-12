/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/repositories/e/parse_annotations.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/dep_parser.hh>
#include <paludis/util/map.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/log.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/dep_spec_annotations.hh>

using namespace paludis;
using namespace paludis::erepository;

const std::shared_ptr<DepSpecAnnotations>
paludis::erepository::parse_annotations(
        const EAPI & eapi,
        const std::shared_ptr<const Map<std::string, std::string> > & m)
{
    auto annotations(std::make_shared<DepSpecAnnotations>());
    for (auto k(m->begin()), k_end(m->end()) ;
            k != k_end ; ++k)
    {
        std::string key(k->first);

        if (key.empty())
            throw EDepParseError(key, "Empty annotation key");

        bool is_star(false);
        if ('*' == key.at(0))
        {
            is_star = true;
            key.erase(0, 1);

            if (key.empty())
                throw EDepParseError(key, "Empty *annotation key");
        }

        DepSpecAnnotationRole role(dsar_none);

        /* blocks */
        if (key == eapi.supported()->annotations()->blocker_resolution())
        {
            if (k->second.empty())
            {
            }
            else if (k->second == eapi.supported()->annotations()->blocker_resolution_manual())
                role = dsar_blocker_manual;
            else if (k->second == eapi.supported()->annotations()->blocker_resolution_uninstall_blocked_after())
                role = dsar_blocker_uninstall_blocked_after;
            else if (k->second == eapi.supported()->annotations()->blocker_resolution_uninstall_blocked_before())
                role = dsar_blocker_uninstall_blocked_before;
            else if (k->second == eapi.supported()->annotations()->blocker_resolution_upgrade_blocked_before())
                role = dsar_blocker_upgrade_blocked_before;
            else
                throw EDepParseError(k->second, "Unknown value '" + k->second + "' for annotation '" + key + "'");
        }

        /* myoptions number-selected */
        if (dsar_none == role)
        {
            if (key == eapi.supported()->annotations()->myoptions_number_selected())
            {
                if (k->second.empty())
                {
                }
                else if (k->second == eapi.supported()->annotations()->myoptions_number_selected_at_least_one())
                    role = dsar_myoptions_n_at_least_one;
                else if (k->second == eapi.supported()->annotations()->myoptions_number_selected_at_most_one())
                    role = dsar_myoptions_n_at_most_one;
                else if (k->second == eapi.supported()->annotations()->myoptions_number_selected_exactly_one())
                    role = dsar_myoptions_n_exactly_one;
                else
                    throw EDepParseError(key, "Unknown value '" + k->second + "' for annotation '" + key + "'");
            }
        }

        /* myoptions requires */
        if (dsar_none == role)
        {
            if (key == eapi.supported()->annotations()->myoptions_requires())
                role = dsar_myoptions_requires;
        }

        /* suggestions */
        if (dsar_none == role)
        {
            if (key == eapi.supported()->annotations()->suggestions_group_name())
                role = dsar_suggestions_group_name;
        }

        /* system */
        if (dsar_none == role)
        {
            if (key == eapi.supported()->annotations()->system_implicit())
                role = dsar_system_implicit;
        }

        /* general */
        if (dsar_none == role)
        {
            if (key == eapi.supported()->annotations()->general_description())
                role = dsar_general_description;
            else if (key == eapi.supported()->annotations()->general_url())
                role = dsar_general_url;
            else if (key == eapi.supported()->annotations()->general_note())
                role = dsar_general_note;
            else if (key == eapi.supported()->annotations()->general_lang())
                role = dsar_general_lang;
            else if (key == eapi.supported()->annotations()->general_author())
                role = dsar_general_author;
            else if (key == eapi.supported()->annotations()->general_date())
                role = dsar_general_date;
            else if (key == eapi.supported()->annotations()->general_token())
                role = dsar_general_token;
            else if (key == eapi.supported()->ebuild_options()->bracket_merged_variables_annotation())
                role = dsar_general_defined_in;
        }

        if (dsar_none == role)
            Log::get_instance()->message("e.dep_parser.unknown_annotation", ll_qa, lc_context)
                << "Unknown annotation '" << key << "' = '" << k->second << "'";

        annotations->add(make_named_values<DepSpecAnnotation>(
                    n::key() = k->first, /* not key */
                    n::kind() = is_star ? dsak_expandable : dsak_literal,
                    n::role() = role,
                    n::value() = k->second));
    }

    return annotations;
}

