/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include "install_task.hh"
#include <paludis/dep_spec.hh>
#include <paludis/action.hh>
#include <paludis/metadata_key.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/query.hh>
#include <paludis/hook.hh>
#include <paludis/repository.hh>
#include <paludis/match_package.hh>
#include <paludis/package_database.hh>
#include <paludis/package_id.hh>
#include <paludis/tasks_exceptions.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/set.hh>
#include <paludis/util/log.hh>
#include <paludis/handled_information.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>
#include <functional>
#include <algorithm>
#include <list>
#include <set>

using namespace paludis;

#include <paludis/install_task-se.cc>

namespace paludis
{
    template<>
    struct Implementation<InstallTask>
    {
        Environment * const env;
        DepList dep_list;
        FetchActionOptions fetch_options;
        InstallActionOptions install_options;
        UninstallActionOptions uninstall_options;

        std::list<std::string> raw_targets;
        tr1::shared_ptr<ConstTreeSequence<SetSpecTree, AllDepSpec> > targets;
        tr1::shared_ptr<std::string> add_to_world_spec;
        tr1::shared_ptr<const DestinationsSet> destinations;

        bool pretend;
        bool fetch_only;
        bool preserve_world;

        bool had_set_targets;
        bool had_package_targets;
        bool override_target_type;

        InstallTaskContinueOnFailure continue_on_failure;

        bool had_action_failures;
        bool had_resolution_failures;

        Implementation<InstallTask>(Environment * const e, const DepListOptions & o,
                tr1::shared_ptr<const DestinationsSet> d) :
            env(e),
            dep_list(e, o),
            fetch_options(
                    FetchActionOptions::create()
                    .safe_resume(false)
                    .fetch_unneeded(false)
                    ),
            install_options(
                    InstallActionOptions::create()
                    .no_config_protect(false)
                    .debug_build(iado_none)
                    .checks(iaco_default)
                    .destination(tr1::shared_ptr<Repository>())
                    ),
            uninstall_options(false),
            targets(new ConstTreeSequence<SetSpecTree, AllDepSpec>(tr1::shared_ptr<AllDepSpec>(new AllDepSpec))),
            destinations(d),
            pretend(false),
            fetch_only(false),
            preserve_world(false),
            had_set_targets(false),
            had_package_targets(false),
            override_target_type(false),
            continue_on_failure(itcof_if_fetch_only),
            had_action_failures(false),
            had_resolution_failures(false)
        {
        }
    };
}

InstallTask::InstallTask(Environment * const env, const DepListOptions & options,
        const tr1::shared_ptr<const DestinationsSet> d) :
    PrivateImplementationPattern<InstallTask>(new Implementation<InstallTask>(env, options, d))
{
}

InstallTask::~InstallTask()
{
}

void
InstallTask::clear()
{
    _imp->targets.reset(new ConstTreeSequence<SetSpecTree, AllDepSpec>(tr1::shared_ptr<AllDepSpec>(new AllDepSpec)));
    _imp->had_set_targets = false;
    _imp->had_package_targets = false;
    _imp->dep_list.clear();
    _imp->raw_targets.clear();
    _imp->had_action_failures = false;
    _imp->had_package_targets = false;
}

void
InstallTask::add_target(const std::string & target)
{
    Context context("When adding install target '" + target + "':");

    tr1::shared_ptr<SetSpecTree::ConstItem> s;
    std::string modified_target(target);

    bool done(false);
    try
    {
        if ((target != "insecurity") && ((s = ((_imp->env->set(SetName(target)))))))
        {
            if (_imp->had_set_targets)
            {
                _imp->had_resolution_failures = true;
                throw MultipleSetTargetsSpecified();
            }

            if (_imp->had_package_targets)
            {
                _imp->had_resolution_failures = true;
                throw HadBothPackageAndSetTargets();
            }

            _imp->had_set_targets = true;
            if (! _imp->override_target_type)
                _imp->dep_list.options()->target_type = dl_target_set;
            _imp->targets->add(s);
            done = true;
        }
    }
    catch (const SetNameError &)
    {
    }

    if (! done)
    {
        Log::get_instance()->message(ll_debug, lc_context) << "target '" << target << "' is a package";

        if (_imp->had_set_targets)
        {
            _imp->had_resolution_failures = true;
            throw HadBothPackageAndSetTargets();
        }

        _imp->had_package_targets = true;
        if (! _imp->override_target_type)
            _imp->dep_list.options()->target_type = dl_target_package;

        if (std::string::npos != target.find('/'))
        {
            tr1::shared_ptr<PackageDepSpec> spec(new PackageDepSpec(target, pds_pm_permissive));
            spec->set_tag(tr1::shared_ptr<const DepTag>(new TargetDepTag));
            _imp->targets->add(tr1::shared_ptr<TreeLeaf<SetSpecTree, PackageDepSpec> >(
                        new TreeLeaf<SetSpecTree, PackageDepSpec>(spec)));
        }
        else
        {
            QualifiedPackageName q(_imp->env->package_database()->fetch_unique_qualified_package_name(
                        PackageNamePart(target)));
            modified_target = stringify(q);
            tr1::shared_ptr<PackageDepSpec> spec(
                new PackageDepSpec(tr1::shared_ptr<QualifiedPackageName>(new QualifiedPackageName(q))));
            spec->set_tag(tr1::shared_ptr<const DepTag>(new TargetDepTag));
            _imp->targets->add(tr1::shared_ptr<TreeLeaf<SetSpecTree, PackageDepSpec> >(
                        new TreeLeaf<SetSpecTree, PackageDepSpec>(spec)));
        }
    }

    _imp->raw_targets.push_back(modified_target);
}

void
InstallTask::add_exact_package(const tr1::shared_ptr<const PackageID> & target)
{
    Context context("When adding install target '" + stringify(*target) + "' from ID:");

    if (_imp->had_set_targets)
    {
        _imp->had_resolution_failures = true;
        throw HadBothPackageAndSetTargets();
    }

    _imp->had_package_targets = true;
    if (! _imp->override_target_type)
        _imp->dep_list.options()->target_type = dl_target_package;

    tr1::shared_ptr<PackageDepSpec> spec(new PackageDepSpec(
                tr1::shared_ptr<QualifiedPackageName>(new QualifiedPackageName(target->name())),
                tr1::shared_ptr<CategoryNamePart>(),
                tr1::shared_ptr<PackageNamePart>(),
                make_equal_to_version_requirements(target->version()),
                vr_and,
                tr1::shared_ptr<SlotName>(new SlotName(target->slot())),
                tr1::shared_ptr<RepositoryName>(new RepositoryName(target->repository()->name()))));

    spec->set_tag(tr1::shared_ptr<const DepTag>(new TargetDepTag));
    _imp->targets->add(tr1::shared_ptr<TreeLeaf<SetSpecTree, PackageDepSpec> >(
                new TreeLeaf<SetSpecTree, PackageDepSpec>(spec)));

    _imp->raw_targets.push_back(stringify(*spec));
}

void
InstallTask::_build_dep_list()
{
    Context context("When building dependency list:");

    on_build_deplist_pre();
    _imp->dep_list.add(*_imp->targets, _imp->destinations);
    on_build_deplist_post();
}

void
InstallTask::_display_task_list()
{
    Context context("When displaying task list:");

    if (_imp->pretend &&
        0 != perform_hook(Hook("install_pretend_pre")("TARGETS", join(_imp->raw_targets.begin(),
                             _imp->raw_targets.end(), " "))).max_exit_status)
        throw InstallActionError("Pretend install aborted by hook");

    on_display_merge_list_pre();

    /* display our task list */
    for (DepList::Iterator dep(_imp->dep_list.begin()), dep_end(_imp->dep_list.end()) ;
            dep != dep_end ; ++dep)
    {
        if (_imp->pretend &&
                0 != perform_hook(Hook("install_pretend_display_item_pre")
                    ("TARGET", stringify(*dep->package_id))
                    ("KIND", stringify(dep->kind))).max_exit_status)
            throw InstallActionError("Pretend install aborted by hook");

        on_display_merge_list_entry(*dep);

        if (_imp->pretend &&
                0 != perform_hook(Hook("install_pretend_display_item_post")
                    ("TARGET", stringify(*dep->package_id))
                    ("KIND", stringify(dep->kind))).max_exit_status)
            throw InstallActionError("Pretend install aborted by hook");
    }

    /* we're done displaying our task list */
    on_display_merge_list_post();
}

namespace
{
    struct SummaryVisitor :
        ConstVisitor<DepListEntryHandledVisitorTypes>
    {
        int total, successes, skipped, failures;
        InstallTask & task;
        const DepListEntry * entry;

        SummaryVisitor(InstallTask & t) :
            total(0),
            successes(0),
            skipped(0),
            failures(0),
            task(t),
            entry(0)
        {
        }

        void visit(const DepListEntryHandledSkippedUnsatisfied & s)
        {
            ++skipped;
            ++total;
            task.on_display_failure_summary_skipped_unsatisfied(*entry, s.spec());
        }

        void visit(const DepListEntryHandledFailed &)
        {
            ++failures;
            ++total;
            task.on_display_failure_summary_failure(*entry);
        }

        void visit(const DepListEntryUnhandled &)
        {
        }

        void visit(const DepListEntryNoHandlingRequired &)
        {
        }

        void visit(const DepListEntryHandledSuccess &)
        {
            ++successes;
            ++total;
            task.on_display_failure_summary_success(*entry);
        }
    };
}

void
InstallTask::_display_failure_summary()
{
    Context context("When displaying summary:");

    if (! _imp->had_action_failures)
        return;

    switch (_imp->continue_on_failure)
    {
        case itcof_if_fetch_only:
            if (! _imp->fetch_only)
            {
                on_display_failure_no_summary();
                return;
            }
            break;

        case itcof_always:
        case itcof_if_satisfied:
            break;

        case itcof_never:
            on_display_failure_no_summary();
            return;

        case last_itcof:
            throw InternalError(PALUDIS_HERE, "Bad continue_on_failure");
    }

    on_display_failure_summary_pre();

    /* display our summary */
    SummaryVisitor s(*this);
    for (DepList::Iterator dep(_imp->dep_list.begin()), dep_end(_imp->dep_list.end()) ;
            dep != dep_end ; ++dep)
    {
        s.entry = &*dep;
        dep->handled->accept(s);
    }

    /* we're done displaying our task list */
    on_display_failure_summary_totals(s.total, s.successes, s.skipped, s.failures);
    on_display_failure_summary_post();
}

bool
InstallTask::_pretend()
{
    Context context("When performing pretend actions:");

    bool pretend_failed(false);

    SupportsActionTest<PretendAction> pretend_action_query;
    for (DepList::Iterator dep(_imp->dep_list.begin()), dep_end(_imp->dep_list.end()) ;
            dep != dep_end ; ++dep)
        if (dep->package_id->supports_action(pretend_action_query))
        {
            PretendAction pretend_action;
            dep->package_id->perform_action(pretend_action);
            pretend_failed |= pretend_action.failed();
        }

    if (_imp->pretend)
    {
        if (0 != perform_hook(Hook("install_pretend_post")("TARGETS", join(
                             _imp->raw_targets.begin(), _imp->raw_targets.end(), " "))).max_exit_status)
            throw InstallActionError("Pretend install aborted by hook");
    }

    return pretend_failed;
}

void
InstallTask::_one(const DepList::Iterator dep, const int x, const int y, const int s, const int f)
{
    std::string cpvr(stringify(*dep->package_id));

    bool live_destination(false);
    if (dep->destination)
        if (dep->destination->destination_interface && dep->destination->destination_interface->want_pre_post_phases())
            live_destination = true;

    /* we're about to fetch / install one item */
    if (_imp->fetch_only)
    {
        if (0 != perform_hook(Hook("fetch_pre")
                     ("TARGET", cpvr)
                     ("X_OF_Y", stringify(x) + " of " + stringify(y))).max_exit_status)
            throw InstallActionError("Fetch of '" + cpvr + "' aborted by hook");
        on_fetch_pre(*dep, x, y, s, f);
    }
    else
    {
        if (0 != perform_hook(Hook("install_pre")
                     ("TARGET", cpvr)
                     ("X_OF_Y", stringify(x) + " of " + stringify(y))
                     ("PALUDIS_NO_LIVE_DESTINATION", live_destination ? "" : "yes")).max_exit_status)
            throw InstallActionError("Install of '" + cpvr + "' aborted by hook");
        on_install_pre(*dep, x, y, s, f);
    }

    /* fetch / install one item */
    try
    {
        SupportsActionTest<FetchAction> test_fetch;
        if (dep->package_id->supports_action(test_fetch))
        {
            FetchAction fetch_action(_imp->fetch_options);
            dep->package_id->perform_action(fetch_action);
        }

        if (! _imp->fetch_only)
        {
            _imp->install_options.destination = dep->destination;
            InstallAction install_action(_imp->install_options);
            dep->package_id->perform_action(install_action);
        }
    }
    catch (const InstallActionError & e)
    {
        on_install_fail(*dep, x, y, s, f);
        HookResult PALUDIS_ATTRIBUTE((unused)) dummy(perform_hook(Hook("install_fail")("TARGET", cpvr)("MESSAGE", e.message())));
        throw;
    }

    /* we've fetched / installed one item */
    if (_imp->fetch_only)
    {
        on_fetch_post(*dep, x, y, s, f);
        if (0 != perform_hook(Hook("fetch_post")
                     ("TARGET", cpvr)
                     ("X_OF_Y", stringify(x) + " of " + stringify(y))).max_exit_status)
            throw InstallActionError("Fetch of '" + cpvr + "' aborted by hook");
    }
    else
    {
        on_install_post(*dep, x, y, s, f);
        if (0 != perform_hook(Hook("install_post")
                     ("TARGET", cpvr)
                     ("X_OF_Y", stringify(x) + " of " + stringify(y))
                     ("PALUDIS_NO_LIVE_DESTINATION", live_destination ? "" : "yes")).max_exit_status)
            throw InstallActionError("Install of '" + cpvr + "' aborted by hook");
    }

    if (_imp->fetch_only || ! live_destination)
        return;

    /* figure out whether we need to unmerge (clean) anything */
    on_build_cleanlist_pre(*dep);

    // manually invalidate any installed repos, they're probably
    // wrong now
    for (PackageDatabase::RepositoryConstIterator r(_imp->env->package_database()->begin_repositories()),
            r_end(_imp->env->package_database()->end_repositories()) ; r != r_end ; ++r)
        if ((*r)->installed_interface)
            ((*r).get())->invalidate();

    // look for packages with the same name in the same slot in the destination repos
    tr1::shared_ptr<const PackageIDSequence> collision_list;

    if (dep->destination)
        collision_list = _imp->env->package_database()->query(
                query::Matches(PackageDepSpec(
                        tr1::shared_ptr<QualifiedPackageName>(new QualifiedPackageName(dep->package_id->name())),
                        tr1::shared_ptr<CategoryNamePart>(),
                        tr1::shared_ptr<PackageNamePart>(),
                        tr1::shared_ptr<VersionRequirements>(),
                        vr_and,
                        tr1::shared_ptr<SlotName>(new SlotName(dep->package_id->slot())),
                        tr1::shared_ptr<RepositoryName>(new RepositoryName(dep->destination->name())))) &
                query::SupportsAction<UninstallAction>(),
                qo_order_by_version);

    // don't clean the thing we just installed
    PackageIDSequence clean_list;
    if (collision_list)
        for (PackageIDSequence::ConstIterator c(collision_list->begin()),
                c_end(collision_list->end()) ; c != c_end ; ++c)
            if (dep->package_id->version() != (*c)->version())
                clean_list.push_back(*c);
    /* no need to sort clean_list here, although if the above is
     * changed then check that this still holds... */

    on_build_cleanlist_post(*dep);

    /* ok, we have the cleanlist. we're about to clean */
    if (clean_list.empty())
        on_no_clean_needed(*dep);
    else
    {
        if (0 != perform_hook(Hook("clean_all_pre")("TARGETS", join(
                             indirect_iterator(clean_list.begin()), indirect_iterator(clean_list.end()), " "))).max_exit_status)
            throw InstallActionError("Clean aborted by hook");
        on_clean_all_pre(*dep, clean_list);

        for (PackageIDSequence::ConstIterator c(clean_list.begin()),
                c_end(clean_list.end()) ; c != c_end ; ++c)
        {
            /* clean one item */
            if (0 != perform_hook(Hook("clean_pre")("TARGET", stringify(**c))
                         ("X_OF_Y", stringify(x) + " of " + stringify(y))).max_exit_status)
                throw InstallActionError("Clean of '" + cpvr + "' aborted by hook");
            on_clean_pre(*dep, **c, x, y, s, f);

            try
            {
                UninstallAction uninstall_action(_imp->uninstall_options);
                (*c)->perform_action(uninstall_action);
            }
            catch (const UninstallActionError & e)
            {
                on_clean_fail(*dep, **c, x, y, s, f);
                HookResult PALUDIS_ATTRIBUTE((unused)) dummy(perform_hook(Hook("clean_fail")
                            ("TARGET", stringify(**c))("MESSAGE", e.message())));
                throw;
            }

            on_clean_post(*dep, **c, x, y, s, f);
            if (0 != perform_hook(Hook("clean_post")("TARGET", stringify(**c))
                         ("X_OF_Y", stringify(x) + " of " + stringify(y))).max_exit_status)
                throw InstallActionError("Clean of '" + cpvr + "' aborted by hook");
        }

        /* we're done cleaning */
        if (0 != perform_hook(Hook("clean_all_post")("TARGETS", join(
                             indirect_iterator(clean_list.begin()), indirect_iterator(clean_list.end()), " "))).max_exit_status)
            throw InstallActionError("Clean aborted by hook");
        on_clean_all_post(*dep, clean_list);
    }

    dep->handled.reset(new DepListEntryHandledSuccess);

    /* if we installed paludis and a re-exec is available, use it. */
    if (_imp->env->is_paludis_package(dep->package_id->name()))
    {
        DepList::Iterator d(dep);
        do
        {
            ++d;
            if (d == _imp->dep_list.end())
                break;
        }
        while (dlk_package != d->kind);

        if (d != _imp->dep_list.end())
            on_installed_paludis();
    }
}

void
InstallTask::_main_actions()
{
    using namespace tr1::placeholders;

    /* we're about to fetch / install the entire list */
    if (_imp->fetch_only)
    {
        if (0 != perform_hook(Hook("fetch_all_pre")("TARGETS", join(
                             _imp->raw_targets.begin(), _imp->raw_targets.end(), " "))).max_exit_status)
            throw InstallActionError("Fetch aborted by hook");
        on_fetch_all_pre();
    }
    else
    {
        bool any_live_destination(false);
        for (DepList::Iterator dep(_imp->dep_list.begin()), dep_end(_imp->dep_list.end()) ;
                dep != dep_end && ! any_live_destination ; ++dep)
            if (dlk_package == dep->kind && dep->destination)
                if (dep->destination->destination_interface && dep->destination->destination_interface->want_pre_post_phases())
                    any_live_destination = true;

        if (0 != perform_hook(Hook("install_all_pre")
                     ("TARGETS", join(_imp->raw_targets.begin(), _imp->raw_targets.end(), " "))
                     ("PALUDIS_NO_LIVE_DESTINATION", any_live_destination ? "" : "yes")).max_exit_status)
            throw InstallActionError("Install aborted by hook");
        on_install_all_pre();
    }

    /* fetch / install our entire list */
    int x(0), y(std::count_if(_imp->dep_list.begin(), _imp->dep_list.end(),
                tr1::bind(std::equal_to<DepListEntryKind>(), dlk_package, tr1::bind<DepListEntryKind>(&DepListEntry::kind, _1)))),
        s(0), f(0);

    for (DepList::Iterator dep(_imp->dep_list.begin()), dep_end(_imp->dep_list.end()) ;
            dep != dep_end ; ++dep)
    {
        if (dlk_package != dep->kind)
            continue;

        ++x;

        if (_imp->had_action_failures)
        {
            switch (_imp->continue_on_failure)
            {
                case itcof_if_fetch_only:
                    if (_imp->fetch_only)
                        break;
                    ++s;
                    continue;

                case itcof_never:
                    ++s;
                    continue;

                case itcof_if_satisfied:
                    {
                        tr1::shared_ptr<const PackageDepSpec> d(_unsatisfied(*dep));
                        if (! d)
                            break;
                        dep->handled.reset(new DepListEntryHandledSkippedUnsatisfied(*d));
                        on_skip_unsatisfied(*dep, *d, x, y, s, f);
                        ++s;
                        continue;
                    }

                case itcof_always:
                    break;

                case last_itcof:
                    throw InternalError(PALUDIS_HERE, "Bad continue_on_failure");
            }
        }

        try
        {
            _one(dep, x, y, s, f);
        }
        catch (const InstallActionError & e)
        {
            _imp->had_action_failures = true;
            dep->handled.reset(new DepListEntryHandledFailed);
            on_install_action_error(e);
            ++f;
        }
        catch (const FetchActionError & e)
        {
            _imp->had_action_failures = true;
            dep->handled.reset(new DepListEntryHandledFailed);
            on_fetch_action_error(e);
            ++f;
        }
    }

    /* go no further if we had failures */
    if (_imp->had_action_failures)
    {
        _display_failure_summary();
        return;
    }

    /* update world */
    if (! _imp->fetch_only)
    {
        if (! _imp->preserve_world)
        {
            on_update_world_pre();

            if (_imp->had_package_targets)
            {
                if (_imp->add_to_world_spec)
                {
                    tr1::shared_ptr<ConstTreeSequence<SetSpecTree, AllDepSpec> > all(new ConstTreeSequence<SetSpecTree, AllDepSpec>(
                                tr1::shared_ptr<AllDepSpec>(new AllDepSpec)));
                    std::list<std::string> tokens;
                    WhitespaceTokeniser::tokenise(*_imp->add_to_world_spec, std::back_inserter(tokens));
                    if ((! tokens.empty()) && ("(" == *tokens.begin()) && (")" == *previous(tokens.end())))
                    {
                        tokens.erase(tokens.begin());
                        tokens.erase(previous(tokens.end()));
                    }

                    for (std::list<std::string>::const_iterator t(tokens.begin()), t_end(tokens.end()) ;
                            t != t_end ; ++t)
                        all->add(tr1::shared_ptr<TreeLeaf<SetSpecTree, PackageDepSpec> >(
                                    new TreeLeaf<SetSpecTree, PackageDepSpec>(tr1::shared_ptr<PackageDepSpec>(
                                            new PackageDepSpec(*t, pds_pm_permissive)))));
                    world_update_packages(all);
                }
                else
                    world_update_packages(_imp->targets);
            }
            else if (_imp->had_set_targets)
            {
                if (_imp->add_to_world_spec)
                    world_update_set(SetName(*_imp->add_to_world_spec));
                else if (! _imp->raw_targets.empty())
                    world_update_set(SetName(*_imp->raw_targets.begin()));
            }

            on_update_world_post();
        }
        else
            on_preserve_world();
    }

    /* we've fetched / installed the entire list */
    if (_imp->fetch_only)
    {
        on_fetch_all_post();
        if (0 != perform_hook(Hook("fetch_all_post")("TARGETS", join(
                             _imp->raw_targets.begin(), _imp->raw_targets.end(), " "))).max_exit_status)
            throw InstallActionError("Fetch aborted by hook");
    }
    else
    {
        on_install_all_post();
        if (0 != perform_hook(Hook("install_all_post")("TARGETS", join(
                             _imp->raw_targets.begin(), _imp->raw_targets.end(), " "))).max_exit_status)
            throw InstallActionError("Install aborted by hook");
    }
}

void
InstallTask::_execute()
{
    Context context("When executing install task:");

    _build_dep_list();
    _display_task_list();
    bool pretend_failed(_pretend());

    if (_imp->pretend)
        return;

    if (_imp->dep_list.has_errors() || pretend_failed)
    {
        on_not_continuing_due_to_errors();
        return;
    }

    _main_actions();
}

void
InstallTask::execute()
{
    try
    {
        _execute();
    }
    catch (const AmbiguousPackageNameError & e)
    {
        _imp->had_resolution_failures = true;
        on_ambiguous_package_name_error(e);
    }
    catch (const NoSuchPackageError & e)
    {
        _imp->had_resolution_failures = true;
        on_no_such_package_error(e);
    }
    catch (const AllMaskedError & e)
    {
        _imp->had_resolution_failures = true;
        on_all_masked_error(e);
    }
    catch (const UseRequirementsNotMetError & e)
    {
        _imp->had_resolution_failures = true;
        on_use_requirements_not_met_error(e);
    }
    catch (const DepListError & e)
    {
        _imp->had_resolution_failures = true;
        on_dep_list_error(e);
    }
    catch (const HadBothPackageAndSetTargets & e)
    {
        _imp->had_resolution_failures = true;
        on_had_both_package_and_set_targets_error(e);
    }
    catch (const MultipleSetTargetsSpecified & e)
    {
        _imp->had_resolution_failures = true;
        on_multiple_set_targets_specified(e);
    }
}

const DepList &
InstallTask::dep_list() const
{
    return _imp->dep_list;
}

void
InstallTask::set_no_config_protect(const bool value)
{
    _imp->install_options.no_config_protect = value;
    _imp->uninstall_options.no_config_protect = value;
}

void
InstallTask::set_fetch_only(const bool value)
{
    _imp->fetch_only = value;
}

void
InstallTask::set_pretend(const bool value)
{
    _imp->pretend = value;
}

void
InstallTask::set_preserve_world(const bool value)
{
    _imp->preserve_world = value;
}

void
InstallTask::set_debug_mode(const InstallActionDebugOption value)
{
    _imp->install_options.debug_build = value;
}

void
InstallTask::set_checks_mode(const InstallActionChecksOption value)
{
    _imp->install_options.checks = value;
}

void
InstallTask::set_add_to_world_spec(const std::string & value)
{
    _imp->add_to_world_spec.reset(new std::string(value));
}

InstallTask::TargetsConstIterator
InstallTask::begin_targets() const
{
    return TargetsConstIterator(_imp->raw_targets.begin());
}

InstallTask::TargetsConstIterator
InstallTask::end_targets() const
{
    return TargetsConstIterator(_imp->raw_targets.end());
}

Environment *
InstallTask::environment()
{
    return _imp->env;
}

const Environment *
InstallTask::environment() const
{
    return _imp->env;
}

void
InstallTask::on_installed_paludis()
{
}

bool
InstallTask::had_set_targets() const
{
    return _imp->had_set_targets;
}

bool
InstallTask::had_package_targets() const
{
    return _imp->had_package_targets;
}

void
InstallTask::set_safe_resume(const bool value)
{
    _imp->fetch_options.safe_resume = value;
}

HookResult
InstallTask::perform_hook(const Hook & hook) const
{
    return _imp->env->perform_hook(hook);
}

void
InstallTask::override_target_type(const DepListTargetType t)
{
    _imp->override_target_type = true;
    _imp->dep_list.options()->target_type = t;
}

void
InstallTask::world_update_set(const SetName & s)
{
    if (s == SetName("world") || s == SetName("system") || s == SetName("security")
            || s == SetName("everything") || s == SetName("insecurity") || s == SetName("ununused"))
    {
        on_update_world_skip(s, "special sets cannot be added to world");
        return;
    }

    for (PackageDatabase::RepositoryConstIterator r(_imp->env->package_database()->begin_repositories()),
            r_end(_imp->env->package_database()->end_repositories()) ;
            r != r_end ; ++r)
        if ((*r)->world_interface)
            (*r)->world_interface->add_to_world(s);

    on_update_world(s);
}

namespace
{
    struct WorldTargetFinder :
        ConstVisitor<SetSpecTree>,
        ConstVisitor<SetSpecTree>::VisitConstSequence<WorldTargetFinder, AllDepSpec>
    {
        using ConstVisitor<SetSpecTree>::VisitConstSequence<WorldTargetFinder, AllDepSpec>::visit_sequence;

        Environment * const env;
        InstallTask * const task;

        WorldTargetFinder(Environment * const e, InstallTask * const t) :
            env(e),
            task(t)
        {
        }

        void visit_leaf(const PackageDepSpec & a)
        {
            if (a.slot_ptr())
                task->on_update_world_skip(a, ":slot restrictions");
            else if (a.version_requirements_ptr() && ! a.version_requirements_ptr()->empty())
                task->on_update_world_skip(a, "version restrictions");
            else
            {
                for (PackageDatabase::RepositoryConstIterator r(env->package_database()->begin_repositories()),
                        r_end(env->package_database()->end_repositories()) ;
                        r != r_end ; ++r)
                    if ((*r)->world_interface && a.package_ptr())
                        (*r)->world_interface->add_to_world(*a.package_ptr());
                task->on_update_world(a);
            }
        }

        void visit_leaf(const NamedSetDepSpec &)
        {
        }
    };
}

void
InstallTask::world_update_packages(tr1::shared_ptr<const SetSpecTree::ConstItem> a)
{
    WorldTargetFinder w(_imp->env, this);
    a->accept(w);
}

bool
InstallTask::had_action_failures() const
{
    return _imp->had_action_failures;
}

bool
InstallTask::had_resolution_failures() const
{
    return _imp->had_resolution_failures;
}

void
InstallTask::set_continue_on_failure(const InstallTaskContinueOnFailure c)
{
    _imp->continue_on_failure = c;
}

namespace
{
    struct CheckSatisfiedVisitor :
        ConstVisitor<DependencySpecTree>,
        ConstVisitor<DependencySpecTree>::VisitConstSequence<CheckSatisfiedVisitor, AllDepSpec>
    {
        using ConstVisitor<DependencySpecTree>::VisitConstSequence<CheckSatisfiedVisitor, AllDepSpec>::visit_sequence;

        const Environment * const env;
        const PackageID & id;
        tr1::shared_ptr<const PackageDepSpec> failure;
        std::set<SetName> recursing_sets;

        CheckSatisfiedVisitor(const Environment * const e,
                const PackageID & i) :
            env(e),
            id(i)
        {
        }

        void visit_leaf(const BlockDepSpec &)
        {
        }

        void visit_leaf(const DependencyLabelsDepSpec &)
        {
        }

        void visit_leaf(const PackageDepSpec & a)
        {
            if (! failure)
                if (env->package_database()->query(query::Matches(a) & query::SupportsAction<InstalledAction>(), qo_whatever)->empty())
                    failure.reset(new PackageDepSpec(a));
        }

        void visit_sequence(const UseDepSpec & u,
                DependencySpecTree::ConstSequenceIterator cur,
                DependencySpecTree::ConstSequenceIterator end)
        {
            if (env->query_use(u.flag(), id) ^ u.inverse())
                std::for_each(cur, end, accept_visitor(*this));
        }

        void visit_sequence(const AnyDepSpec &,
                DependencySpecTree::ConstSequenceIterator cur,
                DependencySpecTree::ConstSequenceIterator end)
        {
            if (failure)
                return;

            failure.reset();
            for ( ; cur != end ; ++cur)
            {
                failure.reset();
                cur->accept(*this);
                if (! failure)
                    break;
            }
        }

        void visit_leaf(const NamedSetDepSpec & s)
        {
            tr1::shared_ptr<const SetSpecTree::ConstItem> set(env->set(s.name()));

            if (! set)
            {
                Log::get_instance()->message(ll_warning, lc_context) << "Unknown set '" << s.name() << "'";
                return;
            }

            if (! recursing_sets.insert(s.name()).second)
            {
                Log::get_instance()->message(ll_warning, lc_context) << "Recursively defined set '" << s.name() << "'";
                return;
            }

            set->accept(*this);

            recursing_sets.erase(s.name());
        }
    };
}

tr1::shared_ptr<const PackageDepSpec>
InstallTask::_unsatisfied(const DepListEntry & e) const
{
    Context context("When checking whether dependencies for '" + stringify(*e.package_id) + "' are satisfied:");

    CheckSatisfiedVisitor v(environment(), *e.package_id);

    if (dl_deps_pre == _imp->dep_list.options()->uninstalled_deps_pre ||
            dl_deps_pre_or_post == _imp->dep_list.options()->uninstalled_deps_pre)
        if (e.package_id->build_dependencies_key())
            e.package_id->build_dependencies_key()->value()->accept(v);

    if (dl_deps_pre == _imp->dep_list.options()->uninstalled_deps_runtime ||
            dl_deps_pre_or_post == _imp->dep_list.options()->uninstalled_deps_runtime)
        if (e.package_id->run_dependencies_key())
            e.package_id->run_dependencies_key()->value()->accept(v);

    if (dl_deps_pre == _imp->dep_list.options()->uninstalled_deps_post ||
            dl_deps_pre_or_post == _imp->dep_list.options()->uninstalled_deps_post)
        if (e.package_id->post_dependencies_key())
            e.package_id->post_dependencies_key()->value()->accept(v);

    if ((dl_deps_pre == _imp->dep_list.options()->uninstalled_deps_suggested ||
                dl_deps_pre_or_post == _imp->dep_list.options()->uninstalled_deps_suggested)
            && dl_suggested_install == _imp->dep_list.options()->suggested)
        if (e.package_id->suggested_dependencies_key())
            e.package_id->suggested_dependencies_key()->value()->accept(v);

    return v.failure;
}

namespace
{
    struct NotYetInstalledVisitor :
        ConstVisitor<DepListEntryHandledVisitorTypes>
    {
        tr1::shared_ptr<const PackageID> id;
        tr1::shared_ptr<PackageIDSequence> result;

        NotYetInstalledVisitor() :
            result(new PackageIDSequence)
        {
        }

        void visit(const DepListEntryHandledSuccess &)
        {
        }

        void visit(const DepListEntryHandledSkippedUnsatisfied &)
        {
            result->push_back(id);
        }

        void visit(const DepListEntryHandledFailed &)
        {
            result->push_back(id);
        }

        void visit(const DepListEntryUnhandled &)
        {
            result->push_back(id);
        }

        void visit(const DepListEntryNoHandlingRequired &)
        {
        }
    };
}

const tr1::shared_ptr<const PackageIDSequence>
InstallTask::packages_not_yet_installed_successfully() const
{
    NotYetInstalledVisitor s;

    for (DepList::ConstIterator dep(_imp->dep_list.begin()), dep_end(_imp->dep_list.end()) ;
            dep != dep_end ; ++dep)
    {
        s.id = dep->package_id;
        dep->handled->accept(s);
    }

    return s.result;
}

