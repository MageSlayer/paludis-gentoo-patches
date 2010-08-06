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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_CONFIRM_HELPER_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_CONFIRM_HELPER_HH 1

#include <paludis/resolver/confirm_helper-fwd.hh>
#include <paludis/resolver/resolution-fwd.hh>
#include <paludis/resolver/required_confirmations-fwd.hh>
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
        class PALUDIS_VISIBLE ConfirmHelper :
            private Pimp<ConfirmHelper>
        {
            public:
                explicit ConfirmHelper(const Environment * const);
                ~ConfirmHelper();

                void add_permit_downgrade_spec(const PackageDepSpec &);
                void add_permit_old_version_spec(const PackageDepSpec &);
                void add_allowed_to_break_spec(const PackageDepSpec &);
                void set_allowed_to_break_system(const bool);

                bool operator() (
                        const std::shared_ptr<const Resolution> &,
                        const std::shared_ptr<const RequiredConfirmation> &) const;
        };
    }

    extern template class Pimp<resolver::ConfirmHelper>;
}

#endif
