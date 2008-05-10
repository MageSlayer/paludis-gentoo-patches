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

#include <paludis/elike_package_dep_spec.hh>
#include <paludis/elike_use_requirement.hh>
#include <paludis/elike_slot_requirement.hh>
#include <paludis/util/options.hh>
#include <paludis/util/log.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/dep_spec.hh>
#include <paludis/version_operator.hh>
#include <paludis/version_spec.hh>
#include <paludis/version_requirements.hh>

using namespace paludis;

#include <paludis/elike_package_dep_spec-se.cc>

PartiallyMadePackageDepSpec
paludis::partial_parse_elike_package_dep_spec(
        const std::string & ss, const ELikePackageDepSpecOptions & options, const std::tr1::shared_ptr<const PackageID> & id)
{
    Context context("When parsing package dep spec '" + ss + "':");

    if (ss.empty())
        throw PackageDepSpecError("Got empty dep spec");

    PartiallyMadePackageDepSpec result;
    std::string s(ss);
    bool had_bracket_version_requirements(false);

    std::string::size_type use_group_p;
    while (std::string::npos != ((use_group_p = s.rfind('['))))
    {
        if (! options[epdso_allow_square_bracket_deps])
        {
            if (options[epdso_strict_parsing])
                throw PackageDepSpecError("[] dependencies not safe for use here");
            else
                Log::get_instance()->message("e.package_dep_spec.brackets_not_allowed", ll_warning, lc_context)
                    << "[] dependencies not safe for use here";
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
                    std::tr1::shared_ptr<const AdditionalPackageDepSpecRequirement> req(parse_elike_use_requirement(flag,
                                id, ELikeUseRequirementOptions() + euro_allow_self_deps));
                    result.additional_requirement(req);
                }
                break;
        };

        s.erase(use_group_p);
    }

    std::string::size_type repo_p;
    if (std::string::npos != ((repo_p = s.rfind("::"))))
    {
        if (! options[epdso_allow_repository_deps])
        {
            if (options[epdso_strict_parsing])
                throw PackageDepSpecError("Repository dependencies not safe for use here");
            else
                Log::get_instance()->message("e.package_dep_spec.repository_not_allowed", ll_warning, lc_context)
                    << "Repository dependencies not safe for use here";
        }

        result.repository(RepositoryName(s.substr(repo_p + 2)));
        s.erase(repo_p);
    }

    std::string::size_type slot_p;
    if (std::string::npos != ((slot_p = s.rfind(':'))))
    {
        std::string match(s.substr(slot_p + 1));
        if (match.empty())
            throw PackageDepSpecError("Empty slot dependency specified");

        if ("*" == match)
        {
            if (! options[epdso_allow_slot_star_deps])
            {
                if (options[epdso_strict_parsing])
                    throw PackageDepSpecError("Slot '*' dependencies not safe for use here");
                else
                    Log::get_instance()->message("e.package_dep_spec.slot_star_not_allowed", ll_warning, lc_context)
                        << "Slot '*' dependencies not safe for use here";
            }
            result.slot_requirement(make_shared_ptr(new ELikeSlotAnyUnlockedRequirement));
        }
        else if ('=' == match.at(0))
        {
            if (! options[epdso_allow_slot_equal_deps])
            {
                if (options[epdso_strict_parsing])
                    throw PackageDepSpecError("Slot '=' dependencies not safe for use here");
                else
                    Log::get_instance()->message("e.package_dep_spec.slot_equals_not_allowed", ll_warning, lc_context)
                        << "Slot '=' dependencies not safe for use here";
            }

            if (1 == match.length())
                result.slot_requirement(make_shared_ptr(new ELikeSlotAnyLockedRequirement));
            else
                result.slot_requirement(make_shared_ptr(new ELikeSlotExactRequirement(SlotName(s.substr(slot_p + 2)), true)));
        }
        else
        {
            if (! options[epdso_allow_slot_deps])
            {
                if (options[epdso_strict_parsing])
                    throw PackageDepSpecError("Slot dependencies not safe for use here");
                else
                    Log::get_instance()->message("e.package_dep_spec.slot_not_allowed", ll_warning, lc_context)
                        << "Slot dependencies not safe for use here";
            }
            result.slot_requirement(make_shared_ptr(new ELikeSlotExactRequirement(SlotName(s.substr(slot_p + 1)), false)));
        }
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
            if (! options[epdso_allow_tilde_greater_deps])
            {
                if (options[epdso_strict_parsing])
                    throw PackageDepSpecError("~> dependencies not safe for use here");
                else
                    Log::get_instance()->message("e.package_dep_spec.tilde_greater_not_allowed", ll_warning, lc_context)
                        << "~> dependencies not safe for use here";
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
            throw PackageDepSpecError("Wildcard '*' not allowed in '" + stringify(ss) + "'");

            if (0 != t.compare(t.length() - 2, 2, "/*"))
                result.package_name_part(PackageNamePart(t.substr(2)));
        }
        else if (t.length() >= 3 && (0 == t.compare(t.length() - 2, 2, "/*")))
        {
            throw PackageDepSpecError("Wildcard '*' not allowed in '" + stringify(ss) + "'");

            result.category_name_part(CategoryNamePart(t.substr(0, t.length() - 2)));
        }
        else
            result.package(QualifiedPackageName(t));

        if ('*' == s.at(s.length() - 1))
        {
            if (op != vo_equal)
            {
                if (! options[epdso_strict_star_operator])
                {
                    if (options[epdso_strict_parsing])
                        throw PackageDepSpecError(
                                "Package dep spec '" + ss + "' uses * "
                                "with operator '" + stringify(op) + "'");
                    else
                        Log::get_instance()->message("e.package_dep_spec.bad_operator", ll_qa, lc_context)
                            << "Package dep spec '" << ss << "' uses * with operator '" << op <<
                            "', pretending it uses the equals operator instead";
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
            throw PackageDepSpecError("Wildcard '*' not allowed in '" + stringify(ss) + "'");

            if (0 != s.compare(s.length() - 2, 2, "/*"))
                result.package_name_part(PackageNamePart(s.substr(2)));
        }
        else if (s.length() >= 3 && (0 == s.compare(s.length() - 2, 2, "/*")))
        {
            throw PackageDepSpecError("Wildcard '*' not allowed in '" + stringify(ss) + "'");

            result.category_name_part(CategoryNamePart(s.substr(0, s.length() - 2)));
        }
        else
            result.package(QualifiedPackageName(s));
    }

    return result;
}

PackageDepSpec
paludis::parse_elike_package_dep_spec(const std::string & ss, const ELikePackageDepSpecOptions & options,
        const std::tr1::shared_ptr<const PackageID> & id)
{
    return partial_parse_elike_package_dep_spec(ss, options, id);
}

