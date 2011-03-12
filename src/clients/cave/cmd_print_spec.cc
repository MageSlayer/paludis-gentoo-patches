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

#include "cmd_print_spec.hh"
#include "exceptions.hh"
#include "format_string.hh"
#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/set.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/options.hh>
#include <paludis/util/map.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/join.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/environment.hh>
#include <paludis/metadata_key.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/additional_package_dep_spec_requirement.hh>
#include <paludis/partially_made_package_dep_spec.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/version_operator.hh>
#include <paludis/version_requirements.hh>
#include <iostream>
#include <algorithm>

#include "command_command_line.hh"

using namespace paludis;
using namespace cave;
using std::cout;
using std::endl;

namespace
{
    struct PrintSpecCommandLine :
        CaveCommandCommandLine
    {
        virtual std::string app_name() const
        {
            return "cave print-spec";
        }

        virtual std::string app_synopsis() const
        {
            return "Prints a dependency spec.";
        }

        virtual std::string app_description() const
        {
            return "Parses a dependency spec and prints it out, possibly after applying certain modifications. No "
                "formatting is used, making the output suitable for parsing by scripts.";
        }

        args::ArgsGroup g_modifications;
        args::StringArg a_package;
        args::StringArg a_slot_requirement;
        args::StringArg a_in_repository;
        args::StringArg a_from_repository;
        args::StringArg a_installable_to_repository;
        args::StringArg a_installed_at_path;
        args::StringArg a_installable_to_path;
        args::StringArg a_package_part;
        args::StringArg a_category_part;
        args::StringSetArg a_version_requirement;
        args::EnumArg a_version_requirements_mode;
        args::StringSetArg a_additional_requirement;

        PrintSpecCommandLine() :
            g_modifications(main_options_section(), "Modification Options", "Options for modifying the spec. If an empty string is "
                    "specified for a modification, that requirement is removed."),
            a_package(&g_modifications, "package", '\0', "Specify the cat/pkg requirement."),
            a_slot_requirement(&g_modifications, "slot", '\0', "Specify the slot requirement."),
            a_in_repository(&g_modifications, "in-repository", '\0', "Specify the in-repository requirement."),
            a_from_repository(&g_modifications, "from-repository", '\0', "Specify the from-repository requirement."),
            a_installable_to_repository(&g_modifications, "installable-to-repository", '\0', "Specify the installable-to-repository requirement."),
            a_installed_at_path(&g_modifications, "installed-at-path", '\0', "Specify the installed-at-path requirement."),
            a_installable_to_path(&g_modifications, "installable-to-path", '\0', "Specify the installable-to-path requirement."),
            a_package_part(&g_modifications, "package-part", '\0', "Specify the /pkg requirement."),
            a_category_part(&g_modifications, "category-part", '\0', "Specify the cat/ requirement."),
            a_version_requirement(&g_modifications, "version-requirement", '\0', "Specify a version requirement. May be specified "
                    "multiple times. If specified at all, replaces all version requirements. Use a single empty string to remove "
                    "all version requirements."),
            a_version_requirements_mode(&g_modifications, "version-requirements-mode", '\0', "Specify the mode of version requirements.",
                    args::EnumArg::EnumArgOptions
                    ("default", "Do not change the version requirements mode")
                    ("and",     "And")
                    ("or",      "Or"),
                    "default"),
            a_additional_requirement(&g_modifications, "additional-requirement", '\0', "Specify an additional requirement. May be "
                    "specified multiple times.")
        {
            add_usage_line("spec");
        }
    };

    void do_one_spec(
            const PackageDepSpec & spec,
            const PrintSpecCommandLine & cmdline
            )
    {
        PartiallyMadePackageDepSpec s(spec);

        if (cmdline.a_package.specified())
        {
            if (cmdline.a_package.argument().empty())
                s.clear_package();
            else
                s.package(QualifiedPackageName(cmdline.a_package.argument()));
        }

        if (cmdline.a_slot_requirement.specified())
        {
            if (cmdline.a_slot_requirement.argument().empty())
                s.clear_slot_requirement();
            else
                s.slot_requirement(std::make_shared<UserSlotExactRequirement>(SlotName(cmdline.a_slot_requirement.argument())));
        }

        if (cmdline.a_in_repository.specified())
        {
            if (cmdline.a_in_repository.argument().empty())
                s.clear_in_repository();
            else
                s.in_repository(RepositoryName(cmdline.a_in_repository.argument()));
        }

        if (cmdline.a_from_repository.specified())
        {
            if (cmdline.a_from_repository.argument().empty())
                s.clear_from_repository();
            else
                s.from_repository(RepositoryName(cmdline.a_from_repository.argument()));
        }

        if (cmdline.a_installable_to_repository.specified())
        {
            if (cmdline.a_installable_to_repository.argument().empty())
                s.clear_installable_to_repository();
            else
            {
                std::string repo(cmdline.a_installable_to_repository.argument());
                bool include_masked(false);
                if ('?' == repo.at(repo.length() - 1))
                {
                    repo.erase(repo.length() - 1);
                    include_masked = true;
                }

                s.installable_to_repository(make_named_values<InstallableToRepository>(
                            n::include_masked() = include_masked,
                            n::repository() = RepositoryName(repo)
                            ));
            }
        }

        if (cmdline.a_installed_at_path.specified())
        {
            if (cmdline.a_installed_at_path.argument().empty())
                s.clear_installed_at_path();
            else
                s.installed_at_path(FSPath(cmdline.a_installed_at_path.argument()));
        }

        if (cmdline.a_installable_to_path.specified())
        {
            if (cmdline.a_installable_to_path.argument().empty())
                s.clear_installable_to_path();
            else
            {
                std::string path(cmdline.a_installable_to_path.argument());
                bool include_masked(false);
                if ('?' == path.at(path.length() - 1))
                {
                    path.erase(path.length() - 1);
                    include_masked = true;
                }

                s.installable_to_path(make_named_values<InstallableToPath>(
                            n::include_masked() = include_masked,
                            n::path() = FSPath(path)
                            ));
            }
        }

        if (cmdline.a_package_part.specified())
        {
            if (cmdline.a_package_part.argument().empty())
                s.clear_package_name_part();
            else
                s.package_name_part(PackageNamePart(cmdline.a_package_part.argument()));
        }

        if (cmdline.a_category_part.specified())
        {
            if (cmdline.a_category_part.argument().empty())
                s.clear_category_name_part();
            else
                s.category_name_part(CategoryNamePart(cmdline.a_category_part.argument()));
        }

        if (cmdline.a_version_requirement.specified())
        {
            s.clear_version_requirements();

            for (args::StringSetArg::ConstIterator a(cmdline.a_version_requirement.begin_args()),
                    a_end(cmdline.a_version_requirement.end_args()) ;
                    a != a_end ; ++a)
                if (! a->empty())
                {
                    std::string::size_type p(a->find_first_not_of("=<>~"));
                    if (std::string::npos == p)
                        throw args::DoHelp("--" + cmdline.a_version_requirement.long_name() + " arguments should be in the form =1.23");

                    std::string op(a->substr(0, p)), ver(a->substr(p));

                    s.version_requirement(make_named_values<VersionRequirement>(
                                n::version_operator() = VersionOperator(op),
                                n::version_spec() = VersionSpec(ver, {})
                                ));
                }
        }

        if (cmdline.a_version_requirements_mode.specified())
        {
            if (cmdline.a_version_requirements_mode.argument() == "and")
                s.version_requirements_mode(vr_and);
            else if (cmdline.a_version_requirements_mode.argument() == "or")
                s.version_requirements_mode(vr_or);
            else
                throw args::DoHelp("Argument for --" + cmdline.a_version_requirements_mode.long_name() + " unrecognised");
        }

        if (cmdline.a_additional_requirement.specified())
        {
            s.clear_additional_requirements();

            for (args::StringSetArg::ConstIterator a(cmdline.a_additional_requirement.begin_args()),
                    a_end(cmdline.a_additional_requirement.end_args()) ;
                    a != a_end ; ++a)
                if (! a->empty())
                    s.additional_requirement(std::make_shared<UserKeyRequirement>(*a));
        }

        cout << PackageDepSpec(s) << endl;
    }
}

int
PrintSpecCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    PrintSpecCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_PRINT_SPEC_OPTIONS", "CAVE_PRINT_SPEC_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (1 != std::distance(cmdline.begin_parameters(), cmdline.end_parameters()))
        throw args::DoHelp("print-spec takes exactly one parameter");

    PackageDepSpec spec(parse_user_package_dep_spec(*cmdline.begin_parameters(), env.get(), { updso_allow_wildcards }));
    do_one_spec(spec, cmdline);

    return EXIT_SUCCESS;
}

std::shared_ptr<args::ArgsHandler>
PrintSpecCommand::make_doc_cmdline()
{
    return std::make_shared<PrintSpecCommandLine>();
}

CommandImportance
PrintSpecCommand::importance() const
{
    return ci_scripting;
}

