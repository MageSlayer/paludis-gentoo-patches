/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/util/sr.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/join.hh>
#include <paludis/util/set-impl.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/query.hh>
#include <paludis/action.hh>
#include <paludis/repository.hh>
#include <paludis/package_database.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/mask.hh>

#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>

#include <algorithm>
#include <set>
#include <iostream>
#include <iomanip>
#include <limits>

using namespace paludis;
using std::endl;

#include <src/output/console_install_task-sr.cc>

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
    _all_tags(new Set<DepTagEntry>),
    _all_use_descriptions(new Set<UseDescription, UseDescriptionComparator>),
    _all_expand_prefixes(new UseFlagNameSet)
{
    std::fill_n(_counts, static_cast<int>(last_count), 0);
}

void
ConsoleInstallTask::on_build_deplist_pre()
{
    output_activity_start_message("Building dependency list");
}

void
ConsoleInstallTask::on_build_deplist_post()
{
    output_activity_end_message();
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
    for (UseFlagNameSet::Iterator f(_all_expand_prefixes->begin()),
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

    tr1::shared_ptr<RepositoryName> repo;
    if (d.destination)
        repo.reset(new RepositoryName(d.destination->name()));

    tr1::shared_ptr<const PackageIDSequence> existing_repo(environment()->package_database()->
            query(query::Matches(PackageDepSpec(
                        tr1::shared_ptr<QualifiedPackageName>(new QualifiedPackageName(d.package_id->name())),
                        tr1::shared_ptr<CategoryNamePart>(),
                        tr1::shared_ptr<PackageNamePart>(),
                        tr1::shared_ptr<VersionRequirements>(),
                        vr_and,
                        tr1::shared_ptr<SlotName>(),
                        repo)),
                qo_order_by_version));

    tr1::shared_ptr<const PackageIDSequence> existing_slot_repo(environment()->package_database()->
            query(query::Matches(PackageDepSpec(
                        tr1::shared_ptr<QualifiedPackageName>(new QualifiedPackageName(d.package_id->name())),
                        tr1::shared_ptr<CategoryNamePart>(),
                        tr1::shared_ptr<PackageNamePart>(),
                        tr1::shared_ptr<VersionRequirements>(),
                        vr_and,
                        tr1::shared_ptr<SlotName>(new SlotName(d.package_id->slot())),
                        repo)),
                qo_order_by_version));

    display_merge_list_entry_start(d, m);
    display_merge_list_entry_package_name(d, m);
    display_merge_list_entry_version(d, m);
    display_merge_list_entry_repository(d, m);

    if (d.package_id->virtual_for_key())
        display_merge_list_entry_for(*d.package_id->virtual_for_key()->value(), m);

    display_merge_list_entry_slot(d, m);

    display_merge_list_entry_status_and_update_counts(d, existing_repo, existing_slot_repo, m);
    display_merge_list_entry_use(d, existing_repo, existing_slot_repo, m);
    display_merge_list_entry_tags(d, m);
    display_merge_list_entry_end(d, m);

    if (d.kind == dlk_masked)
        display_merge_list_entry_mask_reasons(d);
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
    for (Set<DepTagEntry>::Iterator a(all_tags()->begin()),
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

        for (Set<DepTagEntry>::Iterator t(all_tags()->begin()),
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
    for (Set<UseDescription, UseDescriptionComparator>::Iterator i(all_use_descriptions()->begin()),
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
            for (UseFlagNameSet::Iterator f(_all_expand_prefixes->begin()),
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
        Set<UseDescription, UseDescriptionComparator>::Iterator i,
        Set<UseDescription, UseDescriptionComparator>::Iterator i_end)
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
        for (Set<UseDescription, UseDescriptionComparator>::Iterator j(next(i)) ; all_same && j != i_end ; ++j)
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
ConsoleInstallTask::display_merge_list_entry_version(const DepListEntry & d, const DisplayMode)
{
    if ((VersionSpec("0") != d.package_id->version()) ||
            CategoryNamePart("virtual") != d.package_id->name().category)
        output_no_endl("-" + stringify(d.package_id->version()));
}

void
ConsoleInstallTask::display_merge_list_entry_repository(const DepListEntry & d, const DisplayMode)
{
    if (environment()->package_database()->favourite_repository() != d.package_id->repository()->name())
        output_no_endl("::" + stringify(d.package_id->repository()->name()));
}

void
ConsoleInstallTask::display_merge_list_entry_slot(const DepListEntry & d, const DisplayMode m)
{
    switch (m)
    {
        case normal_entry:
        case suggested_entry:
            output_no_endl(render_as_slot_name(" {:" + stringify(d.package_id->slot()) + "}"));
            break;

        case unimportant_entry:
            output_no_endl(render_as_unimportant(" {:" + stringify(d.package_id->slot()) + "}"));
            break;

        case error_entry:
            output_no_endl(render_as_slot_name(" {:" + stringify(d.package_id->slot()) + "}"));
            break;
    }
}

void
ConsoleInstallTask::display_merge_list_entry_status_and_update_counts(const DepListEntry & d,
        tr1::shared_ptr<const PackageIDSequence> existing_repo,
        tr1::shared_ptr<const PackageIDSequence> existing_slot_repo,
        const DisplayMode m)
{
    bool need_comma(false);
    switch (m)
    {
        case unimportant_entry:
            if (d.kind == dlk_provided)
                output_no_endl(render_as_unimportant(" [provided]"));
            else
                output_no_endl(render_as_unimportant(" [-]"));
            break;

        case suggested_entry:
            output_no_endl(render_as_update_mode(" [suggestion]"));
            set_count<suggested_count>(count<suggested_count>() + 1);
            break;

        case normal_entry:
            {
                output_no_endl(render_as_update_mode(" ["));

                if (need_comma)
                    output_no_endl(render_as_update_mode(", "));

                std::string destination_str;
                tr1::shared_ptr<const DestinationsSet> default_destinations(environment()->default_destinations());
                if (default_destinations->end() == default_destinations->find(d.destination))
                    destination_str = " ::" + stringify(d.destination->name());

                if (existing_repo->empty())
                {
                    output_no_endl(render_as_update_mode("N" + destination_str));
                    set_count<new_count>(count<new_count>() + 1);
                    set_count<max_count>(count<max_count>() + 1);
                }
                else if (existing_slot_repo->empty())
                {
                    output_no_endl(render_as_update_mode("S" + destination_str));
                    set_count<new_slot_count>(count<new_slot_count>() + 1);
                    set_count<max_count>(count<max_count>() + 1);
                }
                else if ((*existing_slot_repo->last())->version() < d.package_id->version())
                {
                    output_no_endl(render_as_update_mode("U " +
                                stringify((*existing_slot_repo->last())->canonical_form(idcf_version)) + destination_str));
                    set_count<upgrade_count>(count<upgrade_count>() + 1);
                    set_count<max_count>(count<max_count>() + 1);
                }
                else if ((*existing_slot_repo->last())->version() > d.package_id->version())
                {
                    output_no_endl(render_as_update_mode("D " +
                                stringify((*existing_slot_repo->last())->canonical_form(idcf_version)) + destination_str));
                    set_count<downgrade_count>(count<downgrade_count>() + 1);
                    set_count<max_count>(count<max_count>() + 1);
                }
                else
                {
                    output_no_endl(render_as_update_mode("R" + destination_str));
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
ConsoleInstallTask::_add_descriptions(tr1::shared_ptr<const UseFlagNameSet> c,
        const tr1::shared_ptr<const PackageID> & p, UseDescriptionState s)
{
    for (UseFlagNameSet::Iterator f(c->begin()), f_end(c->end()) ;
            f != f_end ; ++f)
    {
        std::string d;
        const RepositoryUseInterface * const i(p->repository()->use_interface);

        if (i)
            d = i->describe_use_flag(*f, *p);

        UseDescription e(UseDescription::create()
                .flag(*f)
                .state(s)
                .package_id(p)
                .description(d));

        Set<UseDescription, UseDescriptionComparator>::Iterator x(_all_use_descriptions->find(e));
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

    output_no_endl(" ");
    ColourFormatter formatter;

    tr1::shared_ptr<const PackageID> old_id;
    if (! existing_slot_repo->empty())
        old_id = *existing_slot_repo->last();
    else if (! existing_repo->empty())
        old_id = *existing_repo->last();

    if (d.package_id->iuse_key())
    {
        if (old_id)
            output_stream() << d.package_id->iuse_key()->pretty_print_flat_with_comparison(environment(), old_id, formatter);
        else
            output_stream() << d.package_id->iuse_key()->pretty_print_flat(formatter);
    }

    _add_descriptions(formatter.seen_new_use_flag_names(), d.package_id, uds_new);
    _add_descriptions(formatter.seen_changed_use_flag_names(), d.package_id, uds_changed);
    _add_descriptions(formatter.seen_use_flag_names(), d.package_id, uds_all);
    std::copy(formatter.seen_use_expand_prefixes()->begin(), formatter.seen_use_expand_prefixes()->end(),
            _all_expand_prefixes->inserter());
}

void
ConsoleInstallTask::display_merge_list_entry_tags(const DepListEntry & d, const DisplayMode m)
{
    if (d.tags->empty())
        return;

    std::string tag_titles;

    for (Set<DepTagEntry>::Iterator
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
            switch (m)
            {
                case normal_entry:
                case suggested_entry:
                case error_entry:
                    output_no_endl(" " + render_as_tag("<" + tag_titles + ">"));
                    break;

                case unimportant_entry:
                    output_no_endl(" " + render_as_unimportant("<" + tag_titles + ">"));
                    break;
            }
    }

    if (! want_install_reasons())
        if (d.kind != dlk_block)
            return;

    std::string deps;
    std::set<std::string> dependents, unsatisfied_dependents;
    unsigned c(0), max_c(want_full_install_reasons() ? std::numeric_limits<long>::max() : 3);

    for (Set<DepTagEntry>::Iterator
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
            switch (m)
            {
                case normal_entry:
                case suggested_entry:
                case error_entry:
                    output_no_endl(" " + render_as_tag("<" + deps + ">"));
                    break;

                case unimportant_entry:
                    output_no_endl(" " + render_as_unimportant("<" + deps + ">"));
                    break;
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

    for (PackageID::MasksIterator m(e.package_id->begin_masks()), m_end(e.package_id->end_masks()) ;
            m != m_end ; ++m)
    {
        if (need_comma)
            output_no_endl(", ");
        MaskDisplayer d(environment(), e.package_id, true);
        (*m)->accept(d);
        output_no_endl(d.result());
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
    for (AmbiguousPackageNameError::OptionsIterator o(e.begin_options()),
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
        for (Sequence<FetchActionFailure>::Iterator f(e.failures()->begin()), f_end(e.failures()->end()) ;
                f != f_end ; ++f)
        {
            output_stream() << "  * File '" << f->target_file << "': ";

            bool need_comma(false);
            if (f->requires_manual_fetching)
            {
                output_stream() << "requires manual fetching";
                need_comma = true;
            }

            if (f->failed_automatic_fetching)
            {
                if (need_comma)
                    output_stream() << ", ";
                output_stream() << "failed automatic fetching";
                need_comma = true;
            }

            if (! f->failed_integrity_checks.empty())
            {
                if (need_comma)
                    output_stream() << "failed automatic fetching";
                output_stream() << "failed integrity checks: " << f->failed_integrity_checks;
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
    output_stream() << "No such package '" << e.name() << "'" << endl;
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
            output_stream() << "No versions of '" << e.query() << "' are available" << endl;
        }
        else
        {
            output_stream() << endl;
            output_stream() << "Query error:" << endl;
            output_stream() << "  * " << e.backtrace("\n  * ");
            output_stream() << "All versions of '" << e.query() << "' are masked. Candidates are:" << endl;
            for (PackageIDSequence::Iterator pp(p->begin()), pp_end(p->end()) ;
                    pp != pp_end ; ++pp)
            {
                output_stream() << "    * " << colour(cl_package_name, **pp) << ": Masked by ";

                bool need_comma(false);
                for (PackageID::MasksIterator m((*pp)->begin_masks()), m_end((*pp)->end_masks()) ;
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
ConsoleInstallTask::on_use_requirements_not_met_error(const UseRequirementsNotMetError & e)
{
    output_stream() << endl;
    output_stream() << "DepList USE requirements not met error:" << endl;
    output_stream() << "  * " << e.backtrace("\n  * ") << e.message() << endl;
    output_stream() << endl;
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
ConsoleInstallTask::on_display_failure_summary_totals(const int total, const int successes,
        const int skipped, const int failures)
{
    std::ostringstream s;
    s << "Total: " << total << render_plural(total, " package", " packages");
    s << ", " << successes << render_plural(successes, " success", " successes");
    s << ", " << skipped << render_plural(skipped, " skipped", " skipped");
    s << ", " << failures << render_plural(failures, " failure", " failures");

    output_endl();
    output_unstarred_item(s.str());
}

void
ConsoleInstallTask::on_display_failure_summary_post()
{
    show_resume_command();
}

void
ConsoleInstallTask::on_display_failure_no_summary()
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

