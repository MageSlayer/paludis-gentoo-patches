/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/match_package.hh>
#include <paludis/dep_spec.hh>
#include <paludis/dep_spec_flattener.hh>
#include <paludis/environment.hh>
#include <paludis/version_requirements.hh>
#include <paludis/package_database.hh>
#include <paludis/package_id.hh>
#include <paludis/slot_requirement.hh>
#include <paludis/metadata_key.hh>
#include <paludis/util/set.hh>
#include <paludis/util/options.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/sequence.hh>
#include <paludis/action.hh>
#include <paludis/repository.hh>
#include <paludis/additional_package_dep_spec_requirement.hh>

#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/make_null_shared_ptr.hh>

#include <functional>
#include <algorithm>
#include <istream>
#include <ostream>

using namespace paludis;

#include <paludis/match_package-se.cc>

namespace
{
    struct SlotRequirementChecker
    {
        const std::shared_ptr<const PackageID> id;
        bool result;

        SlotRequirementChecker(const std::shared_ptr<const PackageID> & i) :
            id(i),
            result(true)
        {
        }

        void visit(const SlotExactRequirement & s)
        {
            result = id->slot_key() && id->slot_key()->value() == s.slot();
        }

        void visit(const SlotAnyLockedRequirement &)
        {
            result = true;
        }

        void visit(const SlotAnyUnlockedRequirement &)
        {
            result = true;
        }
    };
}

bool
paludis::match_package_with_maybe_changes(
        const Environment & env,
        const PackageDepSpec & spec,
        const ChangedChoices * const maybe_changes_to_owner,
        const std::shared_ptr<const PackageID> & id,
        const std::shared_ptr<const PackageID> & from_id,
        const ChangedChoices * const maybe_changes_to_target,
        const MatchPackageOptions & options)
{
    if (spec.package_ptr() && *spec.package_ptr() != id->name())
        return false;

    if (spec.package_name_part_ptr() && *spec.package_name_part_ptr() != id->name().package())
        return false;

    if (spec.category_name_part_ptr() && *spec.category_name_part_ptr() != id->name().category())
        return false;

    if (spec.version_requirements_ptr())
        switch (spec.version_requirements_mode())
        {
            case vr_and:
                for (VersionRequirements::ConstIterator r(spec.version_requirements_ptr()->begin()),
                        r_end(spec.version_requirements_ptr()->end()) ; r != r_end ; ++r)
                    if (! r->version_operator().as_version_spec_comparator()(id->version(), r->version_spec()))
                        return false;
                break;

            case vr_or:
                {
                    bool matched(false);
                    for (VersionRequirements::ConstIterator r(spec.version_requirements_ptr()->begin()),
                            r_end(spec.version_requirements_ptr()->end()) ; r != r_end ; ++r)
                        if (r->version_operator().as_version_spec_comparator()(id->version(), r->version_spec()))
                        {
                            matched = true;
                            break;
                        }

                    if (! matched)
                        return false;
                }
                break;

            case last_vr:
                ;
        }

    if (spec.in_repository_ptr())
        if (*spec.in_repository_ptr() != id->repository_name())
            return false;

    if (spec.from_repository_ptr())
    {
        if (! id->from_repositories_key())
            return false;

        if (id->from_repositories_key()->value()->end() == id->from_repositories_key()->value()->find(
                    stringify(*spec.from_repository_ptr())))
            return false;
    }

    if (spec.installed_at_path_ptr())
    {
        auto repo(env.package_database()->fetch_repository(id->repository_name()));
        if (! repo->installed_root_key())
            return false;
        if (repo->installed_root_key()->value() != *spec.installed_at_path_ptr())
            return false;
    }

    if (spec.installable_to_repository_ptr())
    {
        if (! id->supports_action(SupportsActionTest<InstallAction>()))
            return false;
        if (! spec.installable_to_repository_ptr()->include_masked())
            if (id->masked())
                return false;

        const std::shared_ptr<const Repository> dest(env.package_database()->fetch_repository(
                    spec.installable_to_repository_ptr()->repository()));
        if (! dest->destination_interface())
            return false;
        if (! dest->destination_interface()->is_suitable_destination_for(id))
            return false;
    }

    if (spec.installable_to_path_ptr())
    {
        if (! id->supports_action(SupportsActionTest<InstallAction>()))
            return false;
        if (! spec.installable_to_path_ptr()->include_masked())
            if (id->masked())
                return false;

        bool ok(false);
        for (PackageDatabase::RepositoryConstIterator d(env.package_database()->begin_repositories()),
                d_end(env.package_database()->end_repositories()) ;
                d != d_end ; ++d)
        {
            if (! (*d)->destination_interface())
                continue;
            if (! (*d)->installed_root_key())
                continue;
            if ((*d)->installed_root_key()->value() != spec.installable_to_path_ptr()->path())
                continue;
            if (! (*d)->destination_interface()->is_suitable_destination_for(id))
                continue;

            ok = true;
            break;
        }

        if (! ok)
            return false;
    }

    if (spec.slot_requirement_ptr())
    {
        SlotRequirementChecker v(id);
        spec.slot_requirement_ptr()->accept(v);
        if (! v.result)
            return false;
    }

    if (! options[mpo_ignore_additional_requirements])
    {
        if (spec.additional_requirements_ptr())
        {
            for (AdditionalPackageDepSpecRequirements::ConstIterator u(spec.additional_requirements_ptr()->begin()),
                    u_end(spec.additional_requirements_ptr()->end()) ; u != u_end ; ++u)
                if (! (*u)->requirement_met(&env, maybe_changes_to_owner, id, from_id, maybe_changes_to_target).first)
                    return false;
        }
    }

    return true;
}

bool
paludis::match_package(
        const Environment & env,
        const PackageDepSpec & spec,
        const std::shared_ptr<const PackageID> & id,
        const std::shared_ptr<const PackageID> & from_id,
        const MatchPackageOptions & options)
{
    return match_package_with_maybe_changes(env, spec, 0, id, from_id, 0, options);
}

bool
paludis::match_package_in_set(
        const Environment & env,
        const SetSpecTree & target,
        const std::shared_ptr<const PackageID> & id,
        const MatchPackageOptions & options)
{
    using namespace std::placeholders;

    DepSpecFlattener<SetSpecTree, PackageDepSpec> f(&env, id);
    target.top()->accept(f);
    return indirect_iterator(f.end()) != std::find_if(
            indirect_iterator(f.begin()), indirect_iterator(f.end()),
            std::bind(&match_package, std::cref(env), _1, std::cref(id), make_null_shared_ptr(), std::cref(options)));
}

