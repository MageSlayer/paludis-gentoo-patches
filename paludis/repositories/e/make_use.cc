/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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

#include <paludis/repositories/e/make_use.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/util/log.hh>
#include <paludis/choice.hh>
#include <paludis/metadata_key.hh>
#include <paludis/elike_choices.hh>

#include <algorithm>

using namespace paludis;
using namespace paludis::erepository;

std::string
paludis::erepository::make_use(const Environment * const,
        const ERepositoryID & id,
        std::shared_ptr<const Profile> profile)
{
    if (! id.eapi()->supported())
    {
        Log::get_instance()->message("e.ebuild.unknown_eapi", ll_warning, lc_context)
            << "Can't make the USE string for '" << id << "' because its EAPI is unsupported";
        return "";
    }

    std::string use;

    if (id.choices_key())
    {
        auto choices(id.choices_key()->parse_value());
        for (const auto & k : *choices)
        {
            if (k->prefix() == canonical_build_options_prefix())
                continue;

            for (const auto & i : *k)
                if (i->enabled())
                    use += stringify(i->name_with_prefix()) + " ";
        }
    }

    if (! id.eapi()->supported()->ebuild_environment_variables()->env_arch().empty())
        use += profile->environment_variable(id.eapi()->supported()->ebuild_environment_variables()->env_arch()) + " ";

    return use;
}

std::shared_ptr<Map<std::string, std::string> >
paludis::erepository::make_expand(const Environment * const,
        const ERepositoryID & e,
        std::shared_ptr<const Profile> profile)
{
    std::shared_ptr<Map<std::string, std::string> > expand_vars(std::make_shared<Map<std::string, std::string> >());

    if (! e.eapi()->supported())
    {
        Log::get_instance()->message("e.ebuild.unknown_eapi", ll_warning, lc_context)
            << "Can't make the USE_EXPAND strings for '" << e << "' because its EAPI is unsupported";
        return expand_vars;
    }

    if (! e.choices_key())
        return expand_vars;

    auto choices(e.choices_key()->parse_value());

    for (const auto & x : *profile->use_expand())
    {
        expand_vars->insert(stringify(x), "");

        Choices::ConstIterator k(std::find_if(choices->begin(), choices->end(),
                    std::bind(std::equal_to<>(), x,
                        std::bind(std::mem_fn(&Choice::raw_name), std::placeholders::_1))));
        if (k == choices->end())
            continue;

        for (const auto & i : *(*k))
            if (i->enabled())
            {
                std::string value;
                Map<std::string, std::string>::ConstIterator v(expand_vars->find(stringify(x)));
                if (expand_vars->end() != v)
                {
                    value = v->second;
                    if (! value.empty())
                        value.append(" ");
                    expand_vars->erase(v);
                }
                value.append(stringify(i->unprefixed_name()));
                expand_vars->insert(stringify(x), value);
            }
    }

    return expand_vars;
}

