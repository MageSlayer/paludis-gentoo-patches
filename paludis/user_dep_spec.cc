/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/elike_package_dep_spec.hh>
#include <paludis/environment.hh>
#include <paludis/elike_use_requirement.hh>
#include <paludis/version_operator.hh>
#include <paludis/version_spec.hh>
#include <paludis/filter.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/dep_label.hh>
#include <paludis/partially_made_package_dep_spec.hh>
#include <paludis/contents.hh>
#include <paludis/repository.hh>

#include <paludis/util/options.hh>
#include <paludis/util/log.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/tribool.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/join.hh>

#include <algorithm>

using namespace paludis;

#include <paludis/user_dep_spec-se.cc>

namespace
{
    void user_add_package_requirement(const std::string & s, PartiallyMadePackageDepSpec & result,
            const Environment * const env, const UserPackageDepSpecOptions & options,
            const Filter & filter)
    {
        if (s.length() >= 3 && (0 == s.compare(0, 2, "*/")))
        {
            if (! options[updso_allow_wildcards])
                throw PackageDepSpecError("Wildcard '*' not allowed");

            if (0 != s.compare(s.length() - 2, 2, "/*"))
                result.package_name_part(PackageNamePart(s.substr(2)));
        }
        else if (s.length() >= 3 && (0 == s.compare(s.length() - 2, 2, "/*")))
        {
            if (! options[updso_allow_wildcards])
                throw PackageDepSpecError("Wildcard '*' not allowed in '" + stringify(s) + "'");

            result.category_name_part(CategoryNamePart(s.substr(0, s.length() - 2)));
        }
        else if (s == "*")
            throw PackageDepSpecError("Use '*/*' not '*' to match everything");
        else if (std::string::npos != s.find('/'))
            result.package(QualifiedPackageName(s));
        else
        {
            if (options[updso_no_disambiguation])
                throw PackageDepSpecError("Need an explicit category specified");
            result.package(env->fetch_unique_qualified_package_name(PackageNamePart(s),
                filter::And(filter, filter::Matches(result, make_null_shared_ptr(), { }))));
        }
    }

    void envless_add_package_requirement(const std::string & s, PartiallyMadePackageDepSpec & result)
    {
        if (s.length() >= 3 && (0 == s.compare(0, 2, "*/")))
        {
            if (0 != s.compare(s.length() - 2, 2, "/*"))
                result.package_name_part(PackageNamePart(s.substr(2)));
        }
        else if (s.length() >= 3 && (0 == s.compare(s.length() - 2, 2, "/*")))
        {
            result.category_name_part(CategoryNamePart(s.substr(0, s.length() - 2)));
        }
        else if (s == "*")
            throw PackageDepSpecError("Use '*/*' not '*' to match everything");
        else if (std::string::npos != s.find('/'))
            result.package(QualifiedPackageName(s));
        else
        {
            throw PackageDepSpecError("Need an explicit category specified");
        }
    }

    void user_check_sanity(const std::string & s, const UserPackageDepSpecOptions & options,
            const Environment * const env)
    {
        if (s.empty())
            throw PackageDepSpecError("Got empty dep spec");

        if (options[updso_throw_if_set] && std::string::npos == s.find_first_of("/[<>=~"))
            try
            {
                SetName sn(s);
                if (options[updso_no_disambiguation] || env->set(sn))
                    throw GotASetNotAPackageDepSpec(s);
            }
            catch (const SetNameError &)
            {
            }
    }

    void test_check_sanity(const std::string & s)
    {
        if (s.empty())
            throw PackageDepSpecError("Got empty dep spec");
    }

    bool user_remove_trailing_square_bracket_if_exists(std::string & s, PartiallyMadePackageDepSpec & result,
            bool & had_bracket_version_requirements)
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
                parse_elike_version_range(flag, result, { epdso_nice_equal_star }, user_version_spec_options(), had_bracket_version_requirements);
                break;

            case '.':
                {
                    auto k(parse_user_key_constraint(flag.substr(1)));
                    result.key_constraint(std::get<0>(k), std::get<1>(k), std::get<2>(k), std::get<3>(k));
                }
                break;

            default:
                result.choice_constraint(parse_elike_use_requirement(flag, { }));
                break;
        };

        s.erase(use_group_p);

        return true;
    }

    void
    user_remove_trailing_slot_if_exists(std::string & s, PartiallyMadePackageDepSpec & result)
    {
        std::string::size_type slot_p(s.rfind(':'));
        if (std::string::npos != slot_p)
        {
            result.exact_slot_constraint(SlotName(s.substr(slot_p + 1)), false);
            s.erase(slot_p);
        }
    }

    void
    parse_rhs(PartiallyMadePackageDepSpec & reqs, const std::string & req)
    {
        if (req.empty())
            throw PackageDepSpecError("Invalid empty :: requirement");

        if ('/' == req.at(0))
        {
            if ('?' == req.at(req.length() - 1))
            {
                if (req.length() >= 2 && '?' == req.at(req.length() - 2))
                    reqs.installable_to_path(FSPath(req.substr(0, req.length() - 2)), true);
                else
                    reqs.installable_to_path(FSPath(req.substr(0, req.length() - 1)), false);
            }
            else
                reqs.installed_at_path(FSPath(req));
        }
        else
        {
            if ('?' == req.at(req.length() - 1))
            {
                if (req.length() >= 3 && '?' == req.at(req.length() - 2))
                    reqs.installable_to_repository(RepositoryName(req.substr(0, req.length() - 2)), true);
                else
                    reqs.installable_to_repository(RepositoryName(req.substr(0, req.length() - 1)), false);
            }
            else
                reqs.in_repository(RepositoryName(req));
        }
    }

    void
    user_remove_trailing_repo_if_exists(std::string & s, PartiallyMadePackageDepSpec & result)
    {
        std::string::size_type repo_p;
        if (std::string::npos == ((repo_p = s.rfind("::"))))
            return;

        std::string req(s.substr(repo_p + 2));
        s.erase(repo_p);
        if (req.empty())
            throw PackageDepSpecError("Need something after ::");

        std::string::size_type arrow_p(req.find("->"));
        if (std::string::npos == arrow_p)
            parse_rhs(result, req);
        else
        {
            std::string left(req.substr(0, arrow_p));
            std::string right(req.substr(arrow_p + 2));

            if (left.empty() && right.empty())
                throw PackageDepSpecError("::-> requires either a from or a to repository");

            if (! right.empty())
                parse_rhs(result, right);

            if (! left.empty())
                result.from_repository(RepositoryName(left));
        }
    }

    const PartiallyMadePackageDepSpecOptions fixed_options_for_partially_made_package_dep_spec(PartiallyMadePackageDepSpecOptions o)
    {
        return o;
    }
}

PackageDepSpec
paludis::parse_user_package_dep_spec(const std::string & ss, const Environment * const env,
        const UserPackageDepSpecOptions & options, const Filter & filter)
{
    using namespace std::placeholders;

    Context context("When parsing user package dep spec '" + ss + "':");

    bool had_bracket_version_requirements(false);
    PartiallyMadePackageDepSpecOptions o;

    return partial_parse_generic_elike_package_dep_spec(ss, make_named_values<GenericELikePackageDepSpecParseFunctions>(
            n::add_package_requirement() = std::bind(&user_add_package_requirement, _1, _2, env, options, filter),
            n::add_version_requirement() = std::bind(&elike_add_version_requirement, _1, _2, _3, _4),
            n::check_sanity() = std::bind(&user_check_sanity, _1, options, env),
            n::get_remove_trailing_version() = std::bind(&elike_get_remove_trailing_version, _1,
                    user_version_spec_options()),
            n::get_remove_version_operator() = std::bind(&elike_get_remove_version_operator, _1,
                    ELikePackageDepSpecOptions() + epdso_allow_tilde_greater_deps + epdso_nice_equal_star),
            n::has_version_operator() = std::bind(&elike_has_version_operator, _1,
                    std::cref(had_bracket_version_requirements), ELikePackageDepSpecOptions()),
            n::options_for_partially_made_package_dep_spec() = std::bind(&fixed_options_for_partially_made_package_dep_spec, std::cref(o)),
            n::remove_trailing_repo_if_exists() = std::bind(&user_remove_trailing_repo_if_exists, _1, _2),
            n::remove_trailing_slot_if_exists() = std::bind(&user_remove_trailing_slot_if_exists, _1, _2),
            n::remove_trailing_square_bracket_if_exists() = std::bind(&user_remove_trailing_square_bracket_if_exists,
                    _1, _2, std::ref(had_bracket_version_requirements))
            ));
}

PackageDepSpec
paludis::envless_parse_package_dep_spec_for_tests(const std::string & ss)
{
    using namespace std::placeholders;

    Context context("When parsing test package dep spec '" + ss + "':");

    bool had_bracket_version_requirements(false);
    PartiallyMadePackageDepSpecOptions o;

    return partial_parse_generic_elike_package_dep_spec(ss, make_named_values<GenericELikePackageDepSpecParseFunctions>(
            n::add_package_requirement() = std::bind(&envless_add_package_requirement, _1, _2),
            n::add_version_requirement() = std::bind(&elike_add_version_requirement, _1, _2, _3, _4),
            n::check_sanity() = std::bind(&test_check_sanity, _1),
            n::get_remove_trailing_version() = std::bind(&elike_get_remove_trailing_version, _1,
                    user_version_spec_options()),
            n::get_remove_version_operator() = std::bind(&elike_get_remove_version_operator, _1,
                    ELikePackageDepSpecOptions() + epdso_allow_tilde_greater_deps + epdso_nice_equal_star),
            n::has_version_operator() = std::bind(&elike_has_version_operator, _1,
                    std::cref(had_bracket_version_requirements), ELikePackageDepSpecOptions()),
            n::options_for_partially_made_package_dep_spec() = std::bind(&fixed_options_for_partially_made_package_dep_spec, std::cref(o)),
            n::remove_trailing_repo_if_exists() = std::bind(&user_remove_trailing_repo_if_exists, _1, _2),
            n::remove_trailing_slot_if_exists() = std::bind(&user_remove_trailing_slot_if_exists, _1, _2),
            n::remove_trailing_square_bracket_if_exists() = std::bind(&user_remove_trailing_square_bracket_if_exists,
                    _1, _2, std::ref(had_bracket_version_requirements))
            ));
}

GotASetNotAPackageDepSpec::GotASetNotAPackageDepSpec(const std::string & s) throw () :
    Exception("'" + s + "' is a set, not a package")
{
}

VersionSpecOptions
paludis::user_version_spec_options()
{
    return { vso_flexible_dashes, vso_flexible_dots,
        vso_ignore_case, vso_letters_anywhere, vso_dotted_suffixes };
}

std::tuple<KeyConstraintKeyType, std::string, KeyConstraintOperation, std::string>
paludis::parse_user_key_constraint(const std::string & s)
{
    std::string::size_type p(s.find_first_of("=<>?~"));
    if (std::string::npos == p)
        throw PackageDepSpecError("[." + s + "] contains no operator");

    std::string key, value;
    KeyConstraintOperation op(last_kco);

    if (s.at(p) == '?')
    {
        if (s.length() - 1 != p)
            throw PackageDepSpecError("[." + s + "] uses a key with operator '?'");
        else
        {
            key = s.substr(0, p);
            op = kco_question;
        }
    }
    else
    {
        switch (s.at(p))
        {
            case '=': op = kco_equals;        break;
            case '~': op = kco_tilde;         break;
            case '<': op = kco_less_than;     break;
            case '>': op = kco_greater_than;  break;
            default:
                throw PackageDepSpecError("[." + s + "] unknown operator");
        }
        key = s.substr(0, p);
        value = s.substr(p + 1);
    }

    KeyConstraintKeyType type(kckt_id);
    if (0 == key.compare(0, 3, "::$", 0, 3))
    {
        type = kckt_repo_role;
        key.erase(0, 3);
    }
    else if (0 == key.compare(0, 2, "::2", 0, 2))
    {
        type = kckt_repo;
        key.erase(0, 2);
    }
    else if (0 == key.compare(0, 1, "$", 0, 1))
    {
        type = kckt_id_role;
        key.erase(0, 1);
    }
    else if (0 == key.compare(0, 1, "(", 0, 1) && ')' == key.at(key.length() - 1))
    {
        type = kckt_id_mask;
        key.erase(0, 1);
        key.erase(key.length() - 1);
    }

    return std::make_tuple(type, key, op, value);
}

