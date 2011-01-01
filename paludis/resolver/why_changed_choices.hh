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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_WHY_CHANGED_CHOICES_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_WHY_CHANGED_CHOICES_HH 1

#include <paludis/resolver/why_changed_choices-fwd.hh>
#include <paludis/resolver/reason-fwd.hh>
#include <paludis/util/named_value.hh>
#include <paludis/changed_choices-fwd.hh>
#include <paludis/serialise-fwd.hh>

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_changed_choices> changed_choices;
        typedef Name<struct name_reasons> reasons;
    }

    namespace resolver
    {
        struct WhyChangedChoices
        {
            NamedValue<n::changed_choices, std::shared_ptr<ChangedChoices> > changed_choices;
            NamedValue<n::reasons, std::shared_ptr<Reasons> > reasons;

            void serialise(Serialiser &) const;

            static const std::shared_ptr<WhyChangedChoices> deserialise(
                    Deserialisation & d) PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }
}

#endif
