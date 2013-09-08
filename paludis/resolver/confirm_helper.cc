/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011, 2013 Ciaran McCreesh
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

#include <paludis/resolver/confirm_helper.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/required_confirmations.hh>
#include <paludis/resolver/decision_utils.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/dep_spec.hh>
#include <paludis/package_id.hh>
#include <paludis/package_dep_spec_collection.hh>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <>
    struct Imp<ConfirmHelper>
    {
        const Environment * const env;
        PackageDepSpecCollection permit_downgrade_specs;
        PackageDepSpecCollection permit_old_version_specs;
        PackageDepSpecCollection allowed_to_break_specs;
        bool allowed_to_break_system;

        Imp(const Environment * const e) :
            env(e),
            permit_downgrade_specs(nullptr),
            permit_old_version_specs(nullptr),
            allowed_to_break_specs(nullptr),
            allowed_to_break_system(false)
        {
        }
    };
}

ConfirmHelper::ConfirmHelper(const Environment * const e) :
    _imp(e)
{
}

ConfirmHelper::~ConfirmHelper() = default;

void
ConfirmHelper::add_permit_downgrade_spec(const PackageDepSpec & spec)
{
    _imp->permit_downgrade_specs.insert(spec);
}

void
ConfirmHelper::add_permit_old_version_spec(const PackageDepSpec & spec)
{
    _imp->permit_old_version_specs.insert(spec);
}

void
ConfirmHelper::add_allowed_to_break_spec(const PackageDepSpec & spec)
{
    _imp->allowed_to_break_specs.insert(spec);
}

void
ConfirmHelper::set_allowed_to_break_system(const bool b)
{
    _imp->allowed_to_break_system = b;
}

bool
ConfirmHelper::operator() (
        const std::shared_ptr<const Resolution> & resolution,
        const std::shared_ptr<const RequiredConfirmation> & confirmation) const
{
    auto id(get_decided_id_or_null(resolution->decision()));
    return confirmation->make_accept_returning(
            [&] (const DowngradeConfirmation &) {
                if (id)
                    if (_imp->permit_downgrade_specs.match_any(_imp->env, id, { }))
                        return true;

                return false;
            },

            [&] (const NotBestConfirmation &) {
                if (id)
                    if (_imp->permit_old_version_specs.match_any(_imp->env, id, { }))
                        return true;

                return false;
            },

            [&] (const BreakConfirmation &) {
                if (id)
                    if (_imp->allowed_to_break_specs.match_any(_imp->env, id, { }))
                        return true;

                return false;
            },

            [&] (const RemoveSystemPackageConfirmation &) {
                return _imp->allowed_to_break_system;
            },

            [&] (const MaskedConfirmation &) {
                return false;
            },

            [&] (const ChangedChoicesConfirmation &) {
                return false;
            },

            [&] (const UninstallConfirmation &) {
                return false;
            }
        );
}

namespace paludis
{
    template class Pimp<ConfirmHelper>;
}

