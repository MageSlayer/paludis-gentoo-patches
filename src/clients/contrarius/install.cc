/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Danny van Dyk <kugelfang@gentoo.org>
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

#include "install.hh"
#include <src/output/colour.hh>
#include <src/output/console_install_task.hh>
#include <src/output/licence.hh>

#include <iostream>
#include <limits>
#include <set>

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <paludis/tasks/install_task.hh>
#include <paludis/util/fd_output_stream.hh>
#include <paludis/util/log.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/environment/default/default_environment.hh>
#include <paludis/dep_list/exceptions.hh>
#include <paludis/query.hh>

/** \file
 * Handle the --install action for the contrarius program.
 */

using namespace paludis;

using std::cerr;
using std::cout;
using std::endl;

namespace
{
    class OurInstallTask :
        public ConsoleInstallTask
    {
        public:
            OurInstallTask(const DepListOptions & options) :
                ConsoleInstallTask(DefaultEnvironment::get_instance(), options,
                        DefaultEnvironment::get_instance()->default_destinations())
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
                return "full" == CommandLine::get_instance()->a_show_reasons.argument() ||
                    "summary" == CommandLine::get_instance()->a_show_reasons.argument();
            }

            virtual bool want_unchanged_use_flags() const
            {
                return false;
            }

            virtual bool want_changed_use_flags() const
            {
                return false;
            }

            virtual bool want_new_use_flags() const
            {
                return false;
            }

            virtual bool want_use_summary() const
            {
                return false;
            }
    };

    void show_resume_command(const InstallTask & task)
    {
        if (CommandLine::get_instance()->a_fetch.specified() ||
                CommandLine::get_instance()->a_pretend.specified())
            return;

        if (task.current_dep_list_entry() != task.dep_list().end())
        {
            std::string resume_command = DefaultEnvironment::get_instance()->paludis_command() + " "
                "--dl-installed-deps-pre discard "
                "--dl-installed-deps-runtime discard "
                "--dl-installed-deps-post discard "
                "--dl-uninstalled-deps-pre discard "
                "--dl-uninstalled-deps-runtime discard "
                "--dl-uninstalled-deps-post discard "
                "--install --preserve-world";
            for (DepList::Iterator i(task.current_dep_list_entry()), i_end(task.dep_list().end()) ;
                    i != i_end ; ++i)
                if (dlk_package == i->kind)
                    resume_command = resume_command + " ="
                        + stringify(i->package.name) + "-"
                        + stringify(i->package.version) + "::"
                        + stringify(i->package.repository);

            if (CommandLine::get_instance()->a_resume_command_template.specified())
            {
                std::string file_name(CommandLine::get_instance()->a_resume_command_template.argument());
                char* resume_template = strdup(file_name.c_str());
                FDOutputStream resume_command_file(mkstemp(resume_template));
                cerr << endl;
                cerr << "Resume command saved to file: " << resume_template;
                cerr << endl;
                resume_command_file << resume_command << endl;
                std::free(resume_template);
            }
            else
                cerr << "Resume command: " << resume_command << endl;
        }
    }

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
}

int
do_install(std::tr1::shared_ptr<const PackageDepAtom> atom)
{
    int return_code(0);

    Context context("When performing install action from command line:");

    DepListOptions options;

    options.circular = dl_circular_discard;
    options.circular = dl_circular_error;
    options.installed_deps_pre = dl_deps_discard;
    options.installed_deps_runtime = dl_deps_discard;
    options.installed_deps_post = dl_deps_discard;
    options.uninstalled_deps_pre = dl_deps_discard;
    options.uninstalled_deps_runtime = dl_deps_discard;
    options.uninstalled_deps_post = dl_deps_discard;
    options.reinstall = dl_reinstall_never;
    options.target_type = dl_target_set;

    OurInstallTask task(options);
    task.set_fetch_only(CommandLine::get_instance()->a_fetch.specified());
    task.set_pretend(CommandLine::get_instance()->a_pretend.specified());

    if (CommandLine::get_instance()->a_debug_build.specified())
    {
        if (CommandLine::get_instance()->a_debug_build.argument() == "none")
            task.set_debug_mode(ido_none);
        else if (CommandLine::get_instance()->a_debug_build.argument() == "split")
            task.set_debug_mode(ido_split);
        else if (CommandLine::get_instance()->a_debug_build.argument() == "internal")
            task.set_debug_mode(ido_internal);
        else
            throw DoHelp("bad value for --debug-build");
    }

    try
    {
        task.add_target(stringify(*atom));

        task.execute();

        cout << endl;

        if (task.dep_list().has_errors())
            return_code |= 1;
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

        return_code |= 1;
    }
    catch (const PackageFetchActionError & e)
    {
        cout << endl;
        cerr << "Fetch error:" << endl;
        cerr << "  * " << e.backtrace("\n  * ");
        cerr << e.message() << endl;
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
            std::tr1::shared_ptr<const PackageDatabaseEntryCollection> p(
                    DefaultEnvironment::get_instance()->package_database()->query(
                        query::Matches(PackageDepAtom(e.query())) & query::RepositoryHasUninstallableInterface(),
                        qo_order_by_version));

            if (p->empty())
            {
                cout << endl;
                cerr << "Query error:" << endl;
                cerr << "  * " << e.backtrace("\n  * ");
                cerr << "All versions of '" << e.query() << "' are masked" << endl;
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
                                std::tr1::shared_ptr<const VersionMetadata> meta(DefaultEnvironment::get_instance()->
                                        package_database()->fetch_repository(
                                            pp->repository)->version_metadata(
                                                pp->name, pp->version));

                                if (meta->license_interface)
                                {
                                    cerr << " ";

                                    LicenceDisplayer ld(cerr, DefaultEnvironment::get_instance(), &*pp);
                                    meta->license_interface->license()->accept(&ld);
                                }
                            }
                            else if (mr_keyword == mm)
                            {
                                std::tr1::shared_ptr<const VersionMetadata> meta(DefaultEnvironment::get_instance()->
                                        package_database()->fetch_repository(
                                            pp->repository)->version_metadata(
                                            pp->name, pp->version));
                                if (meta->ebuild_interface)
                                {
                                    std::set<KeywordName> keywords;
                                    WhitespaceTokeniser::get_instance()->tokenise(
                                            meta->ebuild_interface->keywords,
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

    return return_code;
}

