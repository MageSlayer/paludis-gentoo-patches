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

#ifndef PALUDIS_GUARD_PALUDIS_CHANGED_CHOICES_HH
#define PALUDIS_GUARD_PALUDIS_CHANGED_CHOICES_HH 1

#include <paludis/changed_choices-fwd.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/tribool-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/choice-fwd.hh>
#include <paludis/serialise-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/partially_made_package_dep_spec-fwd.hh>
#include <memory>

namespace paludis
{
    class PALUDIS_VISIBLE ChangedChoices
    {
        private:
            Pimp<ChangedChoices> _imp;

        public:
            ChangedChoices();
            ~ChangedChoices();

            bool add_override_if_possible(const ChoiceNameWithPrefix &, const bool);

            Tribool overridden_value(const ChoiceNameWithPrefix &) const PALUDIS_ATTRIBUTE((warn_unused_result));

            bool empty() const PALUDIS_ATTRIBUTE((warn_unused_result));

            void add_constraints_to(PartiallyMadePackageDepSpec &) const;

            void serialise(Serialiser &) const;

            static const std::shared_ptr<ChangedChoices> deserialise(
                    Deserialisation & d) PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    extern template class Pimp<ChangedChoices>;
}

#endif
