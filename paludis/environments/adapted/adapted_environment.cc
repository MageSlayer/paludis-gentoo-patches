/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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

#include "adapted_environment.hh"
#include <paludis/dep_spec.hh>
#include <paludis/hook.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/hashes.hh>
#include <paludis/match_package.hh>
#include <tr1/unordered_map>

using namespace paludis;

typedef std::tr1::unordered_multimap<UseFlagName, std::pair<std::tr1::shared_ptr<const PackageDepSpec>, UseFlagState>, Hash<UseFlagName> > Use;

namespace paludis
{
    template<>
    struct Implementation<AdaptedEnvironment>
    {
        std::tr1::shared_ptr<Environment> env;
        Use use;

        Implementation(std::tr1::shared_ptr<Environment> e) :
            env(e)
        {
        }
    };
}

AdaptedEnvironment::AdaptedEnvironment(std::tr1::shared_ptr<Environment> e) :
    PrivateImplementationPattern<AdaptedEnvironment>(new Implementation<AdaptedEnvironment>(e))
{
}

AdaptedEnvironment::~AdaptedEnvironment()
{
}

void
AdaptedEnvironment::adapt_use(std::tr1::shared_ptr<const PackageDepSpec> p,
        const UseFlagName & u, const UseFlagState s)
{
    _imp->use.insert(std::make_pair(u, std::make_pair(p, s)));
}

void
AdaptedEnvironment::clear_adaptions()
{
    _imp.reset(new Implementation<AdaptedEnvironment>(_imp->env));
}

std::tr1::shared_ptr<PackageDatabase>
AdaptedEnvironment::package_database()
{
    return _imp->env->package_database();
}

std::tr1::shared_ptr<const PackageDatabase>
AdaptedEnvironment::package_database() const
{
    return _imp->env->package_database();
}

bool
AdaptedEnvironment::query_use(const UseFlagName & u, const PackageID & e) const
{
    UseFlagState result(use_unspecified);
    for (std::pair<Use::const_iterator, Use::const_iterator> p(_imp->use.equal_range(u)) ;
            p.first != p.second ; ++p.first)
        if (match_package(*this, *p.first->second.first, e))
            result = p.first->second.second;

    switch (result)
    {
        case use_enabled:
            return true;

        case use_disabled:
            return false;

        case use_unspecified:
            return _imp->env->query_use(u, e);

        case last_use:
            ;
    }

    throw InternalError(PALUDIS_HERE, "Bad state");
}

std::tr1::shared_ptr<const UseFlagNameSet>
AdaptedEnvironment::known_use_expand_names(const UseFlagName & u, const PackageID & e) const
{
    return _imp->env->known_use_expand_names(u, e);
}

bool
AdaptedEnvironment::accept_license(const std::string & l, const PackageID & e) const
{
    return _imp->env->accept_license(l, e);
}

bool
AdaptedEnvironment::accept_keywords(std::tr1::shared_ptr<const KeywordNameSet> k, const PackageID & e) const
{
    return _imp->env->accept_keywords(k, e);
}

std::tr1::shared_ptr<const FSEntrySequence>
AdaptedEnvironment::bashrc_files() const
{
    return _imp->env->bashrc_files();
}

std::tr1::shared_ptr<const FSEntrySequence>
AdaptedEnvironment::syncers_dirs() const
{
    return _imp->env->syncers_dirs();
}

std::tr1::shared_ptr<const FSEntrySequence>
AdaptedEnvironment::fetchers_dirs() const
{
    return _imp->env->fetchers_dirs();
}

std::tr1::shared_ptr<const FSEntrySequence>
AdaptedEnvironment::hook_dirs() const
{
    return _imp->env->hook_dirs();
}

std::string
AdaptedEnvironment::paludis_command() const
{
    return _imp->env->paludis_command();
}

void
AdaptedEnvironment::set_paludis_command(const std::string & s)
{
    _imp->env->set_paludis_command(s);
}

const FSEntry
AdaptedEnvironment::root() const
{
    return _imp->env->root();
}

uid_t
AdaptedEnvironment::reduced_uid() const
{
    return _imp->env->reduced_uid();
}

gid_t
AdaptedEnvironment::reduced_gid() const
{
    return _imp->env->reduced_gid();
}

std::tr1::shared_ptr<const MirrorsSequence>
AdaptedEnvironment::mirrors(const std::string & m) const
{
    return _imp->env->mirrors(m);
}

std::tr1::shared_ptr<const SetNameSet>
AdaptedEnvironment::set_names() const
{
    return _imp->env->set_names();
}

std::tr1::shared_ptr<SetSpecTree::ConstItem>
AdaptedEnvironment::set(const SetName & s) const
{
    return _imp->env->set(s);
}

std::tr1::shared_ptr<const DestinationsSet>
AdaptedEnvironment::default_destinations() const
{
    return _imp->env->default_destinations();
}

HookResult
AdaptedEnvironment::perform_hook(const Hook & h) const
{
    return _imp->env->perform_hook(h);
}

std::string
AdaptedEnvironment::default_distribution() const
{
    return _imp->env->default_distribution();
}

const std::tr1::shared_ptr<const Mask>
AdaptedEnvironment::mask_for_breakage(const PackageID & id) const
{
    return _imp->env->mask_for_breakage(id);
}

const std::tr1::shared_ptr<const Mask>
AdaptedEnvironment::mask_for_user(const PackageID & id) const
{
    return _imp->env->mask_for_user(id);
}

bool
AdaptedEnvironment::unmasked_by_user(const PackageID & id) const
{
    return _imp->env->unmasked_by_user(id);
}

bool
AdaptedEnvironment::is_paludis_package(const QualifiedPackageName & q) const
{
    return _imp->env->is_paludis_package(q);
}

void
AdaptedEnvironment::add_to_world(const QualifiedPackageName & q) const
{
    _imp->env->add_to_world(q);
}

void
AdaptedEnvironment::add_to_world(const SetName & s) const
{
    _imp->env->add_to_world(s);
}

void
AdaptedEnvironment::remove_from_world(const QualifiedPackageName & q) const
{
    _imp->env->remove_from_world(q);
}

void
AdaptedEnvironment::remove_from_world(const SetName & s) const
{
    _imp->env->remove_from_world(s);
}

std::tr1::shared_ptr<PackageIDSequence>
AdaptedEnvironment::operator[] (const Selection & s) const
{
    return _imp->env->operator[] (s);
}

