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

#include "cmd_info.hh"
#include "cmd_perform.hh"
#include "colour_pretty_printer.hh"
#include "colours.hh"
#include "exceptions.hh"
#include "format_user_config.hh"
#include "parse_spec_with_nice_error.hh"

#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/name.hh>
#include <paludis/environment.hh>
#include <paludis/repository.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/filter.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/slot.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/pretty_print.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/join.hh>
#include <paludis/action.hh>
#include <paludis/about_metadata.hh>
#include <paludis/contents.hh>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <set>
#include <sstream>

#include "command_command_line.hh"

using namespace paludis;
using namespace cave;
using std::cout;
using std::endl;

namespace
{
#include "cmd_info-fmt.hh"

    struct InfoCommandLine :
        CaveCommandCommandLine
    {
        std::string app_name() const override
        {
            return "cave info";
        }

        std::string app_synopsis() const override
        {
            return "Display a summary of configuration and package information.";
        }

        std::string app_description() const override
        {
            return "Displays a formatted summary of configuration and package information, e.g. for "
                "use when submitting bug reports. The package names of any relevant packages for bug "
                "reports should be passed as parameters.";
        }

        InfoCommandLine()
        {
            add_usage_line("[ spec ... ]");
        }
    };

    struct MetadataKeyComparator
    {
        bool operator() (const std::shared_ptr<const MetadataKey> & a, const std::shared_ptr<const MetadataKey> & b) const
        {
            bool a_is_section(visitor_cast<const MetadataSectionKey>(*a));
            bool b_is_section(visitor_cast<const MetadataSectionKey>(*b));
            if (a_is_section != b_is_section)
                return b_is_section;
            if (a->type() != b->type())
                return a->type() < b->type();
            return a->human_name() < b->human_name();
        }
    };

    struct ContentsDisplayer
    {
        const unsigned indent;
        std::stringstream s;

        ContentsDisplayer(const unsigned i) :
            indent(i)
        {
        }

        void visit(const ContentsFileEntry & e)
        {
            s << fuc(fs_contents_file(), fv<'r'>(stringify(e.location_key()->parse_value())), fv<'b'>(indent ? "true" : ""));
        }

        void visit(const ContentsDirEntry & e)
        {
            s << fuc(fs_contents_dir(), fv<'r'>(stringify(e.location_key()->parse_value())), fv<'b'>(indent ? "true" : ""));
        }

        void visit(const ContentsSymEntry & e)
        {
            s << fuc(fs_contents_sym(), fv<'r'>(stringify(e.location_key()->parse_value())), fv<'b'>(indent ? "true" : ""),
                    fv<'v'>(e.target_key()->parse_value()));
        }

        void visit(const ContentsOtherEntry & e)
        {
            s << fuc(fs_contents_other(), fv<'r'>(stringify(e.location_key()->parse_value())), fv<'b'>(indent ? "true" : ""));
        }
    };

    struct InfoDisplayer
    {
        const Environment * const env;
        const InfoCommandLine & cmdline;
        const int indent;

        InfoDisplayer(const Environment * const e, const InfoCommandLine & c, const int i) :
            env(e),
            cmdline(c),
            indent(i)
        {
        }

        void visit(const MetadataSectionKey & k)
        {
            cout << fuc(fs_metadata_subsection(), fv<'i'>(std::string(indent, ' ')), fv<'s'>(k.human_name()));
            std::set<std::shared_ptr<const MetadataKey>, MetadataKeyComparator> keys(k.begin_metadata(), k.end_metadata());
            for (const auto & key : keys)
            {
                InfoDisplayer i(env, cmdline, indent + 1);
                key->accept(i);
            }
        }

        void visit(const MetadataCollectionKey<KeywordNameSet> & k)
        {
            ColourPrettyPrinter printer(env, nullptr, indent);
            cout << fuc(fs_metadata(), fv<'h'>(k.human_name()), fv<'i'>(std::string(indent, ' ')), fv<'s'>(k.pretty_print_value(printer, { })));
        }

        void visit(const MetadataCollectionKey<Set<std::string> > & k)
        {
            ColourPrettyPrinter printer(env, nullptr, indent);
            cout << fuc(fs_metadata(), fv<'h'>(k.human_name()), fv<'i'>(std::string(indent, ' ')), fv<'s'>(k.pretty_print_value(printer, { })));
        }

        void visit(const MetadataCollectionKey<Map<std::string, std::string> > & k)
        {
            ColourPrettyPrinter printer(env, nullptr, indent);
            cout << fuc(fs_metadata(), fv<'h'>(k.human_name()), fv<'i'>(std::string(indent, ' ')), fv<'s'>(k.pretty_print_value(printer, { })));
        }

        void visit(const MetadataCollectionKey<Sequence<std::string> > & k)
        {
            ColourPrettyPrinter printer(env, nullptr, indent);
            cout << fuc(fs_metadata(), fv<'h'>(k.human_name()), fv<'i'>(std::string(indent, ' ')), fv<'s'>(k.pretty_print_value(printer, { })));
        }

        void visit(const MetadataCollectionKey<PackageIDSequence> & k)
        {
            ColourPrettyPrinter printer(env, nullptr, indent);
            cout << fuc(fs_metadata(), fv<'h'>(k.human_name()), fv<'i'>(std::string(indent, ' ')), fv<'s'>(k.pretty_print_value(printer, { })));
        }

        void visit(const MetadataCollectionKey<FSPathSequence> & k)
        {
            auto v(k.parse_value());
            cout << fuc(fs_metadata(), fv<'h'>(k.human_name()), fv<'i'>(std::string(indent, ' ')), fv<'s'>(join(v->begin(), v->end(), "  ")));
        }

        void visit(const MetadataCollectionKey<Maintainers> & k)
        {
            auto v(k.parse_value());
            cout << fuc(fs_metadata(), fv<'h'>(k.human_name()), fv<'i'>(std::string(indent, ' ')), fv<'s'>(join(v->begin(), v->end(), "  ")));
        }

        void visit(const MetadataSpecTreeKey<LicenseSpecTree> & k)
        {
            ColourPrettyPrinter printer(env, nullptr, indent);
            cout << fuc(fs_metadata(), fv<'h'>(k.human_name()), fv<'i'>(std::string(indent, ' ')), fv<'s'>(k.pretty_print_value(printer, { })));
        }

        void visit(const MetadataSpecTreeKey<SimpleURISpecTree> & k)
        {
            ColourPrettyPrinter printer(env, nullptr, indent);
            cout << fuc(fs_metadata(), fv<'h'>(k.human_name()), fv<'i'>(std::string(indent, ' ')), fv<'s'>(k.pretty_print_value(printer, { })));
        }

        void visit(const MetadataSpecTreeKey<PlainTextSpecTree> & k)
        {
            ColourPrettyPrinter printer(env, nullptr, indent);
            cout << fuc(fs_metadata(), fv<'h'>(k.human_name()), fv<'i'>(std::string(indent, ' ')), fv<'s'>(k.pretty_print_value(printer, { })));
        }

        void visit(const MetadataSpecTreeKey<RequiredUseSpecTree> & k)
        {
            ColourPrettyPrinter printer(env, nullptr, indent);
            cout << fuc(fs_metadata(), fv<'h'>(k.human_name()), fv<'i'>(std::string(indent, ' ')), fv<'s'>(k.pretty_print_value(printer, { })));
        }

        void visit(const MetadataSpecTreeKey<FetchableURISpecTree> & k)
        {
            ColourPrettyPrinter printer(env, nullptr, indent);
            cout << fuc(fs_metadata(), fv<'h'>(k.human_name()), fv<'i'>(std::string(indent, ' ')), fv<'s'>(k.pretty_print_value(printer, { })));
        }

        void visit(const MetadataSpecTreeKey<DependencySpecTree> & k)
        {
            ColourPrettyPrinter printer(env, nullptr, indent);
            cout << fuc(fs_metadata(), fv<'h'>(k.human_name()), fv<'i'>(std::string(indent, ' ')), fv<'s'>(k.pretty_print_value(printer, { })));
        }

        void visit(const MetadataValueKey<std::string> & k)
        {
            ColourPrettyPrinter printer(env, nullptr, indent);
            cout << fuc(fs_metadata(), fv<'h'>(k.human_name()), fv<'i'>(std::string(indent, ' ')), fv<'s'>(stringify(k.parse_value())));
        }

        void visit(const MetadataValueKey<Slot> & k)
        {
            ColourPrettyPrinter printer(env, nullptr, indent);
            cout << fuc(fs_metadata(), fv<'h'>(k.human_name()), fv<'i'>(std::string(indent, ' ')), fv<'s'>(stringify(k.parse_value().raw_value())));
        }

        void visit(const MetadataValueKey<long> & k)
        {
            ColourPrettyPrinter printer(env, nullptr, indent);
            cout << fuc(fs_metadata(), fv<'h'>(k.human_name()), fv<'i'>(std::string(indent, ' ')), fv<'s'>(stringify(k.parse_value())));
        }

        void visit(const MetadataValueKey<bool> & k)
        {
            ColourPrettyPrinter printer(env, nullptr, indent);
            cout << fuc(fs_metadata(), fv<'h'>(k.human_name()), fv<'i'>(std::string(indent, ' ')), fv<'s'>(stringify(k.parse_value())));
        }

        void visit(const MetadataValueKey<FSPath> & k)
        {
            ColourPrettyPrinter printer(env, nullptr, indent);
            cout << fuc(fs_metadata(), fv<'h'>(k.human_name()), fv<'i'>(std::string(indent, ' ')), fv<'s'>(stringify(k.parse_value())));
        }

        void visit(const MetadataValueKey<std::shared_ptr<const PackageID> > & k)
        {
            ColourPrettyPrinter printer(env, nullptr, indent);
            cout << fuc(fs_metadata(), fv<'h'>(k.human_name()), fv<'i'>(std::string(indent, ' ')), fv<'s'>(stringify(*k.parse_value())));
        }

        void visit(const MetadataValueKey<std::shared_ptr<const Choices> > &)
        {
        }

        void visit(const MetadataTimeKey & k)
        {
            cout << fuc(fs_metadata(), fv<'h'>(k.human_name()), fv<'i'>(std::string(indent, ' ')), fv<'s'>(pretty_print_time(k.parse_value().seconds())));
        }
    };

    void do_one_repository(
            const InfoCommandLine & cmdline,
            const std::shared_ptr<Environment> & env,
            const std::shared_ptr<const Repository> & repo)
    {
        cout << fuc(fs_repository_heading(), fv<'s'>(stringify(repo->name())));
        std::set<std::shared_ptr<const MetadataKey>, MetadataKeyComparator> keys(repo->begin_metadata(), repo->end_metadata());
        for (const auto & key : keys)
        {
            if (key->type() == mkt_internal)
                continue;

            InfoDisplayer i(env.get(), cmdline, 1);
            key->accept(i);
        }
        cout << endl;
    }

    void do_env(
            const InfoCommandLine & cmdline,
            const std::shared_ptr<Environment> & env)
    {
        cout << fuc(fs_heading(), fv<'s'>("Environment Information"));
        std::set<std::shared_ptr<const MetadataKey>, MetadataKeyComparator> keys(env->begin_metadata(), env->end_metadata());
        for (const auto & key : keys)
        {
            if (key->type() == mkt_internal)
                continue;

            InfoDisplayer i(env.get(), cmdline, 1);
            key->accept(i);
        }
        cout << endl;
    }

    void do_about(
            const InfoCommandLine & cmdline,
            const std::shared_ptr<Environment> & env)
    {
        cout << fuc(fs_heading(), fv<'s'>("Package Manager Information"));
        std::set<std::shared_ptr<const MetadataKey>, MetadataKeyComparator> keys(AboutMetadata::get_instance()->begin_metadata(),
                AboutMetadata::get_instance()->end_metadata());
        for (const auto & key : keys)
        {
            if (key->type() == mkt_internal)
                continue;

            InfoDisplayer i(env.get(), cmdline, 1);
            key->accept(i);
        }
        cout << endl;
    }

    void do_one_id(
            const InfoCommandLine &,
            const std::shared_ptr<Environment> & env,
            const std::shared_ptr<const PackageID> & id)
    {
        if (! id->supports_action(SupportsActionTest<InfoAction>()))
            return;

        cout << fuc(fs_id_heading(), fv<'s'>(stringify(*id)));

        std::shared_ptr<Sequence<std::string> > args(std::make_shared<Sequence<std::string>>());
        args->push_back("info");
        args->push_back("--if-supported");
        args->push_back("--hooks");
        args->push_back(stringify(id->uniquely_identifying_spec()));

        PerformCommand command;
        command.run(env, args);

        cout << endl;
    }

    void do_one_param(
            const InfoCommandLine & cmdline,
            const std::shared_ptr<Environment> & env,
            const std::string & param)
    {
        PackageDepSpec spec(parse_spec_with_nice_error(param, env.get(), { }, filter::All()));

        const std::shared_ptr<const PackageIDSequence> installed_ids((*env)[selection::AllVersionsSorted(generator::Matches(
                        spec, nullptr, { }) | filter::InstalledAtRoot(env->preferred_root_key()->parse_value()))]);
        const std::shared_ptr<const PackageIDSequence> installable_ids((*env)[selection::BestVersionOnly(generator::Matches(
                        spec, nullptr, { }) | filter::SupportsAction<InstallAction>() | filter::NotMasked())]);

        if (installed_ids->empty() && installable_ids->empty())
            nothing_matching_error(env.get(), param, filter::InstalledAtRoot(env->preferred_root_key()->parse_value()));

        for (const auto & id : *installed_ids)
            do_one_id(cmdline, env, id);

        for (const auto & id : *installable_ids)
            do_one_id(cmdline, env, id);
    }
}

int
InfoCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    InfoCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_INFO_OPTIONS", "CAVE_INFO_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    do_about(cmdline, env);
    do_env(cmdline, env);

    for (const auto & repository : env->repositories())
        do_one_repository(cmdline, env, repository);

    if (cmdline.begin_parameters() == cmdline.end_parameters())
    {
        cout << c::bold_red().colour_string() <<
            "No packages were specified on the command line, so detailed information is not" << c::normal().colour_string() << endl;
        cout << c::bold_red().colour_string() <<
            "available. If you are using this information for a bug report, you should pass " << c::normal().colour_string() << endl;
        cout << c::bold_red().colour_string() <<
            "the relevant package names as parameters." << c::normal().colour_string() << endl;
        cout << endl;
    }
    else
    {
        for (const auto & parameter : cmdline.parameters())
            do_one_param(cmdline, env, parameter);
    }

    return EXIT_SUCCESS;
}

std::shared_ptr<args::ArgsHandler>
InfoCommand::make_doc_cmdline()
{
    return std::make_shared<InfoCommandLine>();
}

CommandImportance
InfoCommand::importance() const
{
    return ci_core;
}

