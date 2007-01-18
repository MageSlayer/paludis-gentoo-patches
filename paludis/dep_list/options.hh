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

#ifndef PALUDIS_GUARD_PALUDIS_DEP_LIST_OPTIONS_HH
#define PALUDIS_GUARD_PALUDIS_DEP_LIST_OPTIONS_HH 1

#include <iosfwd>
#include <paludis/util/attributes.hh>
#include <bitset>

namespace paludis
{
    /**
     * What type of target are we handling at the top level.
     *
     * \ingroup grpdepresolver
     */
    enum DepListTargetType
    {
        dl_target_package,   ///< A package, so force reinstalls.
        dl_target_set,       ///< A set, so don't force reinstalls.
        last_dl_target
    };

    /**
     * When should we reinstall.
     *
     * \ingroup grpdepresolver
     */
    enum DepListReinstallOption
    {
        dl_reinstall_never,             ///< Never.
        dl_reinstall_always,            ///< Always.
        dl_reinstall_if_use_changed,    ///< If a USE flag has changed.
        last_dl_reinstall
    };

    /**
     * When can we fall back to installed?
     *
     * \ingroup grpdepresolver
     */
    enum DepListFallBackOption
    {
        dl_fall_back_as_needed_except_targets,
        dl_fall_back_as_needed,
        dl_fall_back_never,
        last_dl_fall_back
    };

    /**
     * When should we reinstall scm.
     *
     * \ingroup grpdepresolver
     */
    enum DepListReinstallScmOption
    {
        dl_reinstall_scm_never,
        dl_reinstall_scm_always,
        dl_reinstall_scm_daily,
        dl_reinstall_scm_weekly,
        last_dl_reinstall_scm
    };

    /**
     * When should we upgrade.
     *
     * \ingroup grpdepresolver
     */
    enum DepListUpgradeOption
    {
        dl_upgrade_always,          ///< Always.
        dl_upgrade_as_needed,       ///< Only as needed.
        last_dl_upgrade
    };

    /**
     * When should we pull in a new slot.
     *
     * \ingroup grpdepresolver
     */
    enum DepListNewSlotsOption
    {
        dl_new_slots_always,
        dl_new_slots_as_needed,
        last_dl_new_slots
    };

    /**
     * How should we handle a dep class.
     *
     * \ingroup grpdepresolver
     */
    enum DepListDepsOption
    {
        dl_deps_discard,           ///< Discard it
        dl_deps_pre,               ///< As a pre dependency
        dl_deps_pre_or_post,       ///< As a pre dependency with fallback to post
        dl_deps_post,              ///< As a post dependency
        dl_deps_try_post,          ///< As an optional post dependency
        last_dl_deps
    };

    /**
     * How should we handle suggested deps.
     *
     * \ingroup grpdepresolver
     */
    enum DepListSuggestedOption
    {
        dl_suggested_show,         ///< Show
        dl_suggested_discard,      ///< Discard
        dl_suggested_install,      ///< Install
        last_dl_suggested
    };

    /**
     * How we handle circular deps.
     *
     * \ingroup grpdepresolver
     */
    enum DepListCircularOption
    {
        dl_circular_error,             ///< As an error
        dl_circular_discard,           ///< Discard them
        dl_circular_discard_silently,  ///< Discard them silently (not for user visible use)
        last_dl_circular
    };

    /**
     * How we handle blocks.
     *
     * \ingroup grpdepresolver
     */
    enum DepListBlocksOption
    {
        dl_blocks_accumulate, ///< Accumulate them and show all errors together
        dl_blocks_error,      ///< Error on the first one
        dl_blocks_discard,    ///< Discard (dangerous)
        last_dl_blocks
    };

    /**
     * How we handle use deps.
     *
     * Not for end user use. Used by adjutrix and qa.
     *
     * \ingroup grpdepresolver
     */
    enum DepListUseOption
    {
        dl_use_deps_standard,  ///< Behave as standard
        dl_use_deps_take_all,  ///< Take both sides of use conditionals except on masks
        last_dl_use_deps
    };

    /**
     * State of a DepListEntry.
     *
     * \ingroup grpdepresolver
     */
    enum DepListEntryState
    {
        dle_no_deps,         ///< Dependencies have yet to be added
        dle_has_pre_deps,    ///< Predependencies have been added
        dle_has_all_deps,    ///< All dependencies have been added
        last_dle
    };

    /**
     * Kind of a DepListEntry.
     *
     * \ingroup grpdepresolver
     */
    enum DepListEntryKind
    {
        dlk_package,           ///< A package to be installed
        dlk_subpackage,        ///< A package to be installed as part of the previous dlk_package
        dlk_already_installed, ///< An already installed package
        dlk_virtual,           ///< A virtual package
        dlk_provided,          ///< A package provided by the previous dlk_package
        dlk_suggested,         ///< A package suggested by the previous dlk_package
        dlk_block,             ///< A blocked package that must be removed
        dlk_masked,            ///< A masked package that must be unmasked
        last_dlk
    };

    /**
     * Masks that can be overridden.
     *
     * \ingroup grpdepresolver
     * \see DepListOverrideMasks
     */
    enum DepListOverrideMask
    {
        dl_override_licenses,         ///< Override unaccepted licences
        dl_override_tilde_keywords,   ///< Override ~keywords
        dl_override_unkeyworded,      ///< Override unkeyworded
        dl_override_repository_masks, ///< Override repository masks
        dl_override_profile_masks,    ///< Override profile masks
        last_dl_override
    };

    /**
     * Set of masks that can be overridden.
     *
     * \ingroup grpdepresolver
     * \see DepListOverrideMask
     */
    typedef std::bitset<last_dl_override> DepListOverrideMasks;

    /**
     * Write a DepListTargetType to a stream.
     *
     * \ingroup grpdepresolver
     */
    std::ostream &
    operator<< (std::ostream &, const DepListTargetType &) PALUDIS_VISIBLE;

    /**
     * Write a DepListReinstallOption to a stream.
     *
     * \ingroup grpdepresolver
     */
    std::ostream &
    operator<< (std::ostream &, const DepListReinstallOption &) PALUDIS_VISIBLE;

    /**
     * Write a DepListFallBackOption to a stream.
     *
     * \ingroup grpdepresolver
     */
    std::ostream &
    operator<< (std::ostream &, const DepListFallBackOption &) PALUDIS_VISIBLE;

    /**
     * Write a DepListReinstallScmOption to a stream.
     *
     * \ingroup grpdepresolver
     */
    std::ostream &
    operator<< (std::ostream &, const DepListReinstallScmOption &) PALUDIS_VISIBLE;

    /**
     * Write a DepListUpgradeOption to a stream.
     *
     * \ingroup grpdepresolver
     */
    std::ostream &
    operator<< (std::ostream &, const DepListUpgradeOption &) PALUDIS_VISIBLE;

    /**
     * Write a DepListNewSlotsOption to a stream.
     *
     * \ingroup grpdepresolver
     */
    std::ostream &
    operator<< (std::ostream &, const DepListNewSlotsOption &) PALUDIS_VISIBLE;

    /**
     * Write a DepListDepsOption to a stream.
     *
     * \ingroup grpdepresolver
     */
    std::ostream &
    operator<< (std::ostream &, const DepListDepsOption &) PALUDIS_VISIBLE;

    /**
     * Write a DepListCircularOption to a stream.
     *
     * \ingroup grpdepresolver
     */
    std::ostream &
    operator<< (std::ostream &, const DepListCircularOption &) PALUDIS_VISIBLE;

    /**
     * Write a DepListCircularOption to a stream.
     *
     * \ingroup grpdepresolver
     */
    std::ostream &
    operator<< (std::ostream &, const DepListBlocksOption &) PALUDIS_VISIBLE;

    /**
     * Write a DepListEntryState to a stream.
     *
     * \ingroup grpdepresolver
     */
    std::ostream &
    operator<< (std::ostream &, const DepListEntryState &) PALUDIS_VISIBLE;

    /**
     * Write a DepListEntryKind to a stream.
     *
     * \ingroup grpdepresolver
     */
    std::ostream &
    operator<< (std::ostream &, const DepListEntryKind &) PALUDIS_VISIBLE;
}

#endif
