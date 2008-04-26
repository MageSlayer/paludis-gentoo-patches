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

#include <paludis/repositories/e/use_requirements.hh>
#include <paludis/util/stringify.hh>
#include <paludis/environment.hh>

using namespace paludis;
using namespace paludis::erepository;

UseRequirement::UseRequirement(const std::string & r, const UseFlagName & f) :
    _raw(r),
    _name(f)
{
}

const std::string
UseRequirement::as_raw_string() const
{
    return _raw;
}

EnabledUseRequirement::EnabledUseRequirement(const std::string & s, const UseFlagName & n) :
    UseRequirement(s, n)
{
}

EnabledUseRequirement::~EnabledUseRequirement()
{
}

bool
EnabledUseRequirement::requirement_met(const Environment * const env, const PackageID & pkg) const
{
    return env->query_use(flag(), pkg);
}

const std::string
EnabledUseRequirement::as_human_string() const
{
    return "Flag '" + stringify(flag()) + "' enabled";
}

DisabledUseRequirement::DisabledUseRequirement(const std::string & s, const UseFlagName & n) :
    UseRequirement(s, n)
{
}

DisabledUseRequirement::~DisabledUseRequirement()
{
}

bool
DisabledUseRequirement::requirement_met(const Environment * const env, const PackageID & pkg) const
{
    return ! env->query_use(flag(), pkg);
}

const std::string
DisabledUseRequirement::as_human_string() const
{
    return "Flag '" + stringify(flag()) + "' disabled";
}

ConditionalUseRequirement::ConditionalUseRequirement(const std::string & s,
        const UseFlagName & n, const std::tr1::shared_ptr<const PackageID> & i) :
    UseRequirement(s, n),
    _id(i)
{
}

ConditionalUseRequirement::~ConditionalUseRequirement()
{
}

IfMineThenUseRequirement::IfMineThenUseRequirement(const std::string & s,
        const UseFlagName & n, const std::tr1::shared_ptr<const PackageID> & i) :
    ConditionalUseRequirement(s, n, i)
{
}

IfMineThenUseRequirement::~IfMineThenUseRequirement()
{
}

bool
IfMineThenUseRequirement::requirement_met(const Environment * const env, const PackageID & pkg) const
{
    return ! env->query_use(flag(), *package_id()) || env->query_use(flag(), pkg);
}

const std::string
IfMineThenUseRequirement::as_human_string() const
{
    return "Flag '" + stringify(flag()) + "' enabled if it is enabled for '" + stringify(*package_id()) + "'";
}

IfNotMineThenUseRequirement::IfNotMineThenUseRequirement(const std::string & s,
        const UseFlagName & n, const std::tr1::shared_ptr<const PackageID> & i) :
    ConditionalUseRequirement(s, n, i)
{
}

IfNotMineThenUseRequirement::~IfNotMineThenUseRequirement()
{
}

bool
IfNotMineThenUseRequirement::requirement_met(const Environment * const env, const PackageID & pkg) const
{
    return env->query_use(flag(), *package_id()) || env->query_use(flag(), pkg);
}

const std::string
IfNotMineThenUseRequirement::as_human_string() const
{
    return "Flag '" + stringify(flag()) + "' enabled if it is disabled for '" + stringify(*package_id()) + "'";
}

IfMineThenNotUseRequirement::IfMineThenNotUseRequirement(const std::string & s,
        const UseFlagName & n, const std::tr1::shared_ptr<const PackageID> & i) :
    ConditionalUseRequirement(s, n, i)
{
}

IfMineThenNotUseRequirement::~IfMineThenNotUseRequirement()
{
}

const std::string
IfMineThenNotUseRequirement::as_human_string() const
{
    return "Flag '" + stringify(flag()) + "' disabled if it is enabled for '" + stringify(*package_id()) + "'";
}

bool
IfMineThenNotUseRequirement::requirement_met(const Environment * const env, const PackageID & pkg) const
{
    return ! env->query_use(flag(), *package_id()) || ! env->query_use(flag(), pkg);
}

IfNotMineThenNotUseRequirement::IfNotMineThenNotUseRequirement(const std::string & s,
        const UseFlagName & n, const std::tr1::shared_ptr<const PackageID> & i) :
    ConditionalUseRequirement(s, n, i)
{
}

IfNotMineThenNotUseRequirement::~IfNotMineThenNotUseRequirement()
{
}

bool
IfNotMineThenNotUseRequirement::requirement_met(const Environment * const env, const PackageID & pkg) const
{
    return env->query_use(flag(), *package_id()) || ! env->query_use(flag(), pkg);
}

const std::string
IfNotMineThenNotUseRequirement::as_human_string() const
{
    return "Flag '" + stringify(flag()) + "' disabled if it is disabled for '" + stringify(*package_id()) + "'";
}

EqualUseRequirement::EqualUseRequirement(const std::string & s,
        const UseFlagName & n, const std::tr1::shared_ptr<const PackageID> & i) :
    ConditionalUseRequirement(s, n, i)
{
}

EqualUseRequirement::~EqualUseRequirement()
{
}

bool
EqualUseRequirement::requirement_met(const Environment * const env, const PackageID & pkg) const
{
    return env->query_use(flag(), pkg) == env->query_use(flag(), *package_id());
}

const std::string
EqualUseRequirement::as_human_string() const
{
    return "Flag '" + stringify(flag()) + "' enabled or disabled like it is for '" + stringify(*package_id()) + "'";
}

NotEqualUseRequirement::NotEqualUseRequirement(const std::string & s,
        const UseFlagName & n, const std::tr1::shared_ptr<const PackageID> & i) :
    ConditionalUseRequirement(s, n, i)
{
}

NotEqualUseRequirement::~NotEqualUseRequirement()
{
}

bool
NotEqualUseRequirement::requirement_met(const Environment * const env, const PackageID & pkg) const
{
    return env->query_use(flag(), pkg) != env->query_use(flag(), *package_id());
}

const std::string
NotEqualUseRequirement::as_human_string() const
{
    return "Flag '" + stringify(flag()) + "' enabled or disabled opposite to how it is for '" + stringify(*package_id()) + "'";
}

