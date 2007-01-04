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

#include <src/output/colour.hh>
#include "install.hh"
#include <src/output/licence.hh>
#include <src/output/console_install_task.hh>
#include <src/common_args/do_help.hh>

#include <iostream>
#include <limits>
#include <set>
#include <cstdlib>
#include <cstring>

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include <paludis/tasks/install_task.hh>
#include <paludis/util/fd_output_stream.hh>
#include <paludis/util/log.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/system.hh>
#include <paludis/environment/default/default_environment.hh>
#include <paludis/environment/default/default_config.hh>

/** \file
 * Handle the --install action for the main paludis program.
 */

using namespace paludis;

using std::cerr;
using std::cout;
using std::endl;

namespace
{
    std::string make_resume_command(const InstallTask & task, bool skip_first)
    {
        std::string resume_command = DefaultEnvironment::get_instance()->paludis_command()
            + " --" + CommandLine::get_instance()->dl_deps_default.long_name() + " discard --"
            + CommandLine::get_instance()->a_install.long_name();

        for (DepList::Iterator i(skip_first ?
                    (task.current_dep_list_entry() == task.dep_list().end() ?
                     task.current_dep_list_entry() :
                     next(task.current_dep_list_entry())) :
                    task.current_dep_list_entry()), i_end(task.dep_list().end()) ;
                i != i_end ; ++i)
            if (! i->skip_install)
                resume_command = resume_command + " '="
                    + stringify(i->package.name) + "-"
                    + stringify(i->package.version) + "::"
                    + stringify(i->package.repository) + "'";

        if (CommandLine::get_instance()->a_add_to_world_atom.specified())
            resume_command = resume_command + " --" + CommandLine::get_instance()->a_add_to_world_atom.long_name()
                + " '" + CommandLine::get_instance()->a_add_to_world_atom.argument() + "'";
        else if (task.had_set_targets())
            resume_command = resume_command + " --" + CommandLine::get_instance()->a_add_to_world_atom.long_name()
                + " '( )'";
        else
            resume_command = resume_command + " --" + CommandLine::get_instance()->a_add_to_world_atom.long_name()
                + " '( " + join(task.begin_targets(), task.end_targets(), " ") + " )'";

        return resume_command;
    }

    void show_resume_command(const InstallTask & task)
    {
        if (CommandLine::get_instance()->a_fetch.specified() ||
                CommandLine::get_instance()->a_pretend.specified())
            return;

        if (task.current_dep_list_entry() != task.dep_list().end())
        {
            std::string resume_command(make_resume_command(task, false));

            if (CommandLine::get_instance()->a_resume_command_template.specified())
            {
                std::string file_name(CommandLine::get_instance()->a_resume_command_template.argument());
                char* resume_template = strdup(file_name.c_str());
                int fd(mkstemp(resume_template));
                if (-1 != fd)
                {
                    ::fchmod(fd, 0644);
                    FDOutputStream resume_command_file(fd);
                    resume_command_file << resume_command << endl;

                    if (resume_command_file)
                    {
                        cerr << endl;
                        cerr << "Resume command saved to file: " << resume_template;
                        cerr << endl;
                    }
                    else
                    {
                        cerr << "Resume command NOT saved to file: " << resume_template << " due to error "
                            << strerror(errno) << endl;
                        cerr << "Resume command: " << resume_command << endl;
                    }
                }
                else
                {
                    cerr << "Resume command NOT saved to file: " << resume_template << " due to error "
                        << strerror(errno) << endl;
                    cerr << "Resume command: " << resume_command << endl;
                }
                std::free(resume_template);
            }
            else
            {
                cerr << endl;
                cerr << "Resume command: " << resume_command << endl;
            }
        }
    }

    class OurInstallTask :
        public ConsoleInstallTask
    {
        public:
            OurInstallTask(const DepListOptions & options) :
                ConsoleInstallTask(DefaultEnvironment::get_instance(), options)
            {
            }

            virtual bool want_full_install_reasons() const
            {
                return "full" == CommandLine::get_instance()->a_show_reasons.argument();
            }

            virtual bool want_tags_summary() const
            {
                return CommandLine::get_instance()->a_pretend.specified();
            }

            virtual bool want_install_reasons() const
            {
                if (! CommandLine::get_instance()->a_pretend.specified())
                    return false;

                return "full" == CommandLine::get_instance()->a_show_reasons.argument() ||
                    "summary" == CommandLine::get_instance()->a_show_reasons.argument();
            }

            virtual bool want_unchanged_use_flags() const
            {
                return "none" != CommandLine::get_instance()->a_show_use_descriptions.argument() &&
                    "new" != CommandLine::get_instance()->a_show_use_descriptions.argument() &&
                    "changed" != CommandLine::get_instance()->a_show_use_descriptions.argument();
            }

            virtual bool want_changed_use_flags() const
            {
                return "none" != CommandLine::get_instance()->a_show_use_descriptions.argument() &&
                    "new" != CommandLine::get_instance()->a_show_use_descriptions.argument();
            }

            virtual bool want_new_use_flags() const
            {
                return "none" != CommandLine::get_instance()->a_show_use_descriptions.argument();
            }

            virtual bool want_use_summary() const
            {
                return "none" != CommandLine::get_instance()->a_show_use_descriptions.argument();
            }

            virtual void on_installed_paludis()
            {
                std::string r(DefaultConfig::get_instance()->root());
                std::string exec_mode(getenv_with_default("PALUDIS_EXEC_PALUDIS", ""));

                if ("always" != exec_mode)
                {
                    if ("never" == exec_mode)
                        return;
                    else if (! (r.empty() || r == "/"))
                        return;
                }

                std::string resume_command(make_resume_command(*this, true));

                output_heading("Paludis has just upgraded Paludis");
                output_starred_item("Using '" + resume_command + "' to start a new Paludis instance...");
                output_endl();

                execl("/bin/sh", "sh", "-c", resume_command.c_str(), static_cast<const char *>(0));
            }
    };

    class InstallKilledCatcher
    {
        private:
            static const InstallTask * _task;

            static void _signal_handler(int sig) PALUDIS_ATTRIBUTE((noreturn));

            sig_t _old;

        public:
            InstallKilledCatcher(const InstallTask & task) :
                _old(signal(SIGINT, &InstallKilledCatcher::_signal_handler))
            {
                _task = &task;
            }

            ~InstallKilledCatcher()
            {
                signal(SIGINT, _old);
                _task = 0;
            }
    };

    const InstallTask * InstallKilledCatcher::_task(0);

    void
    InstallKilledCatcher::_signal_handler(int sig)
    {
        cout << endl;
        cerr << "Caught signal " << sig << endl;
        cerr << "Waiting for children..." << endl;
        while (-1 != wait(0))
            ;
        cerr << endl;
        if (_task)
            show_resume_command(*_task);
        cerr << endl;
        cerr << "Exiting with failure" << endl;
        exit(EXIT_FAILURE);
    }

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

int
do_install()
{
    int return_code(0);

    Context context("When performing install action from command line:");

    DepListOptions options;

    if (CommandLine::get_instance()->dl_reinstall.specified())
    {
        if (CommandLine::get_instance()->dl_reinstall.argument() == "never")
            options.reinstall = dl_reinstall_never;
        else if (CommandLine::get_instance()->dl_reinstall.argument() == "always")
            options.reinstall = dl_reinstall_always;
        else if (CommandLine::get_instance()->dl_reinstall.argument() == "if-use-changed")
            options.reinstall = dl_reinstall_if_use_changed;
        else
            throw args::DoHelp("bad value for --dl-reinstall");
    }

    if (CommandLine::get_instance()->dl_reinstall_scm.specified())
    {
        if (CommandLine::get_instance()->dl_reinstall_scm.argument() == "never")
            options.reinstall_scm = dl_reinstall_scm_never;
        else if (CommandLine::get_instance()->dl_reinstall_scm.argument() == "always")
            options.reinstall_scm = dl_reinstall_scm_always;
        else if (CommandLine::get_instance()->dl_reinstall_scm.argument() == "daily")
            options.reinstall_scm = dl_reinstall_scm_daily;
        else if (CommandLine::get_instance()->dl_reinstall_scm.argument() == "weekly")
            options.reinstall_scm = dl_reinstall_scm_weekly;
        else
            throw args::DoHelp("bad value for --dl-reinstall-scm");
    }

    if (CommandLine::get_instance()->dl_upgrade.specified())
    {
        if (CommandLine::get_instance()->dl_upgrade.argument() == "as-needed")
            options.upgrade = dl_upgrade_as_needed;
        else if (CommandLine::get_instance()->dl_upgrade.argument() == "always")
            options.upgrade = dl_upgrade_always;
        else
            throw args::DoHelp("bad value for --dl-upgrade");
    }

    if (CommandLine::get_instance()->dl_new_slots.specified())
    {
        if (CommandLine::get_instance()->dl_new_slots.argument() == "as-needed")
            options.new_slots = dl_new_slots_as_needed;
        else if (CommandLine::get_instance()->dl_new_slots.argument() == "always")
            options.new_slots = dl_new_slots_always;
        else
            throw args::DoHelp("bad value for --dl-new-slots");
    }

    if (CommandLine::get_instance()->dl_circular.specified())
    {
        if (CommandLine::get_instance()->dl_circular.argument() == "discard")
            options.circular = dl_circular_discard;
        else if (CommandLine::get_instance()->dl_circular.argument() == "error")
            options.circular = dl_circular_error;
        else
            throw args::DoHelp("bad value for --dl-circular");
    }

    if (CommandLine::get_instance()->dl_fall_back.specified())
    {
        if (CommandLine::get_instance()->dl_fall_back.argument() == "as-needed-except-targets")
            options.fall_back = dl_fall_back_as_needed_except_targets;
        else if (CommandLine::get_instance()->dl_fall_back.argument() == "as-needed")
            options.fall_back = dl_fall_back_as_needed;
        else if (CommandLine::get_instance()->dl_fall_back.argument() == "never")
            options.fall_back = dl_fall_back_never;
        else
            throw args::DoHelp("bad value for --dl-fall-back");
    }

    if (CommandLine::get_instance()->dl_deps_default.specified())
    {
        DepListDepsOption x(CommandLine::get_instance()->dl_deps_default.option());
        options.installed_deps_pre = x;
        options.installed_deps_post = x;
        options.installed_deps_runtime = x;
        options.uninstalled_deps_pre = x;
        options.uninstalled_deps_post = x;
        options.uninstalled_deps_runtime = x;
    }

    if (CommandLine::get_instance()->dl_installed_deps_pre.specified())
        options.installed_deps_pre = enum_arg_to_dep_list_deps_option(
                CommandLine::get_instance()->dl_installed_deps_pre);
    if (CommandLine::get_instance()->dl_installed_deps_runtime.specified())
        options.installed_deps_runtime = enum_arg_to_dep_list_deps_option(
                CommandLine::get_instance()->dl_installed_deps_runtime);
    if (CommandLine::get_instance()->dl_installed_deps_post.specified())
        options.installed_deps_post = enum_arg_to_dep_list_deps_option(
                CommandLine::get_instance()->dl_installed_deps_post);

    if (CommandLine::get_instance()->dl_uninstalled_deps_pre.specified())
        options.uninstalled_deps_pre = enum_arg_to_dep_list_deps_option(
                CommandLine::get_instance()->dl_uninstalled_deps_pre);
    if (CommandLine::get_instance()->dl_uninstalled_deps_runtime.specified())
        options.uninstalled_deps_runtime = enum_arg_to_dep_list_deps_option(
                CommandLine::get_instance()->dl_uninstalled_deps_runtime);
    if (CommandLine::get_instance()->dl_uninstalled_deps_post.specified())
        options.uninstalled_deps_post = enum_arg_to_dep_list_deps_option(
                CommandLine::get_instance()->dl_uninstalled_deps_post);

    if ((CommandLine::get_instance()->a_show_reasons.argument() == "summary") ||
            (CommandLine::get_instance()->a_show_reasons.argument() == "full"))
        options.dependency_tags = true;

    OurInstallTask task(options);
    task.set_no_config_protect(CommandLine::get_instance()->a_no_config_protection.specified());
    task.set_fetch_only(CommandLine::get_instance()->a_fetch.specified());
    task.set_pretend(CommandLine::get_instance()->a_pretend.specified());
    task.set_preserve_world(CommandLine::get_instance()->a_preserve_world.specified());
    task.set_safe_resume(CommandLine::get_instance()->a_safe_resume.specified());

    if (CommandLine::get_instance()->a_add_to_world_atom.specified())
        task.set_add_to_world_atom(CommandLine::get_instance()->a_add_to_world_atom.argument());

    if (CommandLine::get_instance()->a_debug_build.specified())
        task.set_debug_mode(CommandLine::get_instance()->a_debug_build.option());

    InstallKilledCatcher install_killed_catcher(task);

    try
    {
        cout << "Building target list... " << std::flush;
        for (CommandLine::ParametersIterator q(CommandLine::get_instance()->begin_parameters()),
                q_end(CommandLine::get_instance()->end_parameters()) ; q != q_end ; ++q)
            task.add_target(*q);
        cout << endl;

        task.execute();

        cout << endl;
    }
    catch (const AmbiguousPackageNameError & e)
    {
        cout << endl;
        cerr << "Query error:" << endl;
        cerr << "  * " << e.backtrace("\n  * ");
        cerr << "Ambiguous package name '" << e.name() << "'. Did you mean:" << endl;
        for (AmbiguousPackageNameError::OptionsIterator o(e.begin_options()),
                o_end(e.end_options()) ; o != o_end ; ++o)
            cerr << "    * " << colour(cl_package_name, *o) << endl;
        cerr << endl;
        return 1;
    }
    catch (const PackageInstallActionError & e)
    {
        cout << endl;
        cerr << "Install error:" << endl;
        cerr << "  * " << e.backtrace("\n  * ");
        cerr << e.message() << endl;
        cerr << endl;
        show_resume_command(task);
        cerr << endl;

        return_code |= 1;
    }
    catch (const PackageFetchActionError & e)
    {
        cout << endl;
        cerr << "Fetch error:" << endl;
        cerr << "  * " << e.backtrace("\n  * ");
        cerr << e.message() << endl;
        cerr << endl;
        show_resume_command(task);
        cerr << endl;

        return_code |= 1;
    }
    catch (const NoSuchPackageError & e)
    {
        cout << endl;
        cerr << "Query error:" << endl;
        cerr << "  * " << e.backtrace("\n  * ");
        cerr << "No such package '" << e.name() << "'" << endl;
        return 1;
    }
    catch (const AllMaskedError & e)
    {
        try
        {
            PackageDatabaseEntryCollection::ConstPointer p(
                    DefaultEnvironment::get_instance()->package_database()->query(
                        PackageDepAtom(e.query()), is_installable_only));
            if (p->empty())
            {
                cout << endl;
                cerr << "Query error:" << endl;
                cerr << "  * " << e.backtrace("\n  * ");
                cerr << "No versions of '" << e.query() << "' are available" << endl;
            }
            else
            {
                cout << endl;
                cerr << "Query error:" << endl;
                cerr << "  * " << e.backtrace("\n  * ");
                cerr << "All versions of '" << e.query() << "' are masked. Candidates are:" << endl;
                for (PackageDatabaseEntryCollection::Iterator pp(p->begin()), pp_end(p->end()) ;
                        pp != pp_end ; ++pp)
                {
                    cerr << "    * " << colour(cl_package_name, *pp) << ": Masked by ";

                    bool need_comma(false);
                    MaskReasons m(DefaultEnvironment::get_instance()->mask_reasons(*pp));
                    for (unsigned mm = 0 ; mm < m.size() ; ++mm)
                        if (m[mm])
                        {
                            if (need_comma)
                                cerr << ", ";
                            cerr << MaskReason(mm);

                            if (mr_eapi == mm)
                            {
                                std::string eapi_str(DefaultEnvironment::get_instance()->
                                        package_database()->fetch_repository(
                                            pp->repository)->version_metadata(
                                            pp->name, pp->version)->eapi);

                                cerr << " ( " << colour(cl_masked, eapi_str) << " )";
                            }
                            else if (mr_license == mm)
                            {
                                cerr << " ";

                                LicenceDisplayer ld(cerr, DefaultEnvironment::get_instance(), &*pp);
                                DefaultEnvironment::get_instance()->package_database()->fetch_repository(
                                        pp->repository)->version_metadata(
                                        pp->name, pp->version)->license()->
                                        accept(&ld);
                            }
                            else if (mr_keyword == mm)
                            {
                                VersionMetadata::ConstPointer meta(DefaultEnvironment::get_instance()->
                                        package_database()->fetch_repository(
                                            pp->repository)->version_metadata(
                                            pp->name, pp->version));
                                if (meta->get_ebuild_interface())
                                {
                                    std::set<KeywordName> keywords;
                                    WhitespaceTokeniser::get_instance()->tokenise(
                                            meta->get_ebuild_interface()->keywords,
                                            create_inserter<KeywordName>(
                                                std::inserter(keywords, keywords.end())));

                                    cerr << " ( " << colour(cl_masked, join(keywords.begin(),
                                                    keywords.end(), " ")) << " )";
                                }
                            }

                            need_comma = true;
                        }
                    cerr << endl;
                }
            }
        }
        catch (...)
        {
            throw e;
        }

        return 1;
    }
    catch (const UseRequirementsNotMetError & e)
    {
        cout << endl;
        cerr << "DepList USE requirements not met error:" << endl;
        cerr << "  * " << e.backtrace("\n  * ") << e.message() << endl;
        cerr << endl;
        cerr << "This error usually indicates that one of the packages you are trying to" << endl;
        cerr << "install requires that another package be built with particular USE flags" << endl;
        cerr << "enabled or disabled. You may be able to work around this restriction by" << endl;
        cerr << "adjusting your use.conf." << endl;
        cerr << endl;

        return_code |= 1;
    }
    catch (const DepListError & e)
    {
        cout << endl;
        cerr << "Dependency error:" << endl;
        cerr << "  * " << e.backtrace("\n  * ") << e.message() << " ("
            << e.what() << ")" << endl;
        cerr << endl;

        return_code |= 1;
    }
    catch (const HadBothPackageAndSetTargets &)
    {
        cout << endl;
        cerr << "Error: both package sets and packages were specified." << endl;
        cerr << endl;
        cerr << "Package sets (like 'system' and 'world') cannot be installed at the same time" << endl;
        cerr << "as ordinary packages." << endl;

        return_code |= 1;
    }
    catch (const MultipleSetTargetsSpecified &)
    {
        cout << endl;
        cerr << "Error: multiple package sets were specified." << endl;
        cerr << endl;
        cerr << "Package sets (like 'system' and 'world') must be installed individually," << endl;
        cerr << "without any other sets or packages." << endl;

        return_code |= 1;
    }

    return return_code;
}

