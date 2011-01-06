/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include "console_install_task.hh"
#include "colour.hh"
#include "colour_pretty_printer.hh"
#include "mask_displayer.hh"

#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/join.hh>
#include <paludis/util/log.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/pretty_print.hh>
#include <paludis/util/safe_ofstream.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/system.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/action.hh>
#include <paludis/repository.hh>
#include <paludis/package_database.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/mask.hh>
#include <paludis/hook.hh>
#include <paludis/fuzzy_finder.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/choice.hh>
#include <paludis/output_manager_from_environment.hh>
#include <paludis/output_manager.hh>
#include <paludis/legacy/dep_list.hh>
#include <paludis/notifier_callback.hh>
#include <paludis/partially_made_package_dep_spec.hh>

#include <functional>
#include <algorithm>
#include <set>
#include <list>
#include <iostream>
#include <iomanip>
#include <limits>
#include <cstring>
#include <cstdlib>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <map>

using namespace paludis;
using std::cout;
using std::cerr;
using std::endl;

namespace
{
    std::list<ConsoleInstallTask *> tasks;
    Mutex tasks_mutex;

    void signal_handler(int sig) PALUDIS_ATTRIBUTE((noreturn));

    void signal_handler(int sig)
    {
        cout << endl;
        cerr << "Caught signal " << sig << endl;
        cerr << "Waiting for children..." << endl;
        while (-1 != wait(0))
            ;
        cerr << endl;
        {
            Lock l(tasks_mutex);
            if (! tasks.empty())
                (*tasks.begin())->show_resume_command();
        }
        cerr << endl;
        cerr << "Exiting with failure" << endl;
        _exit(EXIT_FAILURE);
    }
}

ConsoleInstallTask::ConsoleInstallTask(Environment * const env,
        const DepListOptions & options,
        const std::shared_ptr<const DestinationsSet> & d) :
    InstallTask(env, options, d),
    _download_size(0),
    _download_size_overflow(false),
    _all_tags(std::make_shared<Set<DepTagEntry, DepTagEntryComparator>>()),
    _already_downloaded(std::make_shared<Set<FSPath, FSPathComparator>>()),
    _resolution_finished(false)
{
    std::fill_n(_counts, static_cast<int>(last_count), 0);
}

void
ConsoleInstallTask::execute()
{
    struct sigaction act, oldint, oldterm;
    act.sa_handler = &signal_handler;
    act.sa_flags = 0;
    ::sigemptyset(&act.sa_mask);
    ::sigaddset(&act.sa_mask, SIGINT);
    ::sigaddset(&act.sa_mask, SIGTERM);

    {
        Lock l(tasks_mutex);
        tasks.push_front(this);
        ::sigaction(SIGINT,  &act, &oldint);
        ::sigaction(SIGTERM, &act, &oldterm);
    }

    InstallTask::execute();

    {
        Lock l(tasks_mutex);
        ::sigaction(SIGINT,  &oldint,  0);
        ::sigaction(SIGTERM, &oldterm, 0);
        tasks.remove(this);
    }
}

int
ConsoleInstallTask::exit_status() const
{
    int return_code(0);
    if (dep_list().has_errors())
        return_code |= 1;
    if (had_resolution_failures())
        return_code |= 3;
    if (had_action_failures())
        return_code |= 7;
    return return_code;
}

bool
ConsoleInstallTask::try_to_set_targets_from_user_specs(const std::shared_ptr<const Sequence<std::string> > & s)
{
    bool is_ok(true);
    try
    {
        InstallTask::set_targets_from_user_specs(s);
    }
    catch (const NoSuchPackageError & e)
    {
        output_stream() << endl;
        output_stream() << "Query error:" << endl;
        output_stream() << "  * " << e.backtrace("\n  * ");
        output_stream() << "Could not find '" << e.name() << "'.";

        if (want_suggestions())
        {
            output_stream() << " Looking for suggestions:" << endl;

            FuzzyCandidatesFinder f(*environment(), e.name(), filter::SupportsAction<InstallAction>());

            if (f.begin() == f.end())
                output_stream() << "No suggestions found." << endl;
            else
                output_stream() << "Suggestions:" << endl;

            for (FuzzyCandidatesFinder::CandidatesConstIterator c(f.begin()),
                     c_end(f.end()) ; c != c_end ; ++c)
                output_stream() << "  * " << colour(cl_package_name, *c) << endl;
        }

        output_stream() << endl;
        is_ok = false;;
    }

    return is_ok;
}

struct ConsoleInstallTask::CallbackDisplayer
{
    Mutex mutex;

    std::ostream & stream;
    unsigned width;
    std::map<std::string, int> metadata, steps;

    CallbackDisplayer(std::ostream & s) :
        stream(s),
        width(0)
    {
    }

    void operator() (const NotifierCallbackEvent & event)
    {
        event.accept(*this);
    }

    void visit(const NotifierCallbackGeneratingMetadataEvent & e)
    {
        Lock lock(mutex);
        ++metadata.insert(std::make_pair(stringify(e.repository()), 0)).first->second;
        update();
    }

    void visit(const NotifierCallbackResolverStepEvent &)
    {
        Lock lock(mutex);
        ++steps.insert(std::make_pair("steps", 0)).first->second;
        update();
    }

    void visit(const NotifierCallbackResolverStageEvent &)
    {
    }

    void visit(const NotifierCallbackLinkageStepEvent &)
    {
    }

    void update()
    {
        std::string s;
        if (! steps.empty())
        {
            for (std::map<std::string, int>::const_iterator i(steps.begin()), i_end(steps.end()) ;
                    i != i_end ; ++i)
            {
                if (! s.empty())
                    s.append(", ");

                s.append(stringify(i->second) + " " + i->first);
            }
        }

        if (! metadata.empty())
        {
            std::multimap<int, std::string> biggest;
            for (std::map<std::string, int>::const_iterator i(metadata.begin()), i_end(metadata.end()) ;
                    i != i_end ; ++i)
                biggest.insert(std::make_pair(i->second, i->first));

            int t(0), n(0);
            std::string ss;
            for (std::multimap<int, std::string>::const_reverse_iterator i(biggest.rbegin()), i_end(biggest.rend()) ;
                    i != i_end ; ++i)
            {
                ++n;

                if (n == 4)
                    ss.append(", ...");

                if (n < 4)
                {
                    if (! ss.empty())
                        ss.append(", ");

                    ss.append(stringify(i->first) + " " + i->second);
                }

                t += i->first;
            }

            if (! s.empty())
                s.append(", ");
            s.append(stringify(t) + " metadata (" + ss + ")");
        }

        stream << std::string(width, '\010') << " " << s;

        if (width > s.length())
            stream
                << std::string(width - s.length(), ' ')
                << std::string(width - s.length(), '\010');

        width = s.length() + 1;
        stream << std::flush;
    }
};

void
ConsoleInstallTask::on_build_deplist_pre()
{
    output_activity_start_message("Building dependency list: ");
    output_xterm_title("Building dependency list");

    _callback_displayer = std::make_shared<CallbackDisplayer>(output_stream());
    _notifier_callback = std::make_shared<NotifierCallbackID>(environment()->add_notifier_callback(
                    std::bind(std::mem_fn(&ConsoleInstallTask::_notifier_callback_fn),
                        this, std::placeholders::_1)));
}

void
ConsoleInstallTask::_notifier_callback_fn(const NotifierCallbackEvent & e)
{
    e.accept(*_callback_displayer);
}

void
ConsoleInstallTask::on_build_deplist_post()
{
    output_activity_end_message();
    _resolution_finished = true;

    environment()->remove_notifier_callback(*_notifier_callback);
    _notifier_callback.reset();
}

void
ConsoleInstallTask::on_clean_pre(const DepListEntry &,
        const PackageID & c, const int x, const int y, const int s, const int f)
{
    std::string m("(" + make_x_of_y(x, y, s, f) + ") Cleaning " + stringify(c));
    output_heading(m);
    output_xterm_title(m);
}

void
ConsoleInstallTask::on_clean_post(const DepListEntry &,
        const PackageID &, const int, const int, const int, const int)
{
}

void
ConsoleInstallTask::on_clean_fail(const DepListEntry &,
        const PackageID & c, const int x, const int y, const int s, const int f)
{
    output_xterm_title("(" + make_x_of_y(x, y, s, f) + ") Failed cleaning " + stringify(c));
}

void
ConsoleInstallTask::on_display_merge_list_pre()
{
    output_heading("These packages will be installed:");
}

void
ConsoleInstallTask::on_display_merge_list_post()
{
    output_endl();
    display_merge_list_post_counts();
    display_merge_list_post_tags();
    display_merge_list_post_use_descriptions();
}

void
ConsoleInstallTask::display_merge_list_post_use_descriptions()
{
    if (! want_use_summary())
        return;

    if (_choice_descriptions.empty())
        return;

    display_use_summary_start();
    for (ChoiceDescriptions::const_iterator i(_choice_descriptions.begin()), i_end(_choice_descriptions.end()) ;
            i != i_end ; ++i)
    {
        if (i->second.empty())
            continue;

        display_use_summary_start_choice(i);
        for (ChoiceValueDescriptions::const_iterator j(i->second.begin()), j_end(i->second.end()) ;
                j != j_end ; ++j)
            display_use_summary_entry(j);
        display_use_summary_end_choice(i);
    }
    display_tag_summary_end();
}

void
ConsoleInstallTask::display_use_summary_start()
{
}

void
ConsoleInstallTask::display_use_summary_start_choice(const ChoiceDescriptions::const_iterator & i)
{
    output_heading(i->first + ":");
}

void
ConsoleInstallTask::display_use_summary_end_choice(const ChoiceDescriptions::const_iterator &)
{
}

void
ConsoleInstallTask::display_use_summary_entry(const ChoiceValueDescriptions::const_iterator & i)
{
    if (i->second.empty())
        return;

    bool all_same(true);
    std::string description((*i->second.begin())->choices_key()->value()->find_by_name_with_prefix(
                i->first)->description());
    for (std::list<std::shared_ptr<const PackageID> >::const_iterator j(i->second.begin()), j_end(i->second.end()) ;
            j != j_end ; ++j)
        if ((*j)->choices_key()->value()->find_by_name_with_prefix(i->first)->description() != description)
            all_same = false;

    if (all_same)
    {
        std::stringstream desc;
        desc << std::left << std::setw(30) << (render_as_tag(stringify((*i->second.begin())->choices_key()->value()->find_by_name_with_prefix(
                        i->first)->unprefixed_name())) + ": ");
        desc << description;
        output_starred_item(desc.str());
    }
    else
    {
        {
            std::stringstream desc;
            desc << std::left << std::setw(30) << (render_as_tag(stringify((*i->second.begin())->choices_key()->value()->find_by_name_with_prefix(
                            i->first)->unprefixed_name())) + ": ");
            output_starred_item(desc.str());
        }
        for (std::list<std::shared_ptr<const PackageID> >::const_iterator j(i->second.begin()), j_end(i->second.end()) ;
                j != j_end ; ++j)
        {
            std::stringstream desc;
            desc << std::left << std::setw(30) << (render_as_package_name(stringify(**j)) + ": ");
            desc << (*j)->choices_key()->value()->find_by_name_with_prefix(i->first)->description();
            output_starred_item(desc.str(), 1);
        }
    }
}

void
ConsoleInstallTask::display_use_summary_end()
{
}

void
ConsoleInstallTask::on_not_continuing_due_to_errors()
{
    output_endl();
    output_starred_item(render_as_error("Cannot continue with install due to the errors indicated above"));
}

void
ConsoleInstallTask::on_display_merge_list_entry(const DepListEntry & d)
{
    DisplayMode m;

    do
    {
        switch (d.kind())
        {
            case dlk_already_installed:
                if (! want_full_install_reasons())
                    return;
                m = unimportant_entry;
                continue;

            case dlk_provided:
            case dlk_virtual:
                if (d.tags()->empty())
                {
                    if (! want_full_install_reasons())
                        return;
                    m = unimportant_entry;
                }
                else
                    m = normal_entry;
                continue;

            case dlk_package:
            case dlk_subpackage:
                m = normal_entry;
                continue;

            case dlk_suggested:
                m = suggested_entry;
                continue;

            case dlk_masked:
            case dlk_block:
                m = error_entry;
                continue;

            case last_dlk:
                break;
        }

        throw InternalError(PALUDIS_HERE, "Bad d.kind");
    } while (false);

    if (already_done(d))
        return;

    std::shared_ptr<RepositoryName> repo;
    if (d.destination())
        repo = std::make_shared<RepositoryName>(d.destination()->name());

    std::shared_ptr<const PackageIDSequence> existing_repo((*environment())[selection::AllVersionsSorted(repo ?
                generator::Matches(make_package_dep_spec({ })
                    .package(d.package_id()->name()).in_repository(*repo), d.package_id(), { }) :
                generator::Matches(make_package_dep_spec({ })
                    .package(d.package_id()->name()), d.package_id(), { }) | filter::InstalledAtRoot(environment()->preferred_root_key()->value())
                )]);;

    std::shared_ptr<const PackageIDSequence> existing_slot_repo((*environment())[selection::AllVersionsSorted((repo ?
                    generator::Matches(make_package_dep_spec({ })
                        .package(d.package_id()->name()).in_repository(*repo), d.package_id(), { }) :
                    generator::Matches(make_package_dep_spec({ })
                        .package(d.package_id()->name()), d.package_id(), { }) | filter::InstalledAtRoot(environment()->preferred_root_key()->value()))
                | filter::SameSlot(d.package_id()))]);

    display_merge_list_entry_start(d, m);
    display_merge_list_entry_package_name(d, m);
    display_merge_list_entry_repository(d, m);

    if (d.package_id()->virtual_for_key())
        display_merge_list_entry_for(*d.package_id()->virtual_for_key()->value(), m);

    display_merge_list_entry_slot(d, m);

    display_merge_list_entry_status_and_update_counts(d, existing_repo, existing_slot_repo, m);
    display_merge_list_entry_non_package_tags(d, m);
    if (! want_compact())
        display_merge_list_entry_package_tags(d, m);
    display_merge_list_entry_choices(d, m, existing_repo, existing_slot_repo);
    display_merge_list_entry_description(d, existing_repo, existing_slot_repo, m);
    display_merge_list_entry_distsize(d, m);
    if (want_compact())
        display_merge_list_entry_package_tags(d, m);
    display_merge_list_entry_end(d, m);

    if (d.kind() == dlk_masked)
        display_merge_list_entry_mask_reasons(d);
}

void
ConsoleInstallTask::on_pretend_all_pre()
{
    output_endl();
    output_activity_start_message("Checking for possible errors...");
    output_activity_end_message();
}

void
ConsoleInstallTask::on_pretend_pre(const DepListEntry & d)
{
    std::string m("Pretending for " + stringify(*d.package_id()));
    output_xterm_title(m);
}

void
ConsoleInstallTask::on_pretend_post(const DepListEntry &)
{
}

void
ConsoleInstallTask::on_pretend_all_post()
{
}

void
ConsoleInstallTask::on_fetch_all_pre()
{
}

void
ConsoleInstallTask::on_fetch_pre(const DepListEntry & d, const int x, const int y,
        const int s, const int f)
{
    std::string m("(" + make_x_of_y(x, y, s, f) + ") Fetching " + stringify(*d.package_id()));

    output_heading(m);
    output_xterm_title(m);
}

void
ConsoleInstallTask::on_fetch_post(const DepListEntry &, const int, const int,
        const int, const int)
{
}

void
ConsoleInstallTask::on_fetch_all_post()
{
}

void
ConsoleInstallTask::on_install_all_pre()
{
}

void
ConsoleInstallTask::on_install_pre(const DepListEntry & d, const int x,
        const int y, const int s, const int f)
{
    std::string m("(" + make_x_of_y(x, y, s, f) + ") Installing " + stringify(*d.package_id()));

    output_heading(m);
    output_xterm_title(m);
}

void
ConsoleInstallTask::on_skip_unsatisfied(const DepListEntry & d, const PackageDepSpec & spec,
        const int x, const int y, const int s, const int f)
{
    std::string m("(" + make_x_of_y(x, y, s, f) + ") Skipping " + stringify(*d.package_id()) +
            " (unsatisfied '" + stringify(spec) + "')");

    output_heading(m);
}

void
ConsoleInstallTask::on_skip_dependent(const DepListEntry & d, const std::shared_ptr<const PackageID> & id,
        const int x, const int y, const int s, const int f)
{
    std::string m("(" + make_x_of_y(x, y, s, f) + ") Skipping " + stringify(*d.package_id()) +
            " (dependent upon '" + stringify(*id) + "')");

    output_heading(m);
}

void
ConsoleInstallTask::on_skip_already_done(const DepListEntry & d,
        const int x, const int y, const int s, const int f)
{
    std::string m("(" + make_x_of_y(x, y, s, f) + ") Skipping " + stringify(*d.package_id()) +
            " (already done)");

    output_heading(m);
}

void
ConsoleInstallTask::on_install_post(const DepListEntry &, const int, const int,
        const int, const int)
{
}

void
ConsoleInstallTask::on_install_fail(const DepListEntry & d, const int x, const int y,
        const int s, const int f)
{
    output_xterm_title("(" + make_x_of_y(x, y, s, f) + ") Failed install of " + stringify(*d.package_id()));
}

void
ConsoleInstallTask::on_install_all_post()
{
    output_xterm_title("Completed install");
}

void
ConsoleInstallTask::on_update_world_pre()
{
    output_heading("Updating world file");
}

void
ConsoleInstallTask::on_update_world(const PackageDepSpec & a)
{
    output_starred_item("adding " + render_as_package_name(stringify(a)));
}

void
ConsoleInstallTask::on_update_world(const SetName & a)
{
    output_starred_item("adding " + render_as_set_name(stringify(a)));
}

void
ConsoleInstallTask::on_update_world_skip(const PackageDepSpec & a, const std::string & s)
{
    output_starred_item("skipping " + render_as_package_name(stringify(a)) + " ("
            + s + ")");
}

void
ConsoleInstallTask::on_update_world_skip(const SetName & a, const std::string & s)
{
    output_starred_item("skipping " + render_as_set_name(stringify(a)) + " ("
            + s + ")");
}

void
ConsoleInstallTask::on_update_world_post()
{
}

void
ConsoleInstallTask::on_preserve_world()
{
    output_heading("Updating world file");
    output_starred_item("--preserve-world was specified, skipping world changes");
}

void
ConsoleInstallTask::display_merge_list_post_counts()
{
    std::ostringstream s;
    s << "Total: " << count<max_count>() << render_plural(count<max_count>(), " package",
            " packages");

    if (count<max_count>())
    {
        bool need_comma(false);
        s << " (";
        if (count<new_count>())
        {
            s << count<new_count>() << " new";
            need_comma = true;
        }
        if (count<upgrade_count>())
        {
            if (need_comma)
                s << ", ";
            s << count<upgrade_count>() << render_plural(count<upgrade_count>(),
                    " upgrade", " upgrades");
            need_comma = true;
        }
        if (count<downgrade_count>())
        {
            if (need_comma)
                s << ", ";
            s << count<downgrade_count>() << render_plural(count<downgrade_count>(),
                    " downgrade", " downgrades");
            need_comma = true;
        }
        if (count<new_slot_count>())
        {
            if (need_comma)
                s << ", ";
            s << count<new_slot_count>() << render_plural(count<new_slot_count>(),
                    " in new slot", " in new slots");
            need_comma = true;
        }
        if (count<rebuild_count>())
        {
            if (need_comma)
                s << ", ";
            s << count<rebuild_count>() << render_plural(count<rebuild_count>(),
                    " rebuild", " rebuilds");
            need_comma = true;
        }
        s << ")";

        if (get_download_size())
        {
            if (_download_size_overflow)
                s << ", more than " << pretty_print_bytes(std::numeric_limits<unsigned long>::max()) << " to download";
            else
                s << ", " << pretty_print_bytes(get_download_size()) << " to download";
        }
    }

    if (count<max_count>() && count<error_count>())
    {
        if (count<suggested_count>())
            s << " and ";
        else
            s << ", ";
    }

    if (count<error_count>())
        s << render_as_error(stringify(count<error_count>()) +
                render_plural(count<error_count>(), " error", " errors"));

    if ((count<max_count>() || count<error_count>()) && count<suggested_count>())
        s << " and ";

    if (count<suggested_count>())
        s << count<suggested_count>() << render_plural(count<suggested_count>(), " suggestion", " suggestions");

    output_unstarred_item(s.str());
}

void
ConsoleInstallTask::display_merge_list_post_tags()
{
    if (! want_tags_summary())
        return;

    std::set<std::string> tag_categories;
    for (Set<DepTagEntry, DepTagEntryComparator>::ConstIterator a(all_tags()->begin()),
            a_end(all_tags()->end()) ; a != a_end ; ++a)
        tag_categories.insert(a->tag()->category());

    display_tag_summary_start();

    for (std::set<std::string>::iterator cat(tag_categories.begin()),
            cat_end(tag_categories.end()) ; cat != cat_end ; ++cat)
    {
        std::shared_ptr<const DepTagCategory> c(DepTagCategoryFactory::get_instance()->create(*cat));

        if (! c->visible())
            continue;

        display_tag_summary_tag_title(*c);
        display_tag_summary_tag_pre_text(*c);

        for (Set<DepTagEntry, DepTagEntryComparator>::ConstIterator t(all_tags()->begin()),
                t_end(all_tags()->end()) ; t != t_end ; ++t)
        {
            if (t->tag()->category() != *cat)
                continue;
            display_tag_summary_tag(t->tag());
        }

        display_tag_summary_tag_post_text(*c);
    }

    display_tag_summary_end();
}

void
ConsoleInstallTask::display_tag_summary_start()
{
}

void
ConsoleInstallTask::display_tag_summary_tag_title(const DepTagCategory & c)
{
    if (! c.title().empty())
        output_heading(c.title() + ":");
}

void
ConsoleInstallTask::display_tag_summary_tag_pre_text(const DepTagCategory & c)
{
    if (! c.pre_text().empty())
        output_unstarred_item(c.pre_text());
}

void
ConsoleInstallTask::display_tag_summary_tag(const std::shared_ptr<const DepTag> & t)
{
    std::shared_ptr<DepTagSummaryDisplayer> displayer(make_dep_tag_summary_displayer());
    t->accept(*displayer.get());
}

void
ConsoleInstallTask::display_tag_summary_tag_post_text(const DepTagCategory & c)
{
    if (! c.post_text().empty())
        output_unstarred_item(c.post_text());
}

void
ConsoleInstallTask::display_tag_summary_end()
{
}

DepTagSummaryDisplayer::DepTagSummaryDisplayer(ConsoleInstallTask * t) :
    _task(t)
{
}

DepTagSummaryDisplayer::~DepTagSummaryDisplayer()
{
}

void
DepTagSummaryDisplayer::visit(const GLSADepTag & tag)
{
    task()->output_starred_item(task()->render_as_tag(tag.short_text()) + ": "
            + tag.glsa_title());
}

void
DepTagSummaryDisplayer::visit(const DependencyDepTag &)
{
}

void
DepTagSummaryDisplayer::visit(const TargetDepTag &)
{
}

void
DepTagSummaryDisplayer::visit(const GeneralSetDepTag & tag)
{
    std::string desc;
    if (tag.short_text() == "world")
        desc = ":           Packages that have been explicitly installed";
    else if (tag.short_text() == "everything" || tag.short_text() == "installed-packages"
            || tag.short_text() == "installed-slotted")
        desc = ":      All installed packages";
    else if (tag.short_text() == "system")
        desc = ":          Packages that are part of the base system";

    task()->output_starred_item(task()->render_as_tag(tag.short_text()) + desc);
}

void
ConsoleInstallTask::display_merge_list_entry_start(const DepListEntry & e, const DisplayMode)
{
    switch (e.kind())
    {
        case dlk_subpackage:
        case dlk_suggested:
        case dlk_provided:
            output_no_endl("    ");
            break;

        case dlk_virtual:
        case dlk_masked:
        case dlk_block:
        case dlk_already_installed:
        case dlk_package:
        case last_dlk:
            break;
    }

    output_starred_item_no_endl("");
}

void
ConsoleInstallTask::display_merge_list_entry_package_name(const DepListEntry & d, const DisplayMode m)
{
    switch (m)
    {
        case normal_entry:
        case suggested_entry:
            output_no_endl(render_as_package_name(stringify(d.package_id()->name())));
            break;

        case unimportant_entry:
            output_no_endl(render_as_unimportant(stringify(d.package_id()->name())));
            break;

        case error_entry:
            output_no_endl(render_as_error(stringify(d.package_id()->name())));
            break;
    }
}

void
ConsoleInstallTask::display_merge_list_entry_for(const PackageID & d, const DisplayMode m)
{
    switch (m)
    {
        case normal_entry:
        case suggested_entry:
            break;

        case unimportant_entry:
            output_no_endl(" (for ");
            output_no_endl(render_as_unimportant(stringify(d)));
            output_no_endl(")");
            break;

        case error_entry:
            output_no_endl(" (for ");
            output_no_endl(render_as_package_name(stringify(d)));
            output_no_endl(")");
            break;
    }
}

void
ConsoleInstallTask::display_merge_list_entry_repository(const DepListEntry & d, const DisplayMode m)
{
    std::shared_ptr<const PackageIDSequence> inst((*environment())[selection::BestVersionOnly(
                generator::Package(d.package_id()->name()) |
                filter::SameSlot(d.package_id()) |
                filter::InstalledAtRoot(environment()->preferred_root_key()->value()))]);
    bool changed(normal_entry == m &&
            ! inst->empty() && (*inst->begin())->from_repositories_key() &&
            (*inst->begin())->from_repositories_key()->value()->end() ==
            (*inst->begin())->from_repositories_key()->value()->find(
                stringify(d.package_id()->repository_name())));

    if (changed || environment()->package_database()->favourite_repository() != d.package_id()->repository_name())
        output_no_endl("::" + stringify(d.package_id()->repository_name()));
    if (changed)
        output_no_endl(" (previously ::" + join((*inst->begin())->from_repositories_key()->value()->begin(),
                        (*inst->begin())->from_repositories_key()->value()->end(), ", ::") + ")");
}

void
ConsoleInstallTask::display_merge_list_entry_slot(const DepListEntry & d, const DisplayMode m)
{
    if (! d.package_id()->slot_key())
        return;

    if (d.package_id()->slot_key()->value() == SlotName("0"))
        return;

    switch (m)
    {
        case normal_entry:
        case suggested_entry:
            output_no_endl(render_as_slot_name(" :" + stringify(d.package_id()->slot_key()->value())));
            break;

        case unimportant_entry:
            output_no_endl(render_as_unimportant(" :" + stringify(d.package_id()->slot_key()->value())));
            break;

        case error_entry:
            output_no_endl(render_as_slot_name(" :" + stringify(d.package_id()->slot_key()->value())));
            break;
    }
}

void
ConsoleInstallTask::display_merge_list_entry_status_and_update_counts(const DepListEntry & d,
        const std::shared_ptr<const PackageIDSequence> & existing_repo,
        const std::shared_ptr<const PackageIDSequence> & existing_slot_repo,
        const DisplayMode m)
{
    switch (m)
    {
        case unimportant_entry:
            if (d.kind() == dlk_provided)
                output_no_endl(render_as_unimportant(" [provided " +
                            stringify(d.package_id()->canonical_form(idcf_version)) + "]"));
            else
                output_no_endl(render_as_unimportant(" [- " +
                            stringify(d.package_id()->canonical_form(idcf_version)) + "]"));
            break;

        case suggested_entry:
            output_no_endl(render_as_update_mode(" [suggestion " +
                        stringify(d.package_id()->canonical_form(idcf_version)) + "]"));
            set_count<suggested_count>(count<suggested_count>() + 1);
            break;

        case normal_entry:
            {
                output_no_endl(render_as_update_mode(" ["));

                std::string destination_str;
                if (! d.package_id()->virtual_for_key())
                {
                    std::shared_ptr<const DestinationsSet> default_destinations(environment()->default_destinations());
                    if (default_destinations->end() == default_destinations->find(d.destination()))
                        destination_str = " ::" + stringify(d.destination()->name());
                }

                if (existing_repo->empty())
                {
                    output_no_endl(render_as_update_mode("N " + stringify(d.package_id()->canonical_form(idcf_version) + destination_str)));
                    set_count<new_count>(count<new_count>() + 1);
                    set_count<max_count>(count<max_count>() + 1);
                }
                else if (existing_slot_repo->empty())
                {
                    output_no_endl(render_as_update_mode("S " + d.package_id()->canonical_form(idcf_version) + destination_str));
                    set_count<new_slot_count>(count<new_slot_count>() + 1);
                    set_count<max_count>(count<max_count>() + 1);
                }
                else if ((*existing_slot_repo->last())->version() < d.package_id()->version())
                {
                    output_no_endl(render_as_update_mode("U " +
                            stringify((*existing_slot_repo->last())->canonical_form(idcf_version)) + " -> " +
                            stringify(d.package_id()->canonical_form(idcf_version))));
                    set_count<upgrade_count>(count<upgrade_count>() + 1);
                    set_count<max_count>(count<max_count>() + 1);
                }
                else if ((*existing_slot_repo->last())->version() > d.package_id()->version())
                {
                    output_no_endl(render_as_update_mode("D " +
                            stringify((*existing_slot_repo->last())->canonical_form(idcf_version)) + " -> " +
                            stringify(d.package_id()->canonical_form(idcf_version))));
                    set_count<downgrade_count>(count<downgrade_count>() + 1);
                    set_count<max_count>(count<max_count>() + 1);
                }
                else
                {
                    output_no_endl(render_as_update_mode("R " + stringify(d.package_id()->canonical_form(idcf_version)) +
                                destination_str));
                    set_count<rebuild_count>(count<rebuild_count>() + 1);
                    set_count<max_count>(count<max_count>() + 1);
                }

                output_no_endl(render_as_update_mode("]"));
            }
            break;

        case error_entry:
            set_count<error_count>(count<error_count>() + 1);
            do
            {
                switch (d.kind())
                {
                    case dlk_masked:
                        output_no_endl(render_as_update_mode(" [! masked]"));
                        continue;

                    case dlk_block:
                        output_no_endl(render_as_update_mode(" [! " + stringify(d.package_id()->canonical_form(idcf_version)) + " blocking]"));
                        continue;

                    case dlk_provided:
                    case dlk_virtual:
                    case dlk_already_installed:
                    case dlk_package:
                    case dlk_subpackage:
                    case dlk_suggested:
                    case last_dlk:
                        ;
                }

                throw InternalError(PALUDIS_HERE, "Bad d.kind");
            } while (false);
            break;
    }
}

void
ConsoleInstallTask::display_merge_list_entry_choices(const DepListEntry & d,
        const DisplayMode m,
        const std::shared_ptr<const PackageIDSequence> & existing_repo,
        const std::shared_ptr<const PackageIDSequence> & existing_slot_repo
        )
{
    switch (m)
    {
        case unimportant_entry:
        case error_entry:
        case suggested_entry:
            break;

        case normal_entry:
            {
                if (! d.package_id()->choices_key())
                    break;

                std::shared_ptr<const PackageID> old_id;
                if (existing_slot_repo && ! existing_slot_repo->empty())
                    old_id = *existing_slot_repo->last();
                else if (existing_repo && ! existing_repo->empty())
                    old_id = *existing_repo->last();
                std::shared_ptr<const Choices> old_choices;
                if (old_id && old_id->choices_key())
                    old_choices = old_id->choices_key()->value();

                ColourPrettyPrinter printer(environment(), d.package_id());

                std::string s;
                bool non_blank_prefix(false);
                for (Choices::ConstIterator k(d.package_id()->choices_key()->value()->begin()),
                        k_end(d.package_id()->choices_key()->value()->end()) ;
                        k != k_end ; ++k)
                {
                    if ((*k)->hidden())
                        continue;

                    bool shown_prefix(false);
                    for (Choice::ConstIterator i((*k)->begin()), i_end((*k)->end()) ;
                            i != i_end ; ++i)
                    {
                        if (! (*i)->explicitly_listed())
                            continue;

                        if (! shown_prefix)
                        {
                            if (non_blank_prefix || ! (*k)->show_with_no_prefix())
                            {
                                shown_prefix = true;
                                if (! s.empty())
                                    s.append(" ");
                                s.append((*k)->raw_name() + ":");
                            }
                        }

                        if (! s.empty())
                            s.append(" ");

                        std::string t(printer.prettify(*i));

                        bool changed(false), added(false);
                        if ((*k)->consider_added_or_changed())
                        {
                            if (old_choices)
                            {
                                std::shared_ptr<const ChoiceValue> old_choice(old_choices->find_by_name_with_prefix((*i)->name_with_prefix()));
                                if (! old_choice)
                                    added = true;
                                else if (old_choice->enabled() != (*i)->enabled())
                                    changed = true;
                            }
                            else
                                added = true;
                        }

                        if (changed)
                        {
                            t = t + "*";
                            if (want_changed_use_flags())
                                _choice_descriptions[(*k)->human_name()][(*i)->name_with_prefix()].push_back(d.package_id());
                        }
                        else if (added)
                        {
                            if (old_id)
                                t = t + "*";
                            if (want_new_use_flags())
                                _choice_descriptions[(*k)->human_name()][(*i)->name_with_prefix()].push_back(d.package_id());
                        }
                        else if (want_unchanged_use_flags())
                            _choice_descriptions[(*k)->human_name()][(*i)->name_with_prefix()].push_back(d.package_id());

                        s.append(t);
                    }
                }

                if (s.empty())
                    break;

                if (want_compact())
                    output_no_endl(" " + s);
                else
                {
                    output_endl();
                    output_no_endl("    " + s);
                }
            }
            break;
    }
}

void
ConsoleInstallTask::display_merge_list_entry_description(const DepListEntry & d,
        const std::shared_ptr<const PackageIDSequence> & existing_slot_repo,
        const std::shared_ptr<const PackageIDSequence> &,
        const DisplayMode m)
{
    if ((! d.package_id()->short_description_key()) || d.package_id()->short_description_key()->value().empty())
        return;

    if (existing_slot_repo->empty())
    {
        if (! want_new_descriptions())
            return;
    }
    else
    {
        if (! want_existing_descriptions())
            return;
    }

    switch (m)
    {
        case unimportant_entry:
        case error_entry:
            break;

        case suggested_entry:
        case normal_entry:
            if (want_compact())
                output_no_endl(" \"" + d.package_id()->short_description_key()->value() + "\"");
            else
            {
                output_endl();
                output_no_endl("    \"" + d.package_id()->short_description_key()->value() + "\"");
            }
            break;
    }
}

namespace
{
    struct FindDistfilesSize :
        PretendFetchAction
    {
        std::shared_ptr<Set<FSPath, FSPathComparator> > already_downloaded;
        unsigned long size;
        bool overflow;

        FindDistfilesSize(const FetchActionOptions & o, const std::shared_ptr<Set<FSPath, FSPathComparator> > & a) :
            PretendFetchAction(o),
            already_downloaded(a),
            size(0),
            overflow(false)
        {
        }

        void will_fetch(const FSPath & destination, const unsigned long size_in_bytes)
        {
            if (already_downloaded->end() != already_downloaded->find(destination))
                return;
            already_downloaded->insert(destination);
            unsigned long new_size(size + size_in_bytes);
            if (new_size < size)
                overflow = true;
            else
                size = new_size;
        }
    };
}

void
ConsoleInstallTask::display_merge_list_entry_distsize(const DepListEntry & d,
        const DisplayMode m)
{
    if (normal_entry != m)
        return;

    SupportsActionTest<PretendFetchAction> action_test;
    if (! d.package_id()->supports_action(action_test))
        return;

    OutputManagerFromEnvironment output_manager_holder(environment(), d.package_id(),
            oe_exclusive, ClientOutputFeatures());
    FindDistfilesSize action(make_fetch_action_options(d, output_manager_holder), _already_downloaded);
    d.package_id()->perform_action(action);
    if (output_manager_holder.output_manager_if_constructed())
        output_manager_holder.output_manager_if_constructed()->succeeded();

    if (! action.size)
        return;

    if (want_compact())
        output_no_endl(" ");
    else
    {
        output_endl();
        output_no_endl("    ");
    }

    if (action.overflow)
        output_stream() << "more than " << pretty_print_bytes(std::numeric_limits<unsigned long>::max())
            << " to download";
    else
        output_stream() << pretty_print_bytes(action.size) << " to download";

    if (action.overflow)
        _download_size_overflow = true;
    else
    {
        unsigned long new_size(_download_size + action.size);
        if (new_size < _download_size)
            _download_size_overflow = true;
        else
            _download_size = new_size;
    }
}

void
ConsoleInstallTask::display_merge_list_entry_non_package_tags(const DepListEntry & d, const DisplayMode m)
{
    if (d.tags()->empty())
        return;

    std::string tag_titles;
    std::stringstream s;

    for (Set<DepTagEntry, DepTagEntryComparator>::ConstIterator
            tag(d.tags()->begin()),
            tag_end(d.tags()->end()) ;
            tag != tag_end ; ++tag)
    {
        if (tag->tag()->category() == "dependency")
            continue;

        all_tags()->insert(*tag);

        std::shared_ptr<EntryDepTagDisplayer> displayer(make_entry_dep_tag_displayer());
        tag->tag()->accept(*displayer.get());
        tag_titles.append(displayer->text());
        tag_titles.append(", ");
    }

    if (! tag_titles.empty())
    {
        tag_titles.erase(tag_titles.length() - 2);

        if (! tag_titles.empty())
        {
            switch (m)
            {
                case normal_entry:
                case suggested_entry:
                case error_entry:
                    s << render_as_tag("<" + tag_titles + ">") << " ";
                    break;

                case unimportant_entry:
                    s << render_as_unimportant("<" + tag_titles + ">") << " ";
                    break;
            }
        }
    }

    if (! s.str().empty())
    {
        std::string t(s.str());
        t.erase(t.length() - 1);
        output_no_endl(" " + t);
    }
}

void
ConsoleInstallTask::display_merge_list_entry_package_tags(const DepListEntry & d, const DisplayMode m)
{
    if (d.tags()->empty())
        return;

    std::stringstream s;

    if (! want_install_reasons())
        if (d.kind() != dlk_block)
            return;

    std::string deps;
    std::set<std::string> dependents, unsatisfied_dependents;
    unsigned c(0), max_c(want_full_install_reasons() ? std::numeric_limits<long>::max() : 3);

    for (Set<DepTagEntry, DepTagEntryComparator>::ConstIterator
            tag(d.tags()->begin()),
            tag_end(d.tags()->end()) ;
            tag != tag_end ; ++tag)
    {
        if (tag->tag()->category() != "dependency")
            continue;

        std::shared_ptr<const PackageDepSpec> spec(
            std::static_pointer_cast<const DependencyDepTag>(tag->tag())->dependency());
        std::shared_ptr<const PackageID> id(
                std::static_pointer_cast<const DependencyDepTag>(tag->tag())->package_id());
        if (d.kind() != dlk_masked && d.kind() != dlk_block && (*environment())[selection::SomeArbitraryVersion(
                generator::Matches(*spec, id, { }) |
                filter::InstalledAtRoot(environment()->preferred_root_key()->value()))]->empty())
            unsatisfied_dependents.insert(tag->tag()->short_text());
        else
            dependents.insert(tag->tag()->short_text());
    }

    for (std::set<std::string>::iterator it(unsatisfied_dependents.begin()),
             it_end(unsatisfied_dependents.end()); it_end != it; ++it)
        if (++c < max_c)
        {
            deps.append("*");
            deps.append(*it);
            deps.append(", ");
        }
    for (std::set<std::string>::iterator it(dependents.begin()),
             it_end(dependents.end()); it_end != it; ++it)
        if (unsatisfied_dependents.end() == unsatisfied_dependents.find(*it) &&
            ++c < max_c)
        {
            deps.append(*it);
            deps.append(", ");
        }

    if (! deps.empty())
    {
        if (c >= max_c)
            deps.append(stringify(c - max_c + 1) + " more, ");

        if (! deps.empty())
            deps.erase(deps.length() - 2);

        if (! deps.empty())
        {
            switch (m)
            {
                case normal_entry:
                case suggested_entry:
                case error_entry:
                    s << render_as_tag("Reasons: " + deps) << " ";
                    break;

                case unimportant_entry:
                    s << render_as_unimportant("Reasons: " + deps) << " ";
                    break;
            }
        }
    }

    if (! s.str().empty())
    {
        std::string t(s.str());
        t.erase(t.length() - 1);
        if (want_compact())
            output_no_endl(" " + t);
        else
        {
            output_endl();
            output_no_endl("    " + t);
        }
    }
}

void
ConsoleInstallTask::display_merge_list_entry_end(const DepListEntry &, const DisplayMode)
{
    output_endl();
}

std::shared_ptr<DepTagSummaryDisplayer>
ConsoleInstallTask::make_dep_tag_summary_displayer()
{
    return std::make_shared<DepTagSummaryDisplayer>(this);
}

std::shared_ptr<EntryDepTagDisplayer>
ConsoleInstallTask::make_entry_dep_tag_displayer()
{
    return std::make_shared<EntryDepTagDisplayer>();
}

EntryDepTagDisplayer::EntryDepTagDisplayer()
{
}

EntryDepTagDisplayer::~EntryDepTagDisplayer()
{
}

void
EntryDepTagDisplayer::visit(const GLSADepTag & tag)
{
    text() = tag.short_text();
}

void
EntryDepTagDisplayer::visit(const DependencyDepTag &)
{
}

void
EntryDepTagDisplayer::visit(const TargetDepTag &)
{
    text() = "target";
}

void
EntryDepTagDisplayer::visit(const GeneralSetDepTag & tag)
{
    text() = tag.short_text(); // + "<" + tag->source() + ">";
}

void
ConsoleInstallTask::display_merge_list_entry_mask_reasons(const DepListEntry & e)
{
    bool need_comma(false);
    output_no_endl("    Masked by: ");

    for (PackageID::MasksConstIterator m(e.package_id()->begin_masks()), m_end(e.package_id()->end_masks()) ;
            m != m_end ; ++m)
    {
        if (need_comma)
            output_no_endl(", ");
        MaskDisplayer d(environment(), e.package_id(), true);
        (*m)->accept(d);
        output_no_endl(d.result());
        need_comma = true;
    }

    output_endl();
}

void
ConsoleInstallTask::on_ambiguous_package_name_error(const AmbiguousPackageNameError & e)
{
    output_stream() << endl;
    output_stream() << "Query error:" << endl;
    output_stream() << "  * " << e.backtrace("\n  * ");
    output_stream() << "Ambiguous package name '" << e.name() << "'. Did you mean:" << endl;
    for (AmbiguousPackageNameError::OptionsConstIterator o(e.begin_options()),
            o_end(e.end_options()) ; o != o_end ; ++o)
        output_stream() << "    * " << colour(cl_package_name, *o) << endl;
    output_stream() << endl;
}

void
ConsoleInstallTask::on_non_fetch_action_error(
        const std::shared_ptr<OutputManager> & output_manager, const ActionFailedError & e)
{
    output_manager->stdout_stream() << endl;
    output_manager->stdout_stream() << "Install error:" << endl;
    output_manager->stdout_stream() << "  * " << e.backtrace("\n  * ");
    output_manager->stdout_stream() << e.message() << endl;
    output_manager->stdout_stream() << endl;
    output_manager->stdout_stream() << endl;
}

void
ConsoleInstallTask::on_fetch_action_error(
        const std::shared_ptr<OutputManager> & output_manager, const ActionFailedError & e,
        const std::shared_ptr<const Sequence<FetchActionFailure> > & failures)
{
    output_manager->stdout_stream() << endl;
    output_manager->stdout_stream() << "Fetch error:" << endl;
    output_manager->stdout_stream() << "  * " << e.backtrace("\n  * ");
    output_manager->stdout_stream() << e.message() << endl;
    output_manager->stdout_stream() << endl;

    if (failures)
    {
        for (Sequence<FetchActionFailure>::ConstIterator f(failures->begin()), f_end(failures->end()) ;
                f != f_end ; ++f)
        {
            output_manager->stdout_stream() << "  * File '" << (*f).target_file() << "': ";

            bool need_comma(false);
            if ((*f).requires_manual_fetching())
            {
                output_manager->stdout_stream() << "requires manual fetching";
                need_comma = true;
            }

            if ((*f).failed_automatic_fetching())
            {
                if (need_comma)
                    output_manager->stdout_stream() << ", ";
                output_manager->stdout_stream() << "failed automatic fetching";
                need_comma = true;
            }

            if (! (*f).failed_integrity_checks().empty())
            {
                if (need_comma)
                    output_manager->stdout_stream() << ", ";
                output_manager->stdout_stream() << "failed integrity checks: " << (*f).failed_integrity_checks();
                need_comma = true;
            }

            output_manager->stdout_stream() << endl;
        }
    }

    output_manager->stdout_stream() << endl;
}

void
ConsoleInstallTask::on_no_such_package_error(const NoSuchPackageError & e)
{
    output_stream() << endl;
    output_stream() << "Query error:" << endl;
    output_stream() << "  * " << e.backtrace("\n  * ");
    output_stream() << "Could not find '" << e.name() << "'.";

    if (want_suggestions())
    {
        output_stream() << " Looking for suggestions:" << endl;

        FuzzyCandidatesFinder f(*environment(), e.name(), filter::SupportsAction<InstallAction>());

        if (f.begin() == f.end())
            output_stream() << "No suggestions found." << endl;
        else
            output_stream() << "Suggestions:" << endl;

        for (FuzzyCandidatesFinder::CandidatesConstIterator c(f.begin()), c_end(f.end())
                 ; c != c_end ; ++c)
            output_stream() << "  * " << colour(cl_package_name, *c) << endl;
    }

    output_stream() << endl;
}

void
ConsoleInstallTask::on_all_masked_error(const AllMaskedError & e)
{
    try
    {
        std::shared_ptr<const PackageIDSequence> p(
                (*environment())[selection::AllVersionsSorted(
                    generator::Matches(e.query(), e.from_id(), { })
                    | filter::SupportsAction<InstallAction>())]);
        if (p->empty())
        {
            output_stream() << endl;
            output_stream() << "Query error:" << endl;
            output_stream() << "  * " << e.backtrace("\n  * ");
            output_stream() << "No versions of '" << e.query() << "' are available.";

            if (want_suggestions()
                    && e.query().tag() && simple_visitor_cast<const TargetDepTag>(*e.query().tag()))
            {
                output_stream() << " Looking for suggestions:" << endl;

                try
                {
                    FuzzyCandidatesFinder f(*environment(), stringify(e.query()), filter::SupportsAction<InstallAction>());

                    if (f.begin() == f.end())
                        output_stream() << "No suggestions found." << endl;
                    else
                        output_stream() << "Suggestions:" << endl;

                    for (FuzzyCandidatesFinder::CandidatesConstIterator c(f.begin()), c_end(f.end())
                            ; c != c_end ; ++c)
                        output_stream() << "  * " << colour(cl_package_name, *c) << endl;
                }
                catch (const PackageDepSpecError &)
                {
                    output_stream() << "Query too complicated or confusing to make suggestions." << endl;
                }
            }
        }
        else
        {
            output_stream() << endl;
            output_stream() << "Query error:" << endl;
            output_stream() << "  * " << e.backtrace("\n  * ");
            output_stream() << "All versions of '" << e.query() << "' are masked. Candidates are:" << endl;
            for (PackageIDSequence::ConstIterator pp(p->begin()), pp_end(p->end()) ;
                    pp != pp_end ; ++pp)
            {
                output_stream() << "    * " << colour(cl_package_name, **pp) << ": Masked by ";

                bool need_comma(false);
                for (PackageID::MasksConstIterator m((*pp)->begin_masks()), m_end((*pp)->end_masks()) ;
                        m != m_end ; ++m)
                {
                    if (need_comma)
                        output_stream() << ", ";

                    MaskDisplayer d(environment(), *pp, true);
                    (*m)->accept(d);
                    output_no_endl(d.result());

                    need_comma = true;
                }
                output_stream() << endl;
            }
        }
    }
    catch (...)
    {
        Log::get_instance()->message("console_install_task.on_all_masked_error.no_friendly", ll_warning, lc_context)
            << "Couldn't work out a friendly error message for mask reasons";
        throw e;
    }
}

void
ConsoleInstallTask::on_additional_requirements_not_met_error(const AdditionalRequirementsNotMetError & e)
{
    output_stream() << endl;
    output_stream() << "DepList additional requirements not met error:" << endl;
    output_stream() << "  * " << e.backtrace("\n  * ") << e.message() << endl;
    output_stream() << endl;
    if (e.query().additional_requirements_ptr())
    {
        output_stream() << "Unmet additional requirements are as follows:" << endl;
        for (AdditionalPackageDepSpecRequirements::ConstIterator i(e.query().additional_requirements_ptr()->begin()),
                i_end(e.query().additional_requirements_ptr()->end()) ;
                i != i_end ; ++i)
        {
            const std::pair<bool, std::string> r((*i)->requirement_met(environment(), 0, e.package_id(), e.from_package_id(), 0));
            if (r.first)
                continue;
            output_stream() << "    * " << r.second << endl;
        }
        output_stream() << endl;
    }
    output_stream() << "This error usually indicates that one of the packages you are trying to" << endl;
    output_stream() << "install requires that another package be built with particular USE flags" << endl;
    output_stream() << "enabled or disabled. You may be able to work around this restriction by" << endl;
    output_stream() << "adjusting your use.conf." << endl;
    output_stream() << endl;
}

void
ConsoleInstallTask::on_dep_list_error(const DepListError & e)
{
    output_stream() << endl;
    output_stream() << "Dependency error:" << endl;
    output_stream() << "  * " << e.backtrace("\n  * ") << e.message() << " ("
        << e.what() << ")" << endl;
    output_stream() << endl;
}

void
ConsoleInstallTask::on_had_both_package_and_set_targets_error(const HadBothPackageAndSetTargets &)
{
    output_stream() << endl;
    output_stream() << "Error: both package sets and packages were specified." << endl;
    output_stream() << endl;
    output_stream() << "Package sets (like 'system' and 'world') cannot be installed at the same time" << endl;
    output_stream() << "as ordinary packages." << endl;
}

void
ConsoleInstallTask::on_multiple_set_targets_specified(const MultipleSetTargetsSpecified &)
{
    output_stream() << endl;
    output_stream() << "Error: multiple package sets were specified." << endl;
    output_stream() << endl;
    output_stream() << "Package sets (like 'system' and 'world') must be installed individually," << endl;
    output_stream() << "without any other sets or packages." << endl;
}

void
ConsoleInstallTask::on_display_failure_summary_pre()
{
    output_heading("Summary of failures:");
}

void
ConsoleInstallTask::on_display_failure_summary_success(const DepListEntry &)
{
}

void
ConsoleInstallTask::on_display_failure_summary_failure(const DepListEntry & e)
{
    output_starred_item_no_endl("");
    output_stream() << colour(cl_package_name, *e.package_id()) << ": " << colour(cl_error, "failure");
    output_endl();
}

void
ConsoleInstallTask::on_display_failure_summary_skipped_unsatisfied(const DepListEntry & e,
        const PackageDepSpec & spec)
{
    output_starred_item_no_endl("");
    output_stream() << colour(cl_package_name, *e.package_id()) << ": skipped (dependency '"
        << spec << "' unsatisfied)";
    output_endl();
}

void
ConsoleInstallTask::on_display_failure_summary_skipped_dependent(const DepListEntry & e,
        const std::shared_ptr<const PackageID> & id)
{
    output_starred_item_no_endl("");
    output_stream() << colour(cl_package_name, *e.package_id()) << ": skipped (dependent upon '"
        << *id << "')";
    output_endl();
}

void
ConsoleInstallTask::on_display_failure_summary_totals(const int total, const int successes,
        const int skipped, const int failures, const int unreached)
{
    std::ostringstream s;
    s << "Total: " << total << render_plural(total, " package", " packages");
    s << ", " << successes << render_plural(successes, " success", " successes");
    s << ", " << skipped << render_plural(skipped, " skipped", " skipped");
    s << ", " << failures << render_plural(failures, " failure", " failures");
    s << ", " << unreached << render_plural(failures, " unreached", " unreached");

    output_endl();
    output_unstarred_item(s.str());
}

void
ConsoleInstallTask::on_display_failure_summary_post()
{
    show_resume_command();
}

std::string
ConsoleInstallTask::make_x_of_y(const int x, const int y, const int s, const int f)
{
    std::string result(stringify(x) + " of " + stringify(y));
    if (s > 0)
        result.append(", " + stringify(s) + " skipped");
    if (f > 0)
        result.append(", " + stringify(f) + " failed");
    return result;
}

void
ConsoleInstallTask::show_resume_command() const
{
    show_resume_command("");
}

void
ConsoleInstallTask::show_resume_command(const std::string & resume_command_template) const
{
    if (_resolution_finished)
    {
        std::string resume_command(make_resume_command(true));
        if (resume_command.empty())
            return;

        if (! resume_command_template.empty())
        {
            std::string file_name(resume_command_template);
            int fd;
            if (std::string::npos == file_name.find("XXXXXX"))
                fd = open(file_name.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            else
            {
                char * resume_template = strdup(file_name.c_str());
                fd = mkstemp(resume_template);
                file_name = resume_template;
                std::free(resume_template);
            }

            if (-1 != fd)
            {
                ::fchmod(fd, 0644);
                SafeOFStream resume_command_file(fd, true);
                resume_command_file << resume_command << endl;

                if (resume_command_file)
                {
                    cerr << endl;
                    cerr << "Resume command saved to file: " << file_name;
                    cerr << endl;
                }
                else
                {
                    cerr << "Resume command NOT saved to file: " << file_name << " due to error "
                        << std::strerror(errno) << endl;
                    cerr << "Resume command: " << file_name << endl;
                }
            }
            else
            {
                cerr << "Resume command NOT saved to file: " << file_name << " due to error "
                    << std::strerror(errno) << endl;
                cerr << "Resume command: " << resume_command << endl;
            }
        }
        else
        {
            cerr << endl;
            cerr << "Resume command: " << resume_command << endl;
        }
    }
}

void
ConsoleInstallTask::on_installed_paludis()
{
    std::string r(stringify(environment()->preferred_root_key()->value()));
    std::string exec_mode(getenv_with_default("PALUDIS_EXEC_PALUDIS", ""));

    if ("always" != exec_mode)
    {
        if ("never" == exec_mode)
            return;
        else if (! (r.empty() || r == "/"))
            return;
    }

    std::string resume_command(make_resume_command(false));
    if (resume_command.empty())
        return;

    output_heading("Paludis has just upgraded Paludis");
    output_starred_item("Using '" + resume_command + "' to start a new Paludis instance...");
    output_endl();

    execl("/bin/sh", "sh", "-c", resume_command.c_str(), static_cast<const char *>(0));
}

HookResult
ConsoleInstallTask::perform_hook(const Hook & hook, const std::shared_ptr<OutputManager> & optional_output_manager)
{
    std::string resume_command(make_resume_command(true));
    if (resume_command.empty())
        return InstallTask::perform_hook(hook, optional_output_manager);
    return InstallTask::perform_hook(hook("RESUME_COMMAND", resume_command), optional_output_manager);
}

void
ConsoleInstallTask::on_phase_skip(const std::shared_ptr<OutputManager> & output_manager, const std::string & phase)
{
    output_manager->stdout_stream() << "+++ Skipping phase '" + phase + "' as instructed" << endl;
}

void
ConsoleInstallTask::on_phase_abort(const std::shared_ptr<OutputManager> & output_manager, const std::string & phase)
{
    output_manager->stdout_stream() << "+++ Aborting at phase '" + phase + "' as instructed" << endl;
}

void
ConsoleInstallTask::on_phase_skip_until(const std::shared_ptr<OutputManager> & output_manager, const std::string & phase)
{
    output_manager->stdout_stream() << "+++ Skipping phase '" + phase + "' as instructed since it is before a start phase" << endl;
}

void
ConsoleInstallTask::on_phase_proceed_conditionally(const std::shared_ptr<OutputManager> & output_manager, const std::string & phase)
{
    output_manager->stdout_stream() << "+++ Executing phase '" + phase + "' as instructed" << endl;
}

void
ConsoleInstallTask::on_phase_proceed_unconditionally(const std::shared_ptr<OutputManager> &, const std::string &)
{
}

