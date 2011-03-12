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

#include "cmd_verify.hh"
#include "format_user_config.hh"
#include "colours.hh"
#include "exceptions.hh"
#include "parse_spec_with_nice_error.hh"

#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/md5.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/repository.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>
#include <paludis/selection.hh>
#include <paludis/package_id.hh>
#include <paludis/action.hh>
#include <paludis/hook.hh>
#include <paludis/metadata_key.hh>
#include <paludis/output_manager_from_environment.hh>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <set>

#include "command_command_line.hh"

using namespace paludis;
using namespace cave;
using std::cout;
using std::endl;

namespace
{
#include "cmd_verify-fmt.hh"

    struct VerifyCommandLine :
        CaveCommandCommandLine
    {
        virtual std::string app_name() const
        {
            return "cave verify";
        }

        virtual std::string app_synopsis() const
        {
            return "Verify that an installed package's files haven't changed.";
        }

        virtual std::string app_description() const
        {
            return "Verify than an installed package's files haven't changed. Note that there are "
                "legitimate reasons for some files (such as configuration files) to have been "
                "changed after a merge, and that some packages may install files that are not "
                "directly tracked by the package manager.";
        }

        VerifyCommandLine()
        {
            add_usage_line("spec");
        }
    };

    struct Verifier
    {
        const std::shared_ptr<const PackageID> id;

        int exit_status;
        bool done_heading;

        Verifier(const std::shared_ptr<const PackageID> & i) :
            id(i),
            exit_status(0),
            done_heading(false)
        {
        }

        void message(const FSPath & path, const std::string & text)
        {
            if (! done_heading)
            {
                done_heading = true;
                cout << fuc(fs_package(), fv<'s'>(stringify(*id)));
            }

            exit_status |= 1;
            cout << fuc(fs_error(), fv<'t'>(text), fv<'p'>(stringify(path)));
        }

        bool check_mtime(const ContentsEntry & e, const FSPath & p, const FSStat & f)
        {
            ContentsEntry::MetadataConstIterator k(e.find_metadata("mtime"));
            if (e.end_metadata() != k)
            {
                const MetadataTimeKey * kk(visitor_cast<const MetadataTimeKey>(**k));
                if (kk && (kk->value().seconds() != f.mtim().seconds()))
                {
                    message(p, "Modification time changed");
                    return false;
                }
            }

            return true;
        }

        bool check_md5(const ContentsEntry & e, const FSPath & f)
        {
            ContentsEntry::MetadataConstIterator k(e.find_metadata("md5"));
            if (e.end_metadata() != k)
            {
                const MetadataValueKey<std::string> * kk(visitor_cast<const MetadataValueKey<std::string> >(**k));
                if (kk)
                {
                    SafeIFStream s(f);
                    MD5 md5(s);
                    if (kk->value() != md5.hexsum())
                    {
                        message(f, "Contents (md5) changed");
                        return false;
                    }
                }
            }

            return true;
        }

        void visit(const ContentsFileEntry & e)
        {
            FSPath f(e.location_key()->value());
            FSStat f_stat(f);
            if (! f_stat.exists())
                message(f, "Does not exist");
            else if (! f_stat.is_regular_file())
                message(f, "Not a regular file");
            else
                check_mtime(e, f, f_stat) && check_md5(e, f);
        }

        void visit(const ContentsSymEntry & e)
        {
            FSPath f(e.location_key()->value());
            FSStat f_stat(f);
            if (! f_stat.exists())
                message(f, "Does not exist");
            else if (! f_stat.is_symlink())
                message(f, "Not a symbolic link");
            else
                check_mtime(e, f, f_stat);
        }

        void visit(const ContentsDirEntry & e)
        {
            FSPath f(e.location_key()->value());
            FSStat f_stat(f);
            if (! f_stat.exists())
                message(f, "Does not exist");
            else if (! f_stat.is_directory())
                message(f, "Not a directory");
        }

        void visit(const ContentsOtherEntry &)
        {
        }
    };
}

int
VerifyCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    VerifyCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_VERIFY_OPTIONS", "CAVE_VERIFY_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (1 != std::distance(cmdline.begin_parameters(), cmdline.end_parameters()))
        throw args::DoHelp("verify takes exactly one parameter");

    PackageDepSpec spec(parse_spec_with_nice_error(*cmdline.begin_parameters(), env.get(),
                { updso_allow_wildcards }, filter::InstalledAtRoot(env->preferred_root_key()->value())));

    std::shared_ptr<const PackageIDSequence> entries(
            (*env)[selection::AllVersionsSorted(generator::Matches(spec, make_null_shared_ptr(), { }) |
                filter::InstalledAtRoot(env->preferred_root_key()->value()))]);

    if (entries->empty())
        nothing_matching_error(env.get(), *cmdline.begin_parameters(), filter::InstalledAtRoot(env->preferred_root_key()->value()));

    int exit_status(0);
    for (PackageIDSequence::ConstIterator i(entries->begin()), i_end(entries->end()) ;
            i != i_end ; ++i)
    {
        if (! (*i)->contents_key())
            continue;

        Verifier v(*i);
        std::for_each(indirect_iterator((*i)->contents_key()->value()->begin()),
                indirect_iterator((*i)->contents_key()->value()->end()),
                accept_visitor(v));
        exit_status |= v.exit_status;
    }

    return exit_status;
}

std::shared_ptr<args::ArgsHandler>
VerifyCommand::make_doc_cmdline()
{
    return std::make_shared<VerifyCommandLine>();
}

CommandImportance
VerifyCommand::importance() const
{
    return ci_supplemental;
}

