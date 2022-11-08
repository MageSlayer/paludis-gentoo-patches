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
#include <paludis/dep_spec_annotations.hh>
#include <paludis/dep_spec_flattener.hh>
#include <paludis/environment.hh>
#include <paludis/version_requirements.hh>
#include <paludis/package_id.hh>
#include <paludis/slot_requirement.hh>
#include <paludis/metadata_key.hh>
#include <paludis/action.hh>
#include <paludis/repository.hh>
#include <paludis/additional_package_dep_spec_requirement.hh>
#include <paludis/slot.hh>

#include <paludis/util/set.hh>
#include <paludis/util/options.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/stringify.hh>

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

        void visit(const SlotExactPartialRequirement & s)
        {
            result = id->slot_key() && id->slot_key()->parse_value().match_values().first == s.slot();
        }

        void visit(const SlotExactFullRequirement & s)
        {
            result = id->slot_key() && id->slot_key()->parse_value().match_values() == s.slots();
        }

        void visit(const SlotAnyPartialLockedRequirement & s)
        {
            result = id->slot_key() && id->slot_key()->parse_value().match_values().first == s.slot();
        }

        void visit(const SlotAnyAtAllLockedRequirement &)
        {
            result = true;
        }

        void visit(const SlotAnyUnlockedRequirement &)
        {
            result = true;
        }

        void visit(const SlotUnknownRewrittenRequirement &) PALUDIS_ATTRIBUTE((noreturn))
        {
            throw InternalError(PALUDIS_HERE, "Should not be matching against SlotUnknownRewrittenRequirement");
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
                for (const auto & r : *spec.version_requirements_ptr())
                    if (! r.version_operator().as_version_spec_comparator()(id->version(), r.version_spec()))
                        return false;
                break;

            case vr_or:
                {
                    bool matched(false);
                    for (const auto & r : *spec.version_requirements_ptr())
                        if (r.version_operator().as_version_spec_comparator()(id->version(), r.version_spec()))
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

        auto v(id->from_repositories_key()->parse_value());
        if (v->end() == v->find(stringify(*spec.from_repository_ptr())))
            return false;
    }

    if (spec.installed_at_path_ptr())
    {
        auto repo(env.fetch_repository(id->repository_name()));
        if (! repo->installed_root_key())
            return false;
        if (repo->installed_root_key()->parse_value() != *spec.installed_at_path_ptr())
            return false;
    }

    if (spec.installable_to_repository_ptr())
    {
        if (! id->supports_action(SupportsActionTest<InstallAction>()))
            return false;
        if (! spec.installable_to_repository_ptr()->include_masked())
            if (id->masked())
                return false;

        const std::shared_ptr<const Repository> dest(env.fetch_repository(
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
        for (const auto & repository : env.repositories())
        {
            if (! repository->destination_interface())
                continue;
            if (! repository->installed_root_key())
                continue;
            if (repository->installed_root_key()->parse_value() != spec.installable_to_path_ptr()->path())
                continue;
            if (! repository->destination_interface()->is_suitable_destination_for(id))
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
            for (const auto & u : *spec.additional_requirements_ptr())
                if (! u->requirement_met(&env, maybe_changes_to_owner, id, from_id, maybe_changes_to_target).first)
                    return false;
        }
    }

    if (from_id && *id == *from_id && spec.maybe_annotations() && spec.maybe_annotations()->end() != spec.maybe_annotations()->find(dsar_no_self_match))
        return false;

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
    return match_package_with_maybe_changes(env, spec, nullptr, id, from_id, nullptr, options);
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
            std::bind(&match_package, std::cref(env), _1, std::cref(id), nullptr, std::cref(options)));
}

