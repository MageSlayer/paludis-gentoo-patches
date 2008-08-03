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
#include <paludis/util/join.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/dep_spec.hh>
#include <paludis/name.hh>
#include <paludis/environment.hh>
#include <vector>
#include <functional>

using namespace paludis;

namespace
{
    class PALUDIS_VISIBLE UseRequirement
    {
        private:
            const UseFlagName _name;

        public:
            UseRequirement(const UseFlagName &);
            virtual ~UseRequirement() { }

            const UseFlagName flag() const PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                return _name;
            }

            virtual bool requirement_met(const Environment * const, const PackageID &) const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
            virtual const std::string as_human_string() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    class PALUDIS_VISIBLE EnabledUseRequirement :
        public UseRequirement
    {
        public:
            EnabledUseRequirement(const UseFlagName &);
            ~EnabledUseRequirement();

            virtual bool requirement_met(const Environment * const, const PackageID &) const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string as_human_string() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE DisabledUseRequirement :
        public UseRequirement
    {
        public:
            DisabledUseRequirement(const UseFlagName &);
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
            ConditionalUseRequirement(const UseFlagName &, const std::tr1::shared_ptr<const PackageID> &);
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
            IfMineThenUseRequirement(const UseFlagName &, const std::tr1::shared_ptr<const PackageID> &);
            ~IfMineThenUseRequirement();

            virtual bool requirement_met(const Environment * const, const PackageID &) const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string as_human_string() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE IfNotMineThenUseRequirement :
        public ConditionalUseRequirement
    {
        public:
            IfNotMineThenUseRequirement(const UseFlagName &, const std::tr1::shared_ptr<const PackageID> &);
            ~IfNotMineThenUseRequirement();

            virtual bool requirement_met(const Environment * const, const PackageID &) const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string as_human_string() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE IfMineThenNotUseRequirement :
        public ConditionalUseRequirement
    {
        public:
            IfMineThenNotUseRequirement(const UseFlagName &, const std::tr1::shared_ptr<const PackageID> &);
            ~IfMineThenNotUseRequirement();

            virtual bool requirement_met(const Environment * const, const PackageID &) const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string as_human_string() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE IfNotMineThenNotUseRequirement :
        public ConditionalUseRequirement
    {
        public:
            IfNotMineThenNotUseRequirement(const UseFlagName &, const std::tr1::shared_ptr<const PackageID> &);
            ~IfNotMineThenNotUseRequirement();

            virtual bool requirement_met(const Environment * const, const PackageID &) const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string as_human_string() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE EqualUseRequirement :
        public ConditionalUseRequirement
    {
        public:
            EqualUseRequirement(const UseFlagName &, const std::tr1::shared_ptr<const PackageID> &);
            ~EqualUseRequirement();

            virtual bool requirement_met(const Environment * const, const PackageID &) const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string as_human_string() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE NotEqualUseRequirement :
        public ConditionalUseRequirement
    {
        public:
            NotEqualUseRequirement(const UseFlagName &, const std::tr1::shared_ptr<const PackageID> &);
            ~NotEqualUseRequirement();

            virtual bool requirement_met(const Environment * const, const PackageID &) const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string as_human_string() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class UseRequirements :
        public AdditionalPackageDepSpecRequirement
    {
        private:
            std::string _raw;
            std::vector<std::tr1::shared_ptr<const UseRequirement> > _reqs;

        public:
            UseRequirements(const std::string & r) :
                _raw(r)
            {
            }

            virtual bool requirement_met(const Environment * const env, const PackageID & id) const
            {
                using namespace std::tr1::placeholders;
                return _reqs.end() == std::find_if(_reqs.begin(), _reqs.end(), std::tr1::bind(
                        std::logical_not<bool>(), std::tr1::bind(
                             &UseRequirement::requirement_met, _1, env, std::tr1::cref(id))));
            }

            virtual const std::string as_human_string() const
            {
                return join(_reqs.begin(), _reqs.end(), "; ", std::tr1::mem_fn(&UseRequirement::as_human_string));
            }

            virtual const std::string as_raw_string() const
            {
                return _raw;
            }

            void add_requirement(const std::tr1::shared_ptr<const UseRequirement> & req)
            {
                _reqs.push_back(req);
            }
    };

    std::tr1::shared_ptr<const UseRequirement>
    parse_one_use_requirement(const std::string & s, std::string & flag,
            const std::tr1::shared_ptr<const PackageID> & id, const ELikeUseRequirementOptions & options)
    {
        if (flag.empty())
            throw ELikeUseRequirementError(s, "Invalid [] contents");

        if ('=' == flag.at(flag.length() - 1))
        {
            if ((! options[euro_allow_self_deps]) || (! id))
                throw ELikeUseRequirementError(s, "Cannot use [use=] here");

            flag.erase(flag.length() - 1);
            if (flag.empty())
                throw ELikeUseRequirementError(s, "Invalid [] contents");
            std::string::size_type not_position(options[euro_portage_syntax] ? 0 : flag.length() - 1);
            if ('!' == flag.at(not_position))
            {
                flag.erase(not_position, 1);
                if (flag.empty())
                    throw ELikeUseRequirementError(s, "Invalid [] contents");
                return make_shared_ptr(new NotEqualUseRequirement(UseFlagName(flag), id));
            }
            else
                return make_shared_ptr(new EqualUseRequirement(UseFlagName(flag), id));
        }
        else if ('?' == flag.at(flag.length() - 1))
        {
            if ((! options[euro_allow_self_deps]) || (! id))
                throw ELikeUseRequirementError(s, "Cannot use [use?] here");

            flag.erase(flag.length() - 1);
            if (flag.empty())
                throw ELikeUseRequirementError(s, "Invalid [] contents");
            std::string::size_type not_position(options[euro_portage_syntax] ? 0 : flag.length() - 1);
            if ('!' == flag.at(not_position))
            {
                flag.erase(not_position, 1);
                if (flag.empty())
                    throw ELikeUseRequirementError(s, "Invalid [] contents");
                if (options[euro_portage_syntax])
                    return make_shared_ptr(new IfNotMineThenNotUseRequirement(UseFlagName(flag), id));
                else if ('-' == flag.at(0))
                {
                    flag.erase(0, 1);
                    if (flag.empty())
                        throw ELikeUseRequirementError(s, "Invalid [] contents");

                    return make_shared_ptr(new IfNotMineThenNotUseRequirement(UseFlagName(flag), id));
                }
                else
                    return make_shared_ptr(new IfNotMineThenUseRequirement(UseFlagName(flag), id));
            }
            else
            {
                if (! options[euro_portage_syntax] && '-' == flag.at(0))
                {
                    flag.erase(0, 1);
                    if (flag.empty())
                        throw ELikeUseRequirementError(s, "Invalid [] contents");

                    return make_shared_ptr(new IfMineThenNotUseRequirement(UseFlagName(flag), id));
                }
                else
                    return make_shared_ptr(new IfMineThenUseRequirement(UseFlagName(flag), id));
            }
        }
        else if ('-' == flag.at(0))
        {
            flag.erase(0, 1);
            if (flag.empty())
                throw ELikeUseRequirementError(s, "Invalid [] contents");
            return make_shared_ptr(new DisabledUseRequirement(UseFlagName(flag)));
        }
        else
            return make_shared_ptr(new EnabledUseRequirement(UseFlagName(flag)));
    }
}

UseRequirement::UseRequirement(const UseFlagName & f) :
    _name(f)
{
}

EnabledUseRequirement::EnabledUseRequirement(const UseFlagName & n) :
    UseRequirement(n)
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

DisabledUseRequirement::DisabledUseRequirement(const UseFlagName & n) :
    UseRequirement(n)
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

ConditionalUseRequirement::ConditionalUseRequirement(const UseFlagName & n, const std::tr1::shared_ptr<const PackageID> & i) :
    UseRequirement(n),
    _id(i)
{
}

ConditionalUseRequirement::~ConditionalUseRequirement()
{
}

IfMineThenUseRequirement::IfMineThenUseRequirement(
        const UseFlagName & n, const std::tr1::shared_ptr<const PackageID> & i) :
    ConditionalUseRequirement(n, i)
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

IfNotMineThenUseRequirement::IfNotMineThenUseRequirement(
        const UseFlagName & n, const std::tr1::shared_ptr<const PackageID> & i) :
    ConditionalUseRequirement(n, i)
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

IfMineThenNotUseRequirement::IfMineThenNotUseRequirement(
        const UseFlagName & n, const std::tr1::shared_ptr<const PackageID> & i) :
    ConditionalUseRequirement(n, i)
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

IfNotMineThenNotUseRequirement::IfNotMineThenNotUseRequirement(
        const UseFlagName & n, const std::tr1::shared_ptr<const PackageID> & i) :
    ConditionalUseRequirement(n, i)
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

EqualUseRequirement::EqualUseRequirement(
        const UseFlagName & n, const std::tr1::shared_ptr<const PackageID> & i) :
    ConditionalUseRequirement(n, i)
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

NotEqualUseRequirement::NotEqualUseRequirement(
        const UseFlagName & n, const std::tr1::shared_ptr<const PackageID> & i) :
    ConditionalUseRequirement(n, i)
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

    std::tr1::shared_ptr<UseRequirements> result(new UseRequirements("[" + s + "]"));
    if (options[euro_portage_syntax])
    {
        std::string::size_type pos(0);
        for (;;)
        {
            std::string::size_type comma(s.find(',', pos));
            std::string flag(s.substr(pos, std::string::npos == comma ? comma : comma - pos));
            result->add_requirement(parse_one_use_requirement(s, flag, id, options));
            if (std::string::npos == comma)
                break;
            pos = comma + 1;
        }
    }
    else
    {
        std::string flag(s);
        result->add_requirement(parse_one_use_requirement(s, flag, id, options));
    }

    return result;
}

