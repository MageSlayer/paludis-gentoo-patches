/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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
#include <paludis/dep_atom.hh>
#include <paludis/portage_dep_parser.hh>
#include <paludis/util/collection_concrete.hh>
#include <list>

using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<InstallTask> :
        InternalCounted<Implementation<InstallTask> >
    {
        Environment * const env;
        DepList dep_list;
        InstallOptions install_options;

        std::list<std::string> raw_targets;
        AllDepAtom::Pointer targets;

        bool pretend;
        bool preserve_world;

        bool had_set_targets;
        bool had_package_targets;

        Implementation<InstallTask>(Environment * const e) :
            env(e),
            dep_list(e),
            install_options(false, false),
            targets(new AllDepAtom),
            pretend(false),
            preserve_world(false),
            had_set_targets(false),
            had_package_targets(false)
        {
        }
    };
}

MultipleSetTargetsSpecified::MultipleSetTargetsSpecified() throw () :
    Exception("More than one set target was specified")
{
}

HadBothPackageAndSetTargets::HadBothPackageAndSetTargets() throw () :
    Exception("Both package and set targets were specified")
{
}

InstallTask::InstallTask(Environment * const env) :
    PrivateImplementationPattern<InstallTask>(new Implementation<InstallTask>(env))
{
}

InstallTask::~InstallTask()
{
}

void
InstallTask::set_rdepend_post(const DepListRdependOption value)
{
    _imp->dep_list.set_rdepend_post(value);
}

void
InstallTask::set_drop_self_circular(const bool value)
{
    _imp->dep_list.set_drop_self_circular(value);
}

void
InstallTask::set_drop_circular(const bool value)
{
    _imp->dep_list.set_drop_circular(value);
}

void
InstallTask::set_drop_all(const bool value)
{
    _imp->dep_list.set_drop_all(value);
}

void
InstallTask::set_ignore_installed(const bool value)
{
    _imp->dep_list.set_ignore_installed(value);
}

void
InstallTask::set_recursive_deps(const bool value)
{
    _imp->dep_list.set_recursive_deps(value);
}

void
InstallTask::set_max_stack_depth(const int value)
{
    _imp->dep_list.set_max_stack_depth(value);
}

void
InstallTask::set_no_unnecessary_upgrades(const bool value)
{
    _imp->dep_list.set_no_unnecessary_upgrades(value);
}

void
InstallTask::set_no_config_protect(const bool value)
{
    _imp->install_options.set<io_noconfigprotect>(value);
}

void
InstallTask::set_fetch_only(const bool value)
{
    _imp->install_options.set<io_fetchonly>(value);
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
InstallTask::add_target(const std::string & target)
{
    Context context("When adding install target '" + target + "':");

    DepAtom::Pointer s(0);

    if (s = ((_imp->env->package_set(target))))
    {
        if (_imp->had_set_targets)
            throw MultipleSetTargetsSpecified();

        if (_imp->had_package_targets)
            throw HadBothPackageAndSetTargets();

        _imp->had_set_targets = true;
        _imp->dep_list.set_reinstall(false);
        _imp->targets->add_child(s);
    }
    else
    {
        if (_imp->had_set_targets)
            throw HadBothPackageAndSetTargets();

        _imp->had_package_targets = true;

        if (std::string::npos != target.find('/'))
            _imp->targets->add_child(PortageDepParser::parse(target));
        else
            _imp->targets->add_child(DepAtom::Pointer(new PackageDepAtom(
                            _imp->env->package_database()->fetch_unique_qualified_package_name(
                                PackageNamePart(target)))));
    }

    _imp->raw_targets.push_back(target);
}

namespace
{
    struct WorldCallbacks :
        public Environment::WorldCallbacks
    {
        InstallTask * const t;

        WorldCallbacks(InstallTask * const tt) :
            t(tt)
        {
        }

        virtual void add_callback(const PackageDepAtom * a)
        {
            t->on_update_world(*a);
        }

        virtual void skip_callback(const PackageDepAtom * a,
                const std::string & s)
        {
            t->on_update_world_skip(*a, s);
        }
    };
}

void
InstallTask::execute()
{
    Context context("When executing install task:");

    /* build up our dep list */
    on_build_deplist_pre();
    _imp->dep_list.add(_imp->targets);
    on_build_deplist_post();

    /* we're about to display our task list */
    if (_imp->pretend)
        _imp->env->perform_hook(Hook("install_pretend_pre")("TARGETS", join(_imp->raw_targets.begin(),
                        _imp->raw_targets.end(), " ")));

    on_display_merge_list_pre();

    /* display our task list */
    for (DepList::Iterator dep(_imp->dep_list.begin()), dep_end(_imp->dep_list.end()) ;
            dep != dep_end ; ++dep)
        on_display_merge_list_entry(*dep);

    /* we're done displaying our task list */
    on_display_merge_list_post();

    if (_imp->pretend)
    {
        _imp->env->perform_hook(Hook("install_pretend_post")("TARGETS", join(
                        _imp->raw_targets.begin(), _imp->raw_targets.end(), " ")));
        return;
    }

    /* we're about to fetch / install the entire list */
    if (_imp->install_options.get<io_fetchonly>())
    {
        _imp->env->perform_hook(Hook("fetch_all_pre")("TARGETS", join(
                        _imp->raw_targets.begin(), _imp->raw_targets.end(), " ")));
        on_fetch_all_pre();
    }
    else
    {
        _imp->env->perform_hook(Hook("install_all_pre")("TARGETS", join(
                        _imp->raw_targets.begin(), _imp->raw_targets.end(), " ")));
        on_install_all_pre();
    }

    /* fetch / install our entire list */
    for (DepList::Iterator dep(_imp->dep_list.begin()), dep_end(_imp->dep_list.end()) ;
            dep != dep_end ; ++dep)
    {
        std::string cpvr(stringify(dep->get<dle_name>()) + "-" +
                stringify(dep->get<dle_version>()) + "::" +
                stringify(dep->get<dle_repository>()));

        /* we're about to fetch / install one item */
        if (_imp->install_options.get<io_fetchonly>())
        {
            _imp->env->perform_hook(Hook("fetch_pre")("TARGET", cpvr));
            on_fetch_pre(*dep);
        }
        else
        {
            _imp->env->perform_hook(Hook("install_pre")("TARGET", cpvr));
            on_install_pre(*dep);
        }

        /* fetch / install one item */
        const Repository::InstallableInterface * const installable_interface(
                _imp->env->package_database()->fetch_repository(dep->get<dle_repository>())->
                get_interface<repo_installable>());
        if (! installable_interface)
            throw InternalError(PALUDIS_HERE, "Trying to install from a non-installable repository");

        try
        {
            installable_interface->install(dep->get<dle_name>(), dep->get<dle_version>(), _imp->install_options);
        }
        catch (const PackageInstallActionError & e)
        {
            _imp->env->perform_hook(Hook("install_fail")("TARGET", cpvr)("MESSAGE", e.message()));
            throw;
        }

        /* we've fetched / installed one item */
        if (_imp->install_options.get<io_fetchonly>())
        {
            on_fetch_post(*dep);
            _imp->env->perform_hook(Hook("fetch_post")("TARGET", cpvr));
        }
        else
        {
            on_install_post(*dep);
            _imp->env->perform_hook(Hook("install_post")("TARGET", cpvr));
        }

        if (_imp->install_options.get<io_fetchonly>())
            continue;

        /* figure out whether we need to unmerge (clean) anything */
        on_build_cleanlist_pre(*dep);

        // manually invalidate any installed repos, they're probably
        // wrong now
        for (PackageDatabase::RepositoryIterator r(_imp->env->package_database()->begin_repositories()),
                r_end(_imp->env->package_database()->end_repositories()) ; r != r_end ; ++r)
            if ((*r)->get_interface<repo_installed>())
                (*r)->invalidate();

        // look for packages with the same name in the same slot
        PackageDatabaseEntryCollection::Pointer collision_list(_imp->env->package_database()->query(
                    PackageDepAtom::Pointer(new PackageDepAtom(
                            stringify(dep->get<dle_name>()) + ":" +
                            stringify(dep->get<dle_metadata>()->get<vm_slot>()))),
                    is_installed_only));

        // don't clean the thing we just installed
        PackageDatabaseEntryCollection::Concrete clean_list;
        for (PackageDatabaseEntryCollection::Iterator c(collision_list->begin()),
                c_end(collision_list->end()) ; c != c_end ; ++c)
            if (dep->get<dle_version>() != c->get<pde_version>())
                clean_list.insert(*c);

        on_build_cleanlist_post(*dep);

        /* ok, we have the cleanlist. we're about to clean */
        _imp->env->perform_hook(Hook("uninstall_all_pre")("TARGETS", join(
                        clean_list.begin(), clean_list.end(), " ")));
        on_clean_all_pre(*dep, clean_list);

        for (PackageDatabaseEntryCollection::Iterator c(clean_list.begin()),
                c_end(clean_list.end()) ; c != c_end ; ++c)
        {
            /* clean one item */
            _imp->env->perform_hook(Hook("uninstall_pre")("TARGET", stringify(*c)));
            on_clean_pre(*dep, *c);

            const Repository::UninstallableInterface * const uninstall_interface(
                    _imp->env->package_database()->fetch_repository(c->get<pde_repository>())->
                    get_interface<repo_uninstallable>());
            if (! uninstall_interface)
                throw InternalError(PALUDIS_HERE, "Trying to uninstall from a non-uninstallable repo");

            try
            {
                uninstall_interface->uninstall(c->get<pde_name>(), c->get<pde_version>(), _imp->install_options);
            }
            catch (const PackageUninstallActionError & e)
            {
                _imp->env->perform_hook(Hook("uninstall_fail")("TARGET", stringify(*c))("MESSAGE", e.message()));
                throw;
            }

            on_clean_post(*dep, *c);
            _imp->env->perform_hook(Hook("uninstall_post")("TARGET", stringify(*c)));
        }

        /* we're done cleaning */
        _imp->env->perform_hook(Hook("uninstall_all_post")("TARGETS", join(
                        clean_list.begin(), clean_list.end(), " ")));
        on_clean_all_post(*dep, clean_list);
    }

    /* update world */
    if (! _imp->had_set_targets)
    {
        if (! _imp->preserve_world)
        {
            on_update_world_pre();
            WorldCallbacks w(this);
            _imp->env->add_appropriate_to_world(_imp->targets, &w);
            on_update_world_post();
        }
        else
            on_preserve_world();
    }

    /* we've fetched / installed the entire list */
    if (_imp->install_options.get<io_fetchonly>())
    {
        on_fetch_all_post();
        _imp->env->perform_hook(Hook("fetch_all_post")("TARGETS", join(
                        _imp->raw_targets.begin(), _imp->raw_targets.end(), " ")));
    }
    else
    {
        on_install_all_post();
        _imp->env->perform_hook(Hook("install_all_post")("TARGETS", join(
                        _imp->raw_targets.begin(), _imp->raw_targets.end(), " ")));
    }
}

