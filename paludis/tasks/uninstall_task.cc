/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "uninstall_task.hh"
#include <paludis/environment.hh>
#include <paludis/dep_list/uninstall_list.hh>
#include <paludis/util/collection_concrete.hh>
#include <list>

using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<UninstallTask> :
        InternalCounted<Implementation<UninstallTask> >
    {
        Environment * const env;
        InstallOptions install_options;

        std::list<std::string> raw_targets;
        std::list<PackageDepAtom::Pointer> targets;

        bool pretend;
        bool preserve_world;
        bool with_unused_dependencies;
        bool with_dependencies;
        bool unused;

        Implementation<UninstallTask>(Environment * const e) :
            env(e),
            install_options(false, false, ido_none),
            pretend(false),
            preserve_world(false),
            with_unused_dependencies(false),
            with_dependencies(false),
            unused(false)
        {
        }
    };
}

UninstallTask::UninstallTask(Environment * const e) :
    PrivateImplementationPattern<UninstallTask>(new Implementation<UninstallTask>(e))
{
}

UninstallTask::~UninstallTask()
{
}

void
UninstallTask::set_pretend(const bool v)
{
    _imp->pretend = v;
}

void
UninstallTask::set_no_config_protect(const bool v)
{
    _imp->install_options.no_config_protect = v;
}

void
UninstallTask::set_preserve_world(const bool v)
{
    _imp->preserve_world = v;
}

void
UninstallTask::add_target(const std::string & target)
{
    Context context("When adding uninstall target '" + target + "':");

    /* we might have a dep atom, but we might just have a simple package name
     * without a category. either should work. */
    if (std::string::npos != target.find('/'))
        _imp->targets.push_back(PackageDepAtom::Pointer(new PackageDepAtom(target)));
    else
        _imp->targets.push_back(PackageDepAtom::Pointer(new PackageDepAtom(
                        _imp->env->package_database()->fetch_unique_qualified_package_name(
                            PackageNamePart(target)))));

    _imp->raw_targets.push_back(target);

    if (_imp->unused)
        throw InternalError(PALUDIS_HERE, "Trying to mix unused and normal targets?");
}

void
UninstallTask::add_unused()
{
    Context context("When adding unused packages:");
    _imp->unused = true;

    if (! _imp->raw_targets.empty())
        throw InternalError(PALUDIS_HERE, "Trying to mix unused and normal targets?");
}

namespace
{
    struct WorldCallbacks :
        public Environment::WorldCallbacks
    {
        UninstallTask * const t;

        WorldCallbacks(UninstallTask * const tt) :
            t(tt)
        {
        }

        virtual void remove_callback(const PackageDepAtom * a)
        {
            t->on_update_world(*a);
        }
    };
}

void
UninstallTask::execute()
{
    Context context("When executing uninstall task:");

    on_build_unmergelist_pre();

    UninstallList list(_imp->env, UninstallListOptions::create()
            .with_dependencies(_imp->with_dependencies)
            .with_unused_dependencies(_imp->with_unused_dependencies));

    if (_imp->unused)
        list.add_unused();
    else
        for (std::list<PackageDepAtom::Pointer>::const_iterator t(_imp->targets.begin()),
                t_end(_imp->targets.end()) ; t != t_end ; ++t)
        {
            Context local_context("When looking for target '" + stringify(**t) + "':");

            PackageDatabaseEntryCollection::ConstPointer r(_imp->env->package_database()->query(
                        **t, is_installed_only));
            if (r->empty())
                throw NoSuchPackageError(stringify(**t));
            else if (next(r->begin()) != r->end())
                throw AmbiguousUnmergeTargetError(stringify(**t), r);
            else
                list.add(*r->begin());
        }

    on_build_unmergelist_post();

    on_display_unmerge_list_pre();

    for (UninstallList::Iterator i(list.begin()), i_end(list.end()) ; i != i_end ; ++i)
        on_display_unmerge_list_entry(*i);

    on_display_unmerge_list_post();

    if (_imp->pretend)
        return;

    if (_imp->preserve_world)
        on_preserve_world();
    else
    {
        on_update_world_pre();

        AllDepAtom::Pointer all(new AllDepAtom);
        for (std::list<PackageDepAtom::Pointer>::const_iterator t(_imp->targets.begin()),
                t_end(_imp->targets.end()) ; t != t_end ; ++t)
            all->add_child(*t);

        WorldCallbacks w(this);
        _imp->env->remove_appropriate_from_world(all, &w);

        on_update_world_post();
    }

    _imp->env->perform_hook(Hook("uninstall_all_pre")("TARGETS", join(_imp->raw_targets.begin(),
                    _imp->raw_targets.end(), " ")));
    on_uninstall_all_pre();

    int x(0), y(0);
    for (UninstallList::Iterator i(list.begin()), i_end(list.end()) ; i != i_end ; ++i)
        if (! i->skip_uninstall)
            ++y;

    for (UninstallList::Iterator i(list.begin()), i_end(list.end()) ; i != i_end ; ++i)
    {
        if (i->skip_uninstall)
            continue;
        ++x;

        std::string cpvr(stringify(i->package));

        _imp->env->perform_hook(Hook("uninstall_pre")("TARGET", cpvr)
                ("X_OF_Y", stringify(x) + " of " + stringify(y)));
        on_uninstall_pre(*i);

        const RepositoryUninstallableInterface * const uninstall_interface(
                _imp->env->package_database()->fetch_repository(i->package.repository)->
                uninstallable_interface);
        if (! uninstall_interface)
            throw InternalError(PALUDIS_HERE, "Trying to uninstall from a non-uninstallable repo");

        try
        {
            uninstall_interface->uninstall(i->package.name, i->package.version, _imp->install_options);
        }
        catch (const PackageUninstallActionError & e)
        {
            _imp->env->perform_hook(Hook("uninstall_fail")("TARGET", cpvr)("MESSAGE", e.message()));
            throw;
        }

        on_uninstall_post(*i);
        _imp->env->perform_hook(Hook("uninstall_post")("TARGET", cpvr)
                ("X_OF_Y", stringify(x) + " of " + stringify(y)));
    }

    on_uninstall_all_post();
    _imp->env->perform_hook(Hook("uninstall_all_post")("TARGETS", join(_imp->raw_targets.begin(),
                    _imp->raw_targets.end(), " ")));
}

AmbiguousUnmergeTargetError::~AmbiguousUnmergeTargetError() throw ()
{
}

void
UninstallTask::set_with_unused_dependencies(const bool value)
{
    _imp->with_unused_dependencies = value;
}

void
UninstallTask::set_with_dependencies(const bool value)
{
    _imp->with_dependencies = value;
}

