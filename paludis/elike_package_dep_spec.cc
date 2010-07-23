/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010 Ciaran McCreesh
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
#include <paludis/util/make_named_values.hh>
#include <paludis/dep_spec.hh>
#include <paludis/version_operator.hh>
#include <paludis/version_spec.hh>
#include <paludis/version_requirements.hh>
#include <paludis/user_dep_spec.hh>
#include <strings.h>

using namespace paludis;

#include <paludis/elike_package_dep_spec-se.cc>

PartiallyMadePackageDepSpec
paludis::partial_parse_generic_elike_package_dep_spec(const std::string & ss, const GenericELikePackageDepSpecParseFunctions & fns)
{
    Context context("When parsing generic package dep spec '" + ss + "':");

    /* Check that it's not, e.g. a set with updso_throw_if_set, or empty. */
    fns.check_sanity()(ss);

    std::string s(ss);
    PartiallyMadePackageDepSpec result(fns.options_for_partially_made_package_dep_spec()());

    /* Remove trailing [use], [version] etc parts. */
    while (fns.remove_trailing_square_bracket_if_exists()(s, result))
    {
    }

    /* Remove trailing ::repo and :slot parts. */
    fns.remove_trailing_repo_if_exists()(s, result);
    fns.remove_trailing_slot_if_exists()(s, result);

    if (fns.has_version_operator()(s))
    {
        /* Leading (or maybe =*) operator, so trailing version. */
        VersionOperator op(fns.get_remove_version_operator()(s));
        VersionSpec spec(fns.get_remove_trailing_version()(s));
        fns.add_version_requirement()(op, spec, result);
        fns.add_package_requirement()(s, result);
    }
    else
    {
        /* No leading operator, so no version. */
        fns.add_package_requirement()(s, result);
    }

    return result;
}

PackageDepSpec
paludis::parse_generic_elike_package_dep_spec(const std::string & ss, const GenericELikePackageDepSpecParseFunctions & fns)
{
    return partial_parse_generic_elike_package_dep_spec(ss, fns);
}

void
paludis::elike_check_sanity(const std::string & s)
{
    if (s.empty())
        throw PackageDepSpecError("Got empty dep spec");
}

bool
paludis::elike_remove_trailing_square_bracket_if_exists(std::string & s, PartiallyMadePackageDepSpec & result,
        const ELikePackageDepSpecOptions & options,
        const VersionSpecOptions & version_options,
        bool & had_bracket_version_requirements,
        bool & had_use_requirements, const std::shared_ptr<const PackageID> & id)
{
    std::string::size_type use_group_p;
    if (std::string::npos == ((use_group_p = s.rfind('['))))
        return false;

    if (std::string::npos == s.rfind(']'))
        throw PackageDepSpecError("Mismatched []");

    if (s.at(s.length() - 1) != ']')
        throw PackageDepSpecError("Trailing garbage after [] block");

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
            if (! options[epdso_allow_ranged_deps])
            {
                if (options[epdso_strict_parsing])
                    throw PackageDepSpecError("Version range dependencies not safe for use here");
                else
                    Log::get_instance()->message("e.package_dep_spec.range_not_allowed", ll_warning, lc_context)
                        << "Version range dependencies not safe for use here";
            }

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
                            vop = options[epdso_nice_equal_star] ? vo_nice_equal_star : vo_stupid_equal_star;
                        else
                            throw PackageDepSpecError("Invalid use of * with operator '" + stringify(vop) + " inside []");
                    }

                    VersionSpec vs(ver, version_options);
                    result.version_requirement(make_named_values<VersionRequirement>(
                                n::version_operator() = vop,
                                n::version_spec() = vs));
                    had_bracket_version_requirements = true;
                }
            }
            break;

        case '.':
            {
                if (! options[epdso_allow_key_requirements])
                {
                    if (options[epdso_strict_parsing])
                        throw PackageDepSpecError("Key requirements not safe for use here");
                    else
                        Log::get_instance()->message("e.package_dep_spec.key_not_allowed", ll_warning, lc_context)
                            << "Key requirements not safe for use here";
                }
                std::shared_ptr<const AdditionalPackageDepSpecRequirement> req(std::make_shared<UserKeyRequirement>(flag.substr(1)));
                result.additional_requirement(req);
            }
            break;


        default:
            if (! options[epdso_allow_use_deps] && ! options[epdso_allow_use_deps_portage])
            {
                if (options[epdso_strict_parsing])
                    throw PackageDepSpecError("USE dependencies not safe for use here");
                else
                {
                    Log::get_instance()->message("e.package_dep_spec.use_not_allowed", ll_warning, lc_context)
                        << "USE dependencies not safe for use here";
                }
            }

            if (! options[epdso_allow_use_deps] && had_use_requirements)
            {
                if (options[epdso_strict_parsing])
                    throw PackageDepSpecError("multiple sets of USE dependencies not safe for use here");
                else
                {
                    Log::get_instance()->message("e.package_dep_spec.use_multiple_not_allowed", ll_warning, lc_context)
                        << "multiple sets of USE dependencies not safe for use here";
                }
            }
            had_use_requirements = true;

            ELikeUseRequirementOptions euro;
            euro += euro_allow_self_deps;
            if (options[epdso_allow_use_deps_portage])
                euro += options[epdso_allow_use_deps] ? euro_both_syntaxes : euro_portage_syntax;
            if (options[epdso_allow_use_dep_defaults])
                euro += euro_allow_default_values;
            if (options[epdso_allow_use_dep_question_defaults])
                euro += euro_allow_default_question_values;
            if (options[epdso_strict_parsing])
                euro += euro_strict_parsing;

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req(parse_elike_use_requirement(flag, id, euro));
            result.additional_requirement(req);

            break;
    };

    s.erase(use_group_p);

    return true;
}

void
paludis::elike_remove_trailing_repo_if_exists(std::string & s, PartiallyMadePackageDepSpec & result,
        const ELikePackageDepSpecOptions & options)
{
    std::string::size_type repo_p;
    if (std::string::npos == ((repo_p = s.rfind("::"))))
        return;

    if (! options[epdso_allow_repository_deps])
    {
        if (options[epdso_strict_parsing])
            throw PackageDepSpecError("Repository dependencies not safe for use here");
        else
            Log::get_instance()->message("e.package_dep_spec.repository_not_allowed", ll_warning, lc_context)
                << "Repository dependencies not safe for use here";
    }

    result.in_repository(RepositoryName(s.substr(repo_p + 2)));
    s.erase(repo_p);
}

void
paludis::elike_remove_trailing_slot_if_exists(std::string & s, PartiallyMadePackageDepSpec & result,
        const ELikePackageDepSpecOptions & options)
{
    std::string::size_type slot_p;
    if (std::string::npos == ((slot_p = s.rfind(':'))))
        return;

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
        result.slot_requirement(std::make_shared<ELikeSlotAnyUnlockedRequirement>());
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
            result.slot_requirement(std::make_shared<ELikeSlotAnyLockedRequirement>());
        else
            result.slot_requirement(std::make_shared<ELikeSlotExactRequirement>(SlotName(s.substr(slot_p + 2)), true));
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
        result.slot_requirement(std::make_shared<ELikeSlotExactRequirement>(SlotName(s.substr(slot_p + 1)), false));
    }
    s.erase(slot_p);
}

bool
paludis::elike_has_version_operator(const std::string & s, const bool had_bracket_version_requirements,
        const ELikePackageDepSpecOptions & options)
{
    if ((! s.empty()) && std::string::npos != std::string("<>=~").find(s.at(0)))
    {
        if (had_bracket_version_requirements)
            throw PackageDepSpecError("Cannot mix [] and traditional version specifications");

        if (options[epdso_disallow_nonranged_deps])
            throw PackageDepSpecError("Traditional version specifications are not allowed here");

        return true;
    }
    else
        return false;
}

VersionOperator
paludis::elike_get_remove_version_operator(std::string & s, const ELikePackageDepSpecOptions & options)
{
    std::string::size_type p(1);
    if (s.length() > 1 && std::string::npos != std::string("<>=~").find(s.at(1)))
        ++p;
    VersionOperator op(s.substr(0, p));
    s.erase(0, p);

    if (op == vo_tilde_greater)
        if (! options[epdso_allow_tilde_greater_deps])
        {
            if (options[epdso_strict_parsing])
                throw PackageDepSpecError("~> dependencies not safe for use here");
            else
                Log::get_instance()->message("e.package_dep_spec.tilde_greater_not_allowed", ll_warning, lc_context)
                    << "~> dependencies not safe for use here";
        }

    if ((! s.empty()) && ('*' == s.at(s.length() - 1)))
    {
        if (op != vo_equal)
            throw PackageDepSpecError("Package dep spec uses * with operator '" + stringify(op) + "'");
        op = options[epdso_nice_equal_star] ? vo_nice_equal_star : vo_stupid_equal_star;
        s.erase(s.length() - 1);
    }

    return op;
}

VersionSpec
paludis::elike_get_remove_trailing_version(std::string & s, const VersionSpecOptions & version_options)
{
    /* find the last place a version spec could start (that is, a hyphen
     * followed by a digit, or a hyphen followed by 'scm'). if it's the scm
     * thing, find the second last place instead, if it exists. */
    std::string::size_type hyphen_pos(s.rfind('-')), last_hyphen_pos(std::string::npos);
    while (true)
    {
        if (std::string::npos == hyphen_pos || 0 == hyphen_pos)
        {
            /* - at start or no - is an error. but if we've already found a
             * trailing -scm, use that. */
            if (std::string::npos != last_hyphen_pos)
            {
                hyphen_pos = last_hyphen_pos;
                break;
            }
            else
                throw PackageDepSpecError("No version found");
        }

        /* make sure we've got room for the match */
        if (! (hyphen_pos + 1 >= s.length()))
        {
            if (std::string::npos != std::string("0123456789").find(s.at(hyphen_pos + 1)))
            {
                /* can't have an -scm before this */
                break;
            }
            else if (0 == s.compare(hyphen_pos + 1, 3, "scm") ||
                    (version_options[vso_ignore_case] && 0 == strncasecmp(s.c_str() + hyphen_pos + 1, "scm", 3)))
            {
                if (std::string::npos == last_hyphen_pos)
                {
                    /* we can still go back further, but we don't have to */
                    last_hyphen_pos = hyphen_pos;
                }
                else
                {
                    /* -scm-scm not allowed, use our later match */
                    hyphen_pos = last_hyphen_pos;
                    break;
                }
            }
        }

        hyphen_pos = s.rfind('-', hyphen_pos - 1);
    }

    VersionSpec result(s.substr(hyphen_pos + 1), version_options);
    s.erase(hyphen_pos);
    return result;
}

void
paludis::elike_add_version_requirement(const VersionOperator & op, const VersionSpec & spec, PartiallyMadePackageDepSpec & result)
{
    result.version_requirement(make_named_values<VersionRequirement>(
                n::version_operator() = op,
                n::version_spec() = spec));
}

void
paludis::elike_add_package_requirement(const std::string & s, PartiallyMadePackageDepSpec & result)
{
    if (std::string::npos == s.find('/'))
        throw PackageDepSpecError("No category/ found in '" + s + "' (cat/pkg is required, a simple pkg is not allowed here)");

    if (s.length() >= 3 && (0 == s.compare(0, 2, "*/")))
    {
        throw PackageDepSpecError("Wildcard '*' not allowed here");

        if (0 != s.compare(s.length() - 2, 2, "/*"))
            result.package_name_part(PackageNamePart(s.substr(2)));
    }
    else if (s.length() >= 3 && (0 == s.compare(s.length() - 2, 2, "/*")))
    {
        throw PackageDepSpecError("Wildcard '*' not allowed here");

        result.category_name_part(CategoryNamePart(s.substr(0, s.length() - 2)));
    }
    else
        result.package(QualifiedPackageName(s));
}

namespace
{
    const PartiallyMadePackageDepSpecOptions fixed_options_for_partially_made_package_dep_spec(PartiallyMadePackageDepSpecOptions o)
    {
        return o;
    }
}

PartiallyMadePackageDepSpec
paludis::partial_parse_elike_package_dep_spec(
        const std::string & ss, const ELikePackageDepSpecOptions & options,
        const VersionSpecOptions & version_options,
        const std::shared_ptr<const PackageID> & id)
{
    using namespace std::placeholders;

    Context context("When parsing elike package dep spec '" + ss + "':");

    bool had_bracket_version_requirements(false), had_use_requirements(false);

    PartiallyMadePackageDepSpecOptions o;
    if (options[epdso_disallow_nonranged_deps])
        o += pmpdso_always_use_ranged_deps;

    return partial_parse_generic_elike_package_dep_spec(ss, make_named_values<GenericELikePackageDepSpecParseFunctions>(
                n::add_package_requirement() = std::bind(&elike_add_package_requirement, _1, _2),
                n::add_version_requirement() = std::bind(&elike_add_version_requirement, _1, _2, _3),
                n::check_sanity() = &elike_check_sanity,
                n::get_remove_trailing_version() = std::bind(&elike_get_remove_trailing_version, _1, version_options),
                n::get_remove_version_operator() = std::bind(&elike_get_remove_version_operator, _1, options),
                n::has_version_operator() = std::bind(&elike_has_version_operator, _1,
                        std::cref(had_bracket_version_requirements), options),
                n::options_for_partially_made_package_dep_spec() = std::bind(&fixed_options_for_partially_made_package_dep_spec, o),
                n::remove_trailing_repo_if_exists() = std::bind(&elike_remove_trailing_repo_if_exists, _1, _2, options),
                n::remove_trailing_slot_if_exists() = std::bind(&elike_remove_trailing_slot_if_exists, _1, _2, options),
                n::remove_trailing_square_bracket_if_exists() = std::bind(&elike_remove_trailing_square_bracket_if_exists,
                        _1, _2, options, version_options, std::ref(had_bracket_version_requirements), std::ref(had_use_requirements), id)
                ));
}

PackageDepSpec
paludis::parse_elike_package_dep_spec(const std::string & ss, const ELikePackageDepSpecOptions & options,
        const VersionSpecOptions & version_options,
        const std::shared_ptr<const PackageID> & id)
{
    return partial_parse_elike_package_dep_spec(ss, options, version_options, id);
}

