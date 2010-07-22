/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#include "command_line.hh"
#include <paludis/util/singleton-impl.hh>

using namespace paludis;

template class paludis::Singleton<CommandLine>;

CommandLine::CommandLine() :
    ArgsHandler(),

    action_args(main_options_section(), "Actions",
            "Selects which basic action to perform. Exactly one action should "
            "be specified."),
    a_install(&action_args,   "install",      'i',  "Install one or more packages (default)", false),
    a_version(&action_args,   "version",      'V',  "Display program version", false),
    a_help(&action_args,      "help",         'h',  "Display program help", false),

    general_args(main_options_section(), "General options",
            "Options which are relevant for most or all actions."),
    a_log_level(&general_args, "log-level",  '\0'),
    a_no_colour(&general_args, "no-colour", '\0', "Do not use colour", false),
    a_no_color(&a_no_colour, "no-color"),
    a_force_colour(&general_args, "force-colour", '\0', "Force the use of colour", false),
    a_force_color(&a_force_colour, "force-color"),
    a_environment(&general_args, "environment", 'E', "Environment specification (class:suffix, both parts optional)"),
    a_compact(&general_args, "compact", '\0', "Display output using one line per entry", true),

    source_args(main_options_section(), "Source options",
            "Options affecting the source image"),
    a_location(&source_args, "location", 'l', "Location of source image (default: current directory)"),
    a_install_under(&source_args, "install-under", 'u', "Install under a given directory (default: /)"),
    a_rewrite_ids_over_to_root(&source_args, "rewrite-ids-over-to-root", 'r',
            "Change any UID or GID over this value to 0 (-1 disables, default)"),

    metadata_args(main_options_section(), "Metadata options",
            "Options affecting generated metadata"),
    a_description(&metadata_args, "description", 'D', "Specify a package description"),
    a_build_dependency(&metadata_args, "build-dependency", 'B', "Specify a build dependency"),
    a_run_dependency(&metadata_args, "run-dependency", 'R', "Specify a run dependency"),
    a_preserve_metadata(&metadata_args, "preserve-metadata", 'P', "If replacing a package, copy its description and dependencies", true),

    install_args(main_options_section(), "Install options",
            "Options which are relevant for --install"),

    dl_args(main_options_section())
{
    add_usage_line("[ --install ] [ --location path/ ] category/package [ version ] [ slot ]");

    install_args.a_add_to_world_spec.remove();
    install_args.a_fetch.remove();
    install_args.a_no_safe_resume.remove();

    dl_args.dl_reinstall_targets.remove();

    add_environment_variable("IMPORTARE_OPTIONS", "Default command-line options.");

    add_note(
            "importare requires a repository with format 'installed_unpackaged' configured and available. It cannot use "
            "a standard VDB or suchlike because there is no ebuild available.");
    add_note(
            "Packages installed using importare will not be visible to broken ebuilds that illegally access the VDB. "
            "This means that things like 'built_with_use' will not realise that the package is installed.");

    add_example(
            "importare --location img/ unpackaged/myapp 1.23",
            "Install the contents of img/ (which could be produced, for example, using 'sudo make DESTDIR=img/ install' "
            "from an autotools package) as 'unpackaged/myapp' version 1.23. If 'unpackaged/myapp' is already installed, "
            "it will be replaced.");
    add_example(
            "importare --location img/ unpackaged/myapp 1.23 --run-dependency dev-libs/mpfr --run-dependency dev-libs/gmp",
            "As above, and add dependencies. Dependencies are used for resolution and to provide correct output for "
            "paludis --uninstall(-unused).");
    add_example(
            "importare --location img/ unpackaged/myapp 1.23 --preserve-metadata",
            "If an existing version of unpackaged/myapp was installed using importare, copies metadata (build and run "
            "dependencies, and description) from that version.");
    add_example(
            "importare --location /var/empty sys-apps/portage 2.2",
            "Install an empty fake package named 'sys-apps/portage', version 2.2. DANGEROUS!");
}

std::string
CommandLine::app_name() const
{
    return "importare";
}

std::string
CommandLine::app_synopsis() const
{
    return "The Paludis unpackaged package installer";
}

std::string
CommandLine::app_description() const
{
    return
        "importare can be used to manage packages where no real package file is available. It "
        "treats the contents of a named directory as being the content of the package, and uses "
        "a dummy package name provided on the command line to do the install. Safe merge, unmerge, "
        "upgrade and replace support is provided, as is content tracking for installed files.";
}

CommandLine::~CommandLine()
{
}

