/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2011 Ciaran McCreesh
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

#include <paludis/resolver/get_sameness.hh>

#include <paludis/package_id.hh>
#include <paludis/choice.hh>
#include <paludis/metadata_key.hh>
#include <paludis/version_spec.hh>

#include <set>
#include <algorithm>

using namespace paludis;
using namespace paludis::resolver;

std::tuple<bool, bool, bool>
paludis::resolver::get_sameness(
        const std::shared_ptr<const PackageID> & existing_id,
        const std::shared_ptr<const PackageID> & installable_id)
{
    bool is_same_version(existing_id->version() == installable_id->version());
    bool is_same(false);
    bool is_same_metadata(false);

    if (is_same_version)
    {
        is_same = true;
        is_same_metadata = true;

        std::set<ChoiceNameWithPrefix> common;
        std::shared_ptr<const Choices> installable_choices;
        std::shared_ptr<const Choices> existing_choices;

        if (existing_id->choices_key() && installable_id->choices_key())
        {
            installable_choices = installable_id->choices_key()->parse_value();
            existing_choices = existing_id->choices_key()->parse_value();

            std::set<ChoiceNameWithPrefix> i_common, u_common;
            for (Choices::ConstIterator k(installable_choices->begin()), k_end(installable_choices->end()) ;
                    k != k_end ; ++k)
            {
                if (! (*k)->consider_added_or_changed())
                    continue;

                for (Choice::ConstIterator i((*k)->begin()), i_end((*k)->end()) ;
                        i != i_end ; ++i)
                    if ((*i)->explicitly_listed())
                        i_common.insert((*i)->name_with_prefix());
            }

            for (Choices::ConstIterator k(existing_choices->begin()), k_end(existing_choices->end()) ;
                    k != k_end ; ++k)
            {
                if (! (*k)->consider_added_or_changed())
                    continue;

                for (Choice::ConstIterator i((*k)->begin()), i_end((*k)->end()) ;
                        i != i_end ; ++i)
                    if ((*i)->explicitly_listed())
                        u_common.insert((*i)->name_with_prefix());
            }

            std::set_intersection(
                    i_common.begin(), i_common.end(),
                    u_common.begin(), u_common.end(),
                    std::inserter(common, common.begin()));
        }

        for (std::set<ChoiceNameWithPrefix>::const_iterator f(common.begin()), f_end(common.end()) ;
                f != f_end ; ++f)
            if (installable_choices->find_by_name_with_prefix(*f)->enabled() !=
                    existing_choices->find_by_name_with_prefix(*f)->enabled())
            {
                is_same = false;
                is_same_metadata = false;
                break;
            }
    }

    return std::make_tuple(is_same_version, is_same, is_same_metadata);
}

