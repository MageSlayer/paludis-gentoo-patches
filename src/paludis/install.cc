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
#include <set>

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <paludis/tasks/install_task.hh>
#include <paludis/util/log.hh>
#include <paludis/util/tokeniser.hh>

/** \file
 * Handle the --install action for the main paludis program.
 */

using namespace paludis;

using std::cerr;
using std::cout;
using std::endl;

namespace
{
    struct TagDisplayer :
        DepTagVisitorTypes::ConstVisitor
    {
        void visit(const GLSADepTag * const tag)
        {
            cout << "* " << colour(cl_tag, tag->short_text()) << ": "
                << tag->glsa_title() << endl;
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

            std::set<DepTag::ConstPointer, DepTag::Comparator> _all_tags;

        public:
            OurInstallTask() :
                InstallTask(DefaultEnvironment::get_instance()),
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
                        stringify(d.name) + "-" + stringify(d.version) +
                        "::" + stringify(d.repository)) << endl << endl;
            }

            virtual void on_build_cleanlist_post(const DepListEntry &)
            {
            }

            virtual void on_clean_all_pre(const DepListEntry &,
                    const PackageDatabaseEntryCollection & c)
            {
                if (c.empty())
                    cout << "* No cleaning required" << endl;
                else
                {
                    for (PackageDatabaseEntryCollection::Iterator cc(c.begin()),
                            cc_end(c.end()) ; cc != cc_end ; ++cc)
                        cout << "* " << colour(cl_package_name, *cc) << endl;
                }
                cout << endl;
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
                        stringify(d.name) + "-" + stringify(d.version) +
                        "::" + stringify(d.repository)) << endl << endl;

                cerr << xterm_title("(" + stringify(++_current_count) + " of " +
                        stringify(_max_count + _max_virtual_count) + ") Fetching " +
                        stringify(d.name) + "-" + stringify(d.version) +
                        "::" + stringify(d.repository));
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
                        stringify(d.name) + "-" + stringify(d.version) +
                        "::" + stringify(d.repository)) << endl << endl;

                cerr << xterm_title("(" + stringify(++_current_count) + " of " +
                        stringify(_max_count + _max_virtual_count) + ") Installing " +
                        stringify(d.name) + "-" + stringify(d.version) +
                        "::" + stringify(d.repository));
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
            std::transform(
                    indirect_iterator<const DepTag>(_all_tags.begin()),
                    indirect_iterator<const DepTag>(_all_tags.end()),
                    std::inserter(tag_categories, tag_categories.begin()),
                    std::mem_fun_ref(&DepTag::category));

            for (std::set<std::string>::iterator cat(tag_categories.begin()),
                    cat_end(tag_categories.end()) ; cat != cat_end ; ++cat)
            {
                DepTagCategory::ConstPointer c(DepTagCategoryMaker::get_instance()->
                        find_maker(*cat)());

                if (! c->title().empty())
                    cout << colour(cl_heading, c->title()) << ":" << endl << endl;
                if (! c->pre_text().empty())
                    cout << c->pre_text() << endl << endl;

                for (std::set<DepTag::ConstPointer, DepTag::Comparator>::const_iterator
                        t(_all_tags.begin()), t_end(_all_tags.end()) ;
                        t != t_end ; ++t)
                {
                    if ((*t)->category() != *cat)
                        continue;
                    (*t)->accept(&tag_displayer);
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
        Context context("When displaying entry '" + stringify(d) + "':");

        cout << "* " << colour(cl_package_name, d.name);

        /* display version, unless it's 0 and our category is "virtual" */
        if ((VersionSpec("0") != d.version) ||
                CategoryNamePart("virtual") != d.name.category)
            cout << "-" << d.version;

        /* display repository, unless it's our main repository */
        if (DefaultEnvironment::get_instance()->package_database()->favourite_repository() != d.repository)
            cout << "::" << d.repository;

        /* display slot name, unless it's 0 */
        if (SlotName("0") != d.metadata->slot)
            cout << colour(cl_slot, " {:" + stringify(
                        d.metadata->slot) + "}");

        /* indicate [U], [S] or [N]. display existing version, if we're
         * already installed */
        PackageDatabaseEntryCollection::Pointer existing(DefaultEnvironment::get_instance()->package_database()->
                query(PackageDepAtom::Pointer(new PackageDepAtom(stringify(
                                d.name))), is_installed_only));

        if (existing->empty())
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
                        new PackageDepAtom(stringify(d.name) + ":" +
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
            else if (existing->last()->version < d.version)
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
            else if (existing->last()->version > d.version)
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
        PackageDatabaseEntry p(PackageDatabaseEntry(d.name,
                    d.version, d.repository));

        /* display USE flags */
        std::cout << make_pretty_use_flags_string(DefaultEnvironment::get_instance(), p, d.metadata);

        /* display tag, add tag to our post display list */
        if (! d.tag->empty())
        {
            std::string tag_titles;
            for (SortedCollection<DepTag::ConstPointer, DepTag::Comparator>::Iterator
                    tag(d.tag->begin()),
                    tag_end(d.tag->end()) ;
                    tag != tag_end ; ++tag)
            {
                _all_tags.insert(*tag);
                tag_titles.append((*tag)->short_text());
                tag_titles.append(",");
            }
            tag_titles.erase(tag_titles.length() - 1);
            cout << " " << colour(cl_tag, "<" + tag_titles + ">");
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
            cerr << "Resume command: " << DefaultEnvironment::get_instance()->paludis_command() <<
                " --install --preserve-world --dl-drop-all --dl-no-recursive-deps";
            for (DepList::Iterator i(task.current_dep_list_entry()), i_end(task.dep_list().end()) ;
                    i != i_end ; ++i)
                cerr << " =" << i->name << "-" << i->version << "::" << i->repository;
            cerr << endl;
        }
    }

    class InstallKilledCatcher
    {
        private:
            static const InstallTask * _task;

            static void _signal_handler(int sig) PALUDIS_ATTRIBUTE((noreturn));

            sighandler_t _old;

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
do_install()
{
    int return_code(0);

    Context context("When performing install action from command line:");

    OurInstallTask task;

    task.set_drop_self_circular(CommandLine::get_instance()->a_dl_drop_self_circular.specified());
    task.set_drop_circular(CommandLine::get_instance()->a_dl_drop_circular.specified());
    task.set_drop_all(CommandLine::get_instance()->a_dl_drop_all.specified());
    task.set_ignore_installed(CommandLine::get_instance()->a_dl_ignore_installed.specified());
    task.set_recursive_deps(! CommandLine::get_instance()->a_dl_no_recursive_deps.specified());
    task.set_max_stack_depth(CommandLine::get_instance()->a_dl_max_stack_depth.argument());
    task.set_no_unnecessary_upgrades(CommandLine::get_instance()->a_dl_no_unnecessary_upgrades.specified());

    if (CommandLine::get_instance()->a_dl_rdepend_post.argument() == "always")
        task.set_rdepend_post(dlro_always);
    else if (CommandLine::get_instance()->a_dl_rdepend_post.argument() == "never")
        task.set_rdepend_post(dlro_never);
    else
        task.set_rdepend_post(dlro_as_needed);

    task.set_no_config_protect(CommandLine::get_instance()->a_no_config_protection.specified());
    task.set_fetch_only(CommandLine::get_instance()->a_fetch.specified());
    task.set_pretend(CommandLine::get_instance()->a_pretend.specified());
    task.set_preserve_world(CommandLine::get_instance()->a_preserve_world.specified());

    InstallKilledCatcher install_killed_catcher(task);

    try
    {
        for (CommandLine::ParametersIterator q(CommandLine::get_instance()->begin_parameters()),
                q_end(CommandLine::get_instance()->end_parameters()) ; q != q_end ; ++q)
            task.add_target(*q);

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
                                VersionMetadata::ConstPointer m(DefaultEnvironment::get_instance()->
                                        package_database()->fetch_repository(
                                            pp->repository)->version_metadata(
                                            pp->name, pp->version));
                                if (m->get_ebuild_interface())
                                {
                                    std::set<KeywordName> keywords;
                                    WhitespaceTokeniser::get_instance()->tokenise(
                                            m->get_ebuild_interface()->keywords,
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
    catch (const DepListStackTooDeepError & e)
    {
        cout << endl;
        cerr << "DepList stack too deep error:" << endl;
        cerr << "  * " << e.backtrace("\n  * ") << e.message() << endl;
        cerr << endl;
        cerr << "Try '--dl-max-stack-depth " << std::max(
                CommandLine::get_instance()->a_dl_max_stack_depth.argument() * 2, 100)
            << "'." << endl << endl;

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

