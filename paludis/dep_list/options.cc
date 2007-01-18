/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "options.hh"
#include <ostream>
#include <paludis/util/exception.hh>

using namespace paludis;

std::ostream &
paludis::operator<< (std::ostream & o, const DepListTargetType & s)
{
    do
    {
        switch (s)
        {
            case dl_target_package:
                o << "package";
                continue;

            case dl_target_set:
                o << "set";
                continue;

            case last_dl_target:
                ;
        }

        throw InternalError(PALUDIS_HERE, "Bad DepListTargetType");
    } while (false);

    return o;
}

std::ostream &
paludis::operator<< (std::ostream & o, const DepListReinstallOption & s)
{
    do
    {
        switch (s)
        {
            case dl_reinstall_never:
                o << "never";
                continue;

            case dl_reinstall_always:
                o << "always";
                continue;

            case dl_reinstall_if_use_changed:
                o << "if_use_changed";
                continue;

            case last_dl_reinstall:
                ;
        }

        throw InternalError(PALUDIS_HERE, "Bad DepListReinstallOption");
    } while (false);

    return o;
}

std::ostream &
paludis::operator<< (std::ostream & o, const DepListFallBackOption & s)
{
    do
    {
        switch (s)
        {
            case dl_fall_back_as_needed_except_targets:
                o << "as_needed_except_targets";
                continue;

            case dl_fall_back_as_needed:
                o << "as_needed";
                continue;

            case dl_fall_back_never:
                o << "never";
                continue;

            case last_dl_fall_back:
                ;
        }

        throw InternalError(PALUDIS_HERE, "Bad DepListFallBackOption");
    } while (false);

    return o;
}

std::ostream &
paludis::operator<< (std::ostream & o, const DepListReinstallScmOption & s)
{
    do
    {
        switch (s)
        {
            case dl_reinstall_scm_never:
                o << "never";
                continue;

            case dl_reinstall_scm_always:
                o << "always";
                continue;

            case dl_reinstall_scm_daily:
                o << "daily";
                continue;

            case dl_reinstall_scm_weekly:
                o << "weekly";
                continue;

            case last_dl_reinstall_scm:
                ;
        }

        throw InternalError(PALUDIS_HERE, "Bad DepListReinstallScmOption");
    } while (false);

    return o;
}

std::ostream &
paludis::operator<< (std::ostream & o, const DepListUpgradeOption & s)
{
    do
    {
        switch (s)
        {
            case dl_upgrade_always:
                o << "always";
                continue;

            case dl_upgrade_as_needed:
                o << "as_needed";
                continue;

            case last_dl_upgrade:
                ;
        }

        throw InternalError(PALUDIS_HERE, "Bad DepListUpgradeOption");
    } while (false);

    return o;
}

std::ostream &
paludis::operator<< (std::ostream & o, const DepListDowngradeOption & s)
{
    do
    {
        switch (s)
        {
            case dl_downgrade_as_needed:
                o << "as_needed";
                continue;

            case dl_downgrade_warning:
               o << "warning";
                continue;

            case dl_downgrade_error:
               o << "error";
                continue;

            case last_dl_downgrade:
                ;
        }

        throw InternalError(PALUDIS_HERE, "Bad DepListDowngradeOption");
    } while (false);

    return o;
}

std::ostream &
paludis::operator<< (std::ostream & o, const DepListNewSlotsOption & s)
{
    do
    {
        switch (s)
        {
            case dl_new_slots_always:
                o << "always";
                continue;

            case dl_new_slots_as_needed:
                o << "as_needed";
                continue;

            case last_dl_new_slots:
                ;
        }

        throw InternalError(PALUDIS_HERE, "Bad DepListNewSlotsOption");
    } while (false);

    return o;
}

std::ostream &
paludis::operator<< (std::ostream & o, const DepListDepsOption & s)
{
    do
    {
        switch (s)
        {
            case dl_deps_discard:
                o << "discard";
                continue;

            case dl_deps_pre:
                o << "pre";
                continue;

            case dl_deps_pre_or_post:
                o << "pre_or_post";
                continue;

            case dl_deps_post:
                o << "post";
                continue;

            case dl_deps_try_post:
                o << "try_post";
                continue;

            case last_dl_deps:
                ;
        }

        throw InternalError(PALUDIS_HERE, "Bad DepListDepsOption");
    } while (false);

    return o;
}

std::ostream &
paludis::operator<< (std::ostream & o, const DepListSuggestedOption & s)
{
    do
    {
        switch (s)
        {
            case dl_suggested_show:
                o << "show";
                continue;

            case dl_suggested_discard:
                o << "discard";
                continue;

            case dl_suggested_install:
                o << "install";
                continue;

            case last_dl_suggested:
                ;
        }

        throw InternalError(PALUDIS_HERE, "Bad DepListSuggestedOption");
    } while (false);

    return o;
}

std::ostream &
paludis::operator<< (std::ostream & o, const DepListCircularOption & s)
{
    do
    {
        switch (s)
        {
            case dl_circular_error:
                o << "error";
                continue;

            case dl_circular_discard:
                o << "discard";
                continue;

            case dl_circular_discard_silently:
                o << "circular_discard_silently";
                continue;

            case last_dl_circular:
                ;
        }

        throw InternalError(PALUDIS_HERE, "Bad DepListCircularOption");
    } while (false);

    return o;
}

std::ostream &
paludis::operator<< (std::ostream & o, const DepListBlocksOption & s)
{
    do
    {
        switch (s)
        {
            case dl_blocks_accumulate:
                o << "accumulate";
                continue;

            case dl_blocks_error:
                o << "error";
                continue;

            case dl_blocks_discard:
                o << "discard";
                continue;

            case last_dl_blocks:
                ;
        }

        throw InternalError(PALUDIS_HERE, "Bad DepListBlocksOption");
    } while (false);

    return o;
}

std::ostream &
paludis::operator<< (std::ostream & o, const DepListUseOption & s)
{
    do
    {
        switch (s)
        {
            case dl_use_deps_standard:
                o << "standard";
                continue;

            case dl_use_deps_take_all:
                o << "take_all";
                continue;

            case last_dl_use_deps:
                ;
        }

        throw InternalError(PALUDIS_HERE, "Bad DepListUseOption");
    } while (false);

    return o;
}

std::ostream &
paludis::operator<< (std::ostream & o, const DepListEntryState & s)
{
    do
    {
        switch (s)
        {
            case dle_no_deps:
                o << "no_deps";
                continue;

            case dle_has_pre_deps:
                o << "has_pre_deps";
                continue;

            case dle_has_all_deps:
                o << "has_all_deps";
                continue;

            case last_dle:
                ;
        }

        throw InternalError(PALUDIS_HERE, "Bad DepListEntryState");
    } while (false);

    return o;
}

std::ostream &
paludis::operator<< (std::ostream & o, const DepListEntryKind & s)
{
    do
    {
        switch (s)
        {
            case dlk_package:
                o << "package";
                continue;

            case dlk_subpackage:
                o << "sub_package";
                continue;

            case dlk_suggested:
                o << "suggested";
                continue;

            case dlk_already_installed:
                o << "already_installed";
                continue;

            case dlk_virtual:
                o << "virtual";
                continue;

            case dlk_provided:
                o << "provided";
                continue;

            case dlk_block:
                o << "block";
                continue;

            case dlk_masked:
                o << "masked";
                continue;

            case last_dlk:
                ;
        }

        throw InternalError(PALUDIS_HERE, "Bad DepListEntryKind");
    } while (false);

    return o;
}

std::ostream &
paludis::operator<< (std::ostream & o, const DepListOverrideMask & s)
{
    do
    {
        switch (s)
        {
            case dl_override_licenses:
                o << "licenses";
                continue;

            case dl_override_tilde_keywords:
                o << "tilde_keywords";
                continue;

            case dl_override_unkeyworded:
                o << "unkeyworded";
                continue;

            case dl_override_repository_masks:
                o << "repository_masks";
                continue;

            case dl_override_profile_masks:
                o << "profile_masks";
                continue;

            case last_dl_override:
                ;
        }

        throw InternalError(PALUDIS_HERE, "Bad DepListEntryKind");
    } while (false);

    return o;
}

