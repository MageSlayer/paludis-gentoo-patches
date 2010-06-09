/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010 Ciaran McCreesh
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
#include <paludis/choice.hh>
#include <paludis/name.hh>
#include <tr1/functional>
#include <string>

namespace paludis
{
    struct UseDesc;

    namespace erepository
    {
        class PALUDIS_VISIBLE EChoiceValue :
            public ChoiceValue
        {
            private:
                const ChoicePrefixName _prefix;
                const UnprefixedChoiceName _unprefixed_name;
                const ChoiceNameWithPrefix _name_with_prefix;
                const QualifiedPackageName _package_name;
                const std::tr1::shared_ptr<const UseDesc> _use_desc;
                const bool _enabled;
                const bool _enabled_by_default;
                const bool _locked;
                const bool _explicitly_listed;
                const std::string _override_description;
                const std::string _parameter;

            public:
                EChoiceValue(const ChoicePrefixName & r,
                        const UnprefixedChoiceName & n,
                        const ChoiceNameWithPrefix & np,
                        const QualifiedPackageName & p,
                        const std::tr1::shared_ptr<const UseDesc> & d,
                        bool b, bool def,
                        bool l, bool x,
                        const std::string & o,
                        const std::string & pr);

                const UnprefixedChoiceName unprefixed_name() const;
                const ChoiceNameWithPrefix name_with_prefix() const;
                bool enabled() const;
                bool enabled_by_default() const;
                bool locked() const;
                const std::string description() const;
                bool explicitly_listed() const;
                const std::string parameter() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }
}

#endif
