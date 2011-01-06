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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_GET_INITIAL_CONSTRAINTS_FOR_HELPER_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_GET_INITIAL_CONSTRAINTS_FOR_HELPER_HH 1

#include <paludis/resolver/get_initial_constraints_for_helper-fwd.hh>
#include <paludis/resolver/resolvent-fwd.hh>
#include <paludis/resolver/constraint-fwd.hh>
#include <paludis/resolver/suggest_restart-fwd.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/attributes.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <memory>

namespace paludis
{
    namespace resolver
    {
        class PALUDIS_VISIBLE GetInitialConstraintsForHelper :
            private Pimp<GetInitialConstraintsForHelper>
        {
            private:
                const std::shared_ptr<Constraints> _make_initial_constraints_for(const Resolvent &) const;

            public:
                explicit GetInitialConstraintsForHelper(const Environment * const);
                ~GetInitialConstraintsForHelper();

                void add_without_spec(const PackageDepSpec &);

                void add_preset_spec(const PackageDepSpec &, const std::shared_ptr<const PackageID> & from_id);

                void add_suggested_restart(const SuggestRestart &);

                void set_reinstall_scm_days(const int);

                const std::shared_ptr<Constraints> operator() (
                        const Resolvent &) const;
        };
    }

    extern template class Pimp<resolver::GetInitialConstraintsForHelper>;
}

#endif
