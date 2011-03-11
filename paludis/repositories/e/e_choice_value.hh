/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_E_CHOICE_VALUE_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_E_CHOICE_VALUE_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/util/named_value.hh>
#include <paludis/choice.hh>
#include <paludis/name.hh>
#include <functional>
#include <string>

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_choice_name_with_prefix> choice_name_with_prefix;
        typedef Name<struct name_choice_prefix_name> choice_prefix_name;
        typedef Name<struct name_description> description;
        typedef Name<struct name_enabled> enabled;
        typedef Name<struct name_enabled_by_default> enabled_by_default;
        typedef Name<struct name_explicitly_listed> explicitly_listed;
        typedef Name<struct name_locked> locked;
        typedef Name<struct name_unprefixed_choice_name> unprefixed_choice_name;
    }

    namespace erepository
    {
        struct EChoiceValueParams
        {
            NamedValue<n::choice_name_with_prefix, ChoiceNameWithPrefix> choice_name_with_prefix;
            NamedValue<n::choice_prefix_name, ChoicePrefixName> choice_prefix_name;
            NamedValue<n::description, std::string> description;
            NamedValue<n::enabled, bool> enabled;
            NamedValue<n::enabled_by_default, bool> enabled_by_default;
            NamedValue<n::explicitly_listed, bool> explicitly_listed;
            NamedValue<n::locked, bool> locked;
            NamedValue<n::unprefixed_choice_name, UnprefixedChoiceName> unprefixed_choice_name;
        };

        const std::shared_ptr<const ChoiceValue> create_e_choice_value(const EChoiceValueParams &);
    }
}

#endif
