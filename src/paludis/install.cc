/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "colour.hh"
#include "install.hh"
#include "licence.hh"
#include "use.hh"

#include <iostream>
#include <limits>
#include <set>

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <paludis/tasks/install_task.hh>
#include <paludis/util/log.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/environment/default/default_environment.hh>

/** \file
 * Handle the --install action for the main paludis program.
 */

using namespace paludis;

using std::cerr;
using std::cout;
using std::endl;

namespace
{
    struct ShortTagDisplayer :
        DepTagVisitorTypes::ConstVisitor
    {
        std::string text;

        void visit(const GLSADepTag * const tag)
        {
            text = tag->short_text();
        }

        void visit(const DependencyDepTag * const)
        {
        }

        void visit(const GeneralSetDepTag * const tag)
        {
            text = tag->short_text() + "<" + tag->source() + ">";
        }
    };

    struct TagDisplayer :
        DepTagVisitorTypes::ConstVisitor
    {
        void visit(const GLSADepTag * const tag)
        {
            cout << "* " << colour(cl_tag, tag->short_text()) << ": "
                << tag->glsa_title() << endl;
        }

        void visit(const DependencyDepTag * const)
        {
        }

        void visit(const GeneralSetDepTag * const tag)
        {
            cout << "* " << colour(cl_tag, tag->short_text());
            if (tag->short_text() == "world")
                cout << ":      " << "Packages that have been explicitly installed";
            else if (tag->short_text() == "everything")
                cout << ": " << "All installed packages";
            else if (tag->short_text() == "system")
                cout << ":     " << "Packages that are part of the base system";
            cout << endl;
        }
    };

    class OurInstallTask :
        public InstallTask
    {
        private:
            int _current_count, _max_count, _new_count, _upgrade_count,
                _downgrade_count, _new_slot_count, _rebuild_count,
                _current_virtual_count, _max_virtual_count, _new_virtual_count,
                _upgrade_virtual_count, _downgrade_virtual_count,
                _new_slot_virtual_count, _rebuild_virtual_count;

            std::set<DepTagEntry> _all_tags;

        public:
            OurInstallTask(const DepListOptions & options) :
                InstallTask(DefaultEnvironment::get_instance(), options),
                _current_count(0),
                _max_count(0),
                _new_count(0),
                _upgrade_count(0),
                _downgrade_count(0),
                _new_slot_count(0),
                _rebuild_count(0),
                _current_virtual_count(0),
                _max_virtual_count(0),
                _new_virtual_count(0),
                _upgrade_virtual_count(0),
                _downgrade_virtual_count(0),
                _new_slot_virtual_count(0),
                _rebuild_virtual_count(0)
            {
            }

            void on_build_deplist_pre()
            {
                cout << "Building dependency list... " << std::flush;
            }

            virtual void on_build_deplist_post()
            {
                cout << "done" << endl;
            }

            virtual void on_build_cleanlist_pre(const DepListEntry & d)
            {
                cout << endl << colour(cl_heading, "Cleaning stale versions after installing " +
                        stringify(d.package.name) + "-" + stringify(d.package.version) +
                        "::" + stringify(d.package.repository)) << endl << endl;
            }

            virtual void on_build_cleanlist_post(const DepListEntry &)
            {
            }

            virtual void on_clean_all_pre(const DepListEntry &,
                    const PackageDatabaseEntryCollection & c)
            {
                for (PackageDatabaseEntryCollection::Iterator cc(c.begin()),
                        cc_end(c.end()) ; cc != cc_end ; ++cc)
                    cout << "* " << colour(cl_package_name, *cc) << endl;
                cout << endl;
            }

            virtual void on_no_clean_needed(const DepListEntry &)
            {
                cout << "* No cleaning required" << endl;
            }

            virtual void on_clean_pre(const DepListEntry &,
                    const PackageDatabaseEntry & c)
            {
                cout << colour(cl_heading, "Cleaning " + stringify(c)) << endl << endl;

                cerr << xterm_title("(" + stringify(_current_count) + " of " +
                        stringify(_max_count + _max_virtual_count) + ") Cleaning " + stringify(c));
            }

            virtual void on_clean_post(const DepListEntry &,
                    const PackageDatabaseEntry &)
            {
            }

            virtual void on_clean_all_post(const DepListEntry &,
                    const PackageDatabaseEntryCollection &)
            {
            }

            virtual void on_display_merge_list_pre()
            {
                cout << endl << colour(cl_heading, "These packages will be installed:")
                    << endl << endl;
            }

            virtual void on_display_merge_list_post();
            virtual void on_display_merge_list_entry(const DepListEntry &);

            virtual void on_fetch_all_pre()
            {
            }

            virtual void on_fetch_pre(const DepListEntry & d)
            {
                cout << colour(cl_heading, "Fetching " +
                        stringify(d.package.name) + "-" + stringify(d.package.version) +
                        "::" + stringify(d.package.repository)) << endl << endl;

                cerr << xterm_title("(" + stringify(++_current_count) + " of " +
                        stringify(_max_count + _max_virtual_count) + ") Fetching " +
                        stringify(d.package.name) + "-" + stringify(d.package.version) +
                        "::" + stringify(d.package.repository));
            }

            virtual void on_fetch_post(const DepListEntry &)
            {
            }

            virtual void on_fetch_all_post()
            {
            }

            virtual void on_install_all_pre()
            {
            }

            virtual void on_install_pre(const DepListEntry & d)
            {
                cout << endl << colour(cl_heading, "Installing " +
                        stringify(d.package.name) + "-" + stringify(d.package.version) +
                        "::" + stringify(d.package.repository)) << endl << endl;

                cerr << xterm_title("(" + stringify(++_current_count) + " of " +
                        stringify(_max_count + _max_virtual_count) + ") Installing " +
                        stringify(d.package.name) + "-" + stringify(d.package.version) +
                        "::" + stringify(d.package.repository));
            }

            virtual void on_install_post(const DepListEntry &)
            {
            }

            virtual void on_install_all_post()
            {
            }

            virtual void on_update_world_pre()
            {
                cout << endl << colour(cl_heading, "Updating world file") << endl << endl;
            }

            virtual void on_update_world(const PackageDepAtom & a)
            {
                cout << "* adding " << colour(cl_package_name, a.package()) << endl;
            }

            virtual void on_update_world_skip(const PackageDepAtom & a, const std::string & s)
            {
                cout << "* skipping " << colour(cl_package_name, a.package()) << " ("
                    << s << ")" << endl;
            }

            virtual void on_update_world_post()
            {
                cout << endl;
            }

            virtual void on_preserve_world()
            {
                cout << endl << colour(cl_heading, "Updating world file") << endl << endl;
                cout << "* --preserve-world was specified, skipping world changes" << endl;
                cout << endl;
            }
    };

    void
    OurInstallTask::on_display_merge_list_post()
    {
        if (_max_count != _new_count + _upgrade_count + _downgrade_count + _new_slot_count +
                _rebuild_count)
            Log::get_instance()->message(ll_warning, lc_no_context,
                    "Max count doesn't add up. This is a bug!");

        if (_max_virtual_count != _new_virtual_count + _upgrade_virtual_count +
                _downgrade_virtual_count + _new_slot_virtual_count + _rebuild_virtual_count)
            Log::get_instance()->message(ll_warning, lc_no_context,
                    "Max virtuals count doesn't add up. This is a bug!");

        cout << endl << "Total: " << _max_count << (_max_count == 1 ? " package" : " packages");
        if (_max_count)
        {
            bool need_comma(false);
            cout << " (";
            if (_new_count)
            {
                cout << _new_count << " new";
                need_comma = true;
            }
            if (_upgrade_count)
            {
                if (need_comma)
                    cout << ", ";
                cout << _upgrade_count << (_upgrade_count == 1 ? " upgrade" : " upgrades");
                need_comma = true;
            }
            if (_downgrade_count)
            {
                if (need_comma)
                    cout << ", ";
                cout << _downgrade_count << (_downgrade_count == 1 ? " downgrade" : " downgrades");
                need_comma = true;
            }
            if (_new_slot_count)
            {
                if (need_comma)
                    cout << ", ";
                cout << _new_slot_count << (_new_slot_count == 1 ? " in new slot" : " in new slots");
                need_comma = true;
            }
            if (_rebuild_count)
            {
                if (need_comma)
                    cout << ", ";
                cout << _rebuild_count << (_rebuild_count == 1 ? " rebuild" : " rebuilds");
                need_comma = true;
            }
            cout << ")";
        }

        if (_max_virtual_count)
        {
            cout << " and " << _max_virtual_count << (_max_virtual_count == 1
                    ? " virtual" : " virtuals");
            bool need_comma(false);
            cout << " (";
            if (_new_virtual_count)
            {
                cout << _new_virtual_count << " new";
                need_comma = true;
            }
            if (_upgrade_virtual_count)
            {
                if (need_comma)
                    cout << ", ";
                cout << _upgrade_virtual_count << (_upgrade_virtual_count == 1 ? " upgrade" : " upgrades");
                need_comma = true;
            }
            if (_downgrade_virtual_count)
            {
                if (need_comma)
                    cout << ", ";
                cout << _downgrade_virtual_count << (_downgrade_virtual_count == 1 ?
                        " downgrade" : " downgrades");
                need_comma = true;
            }
            if (_new_slot_virtual_count)
            {
                if (need_comma)
                    cout << ", ";
                cout << _new_slot_virtual_count << (_new_slot_virtual_count == 1 ?
                        " in new slot" : " in new slots");
                need_comma = true;
            }
            if (_rebuild_virtual_count)
            {
                if (need_comma)
                    cout << ", ";
                cout << _rebuild_virtual_count << (_rebuild_virtual_count == 1 ?
                        " rebuild" : " rebuilds");
                need_comma = true;
            }
            cout << ")";
        }
        cout << endl << endl;

        if (CommandLine::get_instance()->a_pretend.specified() && ! _all_tags.empty())
        {
            TagDisplayer tag_displayer;

            std::set<std::string> tag_categories;
            for (std::set<DepTagEntry>::const_iterator a(_all_tags.begin()),
                    a_end(_all_tags.end()) ; a != a_end ; ++a)
                tag_categories.insert(a->tag->category());

            for (std::set<std::string>::iterator cat(tag_categories.begin()),
                    cat_end(tag_categories.end()) ; cat != cat_end ; ++cat)
            {
                DepTagCategory::ConstPointer c(DepTagCategoryMaker::get_instance()->
                        find_maker(*cat)());

                if (! c->visible())
                    continue;

                if (! c->title().empty())
                    cout << colour(cl_heading, c->title()) << ":" << endl << endl;
                if (! c->pre_text().empty())
                    cout << c->pre_text() << endl << endl;

                for (std::set<DepTagEntry>::const_iterator t(_all_tags.begin()), t_end(_all_tags.end()) ;
                        t != t_end ; ++t)
                {
                    if (t->tag->category() != *cat)
                        continue;
                    t->tag->accept(&tag_displayer);
                }
                cout << endl;

                if (! c->post_text().empty())
                    cout << c->post_text() << endl << endl;
            }
        }
    }

    void
    OurInstallTask::on_display_merge_list_entry(const DepListEntry & d)
    {
        if (d.skip_install && CommandLine::get_instance()->a_show_install_reasons.argument() != "full")
            return;

        Context context("When displaying entry '" + stringify(d.package) + "':");

        cout << "* " << colour(d.skip_install ? cl_unimportant : cl_package_name,
                d.package.name);

        /* display version, unless it's 0 and our category is "virtual" */
        if ((VersionSpec("0") != d.package.version) ||
                CategoryNamePart("virtual") != d.package.name.category)
            cout << "-" << d.package.version;

        /* display repository, unless it's our main repository */
        if (DefaultEnvironment::get_instance()->package_database()->favourite_repository() !=
                d.package.repository)
            cout << "::" << d.package.repository;

        /* display slot name, unless it's 0 */
        if (SlotName("0") != d.metadata->slot)
            cout << colour(d.skip_install ? cl_unimportant : cl_slot,
                    " {:" + stringify(d.metadata->slot) + "}");

        /* indicate [U], [S], [N] or [-]. display existing version, if we're
         * already installed */
        PackageDatabaseEntryCollection::Pointer existing(DefaultEnvironment::get_instance()->package_database()->
                query(PackageDepAtom::Pointer(new PackageDepAtom(stringify(
                                d.package.name))), is_installed_only));

        if (d.skip_install)
            cout << colour(cl_unimportant, " [-]");
        else if (existing->empty())
        {
            cout << colour(cl_updatemode, " [N]");
            if (d.metadata->get_virtual_interface())
            {
                ++_new_virtual_count;
                ++_max_virtual_count;
            }
            else
            {
                ++_new_count;
                ++_max_count;
            }
        }
        else
        {
            existing = DefaultEnvironment::get_instance()->package_database()->query(PackageDepAtom::Pointer(
                        new PackageDepAtom(stringify(d.package.name) + ":" +
                            stringify(d.metadata->slot))),
                    is_installed_only);
            if (existing->empty())
            {
                cout << colour(cl_updatemode, " [S]");
                if (d.metadata->get_virtual_interface())
                {
                    ++_new_slot_virtual_count;
                    ++_max_virtual_count;
                }
                else
                {
                    ++_new_slot_count;
                    ++_max_count;
                }
            }
            else if (existing->last()->version < d.package.version)
            {
                cout << colour(cl_updatemode, " [U " + stringify(
                            existing->last()->version) + "]");
                if (d.metadata->get_virtual_interface())
                {
                    ++_upgrade_virtual_count;
                    ++_max_virtual_count;
                }
                else
                {
                    ++_upgrade_count;
                    ++_max_count;
                }
            }
            else if (existing->last()->version > d.package.version)
            {
                cout << colour(cl_updatemode, " [D " + stringify(
                            existing->last()->version) + "]");
                if (d.metadata->get_virtual_interface())
                {
                    ++_downgrade_virtual_count;
                    ++_max_virtual_count;
                }
                else
                {
                    ++_downgrade_count;
                    ++_max_count;
                }

            }
            else
            {
                cout << colour(cl_updatemode, " [R]");
                if (d.metadata->get_virtual_interface())
                {
                    ++_rebuild_virtual_count;
                    ++_max_virtual_count;
                }
                else
                {
                    ++_rebuild_count;
                    ++_max_count;
                }
            }
        }

        /* fetch db entry */
        PackageDatabaseEntry p(d.package);

        /* display USE flags */
        if (! d.skip_install)
            std::cout << make_pretty_use_flags_string(DefaultEnvironment::get_instance(), p, d.metadata,
                    (existing->empty() ? 0 : &*existing->last()));

        /* display tag, add tag to our post display list */
        if (! d.tags->empty())
        {
            std::string tag_titles;
            for (SortedCollection<DepTagEntry>::Iterator
                    tag(d.tags->begin()),
                    tag_end(d.tags->end()) ;
                    tag != tag_end ; ++tag)
            {
                if (tag->tag->category() == "dependency")
                    continue;

                _all_tags.insert(*tag);
                ShortTagDisplayer t;
                tag->tag->accept(&t);
                tag_titles.append(t.text);
                tag_titles.append(", ");
            }
            if (! tag_titles.empty())
            {
                tag_titles.erase(tag_titles.length() - 2);
                cout << " " << colour(d.skip_install ? cl_unimportant : cl_tag,
                        "<" + tag_titles + ">");
            }

            /* display dependency tags */
            if ((CommandLine::get_instance()->a_show_install_reasons.argument() == "summary") ||
                    (CommandLine::get_instance()->a_show_install_reasons.argument() == "full"))
            {
                std::string deps;
                unsigned count(0), max_count;
                if (CommandLine::get_instance()->a_show_install_reasons.argument() == "summary")
                    max_count = 3;
                else
                    max_count = std::numeric_limits<long>::max();

                for (SortedCollection<DepTagEntry>::Iterator
                        tag(d.tags->begin()),
                        tag_end(d.tags->end()) ;
                        tag != tag_end ; ++tag)
                {
                    if (tag->tag->category() != "dependency")
                        continue;

                    if (++count < max_count)
                    {
                        deps.append(tag->tag->short_text());
                        deps.append(", ");
                    }
                }
                if (! deps.empty())
                {
                    if (count >= max_count)
                        deps.append(stringify(count - max_count + 1) + " more, ");

                    deps.erase(deps.length() - 2);
                    cout << " " << colour(d.skip_install ? cl_unimportant : cl_tag,
                            "<" + deps + ">");
                }
            }
        }

        cout << endl;
    }

    void show_resume_command(const InstallTask & task)
    {
        if (CommandLine::get_instance()->a_fetch.specified() ||
                CommandLine::get_instance()->a_pretend.specified())
            return;

        if (task.current_dep_list_entry() != task.dep_list().end())
        {
            cerr << "Resume command: " << DefaultEnvironment::get_instance()->paludis_command() << " "
                "--dl-installed-deps-pre discard "
                "--dl-installed-deps-runtime discard "
                "--dl-installed-deps-post discard "
                "--dl-uninstalled-deps-pre discard "
                "--dl-uninstalled-deps-runtime discard "
                "--dl-uninstalled-deps-post discard "
                "--install --preserve-world";
            for (DepList::Iterator i(task.current_dep_list_entry()), i_end(task.dep_list().end()) ;
                    i != i_end ; ++i)
                if (! i->skip_install)
                    cerr << " =" << i->package.name << "-" << i->package.version << "::" << i->package.repository;
            cerr << endl;
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
            throw DoHelp("bad value for --" + arg.long_name());
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
            throw DoHelp("bad value for --dl-reinstall");
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
            throw DoHelp("bad value for --dl-reinstall-scm");
    }

    if (CommandLine::get_instance()->dl_upgrade.specified())
    {
        if (CommandLine::get_instance()->dl_upgrade.argument() == "as-needed")
            options.upgrade = dl_upgrade_as_needed;
        else if (CommandLine::get_instance()->dl_upgrade.argument() == "always")
            options.upgrade = dl_upgrade_always;
        else
            throw DoHelp("bad value for --dl-upgrade");
    }

    if (CommandLine::get_instance()->dl_circular.specified())
    {
        if (CommandLine::get_instance()->dl_circular.argument() == "discard")
            options.circular = dl_circular_discard;
        else if (CommandLine::get_instance()->dl_circular.argument() == "error")
            options.circular = dl_circular_error;
        else
            throw DoHelp("bad value for --dl-circular");
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

    if ((CommandLine::get_instance()->a_show_install_reasons.argument() == "summary") ||
            (CommandLine::get_instance()->a_show_install_reasons.argument() == "full"))
        options.dependency_tags = true;

    OurInstallTask task(options);
    task.set_no_config_protect(CommandLine::get_instance()->a_no_config_protection.specified());
    task.set_fetch_only(CommandLine::get_instance()->a_fetch.specified());
    task.set_pretend(CommandLine::get_instance()->a_pretend.specified());
    task.set_preserve_world(CommandLine::get_instance()->a_preserve_world.specified());

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
                        PackageDepAtom::ConstPointer(new PackageDepAtom(e.query())),
                        is_uninstalled_only));
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

