/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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
#include "colour_formatter.hh"
#include "mask_displayer.hh"

#include <paludis/util/log.hh>
#include <paludis/util/pretty_print.hh>
#include <paludis/util/sr.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/join.hh>
#include <paludis/util/set-impl.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/util/fd_output_stream.hh>
#include <paludis/util/system.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/kc.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/query.hh>
#include <paludis/action.hh>
#include <paludis/repository.hh>
#include <paludis/package_database.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/mask.hh>
#include <paludis/hook.hh>
#include <paludis/fuzzy_finder.hh>
#include <paludis/user_dep_spec.hh>

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

using namespace paludis;
using std::cout;
using std::cerr;
using std::endl;

#include <src/output/console_install_task-sr.cc>

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
        std::exit(EXIT_FAILURE);
    }
}

bool
UseDescriptionComparator::operator() (const UseDescription & lhs, const UseDescription & rhs) const
{
    if (lhs.flag < rhs.flag)
        return true;
    if (lhs.flag > rhs.flag)
        return false;

    if (lhs.package_id->name() < rhs.package_id->name())
        return true;
    if (lhs.package_id->name() > rhs.package_id->name())
        return false;

    if (lhs.package_id->version() < rhs.package_id->version())
        return true;
    if (lhs.package_id->version() < rhs.package_id->version())
        return false;

    if (lhs.package_id->repository()->name().data() < rhs.package_id->repository()->name().data())
        return true;

    return false;
}

ConsoleInstallTask::ConsoleInstallTask(Environment * const env,
        const DepListOptions & options,
        tr1::shared_ptr<const DestinationsSet> d) :
    InstallTask(env, options, d),
    _download_size(0),
    _all_tags(new Set<DepTagEntry>),
    _all_use_descriptions(new Set<UseDescription, UseDescriptionComparator>),
    _all_expand_prefixes(new UseFlagNameSet),
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
ConsoleInstallTask::try_to_set_targets_from_user_specs(const tr1::shared_ptr<const Sequence<std::string> > & s)
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

            FuzzyCandidatesFinder f(*environment(), e.name(), query::SupportsAction<InstallAction>() & query::NotMasked());

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

void
ConsoleInstallTask::on_build_deplist_pre()
{
    output_activity_start_message("Building dependency list");
    output_xterm_title("Building dependency list");
}

void
ConsoleInstallTask::on_build_deplist_post()
{
    output_activity_end_message();
    _resolution_finished = true;
}

void
ConsoleInstallTask::on_build_cleanlist_pre(const DepListEntry & d)
{
    output_heading("Cleaning stale versions after installing " + stringify(*d.package_id));
}

void
ConsoleInstallTask::on_build_cleanlist_post(const DepListEntry &)
{
}

void
ConsoleInstallTask::on_clean_all_pre(const DepListEntry & d,
        const PackageIDSequence & c)
{
    display_clean_all_pre_list_start(d, c);

    using namespace tr1::placeholders;
    std::for_each(indirect_iterator(c.begin()), indirect_iterator(c.end()),
            tr1::bind(tr1::mem_fn(&ConsoleInstallTask::display_one_clean_all_pre_list_entry), this, _1));

    display_clean_all_pre_list_end(d, c);
}

void
ConsoleInstallTask::on_no_clean_needed(const DepListEntry &)
{
    output_starred_item("No cleaning required");
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
ConsoleInstallTask::on_clean_all_post(const DepListEntry &,
        const PackageIDSequence &)
{
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

    display_merge_list_post_use_descriptions("");
    for (UseFlagNameSet::ConstIterator f(_all_expand_prefixes->begin()),
            f_end(_all_expand_prefixes->end()) ; f != f_end ; ++f)
        display_merge_list_post_use_descriptions(stringify(*f));
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
        switch (d.kind)
        {
            case dlk_provided:
            case dlk_virtual:
            case dlk_already_installed:
                if (! want_full_install_reasons())
                    return;
                m = unimportant_entry;
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

    tr1::shared_ptr<RepositoryName> repo;
    if (d.destination)
        repo.reset(new RepositoryName(d.destination->name()));

    tr1::shared_ptr<const PackageIDSequence> existing_repo(environment()->package_database()->
            query(query::Matches(repo ?
                    make_package_dep_spec().package(d.package_id->name()).repository(*repo) :
                    make_package_dep_spec().package(d.package_id->name())),
                qo_order_by_version));

    tr1::shared_ptr<const PackageIDSequence> existing_slot_repo(environment()->package_database()->
            query(query::Matches(repo ?
                    make_package_dep_spec()
                        .package(d.package_id->name())
                        .slot_requirement(make_shared_ptr(new UserSlotExactRequirement(d.package_id->slot())))
                        .repository(*repo) :
                    make_package_dep_spec()
                        .package(d.package_id->name())
                        .slot_requirement(make_shared_ptr(new UserSlotExactRequirement(d.package_id->slot())))
                        ),
                qo_order_by_version));

    display_merge_list_entry_start(d, m);
    display_merge_list_entry_package_name(d, m);
    display_merge_list_entry_repository(d, m);

    if (d.package_id->virtual_for_key())
        display_merge_list_entry_for(*d.package_id->virtual_for_key()->value(), m);

    display_merge_list_entry_slot(d, m);

    display_merge_list_entry_status_and_update_counts(d, existing_repo, existing_slot_repo, m);
    display_merge_list_entry_non_package_tags(d, m);
    if (! want_compact())
        display_merge_list_entry_package_tags(d, m);
    display_merge_list_entry_description(d, existing_repo, existing_slot_repo, m);
    display_merge_list_entry_use(d, existing_repo, existing_slot_repo, m);
    display_merge_list_entry_distsize(d, m);
    if (want_compact())
        display_merge_list_entry_package_tags(d, m);
    display_merge_list_entry_end(d, m);

    if (d.kind == dlk_masked)
        display_merge_list_entry_mask_reasons(d);
}

void
ConsoleInstallTask::on_pretend_all_pre()
{
}

void
ConsoleInstallTask::on_pretend_pre(const DepListEntry & d)
{
    std::string m("Pretending for " + stringify(*d.package_id));
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
    std::string m("(" + make_x_of_y(x, y, s, f) + ") Fetching " + stringify(*d.package_id));

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
    std::string m("(" + make_x_of_y(x, y, s, f) + ") Installing " + stringify(*d.package_id));

    output_heading(m);
    output_xterm_title(m);
}

void
ConsoleInstallTask::on_skip_unsatisfied(const DepListEntry & d, const PackageDepSpec & spec,
        const int x, const int y, const int s, const int f)
{
    std::string m("(" + make_x_of_y(x, y, s, f) + ") Skipping " + stringify(*d.package_id) +
            " (unsatisfied '" + stringify(spec) + "')");

    output_heading(m);
}

void
ConsoleInstallTask::on_skip_dependent(const DepListEntry & d, const tr1::shared_ptr<const PackageID> & id,
        const int x, const int y, const int s, const int f)
{
    std::string m("(" + make_x_of_y(x, y, s, f) + ") Skipping " + stringify(*d.package_id) +
            " (dependent upon '" + stringify(*id) + "')");

    output_heading(m);
}

void
ConsoleInstallTask::on_skip_already_done(const DepListEntry & d,
        const int x, const int y, const int s, const int f)
{
    std::string m("(" + make_x_of_y(x, y, s, f) + ") Skipping " + stringify(*d.package_id) +
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
    output_xterm_title("(" + make_x_of_y(x, y, s, f) + ") Failed install of " + stringify(*d.package_id));
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
ConsoleInstallTask::display_clean_all_pre_list_start(const DepListEntry &,
        const PackageIDSequence &)
{
}

void
ConsoleInstallTask::display_one_clean_all_pre_list_entry(
        const PackageID & c)
{
    output_starred_item(render_as_package_name(stringify(c)));
}

void
ConsoleInstallTask::display_clean_all_pre_list_end(const DepListEntry &,
        const PackageIDSequence &)
{
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
            s << ", at most " << pretty_print_bytes(get_download_size()) << " to download";
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
    for (Set<DepTagEntry>::ConstIterator a(all_tags()->begin()),
            a_end(all_tags()->end()) ; a != a_end ; ++a)
        tag_categories.insert(a->tag->category());

    display_tag_summary_start();

    for (std::set<std::string>::iterator cat(tag_categories.begin()),
            cat_end(tag_categories.end()) ; cat != cat_end ; ++cat)
    {
        tr1::shared_ptr<const DepTagCategory> c(DepTagCategoryMaker::get_instance()->
                find_maker(*cat)());

        if (! c->visible())
            continue;

        display_tag_summary_tag_title(*c);
        display_tag_summary_tag_pre_text(*c);

        for (Set<DepTagEntry>::ConstIterator t(all_tags()->begin()),
                t_end(all_tags()->end()) ; t != t_end ; ++t)
        {
            if (t->tag->category() != *cat)
                continue;
            display_tag_summary_tag(t->tag);
        }

        display_tag_summary_tag_post_text(*c);
    }

    display_tag_summary_end();
}

void
ConsoleInstallTask::display_merge_list_post_use_descriptions(const std::string & prefix)
{
    if (! want_use_summary())
        return;

    bool started(false);
    UseFlagName old_flag("OFTEN_NOT_BEEN_ON_BOATS");

    tr1::shared_ptr<Set<UseDescription, UseDescriptionComparator> > group(
            new Set<UseDescription, UseDescriptionComparator>);
    for (Set<UseDescription, UseDescriptionComparator>::ConstIterator i(all_use_descriptions()->begin()),
            i_end(all_use_descriptions()->end()) ; i != i_end ; ++i)
    {
        switch (i->state)
        {
            case uds_new:
                if (! want_new_use_flags())
                    continue;
                break;

            case uds_all:
                if (! want_unchanged_use_flags())
                    continue;
                break;

            case uds_changed:
                if (! want_changed_use_flags())
                    continue;
                break;
        }

        if (prefix.empty())
        {
            bool prefixed(false);
            for (UseFlagNameSet::ConstIterator f(_all_expand_prefixes->begin()),
                    f_end(_all_expand_prefixes->end()) ; f != f_end && ! prefixed ; ++f)
                if (stringify(*f).length() < stringify(i->flag).length())
                    if (0 == stringify(i->flag).compare(0, stringify(*f).length(), stringify(*f)))
                        prefixed = true;

            if (prefixed)
                continue;
        }
        else
        {
            if (stringify(i->flag).length() <= prefix.length())
                continue;

            if (0 != stringify(i->flag).compare(0, prefix.length(), prefix))
                continue;
        }

        if (! started)
        {
            display_use_summary_start(prefix);
            started = true;
        }

        if (old_flag != i->flag)
        {
            if (! group->empty())
                display_use_summary_flag(prefix, group->begin(), group->end());
            old_flag = i->flag;
            group.reset(new Set<UseDescription, UseDescriptionComparator>);
        }

        group->insert(*i);
    }

    if (! group->empty())
        display_use_summary_flag(prefix, group->begin(), group->end());

    if (started)
        display_use_summary_end();
}

void
ConsoleInstallTask::display_use_summary_start(const std::string & prefix)
{
    if (! prefix.empty())
        output_heading(prefix + ":");
    else
        output_heading("Use flags:");
}

void
ConsoleInstallTask::display_use_summary_flag(const std::string & prefix,
        Set<UseDescription, UseDescriptionComparator>::ConstIterator i,
        Set<UseDescription, UseDescriptionComparator>::ConstIterator i_end)
{
    Log::get_instance()->message(ll_debug, lc_context) << "display_use_summary_flag: prefix is '" << prefix
        << "', i->flag is '" << i->flag << "', i->package_id is '" << *i->package_id << "', i->state is '" << i->state
        << "', i->description is '" << i->description << "'";

    if (next(i) == i_end)
    {
        std::ostringstream s;
        s << std::left << std::setw(30) << (render_as_tag(stringify(i->flag).substr(prefix.empty() ? 0 : prefix.length() + 1)) + ": ");
        s << i->description;
        output_starred_item(s.str());
    }
    else
    {
        bool all_same(true);
        for (Set<UseDescription, UseDescriptionComparator>::ConstIterator j(next(i)) ; all_same && j != i_end ; ++j)
            if (j->description != i->description)
                all_same = false;

        if (all_same)
        {
            std::ostringstream s;
            s << std::left << std::setw(30) << (render_as_tag(stringify(i->flag).substr(prefix.empty() ? 0 : prefix.length() + 1)) + ": ");
            s << i->description;
            output_starred_item(s.str());
        }
        else
        {
            output_starred_item(render_as_tag(
                        stringify(i->flag).substr(prefix.empty() ? 0 : prefix.length() + 1)) + ":");

            for ( ; i != i_end ; ++i)
            {
                std::ostringstream s;
                s << i->description << " (for " << render_as_package_name(stringify(*i->package_id)) << ")";
                output_starred_item(s.str(), 1);
            }
        }
    }
}

void
ConsoleInstallTask::display_use_summary_end()
{
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
ConsoleInstallTask::display_tag_summary_tag(tr1::shared_ptr<const DepTag> t)
{
    tr1::shared_ptr<DepTagSummaryDisplayer> displayer(make_dep_tag_summary_displayer());
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
    else if (tag.short_text() == "ununused")
        desc = ":           Packages that have been explicitly marked as not unused";
    else if (tag.short_text() == "everything")
        desc = ":      All installed packages";
    else if (tag.short_text() == "system")
        desc = ":          Packages that are part of the base system";

    task()->output_starred_item(task()->render_as_tag(tag.short_text()) + desc);
}

void
ConsoleInstallTask::display_merge_list_entry_start(const DepListEntry & e, const DisplayMode)
{
    switch (e.kind)
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
            output_no_endl(render_as_package_name(stringify(d.package_id->name())));
            break;

        case unimportant_entry:
            output_no_endl(render_as_unimportant(stringify(d.package_id->name())));
            break;

        case error_entry:
            output_no_endl(render_as_error(stringify(d.package_id->name())));
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
    // XXX fix this once the new resolver's in
    tr1::shared_ptr<const PackageIDSequence> inst(
        environment()->package_database()->query(
            query::Matches(make_package_dep_spec()
                           .package(d.package_id->name())
                           .slot_requirement(make_shared_ptr(new UserSlotExactRequirement(d.package_id->slot())))) &
            query::InstalledAtRoot(environment()->root()),
            qo_best_version_only));
    bool changed(normal_entry == m &&
                 ! inst->empty() && (*inst->begin())->source_origin_key() &&
                 (*inst->begin())->source_origin_key()->value() !=
                 stringify(d.package_id->repository()->name()));

    if (changed || environment()->package_database()->favourite_repository() != d.package_id->repository()->name())
        output_no_endl("::" + stringify(d.package_id->repository()->name()));
    if (changed)
        output_no_endl(" (previously ::" + (*inst->begin())->source_origin_key()->value() + ")");
}

void
ConsoleInstallTask::display_merge_list_entry_slot(const DepListEntry & d, const DisplayMode m)
{
    if (d.package_id->slot() == SlotName("0"))
        return;

    switch (m)
    {
        case normal_entry:
        case suggested_entry:
            output_no_endl(render_as_slot_name(" :" + stringify(d.package_id->slot())));
            break;

        case unimportant_entry:
            output_no_endl(render_as_unimportant(" :" + stringify(d.package_id->slot())));
            break;

        case error_entry:
            output_no_endl(render_as_slot_name(" :" + stringify(d.package_id->slot())));
            break;
    }
}

void
ConsoleInstallTask::display_merge_list_entry_status_and_update_counts(const DepListEntry & d,
        tr1::shared_ptr<const PackageIDSequence> existing_repo,
        tr1::shared_ptr<const PackageIDSequence> existing_slot_repo,
        const DisplayMode m)
{
    switch (m)
    {
        case unimportant_entry:
            if (d.kind == dlk_provided)
                output_no_endl(render_as_unimportant(" [provided " +
                            stringify(d.package_id->canonical_form(idcf_version)) + "]"));
            else
                output_no_endl(render_as_unimportant(" [- " +
                            stringify(d.package_id->canonical_form(idcf_version)) + "]"));
            break;

        case suggested_entry:
            output_no_endl(render_as_update_mode(" [suggestion " +
                        stringify(d.package_id->canonical_form(idcf_version)) + "]"));
            set_count<suggested_count>(count<suggested_count>() + 1);
            break;

        case normal_entry:
            {
                output_no_endl(render_as_update_mode(" ["));

                std::string destination_str;
                tr1::shared_ptr<const DestinationsSet> default_destinations(environment()->default_destinations());
                if (default_destinations->end() == default_destinations->find(d.destination))
                    destination_str = " ::" + stringify(d.destination->name());

                if (existing_repo->empty())
                {
                    output_no_endl(render_as_update_mode("N " + stringify(d.package_id->canonical_form(idcf_version) + destination_str)));
                    set_count<new_count>(count<new_count>() + 1);
                    set_count<max_count>(count<max_count>() + 1);
                }
                else if (existing_slot_repo->empty())
                {
                    output_no_endl(render_as_update_mode("S " + d.package_id->canonical_form(idcf_version) + destination_str));
                    set_count<new_slot_count>(count<new_slot_count>() + 1);
                    set_count<max_count>(count<max_count>() + 1);
                }
                else if ((*existing_slot_repo->last())->version() < d.package_id->version())
                {
                    output_no_endl(render_as_update_mode("U " +
                            stringify((*existing_slot_repo->last())->canonical_form(idcf_version)) + " -> " +
                            stringify(d.package_id->canonical_form(idcf_version))));
                    set_count<upgrade_count>(count<upgrade_count>() + 1);
                    set_count<max_count>(count<max_count>() + 1);
                }
                else if ((*existing_slot_repo->last())->version() > d.package_id->version())
                {
                    output_no_endl(render_as_update_mode("D " +
                            stringify((*existing_slot_repo->last())->canonical_form(idcf_version)) + " -> " +
                            stringify(d.package_id->canonical_form(idcf_version))));
                    set_count<downgrade_count>(count<downgrade_count>() + 1);
                    set_count<max_count>(count<max_count>() + 1);
                }
                else
                {
                    output_no_endl(render_as_update_mode("R " + stringify(d.package_id->canonical_form(idcf_version)) +
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
                switch (d.kind)
                {
                    case dlk_masked:
                        output_no_endl(render_as_update_mode(" [! masked]"));
                        continue;

                    case dlk_block:
                        output_no_endl(render_as_update_mode(" [! blocking]"));
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
ConsoleInstallTask::display_merge_list_entry_description(const DepListEntry & d,
        tr1::shared_ptr<const PackageIDSequence> existing_slot_repo,
        tr1::shared_ptr<const PackageIDSequence>,
        const DisplayMode m)
{
    if ((! d.package_id->short_description_key()) || d.package_id->short_description_key()->value().empty())
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
                output_no_endl(" \"" + d.package_id->short_description_key()->value() + "\"");
            else
            {
                output_endl();
                output_no_endl("    \"" + d.package_id->short_description_key()->value() + "\"");
            }
            break;
    }
}

void
ConsoleInstallTask::_add_descriptions(tr1::shared_ptr<const UseFlagNameSet> c,
        const tr1::shared_ptr<const PackageID> & p, UseDescriptionState s)
{
    for (UseFlagNameSet::ConstIterator f(c->begin()), f_end(c->end()) ;
            f != f_end ; ++f)
    {
        std::string d;
        const RepositoryUseInterface * const i((*p->repository())[k::use_interface()]);

        if (i)
            d = i->describe_use_flag(*f, *p);

        UseDescription e(UseDescription::create()
                .flag(*f)
                .state(s)
                .package_id(p)
                .description(d));

        Set<UseDescription, UseDescriptionComparator>::ConstIterator x(_all_use_descriptions->find(e));
        if (_all_use_descriptions->end() == x)
            _all_use_descriptions->insert(e);
        else
        {
            if (x->state < e.state)
            {
                _all_use_descriptions->erase(e);
                _all_use_descriptions->insert(e);
            }
        }
    }
}

void
ConsoleInstallTask::display_merge_list_entry_use(const DepListEntry & d,
        tr1::shared_ptr<const PackageIDSequence> existing_repo,
        tr1::shared_ptr<const PackageIDSequence> existing_slot_repo,
        const DisplayMode m)
{
    if (normal_entry != m && suggested_entry != m)
        return;

    if ((! d.package_id->iuse_key()) || d.package_id->iuse_key()->value()->empty())
        return;

    if (want_compact())
        output_no_endl(" ");
    else
    {
        output_endl();
        output_no_endl("    ");
    }

    tr1::shared_ptr<const PackageID> old_id;
    if (! existing_slot_repo->empty())
        old_id = *existing_slot_repo->last();
    else if (! existing_repo->empty())
        old_id = *existing_repo->last();

    ColourFormatter formatter(old_id ? false : true);
    if (old_id)
        output_stream() << d.package_id->iuse_key()->pretty_print_flat_with_comparison(environment(), old_id, formatter);
    else
        output_stream() << d.package_id->iuse_key()->pretty_print_flat(formatter);

    _add_descriptions(formatter.seen_new_use_flag_names(), d.package_id, uds_new);
    _add_descriptions(formatter.seen_changed_use_flag_names(), d.package_id, uds_changed);
    _add_descriptions(formatter.seen_use_flag_names(), d.package_id, uds_all);
    std::copy(formatter.seen_use_expand_prefixes()->begin(), formatter.seen_use_expand_prefixes()->end(),
            _all_expand_prefixes->inserter());
}

void
ConsoleInstallTask::display_merge_list_entry_distsize(const DepListEntry & d,
        const DisplayMode m)
{
    if (normal_entry != m && suggested_entry != m)
        return;

    if (! d.package_id->size_of_download_required_key() || d.package_id->size_of_download_required_key()->value() == 0)
        return;

    if (want_compact())
        output_no_endl(" ");
    else
    {
        output_endl();
        output_no_endl("    ");
    }

    output_stream() << d.package_id->size_of_download_required_key()->pretty_print()
        << " to download";
    set_download_size(get_download_size() + d.package_id->size_of_download_required_key()->value());
}

void
ConsoleInstallTask::display_merge_list_entry_non_package_tags(const DepListEntry & d, const DisplayMode m)
{
    if (d.tags->empty())
        return;

    std::string tag_titles;
    std::stringstream s;

    for (Set<DepTagEntry>::ConstIterator
            tag(d.tags->begin()),
            tag_end(d.tags->end()) ;
            tag != tag_end ; ++tag)
    {
        if (tag->tag->category() == "dependency")
            continue;

        all_tags()->insert(*tag);

        tr1::shared_ptr<EntryDepTagDisplayer> displayer(make_entry_dep_tag_displayer());
        tag->tag->accept(*displayer.get());
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
    if (d.tags->empty())
        return;

    std::stringstream s;

    if (! want_install_reasons())
        if (d.kind != dlk_block)
            return;

    std::string deps;
    std::set<std::string> dependents, unsatisfied_dependents;
    unsigned c(0), max_c(want_full_install_reasons() ? std::numeric_limits<long>::max() : 3);

    for (Set<DepTagEntry>::ConstIterator
            tag(d.tags->begin()),
            tag_end(d.tags->end()) ;
            tag != tag_end ; ++tag)
    {
        if (tag->tag->category() != "dependency")
            continue;

        tr1::shared_ptr<const PackageDepSpec> spec(
            tr1::static_pointer_cast<const DependencyDepTag>(tag->tag)->dependency());
        if (d.kind != dlk_masked && d.kind != dlk_block && environment()->package_database()->query(
                query::Matches(*spec) &
                query::SupportsAction<InstalledAction>(),
                qo_whatever)->empty())
            unsatisfied_dependents.insert(tag->tag->short_text());
        else
            dependents.insert(tag->tag->short_text());
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

tr1::shared_ptr<DepTagSummaryDisplayer>
ConsoleInstallTask::make_dep_tag_summary_displayer()
{
    return tr1::shared_ptr<DepTagSummaryDisplayer>(new DepTagSummaryDisplayer(this));
}

tr1::shared_ptr<EntryDepTagDisplayer>
ConsoleInstallTask::make_entry_dep_tag_displayer()
{
    return tr1::shared_ptr<EntryDepTagDisplayer>(new EntryDepTagDisplayer());
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

    for (PackageID::MasksConstIterator m(e.package_id->begin_masks()), m_end(e.package_id->end_masks()) ;
            m != m_end ; ++m)
    {
        if (need_comma)
            output_no_endl(", ");
        MaskDisplayer d(environment(), e.package_id, true);
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
ConsoleInstallTask::on_install_action_error(const InstallActionError & e)
{
    output_stream() << endl;
    output_stream() << "Install error:" << endl;
    output_stream() << "  * " << e.backtrace("\n  * ");
    output_stream() << e.message() << endl;
    output_stream() << endl;
    output_stream() << endl;
}

void
ConsoleInstallTask::on_fetch_action_error(const FetchActionError & e)
{
    output_stream() << endl;
    output_stream() << "Fetch error:" << endl;
    output_stream() << "  * " << e.backtrace("\n  * ");
    output_stream() << e.message() << endl;
    output_stream() << endl;

    if (e.failures())
    {
        for (Sequence<FetchActionFailure>::ConstIterator f(e.failures()->begin()), f_end(e.failures()->end()) ;
                f != f_end ; ++f)
        {
            output_stream() << "  * File '" << (*f)[k::target_file()] << "': ";

            bool need_comma(false);
            if ((*f)[k::requires_manual_fetching()])
            {
                output_stream() << "requires manual fetching";
                need_comma = true;
            }

            if ((*f)[k::failed_automatic_fetching()])
            {
                if (need_comma)
                    output_stream() << ", ";
                output_stream() << "failed automatic fetching";
                need_comma = true;
            }

            if (! (*f)[k::failed_integrity_checks()].empty())
            {
                if (need_comma)
                    output_stream() << ", ";
                output_stream() << "failed integrity checks: " << (*f)[k::failed_integrity_checks()];
                need_comma = true;
            }

            output_stream() << endl;
        }
    }

    output_stream() << endl;
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

        FuzzyCandidatesFinder f(*environment(), e.name(),
                query::SupportsAction<InstallAction>() & query::NotMasked());

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
        tr1::shared_ptr<const PackageIDSequence> p(
                environment()->package_database()->query(
                    query::Matches(e.query()) & query::SupportsAction<InstallAction>(), qo_order_by_version));
        if (p->empty())
        {
            output_stream() << endl;
            output_stream() << "Query error:" << endl;
            output_stream() << "  * " << e.backtrace("\n  * ");
            output_stream() << "No versions of '" << e.query() << "' are available.";

            if (want_suggestions()
                    && e.query().tag() && visitor_cast<const TargetDepTag>(*e.query().tag()))
            {
                output_stream() << " Looking for suggestions:" << endl;

                FuzzyCandidatesFinder f(*environment(), stringify(e.query()),
                        query::SupportsAction<InstallAction>() & query::NotMasked());

                if (f.begin() == f.end())
                    output_stream() << "No suggestions found." << endl;
                else
                    output_stream() << "Suggestions:" << endl;

                for (FuzzyCandidatesFinder::CandidatesConstIterator c(f.begin()), c_end(f.end())
                        ; c != c_end ; ++c)
                    output_stream() << "  * " << colour(cl_package_name, *c) << endl;
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
        Log::get_instance()->message(ll_warning, lc_context, "Couldn't work out a friendly error message for mask reasons");
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
        output_stream() << "Additional requirements are as follows:" << endl;
        for (AdditionalPackageDepSpecRequirements::ConstIterator i(e.query().additional_requirements_ptr()->begin()),
                i_end(e.query().additional_requirements_ptr()->end()) ;
                i != i_end ; ++i)
            output_stream() << "    * " << (*i)->as_human_string() << endl;
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
    output_stream() << colour(cl_package_name, *e.package_id) << ": " << colour(cl_error, "failure");
    output_endl();
}

void
ConsoleInstallTask::on_display_failure_summary_skipped_unsatisfied(const DepListEntry & e,
        const PackageDepSpec & spec)
{
    output_starred_item_no_endl("");
    output_stream() << colour(cl_package_name, *e.package_id) << ": skipped (dependency '"
        << spec << "' unsatisfied)";
    output_endl();
}

void
ConsoleInstallTask::on_display_failure_summary_skipped_dependent(const DepListEntry & e,
        const tr1::shared_ptr<const PackageID> & id)
{
    output_starred_item_no_endl("");
    output_stream() << colour(cl_package_name, *e.package_id) << ": skipped (dependent upon '"
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
                fd = open(file_name.c_str(), O_WRONLY | O_CREAT | O_TRUNC);
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
                FDOutputStream resume_command_file(fd);
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
    std::string r(stringify(environment()->root()));
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
ConsoleInstallTask::perform_hook(const Hook & hook) const
{
    std::string resume_command(make_resume_command(true));
    if (resume_command.empty())
        return InstallTask::perform_hook(hook);
    return InstallTask::perform_hook(hook("RESUME_COMMAND", resume_command));
}

