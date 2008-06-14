/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008 Ciaran McCreesh
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

#include <paludis/user_dep_spec.hh>
#include <paludis/environment.hh>
#include <paludis/elike_use_requirement.hh>
#include <paludis/version_operator.hh>
#include <paludis/version_spec.hh>
#include <paludis/version_requirements.hh>
#include <paludis/package_database.hh>
#include <paludis/filter.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/options.hh>
#include <paludis/util/log.hh>
#include <paludis/util/visitor-impl.hh>

using namespace paludis;

#include <paludis/user_dep_spec-se.cc>

namespace
{
    void parse_package_bit(PartiallyMadePackageDepSpec & result, const std::string & ss, const std::string & t,
            const Environment * const env, const UserPackageDepSpecOptions & options,
            const Filter & filter)
    {
        if (t.length() >= 3 && (0 == t.compare(0, 2, "*/")))
        {
            if (! options[updso_allow_wildcards])
                throw PackageDepSpecError("Wildcard '*' not allowed in '" + stringify(ss) + "'");

            if (0 != t.compare(t.length() - 2, 2, "/*"))
                result.package_name_part(PackageNamePart(t.substr(2)));
        }
        else if (t.length() >= 3 && (0 == t.compare(t.length() - 2, 2, "/*")))
        {
            if (! options[updso_allow_wildcards])
                throw PackageDepSpecError("Wildcard '*' not allowed in '" + stringify(ss) + "'");

            result.category_name_part(CategoryNamePart(t.substr(0, t.length() - 2)));
        }
        else if (t == "*")
            throw PackageDepSpecError("Use '*/*' not '*' to match everything in '" + stringify(ss) + "'");
        else if (std::string::npos != t.find('/'))
            result.package(QualifiedPackageName(t));
        else
        {
            if (options[updso_no_disambiguation])
                throw PackageDepSpecError("Need an explicit category specified in '" + stringify(ss) + "'");
            result.package(env->package_database()->fetch_unique_qualified_package_name(PackageNamePart(t), filter));
        }
    }
}

PackageDepSpec
paludis::parse_user_package_dep_spec(const std::string & ss, const Environment * const env,
        const UserPackageDepSpecOptions & options, const Filter & filter)
{
    Context context("When parsing package dep spec '" + ss + "':");

    if (ss.empty())
        throw PackageDepSpecError("Got empty dep spec");

    if (options[updso_throw_if_set] && std::string::npos == ss.find_first_of("/:[<>=~"))
        try
        {
            if (env->set(SetName(ss)))
                throw GotASetNotAPackageDepSpec(ss);
        }
        catch (const SetNameError &)
        {
        }

    std::string s(ss);
    PartiallyMadePackageDepSpec result;
    bool had_bracket_version_requirements(false);

    std::string::size_type use_group_p;
    while (std::string::npos != ((use_group_p = s.rfind('['))))
    {
        if (s.at(s.length() - 1) != ']')
            throw PackageDepSpecError("Mismatched []");

        std::string flag(s.substr(use_group_p + 1));
        if (flag.length() < 2)
            throw PackageDepSpecError("Invalid [] contents");

        flag.erase(flag.length() - 1);

        switch (flag.at(0))
        {
            case '<':
            case '>':
            case '=':
            case '~':
                {
                    char needed_mode(0);

                    while (! flag.empty())
                    {
                        Context cc("When parsing [] segment '" + flag + "':");

                        std::string op;
                        std::string::size_type opos(0);
                        while (opos < flag.length())
                            if (std::string::npos == std::string("><=~").find(flag.at(opos)))
                                break;
                            else
                                ++opos;

                        op = flag.substr(0, opos);
                        flag.erase(0, opos);

                        if (op.empty())
                            throw PackageDepSpecError("Missing operator inside []");

                        VersionOperator vop(op);

                        std::string ver;
                        opos = flag.find_first_of("|&");
                        if (std::string::npos == opos)
                        {
                            ver = flag;
                            flag.clear();
                        }
                        else
                        {
                            if (0 == needed_mode)
                                needed_mode = flag.at(opos);
                            else if (needed_mode != flag.at(opos))
                                throw PackageDepSpecError("Mixed & and | inside []");

                            result.version_requirements_mode(flag.at(opos) == '|' ? vr_or : vr_and);
                            ver = flag.substr(0, opos++);
                            flag.erase(0, opos);
                        }

                        if (ver.empty())
                            throw PackageDepSpecError("Missing version after operator '" + stringify(vop) + " inside []");

                        if ('*' == ver.at(ver.length() - 1))
                        {
                            ver.erase(ver.length() - 1);
                            if (vop == vo_equal)
                                vop = vo_equal_star;
                            else
                                throw PackageDepSpecError("Invalid use of * with operator '" + stringify(vop) + " inside []");
                        }

                        VersionSpec vs(ver);
                        result.version_requirement(VersionRequirement(vop, vs));
                        had_bracket_version_requirements = true;
                    }
                }
                break;

            default:
                {
                    std::tr1::shared_ptr<const AdditionalPackageDepSpecRequirement> req(parse_elike_use_requirement(flag,
                                std::tr1::shared_ptr<const PackageID>(), ELikeUseRequirementOptions()));
                    result.additional_requirement(req);
                }
                break;
        };

        s.erase(use_group_p);
    }

    std::string::size_type repo_p;
    if (std::string::npos != ((repo_p = s.rfind("::"))))
    {
        result.repository(RepositoryName(s.substr(repo_p + 2)));
        s.erase(repo_p);
    }

    std::string::size_type slot_p;
    if (std::string::npos != ((slot_p = s.rfind(':'))))
    {
        result.slot_requirement(make_shared_ptr(new UserSlotExactRequirement(SlotName(s.substr(slot_p + 1)))));
        s.erase(slot_p);
    }

    if (std::string::npos != std::string("<>=~").find(s.at(0)))
    {
        if (had_bracket_version_requirements)
            throw PackageDepSpecError("Cannot mix [] and traditional version specifications");

        std::string::size_type p(1);
        if (s.length() > 1 && std::string::npos != std::string("<>=~").find(s.at(1)))
            ++p;
        VersionOperator op(s.substr(0, p));
        std::string::size_type q(p);

        while (true)
        {
            if (p >= s.length())
                throw PackageDepSpecError("Couldn't parse dep spec '" + ss + "'");
            q = s.find('-', q + 1);
            if ((std::string::npos == q) || (++q >= s.length()))
                throw PackageDepSpecError("Couldn't parse dep spec '" + ss + "'");
            if ((s.at(q) >= '0' && s.at(q) <= '9') || (0 == s.compare(q, 3, "scm")))
                break;
        }

        std::string::size_type new_q(q);
        while (true)
        {
            if (new_q >= s.length())
                break;
            new_q = s.find('-', new_q + 1);
            if ((std::string::npos == new_q) || (++new_q >= s.length()))
                break;
            if (s.at(new_q) >= '0' && s.at(new_q) <= '9')
                q = new_q;
        }

        std::string t(s.substr(p, q - p - 1));
        parse_package_bit(result, ss, t, env, options, filter);

        if ('*' == s.at(s.length() - 1))
        {
            if (op != vo_equal)
                Log::get_instance()->message("user_dep_spec.bad_operator", ll_qa, lc_context) <<
                    "Package dep spec '" << ss << "' uses * "
                    "with operator '" << op << "', pretending it uses the equals operator instead";
            op = vo_equal_star;

            result.version_requirement(VersionRequirement(op, VersionSpec(s.substr(q, s.length() - q - 1))));
        }
        else
            result.version_requirement(VersionRequirement(op, VersionSpec(s.substr(q))));
    }
    else
        parse_package_bit(result, ss, s, env, options, filter);

    return result;
}

UserSlotExactRequirement::UserSlotExactRequirement(const SlotName & s) :
    _s(s)
{
}
const SlotName
UserSlotExactRequirement::slot() const
{
    return _s;
}

const std::string
UserSlotExactRequirement::as_string() const
{
    return ":" + stringify(_s);
}

GotASetNotAPackageDepSpec::GotASetNotAPackageDepSpec(const std::string & s) throw () :
    Exception("'" + s + "' is a set, not a package")
{
}

