/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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

#include <paludis/elike_use_requirement.hh>
#include <paludis/util/options.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/stringify.hh>
#include <paludis/dep_spec.hh>
#include <paludis/name.hh>
#include <paludis/environment.hh>

using namespace paludis;

namespace
{
    class PALUDIS_VISIBLE UseRequirement :
        public AdditionalPackageDepSpecRequirement
    {
        private:
            const std::string _raw;
            const UseFlagName _name;

        public:
            UseRequirement(const std::string &, const UseFlagName &);

            const UseFlagName flag() const PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                return _name;
            }

            virtual const std::string as_raw_string() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE EnabledUseRequirement :
        public UseRequirement
    {
        public:
            EnabledUseRequirement(const std::string &, const UseFlagName &);
            ~EnabledUseRequirement();

            virtual bool requirement_met(const Environment * const, const PackageID &) const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string as_human_string() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE DisabledUseRequirement :
        public UseRequirement
    {
        public:
            DisabledUseRequirement(const std::string &, const UseFlagName &);
            ~DisabledUseRequirement();

            virtual bool requirement_met(const Environment * const, const PackageID &) const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string as_human_string() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE ConditionalUseRequirement :
        public UseRequirement
    {
        private:
            const std::tr1::shared_ptr<const PackageID> _id;

        public:
            ConditionalUseRequirement(const std::string &, const UseFlagName &, const std::tr1::shared_ptr<const PackageID> &);
            ~ConditionalUseRequirement();

            const std::tr1::shared_ptr<const PackageID> package_id() const PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                return _id;
            }
    };

    class PALUDIS_VISIBLE IfMineThenUseRequirement :
        public ConditionalUseRequirement
    {
        public:
            IfMineThenUseRequirement(const std::string &, const UseFlagName &, const std::tr1::shared_ptr<const PackageID> &);
            ~IfMineThenUseRequirement();

            virtual bool requirement_met(const Environment * const, const PackageID &) const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string as_human_string() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE IfNotMineThenUseRequirement :
        public ConditionalUseRequirement
    {
        public:
            IfNotMineThenUseRequirement(const std::string &, const UseFlagName &, const std::tr1::shared_ptr<const PackageID> &);
            ~IfNotMineThenUseRequirement();

            virtual bool requirement_met(const Environment * const, const PackageID &) const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string as_human_string() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE IfMineThenNotUseRequirement :
        public ConditionalUseRequirement
    {
        public:
            IfMineThenNotUseRequirement(const std::string &, const UseFlagName &, const std::tr1::shared_ptr<const PackageID> &);
            ~IfMineThenNotUseRequirement();

            virtual bool requirement_met(const Environment * const, const PackageID &) const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string as_human_string() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE IfNotMineThenNotUseRequirement :
        public ConditionalUseRequirement
    {
        public:
            IfNotMineThenNotUseRequirement(const std::string &, const UseFlagName &, const std::tr1::shared_ptr<const PackageID> &);
            ~IfNotMineThenNotUseRequirement();

            virtual bool requirement_met(const Environment * const, const PackageID &) const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string as_human_string() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE EqualUseRequirement :
        public ConditionalUseRequirement
    {
        public:
            EqualUseRequirement(const std::string &, const UseFlagName &, const std::tr1::shared_ptr<const PackageID> &);
            ~EqualUseRequirement();

            virtual bool requirement_met(const Environment * const, const PackageID &) const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string as_human_string() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE NotEqualUseRequirement :
        public ConditionalUseRequirement
    {
        public:
            NotEqualUseRequirement(const std::string &, const UseFlagName &, const std::tr1::shared_ptr<const PackageID> &);
            ~NotEqualUseRequirement();

            virtual bool requirement_met(const Environment * const, const PackageID &) const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string as_human_string() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };
}

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

ELikeUseRequirementError::ELikeUseRequirementError(const std::string & s, const std::string & m) throw () :
    Exception("Error parsing use requirement '" + s + "': " + m)
{
}

std::tr1::shared_ptr<const AdditionalPackageDepSpecRequirement>
paludis::parse_elike_use_requirement(const std::string & s,
        const std::tr1::shared_ptr<const PackageID> & id, const ELikeUseRequirementOptions & options)
{
    Context context("When parsing use requirement '" + s + "':");

    std::string flag(s), raw_flag("[" + s + "]");
    if ('=' == flag.at(flag.length() - 1))
    {
        if ((! options[euro_allow_self_deps]) || (! id))
            throw ELikeUseRequirementError(s, "Cannot use [use=] here");

        flag.erase(flag.length() - 1);
        if (flag.empty())
            throw ELikeUseRequirementError(s, "Invalid [] contents");
        if ('!' == flag.at(flag.length() - 1))
        {
            flag.erase(flag.length() - 1);
            if (flag.empty())
                throw ELikeUseRequirementError(s, "Invalid [] contents");
            return make_shared_ptr(new NotEqualUseRequirement(raw_flag, UseFlagName(flag), id));
        }
        else
            return make_shared_ptr(new EqualUseRequirement(raw_flag, UseFlagName(flag), id));
    }
    else if ('?' == flag.at(flag.length() - 1))
    {
        if ((! options[euro_allow_self_deps]) || (! id))
            throw ELikeUseRequirementError(s, "Cannot use [use?] here");

        flag.erase(flag.length() - 1);
        if (flag.empty())
            throw ELikeUseRequirementError(s, "Invalid [] contents");
        if ('!' == flag.at(flag.length() - 1))
        {
            flag.erase(flag.length() - 1);
            if (flag.empty())
                throw ELikeUseRequirementError(s, "Invalid [] contents");
            if ('-' == flag.at(0))
            {
                flag.erase(0, 1);
                if (flag.empty())
                    throw ELikeUseRequirementError(s, "Invalid [] contents");

                return make_shared_ptr(new IfNotMineThenNotUseRequirement(raw_flag, UseFlagName(flag), id));
            }
            else
                return make_shared_ptr(new IfNotMineThenUseRequirement(raw_flag, UseFlagName(flag), id));
        }
        else
        {
            if ('-' == flag.at(0))
            {
                flag.erase(0, 1);
                if (flag.empty())
                    throw ELikeUseRequirementError(s, "Invalid [] contents");

                return make_shared_ptr(new IfMineThenNotUseRequirement(raw_flag, UseFlagName(flag), id));
            }
            else
                return make_shared_ptr(new IfMineThenUseRequirement(raw_flag, UseFlagName(flag), id));
        }
    }
    else if ('-' == flag.at(0))
    {
        flag.erase(0, 1);
        if (flag.empty())
            throw ELikeUseRequirementError(s, "Invalid [] contents");
        return make_shared_ptr(new DisabledUseRequirement(raw_flag, UseFlagName(flag)));
    }
    else
        return make_shared_ptr(new EnabledUseRequirement(raw_flag, UseFlagName(flag)));
}

