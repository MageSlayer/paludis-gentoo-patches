/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Stephan Friedrichs
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

#include "cmd_print_unused_distfiles.hh"
#include "command_command_line.hh"

#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/environment.hh>
#include <paludis/filter.hh>
#include <paludis/filter_handler.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/generator.hh>
#include <paludis/mask.hh>
#include <paludis/metadata_key.hh>
#include <paludis/name.hh>
#include <paludis/package_id.hh>
#include <paludis/repository.hh>
#include <paludis/selection.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/util/fs_iterator.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/map.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/wrapped_forward_iterator.hh>

#include <iostream>
#include <algorithm>
#include <set>
#include <list>


using namespace paludis;
using namespace cave;
using std::cout;
using std::endl;

namespace
{
    struct PrintUnusedDistfilesCommandLine :
        CaveCommandCommandLine
    {
        std::string app_name() const override
        {
            return "cave print-unused-distfiles";
        }

        std::string app_synopsis() const override
        {
            return "Prints unused distfiles.";
        }

        std::string app_description() const override
        {
            return "Prints all distfiles not used by any installed package. No "
                "formatting is used, making the output suitable for parsing by scripts.";
        }

        args::ArgsGroup g_repository_options;
        args::StringSetArg a_include;

        PrintUnusedDistfilesCommandLine() :
            g_repository_options(main_options_section(), "Repository Options", "Alter how repositories are handled."),
            a_include(&g_repository_options, "include", 'i', "Treat all distfiles from IDs in the specified repository "
                    "as used. May be specified multiple times. Typically this is used for binary repositories, to avoid "
                    "treating non-installed binary distfiles as unused.")
        {
            add_usage_line("[ --include mybinrepo ... ]");
        }
    };

    /* This visitor finds all distfiles used by installed packages. */
    class DistfilesFilter
    {
        private:
            const Environment * const env;
            const std::shared_ptr<const PackageID> id;
            std::set<std::string> & files;

        public:
            DistfilesFilter(const Environment * const e,
                    const std::shared_ptr<const PackageID> & i,
                    std::set<std::string> & f) :
                env(e),
                id(i),
                files(f)
            {
            }

            void visit(const FetchableURISpecTree::NodeType<AllDepSpec>::Type & node)
            {
                std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
            }

            void visit(const FetchableURISpecTree::NodeType<ConditionalDepSpec>::Type & node)
            {
                // recurse iff the conditions (e.g. useflags) are met
                if (node.spec()->condition_met(env, id))
                    std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
            }

            void visit(const FetchableURISpecTree::NodeType<FetchableURIDepSpec>::Type & node)
            {
                // found used distfile
                files.insert(node.spec()->filename());
            }

            void visit(const FetchableURISpecTree::NodeType<URILabelsDepSpec>::Type &)
            {
            }
    };
}

int
PrintUnusedDistfilesCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    //
    // Handle parameteres
    //

    PrintUnusedDistfilesCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_PRINT_UNUSED_DISTFILES_OPTIONS", "CAVE_PRINT_UNUSED_DISTFILES_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (cmdline.begin_parameters() != cmdline.end_parameters())
        throw args::DoHelp("print-unused-distfiles takes no parameters");

    //
    // Find all distfiles needed by installed packages
    //

    std::set<std::string> used_distfiles;

    std::list<Selection> selections;
    selections.push_back(selection::AllVersionsUnsorted(generator::All() | filter::InstalledAtRoot(env->preferred_root_key()->parse_value())));

    for (const auto & repository_name : cmdline.a_include.args())
        selections.push_back(selection::AllVersionsUnsorted(generator::InRepository(RepositoryName(repository_name))));

    std::set<std::shared_ptr<const PackageID>, PackageIDComparator> already_done(PackageIDComparator(env.get()));
    for (const auto & selection : selections)
    {
        auto ids((*env)[selection]);

        for (const auto & iter : *ids)
        {
            if (! already_done.insert(iter).second)
                continue;

            if (iter->fetches_key())
            {
                DistfilesFilter filter(env.get(), iter, used_distfiles);
                iter->fetches_key()->parse_value()->top()->accept(filter);
            }
        }
    }

    //
    // Find all distdirs
    //

    std::set<FSPath, FSPathComparator> distdirs;

    for (const auto & repository : env->repositories())
    {
        auto distdir_metadata(repository->find_metadata("distdir"));
        if (distdir_metadata != repository->end_metadata())
        {
            auto path_key(visitor_cast<const MetadataValueKey<FSPath>>(**distdir_metadata));
            if (path_key)
                distdirs.insert(path_key->parse_value().realpath());
        }
    }

    //
    // Iterate through the distdirs and compare their contents with the used distfiles
    //

    for (const auto & distdir : distdirs)
    {
        for (FSIterator file(distdir, {fsio_include_dotfiles, fsio_want_regular_files}), f_end ;
            file != f_end ; ++file)
        {
            if (used_distfiles.find(file->basename()) == used_distfiles.end())
                cout << *file << endl;
        }
    }

    return EXIT_SUCCESS;
}

std::shared_ptr<args::ArgsHandler>
PrintUnusedDistfilesCommand::make_doc_cmdline()
{
    return std::make_shared<PrintUnusedDistfilesCommandLine>();
}

CommandImportance
PrintUnusedDistfilesCommand::importance() const
{
    return ci_scripting;
}

