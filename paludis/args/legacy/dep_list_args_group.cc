/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
 * Copyright (c) 2007 David Leverton
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

#include "dep_list_args_group.hh"

#include <paludis/util/log.hh>
#include <paludis/util/sequence.hh>

#include <paludis/args/do_help.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/override_functions.hh>

using namespace paludis;
using namespace paludis::args;

namespace
{
    DepListDepsOption
    enum_arg_to_dep_list_deps_option(const args::EnumArg & arg)
    {
        if (arg.argument() == "pre")
            return dl_deps_pre;
        else if (arg.argument() == "pre-or-post")
            return dl_deps_pre_or_post;
        else if (arg.argument() == "post")
            return dl_deps_post;
        else if (arg.argument() == "try-post")
            return dl_deps_try_post;
        else if (arg.argument() == "discard")
            return dl_deps_discard;
        else
            throw args::DoHelp("bad value for --" + arg.long_name());
    }
}

DepListArgsGroup::DepListArgsGroup(ArgsSection * h) :
    ArgsGroup(h, "DepList behaviour",
            "Modify dependency list generation behaviour. Use with caution."),

    dl_reinstall(this, "dl-reinstall", '\0', "When to reinstall packages",
            args::EnumArg::EnumArgOptions
            ("never",          "Never")
            ("always",         "Always")
            ("if-use-changed", "If USE flags have changed"),
            "never"),
    dl_reinstall_scm(this, "dl-reinstall-scm", '\0', "When to reinstall scm packages",
            args::EnumArg::EnumArgOptions
            ("never",          "Never")
            ("always",         "Always")
            ("daily",          "If they are over a day old")
            ("weekly",         "If they are over a week old"),
            "never"),
    dl_reinstall_targets(this, "dl-reinstall-targets", '\0', "Whether to reinstall targets",
            args::EnumArg::EnumArgOptions
            ("auto",           "If the target is a set, never, otherwise always")
            ("never",          "Never")
            ("always",         "Always"),
            "auto"),

    dl_upgrade(this, "dl-upgrade", '\0', "When to upgrade packages",
            args::EnumArg::EnumArgOptions
            ("always",        "Always")
            ("as-needed",     "As needed"),
            "always"),
    dl_new_slots(this, "dl-new-slots", '\0', "When to pull in new slots (works with --dl-upgrade)",
            args::EnumArg::EnumArgOptions
            ("always",        "Always")
            ("as-needed",     "As needed"),
            "always"),
    dl_downgrade(this, "dl-downgrade", '\0', "When to downgrade packages",
            args::EnumArg::EnumArgOptions
            ("as-needed",     "As needed")
            ("warning",       "As needed, but warn when doing so")
            ("error",         "Downgrades should be treated as errors"),
            "warning"),

    dl_deps_default(this, "dl-deps-default", '\0',
            "Override default behaviour for all dependency classes",
            static_cast<DepListDepsOption>(-1)),

    dl_installed_deps_pre(this, "dl-installed-deps-pre", '\0',
            "How to handle pre dependencies for installed packages",
            dl_deps_discard),
    dl_installed_deps_runtime(this, "dl-installed-deps-runtime", '\0',
            "How to handle runtime dependencies for installed packages",
            dl_deps_try_post),
    dl_installed_deps_post(this, "dl-installed-deps-post", '\0',
            "How to handle post dependencies for installed packages",
            dl_deps_try_post),

    dl_uninstalled_deps_pre(this, "dl-uninstalled-deps-pre", '\0',
            "How to handle pre dependencies for uninstalled packages",
            dl_deps_pre),
    dl_uninstalled_deps_runtime(this, "dl-uninstalled-deps-runtime", '\0',
            "How to handle runtime dependencies for uninstalled packages",
            dl_deps_pre_or_post),
    dl_uninstalled_deps_post(this, "dl-uninstalled-deps-post", '\0',
            "How to handle post dependencies for uninstalled packages",
            dl_deps_post),
    dl_uninstalled_deps_suggested(this, "dl-uninstalled-deps-suggested", '\0',
            "How to handle suggested dependencies for uninstalled packages (only with --dl-suggested install)",
            dl_deps_post),

    dl_suggested(this, "dl-suggested", '\0', "How to handle suggested dependencies",
            args::EnumArg::EnumArgOptions
            ("show",         "Display, but do not install")
            ("install",      "Install")
            ("discard",      "Discard"),
            "show"),
    dl_circular(this, "dl-circular", '\0', "How to handle circular dependencies",
            args::EnumArg::EnumArgOptions
            ("error",         "Raise an error")
            ("discard",       "Discard"),
            "error"),
    dl_blocks(this, "dl-blocks", '\0', "How to handle blocks",
            args::EnumArg::EnumArgOptions
            ("accumulate",         "Accumulate and show in the dependency list")
            ("error",              "Error straight away")
            ("discard",            "Discard (dangerous)"),
            "accumulate"),
    dl_override_masks(this, "dl-override-masks", '\0',
            "Zero or more mask kinds that can be overridden as necessary (default: tilde-keyword and license)",
            args::StringSetArg::StringSetArgOptions
            ("none",                    "None (overrides defaults, not user selections)")
            ("tilde-keyword",           "Keyword masks where accepting ~ would work")
            ("unkeyworded",             "Keyword masks where a package is unkeyworded")
            ("repository",              "Repository masks")
            ("license",                 "License masks")),

    dl_fall_back(this, "dl-fall-back", '\0', "When to fall back to installed packages",
            args::EnumArg::EnumArgOptions
            ("as-needed-except-targets", "Where necessary, but not for target packages")
            ("as-needed",                "Where necessary, including for target packages")
            ("never",                    "Never"),
            "as-needed-except-targets")
{
}

DepListArgsGroup::~DepListArgsGroup()
{
}

void
DepListArgsGroup::populate_dep_list_options(const Environment * env, DepListOptions & options) const
{
    using namespace std::placeholders;

    if (dl_reinstall.argument() == "never")
        options.reinstall() = dl_reinstall_never;
    else if (dl_reinstall.argument() == "always")
        options.reinstall() = dl_reinstall_always;
    else if (dl_reinstall.argument() == "if-use-changed")
        options.reinstall() = dl_reinstall_if_use_changed;
    else
        throw args::DoHelp("bad value for --dl-reinstall");

    if (dl_reinstall_scm.argument() == "never")
        options.reinstall_scm() = dl_reinstall_scm_never;
    else if (dl_reinstall_scm.argument() == "always")
        options.reinstall_scm() = dl_reinstall_scm_always;
    else if (dl_reinstall_scm.argument() == "daily")
        options.reinstall_scm() = dl_reinstall_scm_daily;
    else if (dl_reinstall_scm.argument() == "weekly")
        options.reinstall_scm() = dl_reinstall_scm_weekly;
    else
        throw args::DoHelp("bad value for --dl-reinstall-scm");

    if (dl_upgrade.argument() == "as-needed")
        options.upgrade() = dl_upgrade_as_needed;
    else if (dl_upgrade.argument() == "always")
        options.upgrade() = dl_upgrade_always;
    else
        throw args::DoHelp("bad value for --dl-upgrade");

    if (dl_new_slots.argument() == "as-needed")
        options.new_slots() = dl_new_slots_as_needed;
    else if (dl_new_slots.argument() == "always")
        options.new_slots() = dl_new_slots_always;
    else
        throw args::DoHelp("bad value for --dl-new-slots");

    if (dl_downgrade.argument() == "as-needed")
        options.downgrade() = dl_downgrade_as_needed;
    else if (dl_downgrade.argument() == "warning")
        options.downgrade() = dl_downgrade_warning;
    else if (dl_downgrade.argument() == "error")
        options.downgrade() = dl_downgrade_error;
    else
        throw args::DoHelp("bad value for --dl-downgrade");

    if (dl_circular.argument() == "discard")
        options.circular() = dl_circular_discard;
    else if (dl_circular.argument() == "error")
        options.circular() = dl_circular_error;
    else
        throw args::DoHelp("bad value for --dl-circular");

    if (dl_suggested.argument() == "show")
        options.suggested() = dl_suggested_show;
    else if (dl_suggested.argument() == "discard")
        options.suggested() = dl_suggested_discard;
    else if (dl_suggested.argument() == "install")
        options.suggested() = dl_suggested_install;
    else
        throw args::DoHelp("bad value for --dl-suggested");

    if (dl_blocks.argument() == "discard")
        options.blocks() = dl_blocks_discard;
    else if (dl_blocks.argument() == "error")
        options.blocks() = dl_blocks_error;
    else if (dl_blocks.argument() == "accumulate")
        options.blocks() = dl_blocks_accumulate;
    else
        throw args::DoHelp("bad value for --dl-blocks");

    if (! options.override_masks())
        options.override_masks() = std::make_shared<DepListOverrideMasksFunctions>();
    options.override_masks()->push_back(std::bind(&override_tilde_keywords, env, _1, _2));
    options.override_masks()->push_back(std::bind(&override_license, _2));

    if (dl_override_masks.specified())
    {
        for (args::StringSetArg::ConstIterator a(dl_override_masks.begin_args()),
                a_end(dl_override_masks.end_args()) ; a != a_end ; ++a)
            if (*a == "none")
                options.override_masks() = std::make_shared<DepListOverrideMasksFunctions>();

        for (args::StringSetArg::ConstIterator a(dl_override_masks.begin_args()),
                a_end(dl_override_masks.end_args()) ; a != a_end ; ++a)
        {
            if (*a == "tilde-keyword")
                options.override_masks()->push_back(std::bind(&override_tilde_keywords, env, _1, _2));
            else if (*a == "unkeyworded")
                options.override_masks()->push_back(std::bind(&override_unkeyworded, env, _1, _2));
            else if (*a == "repository")
                options.override_masks()->push_back(std::bind(&override_repository_masks, _2));
            else if (*a == "license")
                options.override_masks()->push_back(std::bind(&override_license, _2));
            else if (*a == "none")
            {
            }
            else
                throw args::DoHelp("bad value for --dl-override-masks");
        }
    }

    if (dl_fall_back.argument() == "as-needed-except-targets")
        options.fall_back() = dl_fall_back_as_needed_except_targets;
    else if (dl_fall_back.argument() == "as-needed")
        options.fall_back() = dl_fall_back_as_needed;
    else if (dl_fall_back.argument() == "never")
        options.fall_back() = dl_fall_back_never;
    else
        throw args::DoHelp("bad value for --dl-fall-back");

    if (dl_deps_default.specified())
    {
        DepListDepsOption x(dl_deps_default.option());
        options.installed_deps_pre() = x;
        options.installed_deps_post() = x;
        options.installed_deps_runtime() = x;
        options.uninstalled_deps_pre() = x;
        options.uninstalled_deps_post() = x;
        options.uninstalled_deps_runtime() = x;
        options.uninstalled_deps_suggested() = x;
    }

    if (dl_installed_deps_pre.specified() || ! dl_deps_default.specified())
        options.installed_deps_pre() = enum_arg_to_dep_list_deps_option(dl_installed_deps_pre);
    if (dl_installed_deps_runtime.specified() || ! dl_deps_default.specified())
        options.installed_deps_runtime() = enum_arg_to_dep_list_deps_option(dl_installed_deps_runtime);
    if (dl_installed_deps_post.specified() || ! dl_deps_default.specified())
        options.installed_deps_post() = enum_arg_to_dep_list_deps_option(dl_installed_deps_post);

    if (dl_uninstalled_deps_pre.specified() || ! dl_deps_default.specified())
        options.uninstalled_deps_pre() = enum_arg_to_dep_list_deps_option(dl_uninstalled_deps_pre);
    if (dl_uninstalled_deps_runtime.specified() || ! dl_deps_default.specified())
        options.uninstalled_deps_runtime() = enum_arg_to_dep_list_deps_option(dl_uninstalled_deps_runtime);
    if (dl_uninstalled_deps_post.specified() || ! dl_deps_default.specified())
        options.uninstalled_deps_post() = enum_arg_to_dep_list_deps_option(dl_uninstalled_deps_post);
    if (dl_uninstalled_deps_suggested.specified() || ! dl_deps_default.specified())
        options.uninstalled_deps_suggested() = enum_arg_to_dep_list_deps_option(dl_uninstalled_deps_suggested);
}

void
DepListArgsGroup::populate_install_task(const Environment *, InstallTask & task) const
{
    if (dl_reinstall_targets.specified())
    {
        if (dl_reinstall_targets.argument() == "auto")
        {
        }
        else if (dl_reinstall_targets.argument() == "always")
            task.override_target_type(dl_target_package);
        else if (dl_reinstall_targets.argument() == "never")
            task.override_target_type(dl_target_set);
        else
            throw args::DoHelp("bad value for --dl-reinstall-targets");
    }
}

std::string
DepListArgsGroup::paludis_command_fragment() const
{
    return "";
}

std::string
DepListArgsGroup::resume_command_fragment(const InstallTask &) const
{
    return "";
}

