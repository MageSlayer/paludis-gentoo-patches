/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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

#include <paludis/repositories/e/package_dep_spec.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/log.hh>
#include <paludis/dep_spec.hh>
#include <paludis/version_operator.hh>
#include <paludis/version_spec.hh>
#include <paludis/version_requirements.hh>
#include <paludis/use_requirements.hh>

using namespace paludis;
using namespace paludis::erepository;

PackageDepSpec
paludis::erepository::parse_e_package_dep_spec(const std::string & ss, const EAPI & eapi, const tr1::shared_ptr<const PackageID> & id)
{
    Context context("When parsing package dep spec '" + ss + "' with eapi '" + stringify(eapi.name) + "':");

    if (ss.empty())
        throw PackageDepSpecError("Got empty dep spec");

    if (! eapi.supported)
        throw PackageDepSpecError("Don't know how to parse dep specs using EAPI '" + eapi.name + "'");

    PartiallyMadePackageDepSpec result;
    std::string s(ss);
    bool had_bracket_version_requirements(false);

    std::string::size_type use_group_p;
    while (std::string::npos != ((use_group_p = s.rfind('['))))
    {
        if (! eapi.supported->package_dep_spec_parse_options[pdspo_allow_square_bracket_deps])
        {
            if (eapi.supported->package_dep_spec_parse_options[pdspo_strict_parsing])
                throw PackageDepSpecError("[] dependencies not safe for use with this EAPI");
            else
                Log::get_instance()->message(ll_warning, lc_context, "[] dependencies not safe for use with this EAPI");
        }

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

                            result.version_requirements_mode((flag.at(opos) == '|' ? vr_or : vr_and));
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
                    tr1::shared_ptr<const UseRequirement> req;
                    if ('=' == flag.at(flag.length() - 1))
                    {
                        if (! id)
                            throw PackageDepSpecError("Cannot use [use=] without an associated ID");

                        flag.erase(flag.length() - 1);
                        if (flag.empty())
                            throw PackageDepSpecError("Invalid [] contents");
                        if ('!' == flag.at(flag.length() - 1))
                        {
                            flag.erase(flag.length() - 1);
                            if (flag.empty())
                                throw PackageDepSpecError("Invalid [] contents");
                            req.reset(new NotEqualUseRequirement(UseFlagName(flag), id));
                        }
                        else
                            req.reset(new EqualUseRequirement(UseFlagName(flag), id));
                    }
                    else if ('?' == flag.at(flag.length() - 1))
                    {
                        if (! id)
                            throw PackageDepSpecError("Cannot use [use?] without an associated ID");

                        flag.erase(flag.length() - 1);
                        if (flag.empty())
                            throw PackageDepSpecError("Invalid [] contents");
                        if ('!' == flag.at(flag.length() - 1))
                        {
                            flag.erase(flag.length() - 1);
                            if (flag.empty())
                                throw PackageDepSpecError("Invalid [] contents");
                            if ('-' == flag.at(0))
                            {
                                flag.erase(0, 1);
                                if (flag.empty())
                                    throw PackageDepSpecError("Invalid [] contents");

                                req.reset(new IfNotMineThenNotUseRequirement(UseFlagName(flag), id));
                            }
                            else
                                req.reset(new IfNotMineThenUseRequirement(UseFlagName(flag), id));
                        }
                        else
                        {
                            if ('-' == flag.at(0))
                            {
                                flag.erase(0, 1);
                                if (flag.empty())
                                    throw PackageDepSpecError("Invalid [] contents");

                                req.reset(new IfMineThenNotUseRequirement(UseFlagName(flag), id));
                            }
                            else
                                req.reset(new IfMineThenUseRequirement(UseFlagName(flag), id));
                        }
                    }
                    else if ('-' == flag.at(0))
                    {
                        flag.erase(0, 1);
                        if (flag.empty())
                            throw PackageDepSpecError("Invalid [] contents");
                        req.reset(new DisabledUseRequirement(UseFlagName(flag)));
                    }
                    else
                        req.reset(new EnabledUseRequirement(UseFlagName(flag)));
                    result.use_requirement(req);
                }
                break;
        };

        s.erase(use_group_p);
    }

    std::string::size_type repo_p;
    if (std::string::npos != ((repo_p = s.rfind("::"))))
    {
        if (! eapi.supported->package_dep_spec_parse_options[pdspo_allow_repository_deps])
        {
            if (eapi.supported->package_dep_spec_parse_options[pdspo_strict_parsing])
                throw PackageDepSpecError("Repository dependencies not safe for use with this EAPI");
            else
                Log::get_instance()->message(ll_warning, lc_context, "Repository dependencies not safe for use with this EAPI");
        }

        result.repository(RepositoryName(s.substr(repo_p + 2)));
        s.erase(repo_p);
    }

    std::string::size_type slot_p;
    if (std::string::npos != ((slot_p = s.rfind(':'))))
    {
        if (! eapi.supported->package_dep_spec_parse_options[pdspo_allow_slot_deps])
        {
            if (eapi.supported->package_dep_spec_parse_options[pdspo_strict_parsing])
                throw PackageDepSpecError("Slot dependencies not safe for use with this EAPI");
            else
                Log::get_instance()->message(ll_warning, lc_context, "Slot dependencies not safe for use with this EAPI");
        }

        result.slot(SlotName(s.substr(slot_p + 1)));
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

        if (op == vo_tilde_greater)
            if (! eapi.supported->package_dep_spec_parse_options[pdspo_allow_tilde_greater_deps])
            {
                if (eapi.supported->package_dep_spec_parse_options[pdspo_strict_parsing])
                    throw PackageDepSpecError("~> dependencies not safe for use with this EAPI");
                else
                    Log::get_instance()->message(ll_warning, lc_context, "~> dependencies not safe for use with this EAPI");
            }

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
        if (t.length() >= 3 && (0 == t.compare(0, 2, "*/")))
        {
            throw PackageDepSpecError("Wildcard '*' not allowed in '" + stringify(ss) + "' with eapi '"
                    + stringify(eapi.name) + "'");

            if (0 != t.compare(t.length() - 2, 2, "/*"))
                result.package_name_part(PackageNamePart(t.substr(2)));
        }
        else if (t.length() >= 3 && (0 == t.compare(t.length() - 2, 2, "/*")))
        {
            throw PackageDepSpecError("Wildcard '*' not allowed in '" + stringify(ss) + "' with eapi '"
                    + stringify(eapi.name) + "'");

            result.category_name_part(CategoryNamePart(t.substr(0, t.length() - 2)));
        }
        else
            result.package(QualifiedPackageName(t));

        if ('*' == s.at(s.length() - 1))
        {
            if (op != vo_equal)
            {
                if (! eapi.supported->package_dep_spec_parse_options[pdspo_strict_star_operator])
                {
                    if (eapi.supported->package_dep_spec_parse_options[pdspo_strict_parsing])
                        throw PackageDepSpecError(
                                "Package dep spec '" + ss + "' uses * "
                                "with operator '" + stringify(op) + "'");
                    else
                        Log::get_instance()->message(ll_qa, lc_context,
                                "Package dep spec '" + ss + "' uses * "
                                "with operator '" + stringify(op) +
                                "', pretending it uses the equals operator instead");
                }
            }
            op = vo_equal_star;

            result.version_requirement(VersionRequirement(op, VersionSpec(s.substr(q, s.length() - q - 1))));
        }
        else
            result.version_requirement(VersionRequirement(op, VersionSpec(s.substr(q))));
    }
    else
    {
        if (s.length() >= 3 && (0 == s.compare(0, 2, "*/")))
        {
            throw PackageDepSpecError("Wildcard '*' not allowed in '" + stringify(ss) + "' with parse eapi '"
                    + stringify(eapi.name) + "'");

            if (0 != s.compare(s.length() - 2, 2, "/*"))
                result.package_name_part(PackageNamePart(s.substr(2)));
        }
        else if (s.length() >= 3 && (0 == s.compare(s.length() - 2, 2, "/*")))
        {
            throw PackageDepSpecError("Wildcard '*' not allowed in '" + stringify(ss) + "' with EAPI '"
                    + stringify(eapi.name) + "'");

            result.category_name_part(CategoryNamePart(s.substr(0, s.length() - 2)));
        }
        else
            result.package(QualifiedPackageName(s));
    }

    return result;
}

