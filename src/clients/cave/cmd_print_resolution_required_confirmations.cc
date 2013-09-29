/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011, 2012, 2013 Ciaran McCreesh
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

#include "cmd_print_resolution_required_confirmations.hh"
#include "resolve_cmdline.hh"
#include "exceptions.hh"
#include "command_command_line.hh"

#include <paludis/args/do_help.hh>

#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/system.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/join.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/options.hh>
#include <paludis/util/set.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/log.hh>
#include <paludis/util/pretty_print.hh>
#include <paludis/util/enum_iterator.hh>

#include <paludis/resolver/resolutions_by_resolvent.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/sanitised_dependencies.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/resolver.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/destination.hh>
#include <paludis/resolver/unsuitable_candidates.hh>
#include <paludis/resolver/decisions.hh>
#include <paludis/resolver/required_confirmations.hh>
#include <paludis/resolver/orderer_notes.hh>
#include <paludis/resolver/change_by_resolvent.hh>
#include <paludis/resolver/match_qpns.hh>
#include <paludis/resolver/why_changed_choices.hh>
#include <paludis/resolver/collect_depped_upon.hh>

#include <paludis/package_id.hh>
#include <paludis/version_spec.hh>
#include <paludis/metadata_key.hh>
#include <paludis/choice.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/match_package.hh>
#include <paludis/repository.hh>
#include <paludis/package_dep_spec_properties.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/environment.hh>
#include <paludis/mask.hh>
#include <paludis/serialise.hh>
#include <paludis/action.hh>
#include <paludis/output_manager_from_environment.hh>
#include <paludis/output_manager.hh>
#include <paludis/changed_choices.hh>
#include <paludis/mask_utils.hh>
#include <paludis/dep_spec_annotations.hh>
#include <paludis/slot.hh>

#include <set>
#include <iterator>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <map>
#include <limits>
#include <unistd.h>

using namespace paludis;
using namespace cave;
using namespace paludis::resolver;

using std::cout;
using std::endl;

namespace
{
    struct PrintResolutionRequiredConfirmationsCommandCommandLine :
        CaveCommandCommandLine
    {
        ResolveCommandLineImportOptions import_options;

        PrintResolutionRequiredConfirmationsCommandCommandLine() :
            import_options(this)
        {
            add_environment_variable("PALUDIS_SERIALISED_RESOLUTION_FD",
                    "The file descriptor on which the serialised resolution can be found.");
        }

        virtual std::string app_name() const
        {
            return "cave print-resolution-required-confirmations";
        }

        virtual std::string app_synopsis() const
        {
            return "Displays a machine-readable description of changes required for a resolution created using 'cave resolve'.";
        }

        virtual std::string app_description() const
        {
            return "Displays a a machine-readable description of changes required for a resolution created by "
                "'cave resolve'. Mostly for internal use; most users will not use this command directly.";
        }
    };

    void display_confirmation(
            const std::shared_ptr<Environment> &,
            const std::shared_ptr<const Resolution> & r,
            const RequiredConfirmation & confirmation,
            const std::shared_ptr<const WhyChangedChoices> & why_changed_choices,
            const std::shared_ptr<const PackageID> & id)
    {
        cout << r->resolvent() << " ";
        confirmation.make_accept(
                [&] (const DowngradeConfirmation &)           { cout << "downgrade"; },
                [&] (const NotBestConfirmation &)             { cout << "not_best"; },
                [&] (const BreakConfirmation &)               { cout << "break"; },
                [&] (const RemoveSystemPackageConfirmation &) { cout << "remove_system_package"; },
                [&] (const UninstallConfirmation &)           { cout << "uninstall"; },
                [&] (const MaskedConfirmation &)              { cout << "masked"; },
                [&] (const ChangedChoicesConfirmation &)      {
                        cout << "changed_choices";
                        if (id && id->choices_key()) {
                            auto choices(id->choices_key()->parse_value());
                            for (const auto & c : *choices) {
                                for (const auto & v : *c) {
                                    auto changed_state = why_changed_choices->changed_choices()->overridden_value(v->name_with_prefix());
                                    if (! changed_state.is_indeterminate()) {
                                        cout << " ";
                                        if (changed_state.is_false())
                                            cout << "-";
                                        cout << v->name_with_prefix();
                                    }
                                }
                            }
                        }
                    }
                );
        cout << endl;
    }

    struct DisplayAVisitor
    {
        const std::shared_ptr<Environment> env;
        const std::shared_ptr<const Resolution> resolution;

        void visit(const ChangesToMakeDecision & decision)
        {
            const auto r(decision.required_confirmations_if_any());
            if (r && ! r->empty())
                for (auto & c : *r)
                    display_confirmation(env, resolution, *c, decision.if_changed_choices(), decision.origin_id());
        }

        void visit(const RemoveDecision & decision)
        {
            const auto r(decision.required_confirmations_if_any());
            if (r && ! r->empty())
                for (auto & c : *r)
                    display_confirmation(env, resolution, *c, nullptr, nullptr);
        }

        void visit(const BreakDecision & decision)
        {
            const auto r(decision.required_confirmations_if_any());
            if (r && ! r->empty())
                for (auto & c : *r)
                    display_confirmation(env, resolution, *c, nullptr, nullptr);
        }
    };

    void display(
            const std::shared_ptr<Environment> & env,
            const std::shared_ptr<const Resolved> & resolved)
    {
        Context context("When displaying changes and removes:");

        for (const auto & decision : *resolved->taken_unconfirmed_decisions())
        {
            DisplayAVisitor v{ env, *resolved->resolutions_by_resolvent()->find(decision->resolvent()) };
            decision->accept(v);
        }
    }
}

int
PrintResolutionRequiredConfirmationsCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args,
        const std::shared_ptr<const Resolved> & maybe_resolved
        )
{
    PrintResolutionRequiredConfirmationsCommandCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_PRINT_RESOLUTION_REQUIRED_CONFIRMATIONS_OPTIONS", "CAVE_PRINT_RESOLUTION_REQUIRED_CONFIRMATIONS_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    cmdline.import_options.apply(env);

    std::shared_ptr<const Resolved> resolved(maybe_resolved);
    if (! resolved)
    {
        if (getenv_with_default("PALUDIS_SERIALISED_RESOLUTION_FD", "").empty())
            throw args::DoHelp("PALUDIS_SERIALISED_RESOLUTION_FD must be provided");

        int fd(destringify<int>(getenv_with_default("PALUDIS_SERIALISED_RESOLUTION_FD", "")));
        SafeIFStream deser_stream(fd);
        Deserialiser deserialiser(env.get(), deser_stream);
        Deserialisation deserialisation("Resolved", deserialiser);
        resolved = make_shared_copy(Resolved::deserialise(deserialisation));
        close(fd);
    }

    display(env, resolved);

    return 0;
}

int
PrintResolutionRequiredConfirmationsCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args)
{
    return run(env, args, nullptr);
}

std::shared_ptr<args::ArgsHandler>
PrintResolutionRequiredConfirmationsCommand::make_doc_cmdline()
{
    return std::make_shared<PrintResolutionRequiredConfirmationsCommandCommandLine>();
}

CommandImportance
PrintResolutionRequiredConfirmationsCommand::importance() const
{
    return ci_internal;
}

