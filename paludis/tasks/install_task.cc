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

#include "install_task.hh"
#include <paludis/dep_spec.hh>
#include <paludis/portage_dep_parser.hh>
#include <paludis/dep_spec_pretty_printer.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/query.hh>
#include <paludis/hook.hh>
#include <paludis/repository.hh>
#include <paludis/package_database.hh>
#include <paludis/tasks/exceptions.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/log.hh>
#include <list>

using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<InstallTask>
    {
        Environment * const env;
        DepList dep_list;
        DepList::Iterator current_dep_list_entry;
        InstallOptions install_options;
        UninstallOptions uninstall_options;

        std::list<std::string> raw_targets;
        tr1::shared_ptr<ConstTreeSequence<SetSpecTree, AllDepSpec> > targets;
        tr1::shared_ptr<std::string> add_to_world_spec;
        tr1::shared_ptr<const DestinationsCollection> destinations;

        bool pretend;
        bool preserve_world;

        bool had_set_targets;
        bool had_package_targets;
        bool override_target_type;

        Implementation<InstallTask>(Environment * const e, const DepListOptions & o,
                tr1::shared_ptr<const DestinationsCollection> d) :
            env(e),
            dep_list(e, o),
            current_dep_list_entry(dep_list.begin()),
            install_options(false, false, ido_none, false, tr1::shared_ptr<Repository>()),
            uninstall_options(false),
            targets(new ConstTreeSequence<SetSpecTree, AllDepSpec>(tr1::shared_ptr<AllDepSpec>(new AllDepSpec))),
            destinations(d),
            pretend(false),
            preserve_world(false),
            had_set_targets(false),
            had_package_targets(false),
            override_target_type(false)
        {
        }
    };
}

InstallTask::InstallTask(Environment * const env, const DepListOptions & options,
        const tr1::shared_ptr<const DestinationsCollection> d) :
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
            DepSpecPrettyPrinter p(0, false);
            s->accept(p);
            Log::get_instance()->message(ll_debug, lc_context) << "target '" << target << "' is set '" << p << "'";

            if (_imp->had_set_targets)
                throw MultipleSetTargetsSpecified();

            if (_imp->had_package_targets)
                throw HadBothPackageAndSetTargets();

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
            throw HadBothPackageAndSetTargets();

        _imp->had_package_targets = true;
        if (! _imp->override_target_type)
            _imp->dep_list.options()->target_type = dl_target_package;

        if (std::string::npos != target.find('/'))
            _imp->targets->add(tr1::shared_ptr<TreeLeaf<SetSpecTree, PackageDepSpec> >(
                        new TreeLeaf<SetSpecTree, PackageDepSpec>(tr1::shared_ptr<PackageDepSpec>(
                                new PackageDepSpec(target, pds_pm_permissive)))));
        else
        {
            QualifiedPackageName q(_imp->env->package_database()->fetch_unique_qualified_package_name(
                        PackageNamePart(target)));
            modified_target = stringify(q);
            _imp->targets->add(tr1::shared_ptr<TreeLeaf<SetSpecTree, PackageDepSpec> >(
                        new TreeLeaf<SetSpecTree, PackageDepSpec>(tr1::shared_ptr<PackageDepSpec>(
                                new PackageDepSpec(tr1::shared_ptr<QualifiedPackageName>(new QualifiedPackageName(q)))))));
        }
    }

    _imp->raw_targets.push_back(modified_target);
}

void
InstallTask::execute()
{
    Context context("When executing install task:");

    /* build up our dep list */
    on_build_deplist_pre();
    DepSpecPrettyPrinter p(0, false);
    _imp->targets->accept(p);
    Log::get_instance()->message(ll_debug, lc_context) << "_imp->targets is '" << p << "'";
    _imp->dep_list.add(*_imp->targets, _imp->destinations);
    on_build_deplist_post();

    /* we're about to display our task list */
    if (_imp->pretend &&
        0 != perform_hook(Hook("install_pretend_pre")("TARGETS", join(_imp->raw_targets.begin(),
                             _imp->raw_targets.end(), " "))).max_exit_status)
        throw PackageInstallActionError("Pretend install aborted by hook");

    on_display_merge_list_pre();

    /* display our task list */
    for (DepList::Iterator dep(_imp->dep_list.begin()), dep_end(_imp->dep_list.end()) ;
            dep != dep_end ; ++dep)
    {
        if (_imp->pretend &&
                0 != perform_hook(Hook("install_pretend_display_item_pre")
                    ("TARGET", stringify(dep->package))
                    ("KIND", stringify(dep->kind))).max_exit_status)
            throw PackageInstallActionError("Pretend install aborted by hook");

        _imp->current_dep_list_entry = dep;
        on_display_merge_list_entry(*dep);

        if (_imp->pretend &&
                0 != perform_hook(Hook("install_pretend_display_item_post")
                    ("TARGET", stringify(dep->package))
                    ("KIND", stringify(dep->kind))).max_exit_status)
            throw PackageInstallActionError("Pretend install aborted by hook");
    }

    /* we're done displaying our task list */
    on_display_merge_list_post();

    /* do pretend phase things */
    bool pretend_failed(false);

    for (DepList::Iterator dep(_imp->dep_list.begin()), dep_end(_imp->dep_list.end()) ;
            dep != dep_end ; ++dep)
    {
        const RepositoryPretendInterface * const pretend_interface(
                _imp->env->package_database()->fetch_repository(dep->package.repository)->pretend_interface);
        if (pretend_interface)
            pretend_failed |= ! pretend_interface->pretend(dep->package.name, dep->package.version);
    }

    if (_imp->pretend)
    {
        if (0 != perform_hook(Hook("install_pretend_post")("TARGETS", join(
                             _imp->raw_targets.begin(), _imp->raw_targets.end(), " "))).max_exit_status)
            throw PackageInstallActionError("Pretend install aborted by hook");
        return;
    }

    if (_imp->dep_list.has_errors() || pretend_failed)
    {
        on_not_continuing_due_to_errors();
        return;
    }

    /* we're about to fetch / install the entire list */
    if (_imp->install_options.fetch_only)
    {
        if (0 != perform_hook(Hook("fetch_all_pre")("TARGETS", join(
                             _imp->raw_targets.begin(), _imp->raw_targets.end(), " "))).max_exit_status)
            throw PackageInstallActionError("Fetch aborted by hook");
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
            throw PackageInstallActionError("Install aborted by hook");
        on_install_all_pre();
    }

    /* fetch / install our entire list */
    int x(0), y(0);
    for (DepList::Iterator dep(_imp->dep_list.begin()), dep_end(_imp->dep_list.end()) ;
            dep != dep_end ; ++dep)
        if (dlk_package == dep->kind)
            ++y;

    for (DepList::Iterator dep(_imp->dep_list.begin()), dep_end(_imp->dep_list.end()) ;
            dep != dep_end ; ++dep)
    {
        if (dlk_package != dep->kind)
            continue;

        bool live_destination(false);
        if (dep->destination)
            if (dep->destination->destination_interface && dep->destination->destination_interface->want_pre_post_phases())
                live_destination = true;

        ++x;
        _imp->current_dep_list_entry = dep;

        std::string cpvr(stringify(dep->package.name) + "-" +
                stringify(dep->package.version) + "::" +
                stringify(dep->package.repository));

        /* we're about to fetch / install one item */
        if (_imp->install_options.fetch_only)
        {
            if (0 != perform_hook(Hook("fetch_pre")
                         ("TARGET", cpvr)
                         ("X_OF_Y", stringify(x) + " of " + stringify(y))).max_exit_status)
                throw PackageInstallActionError("Fetch of '" + cpvr + "' aborted by hook");
            on_fetch_pre(*dep);
        }
        else
        {
            if (0 != perform_hook(Hook("install_pre")
                         ("TARGET", cpvr)
                         ("X_OF_Y", stringify(x) + " of " + stringify(y))
                         ("PALUDIS_NO_LIVE_DESTINATION", live_destination ? "" : "yes")).max_exit_status)
                throw PackageInstallActionError("Install of '" + cpvr + "' aborted by hook");
            on_install_pre(*dep);
        }

        /* fetch / install one item */
        const RepositoryInstallableInterface * const installable_interface(
                _imp->env->package_database()->fetch_repository(dep->package.repository)->
                installable_interface);
        if (! installable_interface)
            throw InternalError(PALUDIS_HERE, "Trying to install from a non-installable repository");

        try
        {
            _imp->install_options.destination = dep->destination;
            installable_interface->install(dep->package.name, dep->package.version, _imp->install_options);
        }
        catch (const PackageInstallActionError & e)
        {
            on_install_fail(*dep);
            HookResult PALUDIS_ATTRIBUTE((unused)) dummy(perform_hook(Hook("install_fail")("TARGET", cpvr)("MESSAGE", e.message())));
            throw;
        }

        /* we've fetched / installed one item */
        if (_imp->install_options.fetch_only)
        {
            on_fetch_post(*dep);
            if (0 != perform_hook(Hook("fetch_post")
                         ("TARGET", cpvr)
                         ("X_OF_Y", stringify(x) + " of " + stringify(y))).max_exit_status)
                throw PackageInstallActionError("Fetch of '" + cpvr + "' aborted by hook");
        }
        else
        {
            on_install_post(*dep);
            if (0 != perform_hook(Hook("install_post")
                         ("TARGET", cpvr)
                         ("X_OF_Y", stringify(x) + " of " + stringify(y))
                         ("PALUDIS_NO_LIVE_DESTINATION", live_destination ? "" : "yes")).max_exit_status)
                throw PackageInstallActionError("Install of '" + cpvr + "' aborted by hook");
        }

        if (_imp->install_options.fetch_only || ! live_destination)
            continue;

        /* figure out whether we need to unmerge (clean) anything */
        on_build_cleanlist_pre(*dep);

        // manually invalidate any installed repos, they're probably
        // wrong now
        for (PackageDatabase::RepositoryIterator r(_imp->env->package_database()->begin_repositories()),
                r_end(_imp->env->package_database()->end_repositories()) ; r != r_end ; ++r)
            if ((*r)->installed_interface)
                ((*r).get())->invalidate();

        // look for packages with the same name in the same slot in the destination repos
        tr1::shared_ptr<PackageDatabaseEntryCollection> collision_list;

        if (dep->destination)
            if (dep->destination->uninstallable_interface)
                collision_list = _imp->env->package_database()->query(
                        query::Matches(PackageDepSpec(
                                tr1::shared_ptr<QualifiedPackageName>(new QualifiedPackageName(dep->package.name)),
                                tr1::shared_ptr<CategoryNamePart>(),
                                tr1::shared_ptr<PackageNamePart>(),
                                tr1::shared_ptr<VersionRequirements>(),
                                vr_and,
                                tr1::shared_ptr<SlotName>(new SlotName(dep->metadata->slot)),
                                tr1::shared_ptr<RepositoryName>(new RepositoryName(dep->destination->name())))) &
                        query::RepositoryHasInstalledInterface(), qo_order_by_version);

        // don't clean the thing we just installed
        PackageDatabaseEntryCollection::Concrete clean_list;
        if (collision_list)
            for (PackageDatabaseEntryCollection::Iterator c(collision_list->begin()),
                    c_end(collision_list->end()) ; c != c_end ; ++c)
                if (dep->package.version != c->version)
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
                                 clean_list.begin(), clean_list.end(), " "))).max_exit_status)
                throw PackageInstallActionError("Clean aborted by hook");
            on_clean_all_pre(*dep, clean_list);

            for (PackageDatabaseEntryCollection::Iterator c(clean_list.begin()),
                    c_end(clean_list.end()) ; c != c_end ; ++c)
            {
                /* clean one item */
                if (0 != perform_hook(Hook("clean_pre")("TARGET", stringify(*c))
                             ("X_OF_Y", stringify(x) + " of " + stringify(y))).max_exit_status)
                    throw PackageInstallActionError("Clean of '" + cpvr + "' aborted by hook");
                on_clean_pre(*dep, *c);

                const RepositoryUninstallableInterface * const uninstall_interface(
                        _imp->env->package_database()->fetch_repository(c->repository)->
                        uninstallable_interface);
                if (! uninstall_interface)
                    throw InternalError(PALUDIS_HERE, "Trying to uninstall from a non-uninstallable repo");

                try
                {
                    uninstall_interface->uninstall(c->name, c->version, _imp->uninstall_options);
                }
                catch (const PackageUninstallActionError & e)
                {
                    on_clean_fail(*dep, *c);
                    HookResult PALUDIS_ATTRIBUTE((unused)) dummy(perform_hook(Hook("clean_fail")("TARGET", stringify(*c))("MESSAGE", e.message())));
                    throw;
                }

                on_clean_post(*dep, *c);
                if (0 != perform_hook(Hook("clean_post")("TARGET", stringify(*c))
                             ("X_OF_Y", stringify(x) + " of " + stringify(y))).max_exit_status)
                    throw PackageInstallActionError("Clean of '" + cpvr + "' aborted by hook");
            }

            /* we're done cleaning */
            if (0 != perform_hook(Hook("clean_all_post")("TARGETS", join(
                                 clean_list.begin(), clean_list.end(), " "))).max_exit_status)
                throw PackageInstallActionError("Clean aborted by hook");
            on_clean_all_post(*dep, clean_list);
        }

        /* if we installed paludis and a re-exec is available, use it. */
        if (dep->package.name == QualifiedPackageName("sys-apps/paludis"))
        {
            DepList::Iterator d(dep);
            do
            {
                ++d;
                if (d == dep_end)
                    break;
            }
            while (dlk_package != d->kind);

            if (d != dep_end)
                on_installed_paludis();
        }
    }

    /* update world */
    if (! _imp->install_options.fetch_only)
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
                    WhitespaceTokeniser::get_instance()->tokenise(*_imp->add_to_world_spec, std::back_inserter(tokens));
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
    if (_imp->install_options.fetch_only)
    {
        on_fetch_all_post();
        if (0 != perform_hook(Hook("fetch_all_post")("TARGETS", join(
                             _imp->raw_targets.begin(), _imp->raw_targets.end(), " "))).max_exit_status)
            throw PackageInstallActionError("Fetch aborted by hook");
    }
    else
    {
        on_install_all_post();
        if (0 != perform_hook(Hook("install_all_post")("TARGETS", join(
                             _imp->raw_targets.begin(), _imp->raw_targets.end(), " "))).max_exit_status)
            throw PackageInstallActionError("Install aborted by hook");
    }
}

const DepList &
InstallTask::dep_list() const
{
    return _imp->dep_list;
}

DepList::Iterator
InstallTask::current_dep_list_entry() const
{
    return _imp->current_dep_list_entry;
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
    _imp->install_options.fetch_only = value;
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
InstallTask::set_debug_mode(const InstallDebugOption value)
{
    _imp->install_options.debug_build = value;
}

void
InstallTask::set_add_to_world_spec(const std::string & value)
{
    _imp->add_to_world_spec.reset(new std::string(value));
}

InstallTask::TargetsIterator
InstallTask::begin_targets() const
{
    return TargetsIterator(_imp->raw_targets.begin());
}

InstallTask::TargetsIterator
InstallTask::end_targets() const
{
    return TargetsIterator(_imp->raw_targets.end());
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
    _imp->install_options.safe_resume = value;
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
            || s == SetName("everything") || s == SetName("insecurity"))
    {
        on_update_world_skip(s, "special sets cannot be added to world");
        return;
    }

    for (PackageDatabase::RepositoryIterator r(_imp->env->package_database()->begin_repositories()),
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

        InstallTask * const task;
        std::list<const PackageDepSpec *> items;
        bool inside_any;
        bool inside_use;

        WorldTargetFinder(InstallTask * const t) :
            task(t),
            inside_any(false),
            inside_use(false)
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
                items.push_back(&a);
                task->on_update_world(a);
            }
        }

        void visit_leaf(const BlockDepSpec &)
        {
        }
    };
}

void
InstallTask::world_update_packages(tr1::shared_ptr<const SetSpecTree::ConstItem> a)
{
    WorldTargetFinder w(this);
    a->accept(w);
    for (std::list<const PackageDepSpec *>::const_iterator i(w.items.begin()),
            i_end(w.items.end()) ; i != i_end ; ++i)
    {
        for (PackageDatabase::RepositoryIterator r(_imp->env->package_database()->begin_repositories()),
                r_end(_imp->env->package_database()->end_repositories()) ;
                r != r_end ; ++r)
            if ((*r)->world_interface && (*i)->package_ptr())
                (*r)->world_interface->add_to_world(*(*i)->package_ptr());
    }
}

