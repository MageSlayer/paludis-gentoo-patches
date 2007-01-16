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
#include "use_flag_pretty_printer.hh"

#include <paludis/util/log.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/compare.hh>
#include <paludis/util/sr.hh>
#include <paludis/util/strip.hh>

#include <algorithm>
#include <set>
#include <iostream>
#include <iomanip>
#include <limits>

using namespace paludis;

#include <src/output/console_install_task-sr.cc>

bool
UseDescriptionComparator::operator() (const UseDescription & lhs, const UseDescription & rhs) const
{
    if (lhs.flag < rhs.flag)
        return true;
    if (lhs.flag > rhs.flag)
        return false;

    if (lhs.package.name < rhs.package.name)
        return true;
    if (lhs.package.name > rhs.package.name)
        return false;

    if (lhs.package.version < rhs.package.version)
        return true;
    if (lhs.package.version < rhs.package.version)
        return false;

    if (lhs.package.repository.data() < rhs.package.repository.data())
        return true;

    return false;
}

ConsoleInstallTask::ConsoleInstallTask(Environment * const env,
        const DepListOptions & options) :
    InstallTask(env, options),
    _all_tags(new SortedCollection<DepTagEntry>::Concrete),
    _all_use_descriptions(new SortedCollection<UseDescription, UseDescriptionComparator>::Concrete),
    _all_expand_prefixes(new UseFlagNameCollection::Concrete)
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
    output_heading("Cleaning stale versions after installing " +
            stringify(d.package));
}

void
ConsoleInstallTask::on_build_cleanlist_post(const DepListEntry &)
{
}

void
ConsoleInstallTask::on_clean_all_pre(const DepListEntry & d,
        const PackageDatabaseEntryCollection & c)
{
    display_clean_all_pre_list_start(d, c);

    for (PackageDatabaseEntryCollection::Iterator cc(c.begin()),
            cc_end(c.end()) ; cc != cc_end ; ++cc)
        display_one_clean_all_pre_list_entry(*cc);

    display_clean_all_pre_list_end(d, c);
}

void
ConsoleInstallTask::on_no_clean_needed(const DepListEntry &)
{
    output_starred_item("No cleaning required");
}

void
ConsoleInstallTask::on_clean_pre(const DepListEntry &,
        const PackageDatabaseEntry & c)
{
    output_heading("Cleaning " + stringify(c));
    output_xterm_title("(" + stringify(count<current_count>()) + " of "
            + stringify(count<max_count>()) + ") Cleaning " + stringify(c));
}

void
ConsoleInstallTask::on_clean_post(const DepListEntry &,
        const PackageDatabaseEntry &)
{
}

void
ConsoleInstallTask::on_clean_all_post(const DepListEntry &,
        const PackageDatabaseEntryCollection &)
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
    for (UseFlagNameCollection::Iterator f(_all_expand_prefixes->begin()),
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

    PackageDatabaseEntryCollection::Pointer existing(environment()->package_database()->
            query(PackageDepAtom(d.package.name), is_installed_only, qo_order_by_version));

    PackageDatabaseEntryCollection::Pointer existing_slot(environment()->package_database()->
            query(PackageDepAtom(stringify(d.package.name) + ":" + stringify(d.metadata->slot)),
                is_installed_only, qo_order_by_version));

    display_merge_list_entry_start(d, m);
    display_merge_list_entry_package_name(d, m);
    display_merge_list_entry_version(d, m);
    display_merge_list_entry_repository(d, m);
    display_merge_list_entry_slot(d, m);
    display_merge_list_entry_status_and_update_counts(d, existing, existing_slot, m);
    display_merge_list_entry_use(d, existing, existing_slot, m);
    display_merge_list_entry_tags(d, m);
    display_merge_list_entry_end(d, m);
}

void
ConsoleInstallTask::on_fetch_all_pre()
{
}

void
ConsoleInstallTask::on_fetch_pre(const DepListEntry & d)
{
    set_count<current_count>(count<current_count>() + 1);

    output_heading("Fetching " + stringify(d.package));
    output_xterm_title("(" + stringify(count<current_count>()) + " of "
            + stringify(count<max_count>()) + ") Fetching " + stringify(d.package));
}

void
ConsoleInstallTask::on_fetch_post(const DepListEntry &)
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
ConsoleInstallTask::on_install_pre(const DepListEntry & d)
{
    set_count<current_count>(count<current_count>() + 1);

    output_heading("Installing " + stringify(d.package));
    output_xterm_title("(" + stringify(count<current_count>()) + " of "
            + stringify(count<max_count>()) + ") Installing " + stringify(d.package));
}

void
ConsoleInstallTask::on_install_post(const DepListEntry &)
{
}

void
ConsoleInstallTask::on_install_all_post()
{
}

void
ConsoleInstallTask::on_update_world_pre()
{
    output_heading("Updating world file");
}

void
ConsoleInstallTask::on_update_world(const PackageDepAtom & a)
{
    output_starred_item("adding " + render_as_package_name(stringify(a)));
}

void
ConsoleInstallTask::on_update_world_skip(const PackageDepAtom & a, const std::string & s)
{
    output_starred_item("skipping " + render_as_package_name(stringify(a)) + " ("
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
        const PackageDatabaseEntryCollection &)
{
}

void
ConsoleInstallTask::display_one_clean_all_pre_list_entry(
        const PackageDatabaseEntry & c)
{
    output_starred_item(render_as_package_name(stringify(c)));
}

void
ConsoleInstallTask::display_clean_all_pre_list_end(const DepListEntry &,
        const PackageDatabaseEntryCollection &)
{
}

void
ConsoleInstallTask::display_merge_list_post_counts()
{
    if (count<max_count>() != count<new_count>() + count<upgrade_count>()
            + count<downgrade_count>() + count<new_slot_count>() + count<rebuild_count>())
        Log::get_instance()->message(ll_warning, lc_no_context,
                "Max count doesn't add up. This is a bug!");

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
    for (SortedCollection<DepTagEntry>::Iterator a(all_tags()->begin()),
            a_end(all_tags()->end()) ; a != a_end ; ++a)
        tag_categories.insert(a->tag->category());

    display_tag_summary_start();

    for (std::set<std::string>::iterator cat(tag_categories.begin()),
            cat_end(tag_categories.end()) ; cat != cat_end ; ++cat)
    {
        DepTagCategory::ConstPointer c(DepTagCategoryMaker::get_instance()->
                find_maker(*cat)());

        if (! c->visible())
            continue;

        display_tag_summary_tag_title(*c);
        display_tag_summary_tag_pre_text(*c);

        for (SortedCollection<DepTagEntry>::Iterator t(all_tags()->begin()),
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

    SortedCollection<UseDescription, UseDescriptionComparator>::Pointer group(
            new SortedCollection<UseDescription, UseDescriptionComparator>::Concrete);
    for (SortedCollection<UseDescription, UseDescriptionComparator>::Iterator i(all_use_descriptions()->begin()),
            i_end(all_use_descriptions()->end()) ; i != i_end ; ++i)
    {
        switch (i->state)
        {
            case uds_new:
                if (! want_new_use_flags())
                    continue;
                break;

            case uds_unchanged:
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
            for (UseFlagNameCollection::Iterator f(_all_expand_prefixes->begin()),
                    f_end(_all_expand_prefixes->end()) ; f != f_end && ! prefixed ; ++f)
                if (0 == stringify(i->flag).compare(0, stringify(*f).length(), stringify(*f)))
                    prefixed = true;

            if (prefixed)
                continue;
        }
        else
        {
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
            group.assign(new SortedCollection<UseDescription, UseDescriptionComparator>::Concrete);
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
        SortedCollection<UseDescription, UseDescriptionComparator>::Iterator i,
        SortedCollection<UseDescription, UseDescriptionComparator>::Iterator i_end)
{
    if (next(i) == i_end)
    {
        std::ostringstream s;
        s << std::left << std::setw(30) << (render_as_tag(
                    strip_leading_string(stringify(i->flag), prefix + "_")) + ": ");
        s << i->description;
        output_starred_item(s.str());
    }
    else
    {
        bool all_same(true);
        for (SortedCollection<UseDescription, UseDescriptionComparator>::Iterator j(next(i)) ; all_same && j != i_end ; ++j)
            if (j->description != i->description)
                all_same = false;

        if (all_same)
        {
            std::ostringstream s;
            s << std::left << std::setw(30) << (render_as_tag(
                        strip_leading_string(stringify(i->flag), prefix + "_")) + ": ");
            s << i->description;
            output_starred_item(s.str());
        }
        else
        {
            output_starred_item(render_as_tag(
                        strip_leading_string(stringify(i->flag), prefix + "_")) + ":");

            for ( ; i != i_end ; ++i)
            {
                std::ostringstream s;
                s << i->description << " (for " << render_as_package_name(stringify(i->package)) << ")";
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
ConsoleInstallTask::display_tag_summary_tag(DepTag::ConstPointer t)
{
    DepTagSummaryDisplayer::Pointer displayer(make_dep_tag_summary_displayer());
    t->accept(displayer.raw_pointer());
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
DepTagSummaryDisplayer::visit(const GLSADepTag * const tag)
{
    task()->output_starred_item(task()->render_as_tag(tag->short_text()) + ": "
            + tag->glsa_title());
}

void
DepTagSummaryDisplayer::visit(const DependencyDepTag * const)
{
}

void
DepTagSummaryDisplayer::visit(const GeneralSetDepTag * const tag)
{
    std::string desc;
    if (tag->short_text() == "world")
        desc = ":           Packages that have been explicitly installed";
    else if (tag->short_text() == "everything")
        desc = ":      All installed packages";
    else if (tag->short_text() == "system")
        desc = ":          Packages that are part of the base system";

    task()->output_starred_item(task()->render_as_tag(tag->short_text()) + desc);
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
            output_no_endl(render_as_package_name(stringify(d.package.name)));
            break;

        case unimportant_entry:
            output_no_endl(render_as_unimportant(stringify(d.package.name)));
            break;

        case error_entry:
            output_no_endl(render_as_error(stringify(d.package.name)));
            break;
    }
}

void
ConsoleInstallTask::display_merge_list_entry_version(const DepListEntry & d, const DisplayMode)
{
    if ((VersionSpec("0") != d.package.version) ||
            CategoryNamePart("virtual") != d.package.name.category)
        output_no_endl("-" + stringify(d.package.version));
}

void
ConsoleInstallTask::display_merge_list_entry_repository(const DepListEntry & d, const DisplayMode)
{
    if (environment()->package_database()->favourite_repository() != d.package.repository)
        output_no_endl("::" + stringify(d.package.repository));
}

void
ConsoleInstallTask::display_merge_list_entry_slot(const DepListEntry & d, const DisplayMode m)
{
    switch (m)
    {
        case normal_entry:
        case suggested_entry:
            output_no_endl(render_as_slot_name(" {:" + stringify(d.metadata->slot) + "}"));
            break;

        case unimportant_entry:
            output_no_endl(render_as_unimportant(" {:" + stringify(d.metadata->slot) + "}"));
            break;

        case error_entry:
            output_no_endl(render_as_slot_name(" {:" + stringify(d.metadata->slot) + "}"));
            break;
    }
}

void
ConsoleInstallTask::display_merge_list_entry_status_and_update_counts(const DepListEntry & d,
        PackageDatabaseEntryCollection::ConstPointer existing,
        PackageDatabaseEntryCollection::ConstPointer existing_slot,
        const DisplayMode m)
{
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
            if (existing->empty())
            {
                output_no_endl(render_as_update_mode(" [N]"));
                set_count<new_count>(count<new_count>() + 1);
                set_count<max_count>(count<max_count>() + 1);
            }
            else if (existing_slot->empty())
            {
                output_no_endl(render_as_update_mode(" [S]"));
                set_count<new_slot_count>(count<new_slot_count>() + 1);
                set_count<max_count>(count<max_count>() + 1);
            }
            else if (existing_slot->last()->version < d.package.version)
            {
                output_no_endl(render_as_update_mode(" [U " +
                            stringify(existing_slot->last()->version) + "]"));
                set_count<upgrade_count>(count<upgrade_count>() + 1);
                set_count<max_count>(count<max_count>() + 1);
            }
            else if (existing_slot->last()->version > d.package.version)
            {
                output_no_endl(render_as_update_mode(" [D " +
                            stringify(existing_slot->last()->version) + "]"));
                set_count<downgrade_count>(count<downgrade_count>() + 1);
                set_count<max_count>(count<max_count>() + 1);
            }
            else
            {
                output_no_endl(render_as_update_mode(" [R]"));
                set_count<rebuild_count>(count<rebuild_count>() + 1);
                set_count<max_count>(count<max_count>() + 1);
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
ConsoleInstallTask::_add_descriptions(UseFlagNameCollection::ConstPointer c,
        const PackageDatabaseEntry & p, UseDescriptionState s)
{
    for (UseFlagNameCollection::Iterator f(c->begin()), f_end(c->end()) ;
            f != f_end ; ++f)
    {
        std::string d;
        const RepositoryUseInterface * const i(environment()->package_database()->
                fetch_repository(p.repository)->use_interface);

        if (i)
            d = i->describe_use_flag(*f, &p);

        _all_use_descriptions->insert(UseDescription::create()
                .flag(*f)
                .state(s)
                .package(p)
                .description(d));
    }
}

void
ConsoleInstallTask::display_merge_list_entry_use(const DepListEntry & d,
        PackageDatabaseEntryCollection::ConstPointer existing,
        PackageDatabaseEntryCollection::ConstPointer,
        const DisplayMode m)
{
    if (normal_entry != m && suggested_entry != m)
        return;

    output_no_endl(" ");
    UseFlagPrettyPrinter::Pointer printer(make_use_flag_pretty_printer());
    printer->print_package_flags(d.package, existing->empty() ? 0 : &*existing->last());

    _add_descriptions(printer->new_flags(), d.package, uds_new);
    _add_descriptions(printer->changed_flags(), d.package, uds_changed);
    _add_descriptions(printer->unchanged_flags(), d.package, uds_unchanged);
    _all_expand_prefixes->insert(printer->expand_prefixes()->begin(),
            printer->expand_prefixes()->end());
}

void
ConsoleInstallTask::display_merge_list_entry_tags(const DepListEntry & d, const DisplayMode m)
{
    if (d.tags->empty())
        return;

    std::string tag_titles;

    for (SortedCollection<DepTagEntry>::Iterator
            tag(d.tags->begin()),
            tag_end(d.tags->end()) ;
            tag != tag_end ; ++tag)
    {
        if (tag->tag->category() == "dependency")
            continue;

        all_tags()->insert(*tag);

        EntryDepTagDisplayer::Pointer displayer(make_entry_dep_tag_displayer());
        tag->tag->accept(displayer.raw_pointer());
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
    unsigned c(0), max_c(want_full_install_reasons() ? std::numeric_limits<long>::max() : 3);

    for (SortedCollection<DepTagEntry>::Iterator
            tag(d.tags->begin()),
            tag_end(d.tags->end()) ;
            tag != tag_end ; ++tag)
    {
        if (tag->tag->category() != "dependency")
            continue;

        if (++c < max_c)
        {
            deps.append(tag->tag->short_text());
            deps.append(", ");
        }
    }

    if (! deps.empty())
    {
        if (c >= max_c)
            deps.append(stringify(c - max_c + 1) + " more, ");

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

void
ConsoleInstallTask::output_activity_start_message(const std::string & s) const
{
    output_stream() << s << "..." << std::flush;
}

void
ConsoleInstallTask::output_activity_end_message() const
{
    output_stream() << std::endl;
}

void
ConsoleInstallTask::output_heading(const std::string & s) const
{
    output_stream() << std::endl << colour(cl_heading, s) << std::endl << std::endl;
}

void
ConsoleInstallTask::output_xterm_title(const std::string & s) const
{
    output_xterm_stream() << xterm_title(s);
}

void
ConsoleInstallTask::output_starred_item(const std::string & s, const unsigned indent) const
{
    if (0 != indent)
        output_stream() << std::string(2 * indent, ' ') << "* " << s << std::endl;
    else
        output_stream() << "* " << s << std::endl;
}

void
ConsoleInstallTask::output_starred_item_no_endl(const std::string & s) const
{
    output_stream() << "* " << s;
}

void
ConsoleInstallTask::output_unstarred_item(const std::string & s) const
{
    output_stream() << s << std::endl;
}

void
ConsoleInstallTask::output_no_endl(const std::string & s) const
{
    output_stream() << s;
}

void
ConsoleInstallTask::output_endl() const
{
    output_stream() << std::endl;
}

std::ostream &
ConsoleInstallTask::output_stream() const
{
    return std::cout;
}

std::ostream &
ConsoleInstallTask::output_xterm_stream() const
{
    return std::cerr;
}

std::string
ConsoleInstallTask::render_as_package_name(const std::string & s) const
{
    return colour(cl_package_name, s);
}

std::string
ConsoleInstallTask::render_as_tag(const std::string & s) const
{
    return colour(cl_tag, s);
}

std::string
ConsoleInstallTask::render_as_unimportant(const std::string & s) const
{
    return colour(cl_unimportant, s);
}

std::string
ConsoleInstallTask::render_as_slot_name(const std::string & s) const
{
    return colour(cl_slot, s);
}

std::string
ConsoleInstallTask::render_as_update_mode(const std::string & s) const
{
    return colour(cl_updatemode, s);
}

std::string
ConsoleInstallTask::render_as_error(const std::string & s) const
{
    return colour(cl_error, s);
}

std::string
ConsoleInstallTask::render_plural(int c, const std::string & s, const std::string & p) const
{
    return 1 == c ? s : p;
}

DepTagSummaryDisplayer::Pointer
ConsoleInstallTask::make_dep_tag_summary_displayer()
{
    return DepTagSummaryDisplayer::Pointer(new DepTagSummaryDisplayer(this));
}

EntryDepTagDisplayer::Pointer
ConsoleInstallTask::make_entry_dep_tag_displayer()
{
    return EntryDepTagDisplayer::Pointer(new EntryDepTagDisplayer());
}

UseFlagPrettyPrinter::Pointer
ConsoleInstallTask::make_use_flag_pretty_printer()
{
    return UseFlagPrettyPrinter::Pointer(new UseFlagPrettyPrinter(environment()));
}

EntryDepTagDisplayer::EntryDepTagDisplayer()
{
}

EntryDepTagDisplayer::~EntryDepTagDisplayer()
{
}

void
EntryDepTagDisplayer::visit(const GLSADepTag * const tag)
{
    text() = tag->short_text();
}

void
EntryDepTagDisplayer::visit(const DependencyDepTag * const)
{
}

void
EntryDepTagDisplayer::visit(const GeneralSetDepTag * const tag)
{
    text() = tag->short_text(); // + "<" + tag->source() + ">";
}

