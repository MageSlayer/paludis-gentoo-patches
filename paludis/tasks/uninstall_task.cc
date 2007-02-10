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

#include "uninstall_task.hh"
#include <paludis/environment.hh>
#include <paludis/dep_list/uninstall_list.hh>
#include <paludis/dep_atom_flattener.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/tasks/exceptions.hh>
#include <list>

using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<UninstallTask>
    {
        Environment * const env;
        InstallOptions install_options;

        std::list<std::string> raw_targets;
        std::list<std::tr1::shared_ptr<PackageDepAtom> > targets;

        bool pretend;
        bool preserve_world;
        bool all_versions;
        bool with_unused_dependencies;
        bool with_dependencies;
        bool unused;

        bool had_set_targets;
        bool had_package_targets;

        Implementation<UninstallTask>(Environment * const e) :
            env(e),
            install_options(false, false, ido_none, false, std::tr1::shared_ptr<Repository>()),
            pretend(false),
            preserve_world(false),
            all_versions(false),
            with_unused_dependencies(false),
            with_dependencies(false),
            unused(false),
            had_set_targets(false),
            had_package_targets(false)
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
    {
        if (_imp->had_set_targets)
            throw HadBothPackageAndSetTargets();

        _imp->had_package_targets = true;
        _imp->targets.push_back(std::tr1::shared_ptr<PackageDepAtom>(new PackageDepAtom(target)));
    }
    else
        try
        {
            std::tr1::shared_ptr<DepAtom> atom(_imp->env->package_set(SetName(target)));
            if (atom)
            {
                if (_imp->had_package_targets)
                    throw HadBothPackageAndSetTargets();

                if (_imp->had_set_targets)
                    throw MultipleSetTargetsSpecified();

                _imp->had_set_targets = true;
                DepAtomFlattener f(_imp->env, 0, atom);
                for (DepAtomFlattener::Iterator i(f.begin()), i_end(f.end()) ; i != i_end ; ++i)
                    _imp->targets.push_back(std::tr1::shared_ptr<PackageDepAtom>(new PackageDepAtom(
                                    stringify((*i)->text()))));
            }
            else
            {
                if (_imp->had_set_targets)
                    throw HadBothPackageAndSetTargets();

                _imp->had_package_targets = false;
                _imp->targets.push_back(std::tr1::shared_ptr<PackageDepAtom>(new PackageDepAtom(
                                _imp->env->package_database()->fetch_unique_qualified_package_name(
                                    PackageNamePart(target)))));
            }
        }
        catch (const SetNameError &)
        {
            if (_imp->had_set_targets)
                throw HadBothPackageAndSetTargets();

            _imp->had_package_targets = false;
            _imp->targets.push_back(std::tr1::shared_ptr<PackageDepAtom>(new PackageDepAtom(
                            _imp->env->package_database()->fetch_unique_qualified_package_name(
                                PackageNamePart(target)))));
        }

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

        virtual void remove_callback(const PackageDepAtom & a)
        {
            t->on_update_world(a);
        }

        virtual void remove_callback(const SetName & a)
        {
            t->on_update_world(a);
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
        for (std::list<std::tr1::shared_ptr<PackageDepAtom> >::const_iterator t(_imp->targets.begin()),
                t_end(_imp->targets.end()) ; t != t_end ; ++t)
        {
            Context local_context("When looking for target '" + stringify(**t) + "':");

            std::tr1::shared_ptr<const PackageDatabaseEntryCollection> r(_imp->env->package_database()->query(
                        **t, is_installed_only, qo_order_by_version));
            if (r->empty())
            {
                if (! _imp->had_set_targets)
                    throw NoSuchPackageError(stringify(**t));
            }
            else if (next(r->begin()) != r->end())
            {
                if (_imp->all_versions)
                {
                    /* all_versions, not all_packages. */
                    for (PackageDatabaseEntryCollection::Iterator i_start(r->begin()), i(r->begin()),
                            i_end(r->end()) ; i != i_end ; ++i)
                        if (i->name != i_start->name)
                            throw AmbiguousUnmergeTargetError(stringify(**t), r);

                    for (PackageDatabaseEntryCollection::Iterator i(r->begin()), i_end(r->end()) ;
                            i != i_end ; ++i)
                        list.add(*i);
                }
                else
                    throw AmbiguousUnmergeTargetError(stringify(**t), r);
            }
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

        std::tr1::shared_ptr<AllDepAtom> all(new AllDepAtom);

        std::map<QualifiedPackageName, std::set<VersionSpec> > being_removed;
        for (UninstallList::Iterator i(list.begin()), i_end(list.end()) ; i != i_end ; ++i)
            if (! i->skip_uninstall)
                being_removed[i->package.name].insert(i->package.version);

        for (std::map<QualifiedPackageName, std::set<VersionSpec> >::const_iterator
                i(being_removed.begin()), i_end(being_removed.end()) ; i != i_end ; ++i)
        {
            bool remove(true);
            std::tr1::shared_ptr<PackageDatabaseEntryCollection> installed(
                    _imp->env->package_database()->query(PackageDepAtom(i->first),
                        is_installed_only, qo_whatever));
            for (PackageDatabaseEntryCollection::Iterator r(installed->begin()), r_end(installed->end()) ;
                    r != r_end && remove ; ++r)
                if (i->second.end() == i->second.find(r->version))
                    remove = false;

            if (remove)
                all->add_child(std::tr1::shared_ptr<PackageDepAtom>(new PackageDepAtom(i->first)));
        }

        WorldCallbacks w(this);
        _imp->env->remove_appropriate_from_world(all, &w);

        if (_imp->had_set_targets)
            for (std::list<std::string>::const_iterator t(_imp->raw_targets.begin()),
                    t_end(_imp->raw_targets.end()) ; t != t_end ; ++t)
                _imp->env->remove_set_from_world(SetName(*t), &w);

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
            _imp->install_options.destination = _imp->env->package_database()->fetch_repository(i->package.repository);
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

void
UninstallTask::set_all_versions(const bool value)
{
    _imp->all_versions = value;
}

