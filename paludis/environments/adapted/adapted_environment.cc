/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/hashed_containers.hh>
#include <paludis/dep_spec.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/match_package.hh>

using namespace paludis;

typedef MakeHashedMultiMap<UseFlagName, std::pair<tr1::shared_ptr<const PackageDepSpec>, UseFlagState> >::Type Use;

namespace paludis
{
    template<>
    struct Implementation<AdaptedEnvironment>
    {
        tr1::shared_ptr<Environment> env;
        Use use;

        Implementation(tr1::shared_ptr<Environment> e) :
            env(e)
        {
        }
    };
}

AdaptedEnvironment::AdaptedEnvironment(tr1::shared_ptr<Environment> e) :
    PrivateImplementationPattern<AdaptedEnvironment>(new Implementation<AdaptedEnvironment>(e))
{
}

AdaptedEnvironment::~AdaptedEnvironment()
{
}

void
AdaptedEnvironment::adapt_use(tr1::shared_ptr<const PackageDepSpec> p,
        const UseFlagName & u, const UseFlagState s)
{
    _imp->use.insert(std::make_pair(u, std::make_pair(p, s)));
}

void
AdaptedEnvironment::clear_adaptions()
{
    _imp.reset(new Implementation<AdaptedEnvironment>(_imp->env));
}

tr1::shared_ptr<PackageDatabase>
AdaptedEnvironment::package_database()
{
    return _imp->env->package_database();
}

tr1::shared_ptr<const PackageDatabase>
AdaptedEnvironment::package_database() const
{
    return _imp->env->package_database();
}

bool
AdaptedEnvironment::query_use(const UseFlagName & u, const PackageDatabaseEntry & e) const
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

tr1::shared_ptr<const UseFlagNameCollection>
AdaptedEnvironment::known_use_expand_names(const UseFlagName & u, const PackageDatabaseEntry & e) const
{
    return _imp->env->known_use_expand_names(u, e);
}

MaskReasons
AdaptedEnvironment::mask_reasons(const PackageDatabaseEntry & e, const MaskReasonsOptions & r) const
{
    return _imp->env->mask_reasons(e, r);
}

bool
AdaptedEnvironment::accept_license(const std::string & l, const PackageDatabaseEntry & e) const
{
    return _imp->env->accept_license(l, e);
}

bool
AdaptedEnvironment::accept_keywords(tr1::shared_ptr<const KeywordNameCollection> k, const PackageDatabaseEntry & e) const
{
    return _imp->env->accept_keywords(k, e);
}

tr1::shared_ptr<const FSEntryCollection>
AdaptedEnvironment::bashrc_files() const
{
    return _imp->env->bashrc_files();
}

tr1::shared_ptr<const FSEntryCollection>
AdaptedEnvironment::syncers_dirs() const
{
    return _imp->env->syncers_dirs();
}

tr1::shared_ptr<const FSEntryCollection>
AdaptedEnvironment::fetchers_dirs() const
{
    return _imp->env->fetchers_dirs();
}

tr1::shared_ptr<const FSEntryCollection>
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

tr1::shared_ptr<const MirrorsCollection>
AdaptedEnvironment::mirrors(const std::string & m) const
{
    return _imp->env->mirrors(m);
}

tr1::shared_ptr<const SetNameCollection>
AdaptedEnvironment::set_names() const
{
    return _imp->env->set_names();
}

tr1::shared_ptr<SetSpecTree::ConstItem>
AdaptedEnvironment::set(const SetName & s) const
{
    return _imp->env->set(s);
}

tr1::shared_ptr<const DestinationsCollection>
AdaptedEnvironment::default_destinations() const
{
    return _imp->env->default_destinations();
}

HookResult
AdaptedEnvironment::perform_hook(const Hook & h) const
{
    return _imp->env->perform_hook(h);
}

