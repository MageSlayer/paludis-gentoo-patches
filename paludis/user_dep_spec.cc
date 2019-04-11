/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012 Ciaran McCreesh
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
#include <paludis/version_requirements.hh>
#include <paludis/filter.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/dep_label.hh>
#include <paludis/partially_made_package_dep_spec.hh>
#include <paludis/contents.hh>
#include <paludis/repository.hh>
#include <paludis/mask.hh>
#include <paludis/slot.hh>
#include <paludis/match_package.hh>

#include <paludis/util/options.hh>
#include <paludis/util/log.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/tribool.hh>
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
                filter::And(filter, filter::Matches(result, nullptr, { }))));
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
            bool & had_bracket_version_requirements, const Environment * const env)
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

                        VersionSpec vs(ver, user_version_spec_options());
                        result.version_requirement(make_named_values<VersionRequirement>(
                                    n::version_operator() = vop,
                                    n::version_spec() = vs));
                        had_bracket_version_requirements = true;
                    }
                }
                break;

            case '.':
                {
                    std::string exclude(".!exclude=");
                    if (0 == flag.compare(0, exclude.size(), exclude))
                    {
                        flag.erase(0, exclude.size());

                        Context cc("When parsing exclude requirement '" + flag + "':");
                        if (!env) throw PackageDepSpecError("Environment is null");

                        std::shared_ptr<const AdditionalPackageDepSpecRequirement> req(std::make_shared<ExcludeRequirement>(
                            parse_user_package_dep_spec(flag, env, { updso_allow_wildcards })
                            ));
                        result.additional_requirement(req);
                    }
                    else
                    {
                        std::shared_ptr<const AdditionalPackageDepSpecRequirement> req(std::make_shared<UserKeyRequirement>(flag.substr(1)));
                        result.additional_requirement(req);
                    }
                }
                break;

            default:
                {
                    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req(parse_elike_use_requirement(flag, { }, nullptr));
                    result.additional_requirement(req);
                }
                break;
        };

        s.erase(use_group_p);

        return true;
    }

    void
    user_remove_trailing_slot_if_exists(std::string & s, PartiallyMadePackageDepSpec & result)
    {
        std::string::size_type slot_p;
        if (std::string::npos == ((slot_p = s.rfind(':'))))
            return;

        std::string text(s.substr(slot_p + 1));
        s.erase(slot_p);

        auto p(text.find('/'));
        if (std::string::npos != p)
        {
            result.slot_requirement(std::make_shared<UserSlotExactFullRequirement>(std::make_pair(SlotName(text.substr(0, p)), SlotName(text.substr(p + 1)))));
        }
        else
        {
            result.slot_requirement(std::make_shared<UserSlotExactPartialRequirement>(SlotName(text)));
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
                    reqs.installable_to_path(make_named_values<InstallableToPath>(
                                n::include_masked() = true,
                                n::path() = FSPath(req.substr(0, req.length() - 2))));
                else
                    reqs.installable_to_path(make_named_values<InstallableToPath>(
                                n::include_masked() = false,
                                n::path() = FSPath(req.substr(0, req.length() - 1))));
            }
            else
                reqs.installed_at_path(FSPath(req));
        }
        else
        {
            if ('?' == req.at(req.length() - 1))
            {
                if (req.length() >= 3 && '?' == req.at(req.length() - 2))
                    reqs.installable_to_repository(make_named_values<InstallableToRepository>(
                                n::include_masked() = true,
                                n::repository() = RepositoryName(req.substr(0, req.length() - 2))));
                else
                    reqs.installable_to_repository(make_named_values<InstallableToRepository>(
                                n::include_masked() = false,
                                n::repository() = RepositoryName(req.substr(0, req.length() - 1))));
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
            n::add_version_requirement() = std::bind(&elike_add_version_requirement, _1, _2, _3),
            n::check_sanity() = std::bind(&user_check_sanity, _1, options, env),
            n::get_remove_trailing_version() = std::bind(&elike_get_remove_trailing_version, _1,
                    user_version_spec_options()),
            n::get_remove_version_operator() = std::bind(&elike_get_remove_version_operator, _1,
                    ELikePackageDepSpecOptions() + epdso_allow_tilde_greater_deps),
            n::has_version_operator() = std::bind(&elike_has_version_operator, _1,
                    std::cref(had_bracket_version_requirements), ELikePackageDepSpecOptions()),
            n::options_for_partially_made_package_dep_spec() = std::bind(&fixed_options_for_partially_made_package_dep_spec, std::cref(o)),
            n::remove_trailing_repo_if_exists() = std::bind(&user_remove_trailing_repo_if_exists, _1, _2),
            n::remove_trailing_slot_if_exists() = std::bind(&user_remove_trailing_slot_if_exists, _1, _2),
            n::remove_trailing_square_bracket_if_exists() = std::bind(&user_remove_trailing_square_bracket_if_exists,
                    _1, _2, std::ref(had_bracket_version_requirements), env)
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
            n::add_version_requirement() = std::bind(&elike_add_version_requirement, _1, _2, _3),
            n::check_sanity() = std::bind(&test_check_sanity, _1),
            n::get_remove_trailing_version() = std::bind(&elike_get_remove_trailing_version, _1,
                    user_version_spec_options()),
            n::get_remove_version_operator() = std::bind(&elike_get_remove_version_operator, _1,
                    ELikePackageDepSpecOptions() + epdso_allow_tilde_greater_deps),
            n::has_version_operator() = std::bind(&elike_has_version_operator, _1,
                    std::cref(had_bracket_version_requirements), ELikePackageDepSpecOptions()),
            n::options_for_partially_made_package_dep_spec() = std::bind(&fixed_options_for_partially_made_package_dep_spec, std::cref(o)),
            n::remove_trailing_repo_if_exists() = std::bind(&user_remove_trailing_repo_if_exists, _1, _2),
            n::remove_trailing_slot_if_exists() = std::bind(&user_remove_trailing_slot_if_exists, _1, _2),
            n::remove_trailing_square_bracket_if_exists() = std::bind(&user_remove_trailing_square_bracket_if_exists,
                    _1, _2, std::ref(had_bracket_version_requirements), nullptr)
            ));
}


UserSlotExactFullRequirement::UserSlotExactFullRequirement(const std::pair<SlotName, SlotName> & s) :
    _s(s)
{
}

const std::pair<SlotName, SlotName>
UserSlotExactFullRequirement::slots() const
{
    return _s;
}

const std::string
UserSlotExactFullRequirement::as_string() const
{
    return ":" + stringify(_s.first) + "/" + stringify(_s.second);
}

const std::shared_ptr<const SlotRequirement>
UserSlotExactFullRequirement::maybe_original_requirement_if_rewritten() const
{
    return nullptr;
}

UserSlotExactPartialRequirement::UserSlotExactPartialRequirement(const SlotName & s) :
    _s(s)
{
}
const SlotName
UserSlotExactPartialRequirement::slot() const
{
    return _s;
}

const std::string
UserSlotExactPartialRequirement::as_string() const
{
    return ":" + stringify(_s);
}

const std::shared_ptr<const SlotRequirement>
UserSlotExactPartialRequirement::maybe_original_requirement_if_rewritten() const
{
    return nullptr;
}

GotASetNotAPackageDepSpec::GotASetNotAPackageDepSpec(const std::string & s) noexcept :
    Exception("'" + s + "' is a set, not a package")
{
}

namespace paludis
{
    template <>
    struct Imp<UserKeyRequirement>
    {
        std::string key;
        std::string value;
        UserKeyRequirementOperator op;

        Imp(const std::string & s)
        {
            int op_size = 1;
            std::string::size_type p = std::string::npos;

            if (std::string::npos != (p = s.find("!=")))
            {
                op_size = 2;
                op = ukro_not_equal;
            }
            else if (std::string::npos != (p = s.find('=')))
                op = ukro_equal;
            else if (std::string::npos != (p = s.find('>')))
                op = ukro_greater;
            else if (std::string::npos != (p = s.find('<')))
                op = ukro_less_or_subset;
            else if (std::string::npos != (p = s.find('?')))
                op = ukro_exists;
            else
                throw PackageDepSpecError("Expected an =, an !=, a <, a > or a ? inside '[." + s + "]'");

            key = s.substr(0, p);
            value = s.substr(p + op_size);

            if (op == ukro_exists && ! value.empty())
                throw PackageDepSpecError("Operator '?' takes no value inside '[." + s + "]'");
        }
    };
}

UserKeyRequirement::UserKeyRequirement(const std::string & s) :
    _imp(s)
{
}

UserKeyRequirement::~UserKeyRequirement() = default;

namespace
{
    std::string stringify_contents_entry(const ContentsEntry & e)
    {
        return stringify(e.location_key()->parse_value());
    }

    struct StringifyEqual
    {
        const std::string pattern;

        StringifyEqual(const std::string & p) :
            pattern(p)
        {
        }

        template <typename T_>
        bool operator() (const T_ & t) const
        {
            return stringify(t) == pattern;
        }

        bool operator() (const ContentsEntry & e) const
        {
            return stringify_contents_entry(e) == pattern;
        }
    };

    struct SpecTreeSearcher
    {
        const Environment * const env;
        const std::shared_ptr<const PackageID> id;
        const std::string pattern;

        SpecTreeSearcher(const Environment * const e, const std::shared_ptr<const PackageID> & i, const std::string & p) :
            env(e),
            id(i),
            pattern(p)
        {
        }

        bool visit(const GenericSpecTree::NodeType<AllDepSpec>::Type & n) const
        {
            return indirect_iterator(n.end()) != std::find_if(indirect_iterator(n.begin()), indirect_iterator(n.end()),
                    accept_visitor_returning<bool>(*this));
        }

        bool visit(const GenericSpecTree::NodeType<AnyDepSpec>::Type & n) const
        {
            return indirect_iterator(n.end()) != std::find_if(indirect_iterator(n.begin()), indirect_iterator(n.end()),
                    accept_visitor_returning<bool>(*this));
        }

        bool visit(const GenericSpecTree::NodeType<ExactlyOneDepSpec>::Type & n) const
        {
            return indirect_iterator(n.end()) != std::find_if(indirect_iterator(n.begin()), indirect_iterator(n.end()),
                    accept_visitor_returning<bool>(*this));
        }

        bool visit(const GenericSpecTree::NodeType<AtMostOneDepSpec>::Type & n) const
        {
            return indirect_iterator(n.end()) != std::find_if(indirect_iterator(n.begin()), indirect_iterator(n.end()),
                    accept_visitor_returning<bool>(*this));
        }

        bool visit(const GenericSpecTree::NodeType<ConditionalDepSpec>::Type & n) const
        {
            if (n.spec()->condition_met(env, id))
                return indirect_iterator(n.end()) != std::find_if(indirect_iterator(n.begin()), indirect_iterator(n.end()),
                        accept_visitor_returning<bool>(*this));
            else
                return false;
        }

        bool visit(const GenericSpecTree::NodeType<NamedSetDepSpec>::Type & n) const
        {
            return stringify(*n.spec()) == pattern;
        }

        bool visit(const GenericSpecTree::NodeType<PlainTextDepSpec>::Type & n) const
        {
            return stringify(*n.spec()) == pattern;
        }

        bool visit(const GenericSpecTree::NodeType<PackageDepSpec>::Type & n) const
        {
            return stringify(*n.spec()) == pattern;
        }

        bool visit(const GenericSpecTree::NodeType<BlockDepSpec>::Type & n) const
        {
            return stringify(*n.spec()) == pattern;
        }

        bool visit(const GenericSpecTree::NodeType<LicenseDepSpec>::Type & n) const
        {
            return stringify(*n.spec()) == pattern;
        }

        bool visit(const GenericSpecTree::NodeType<SimpleURIDepSpec>::Type & n) const
        {
            return stringify(*n.spec()) == pattern;
        }

        bool visit(const GenericSpecTree::NodeType<FetchableURIDepSpec>::Type & n) const
        {
            return stringify(*n.spec()) == pattern;
        }

        bool visit(const GenericSpecTree::NodeType<DependenciesLabelsDepSpec>::Type & n) const
        {
            return indirect_iterator(n.spec()->end()) != std::find_if(indirect_iterator(n.spec()->begin()),
                    indirect_iterator(n.spec()->end()), StringifyEqual(pattern));
        }

        bool visit(const GenericSpecTree::NodeType<URILabelsDepSpec>::Type & n) const
        {
            return indirect_iterator(n.spec()->end()) != std::find_if(indirect_iterator(n.spec()->begin()),
                    indirect_iterator(n.spec()->end()), StringifyEqual(pattern));
        }

        bool visit(const GenericSpecTree::NodeType<PlainTextLabelDepSpec>::Type & n) const
        {
            return stringify(*n.spec()) == pattern;
        }
    };

    struct KeyComparator
    {
        const Environment * const env;
        const std::shared_ptr<const PackageID> id;
        const std::string pattern;
        const char op;

        KeyComparator(const Environment * const e, const std::shared_ptr<const PackageID> & i,
                const std::string & p, const char o) :
            env(e),
            id(i),
            pattern(p),
            op(o)
        {
        }

        bool visit(const MetadataSectionKey &) const
        {
            return false;
        }

        bool visit(const MetadataTimeKey & k) const
        {
            switch (op)
            {
                case ukro_equal:
                    return pattern == stringify(k.parse_value().seconds());
                case ukro_not_equal:
                    return pattern != stringify(k.parse_value().seconds());
                case ukro_less_or_subset:
                    return k.parse_value().seconds() < destringify<time_t>(pattern);
                case ukro_greater:
                    return k.parse_value().seconds() > destringify<time_t>(pattern);
            }

            return false;
        }

        bool visit(const MetadataValueKey<std::string> & k) const
        {
            switch (op)
            {
                case ukro_equal:
                    return pattern == stringify(k.parse_value());
                case ukro_not_equal:
                    return pattern != stringify(k.parse_value());
            }

            return false;
        }

        bool visit(const MetadataValueKey<Slot> & k) const
        {
            switch (op)
            {
                case ukro_equal:
                    return pattern == stringify(k.parse_value().raw_value());
                case ukro_not_equal:
                    return pattern != stringify(k.parse_value().raw_value());
            }

            return false;
        }

        bool visit(const MetadataValueKey<FSPath> & k) const
        {
            switch (op)
            {
                case ukro_equal:
                    return pattern == stringify(k.parse_value());
                case ukro_not_equal:
                    return pattern != stringify(k.parse_value());
            }

            return false;
        }

        bool visit(const MetadataValueKey<bool> & k) const
        {
            switch (op)
            {
                case ukro_equal:
                    return pattern == stringify(k.parse_value());
                case ukro_not_equal:
                    return pattern != stringify(k.parse_value());
            }

            return false;
        }

        bool visit(const MetadataValueKey<long> & k) const
        {
            switch (op)
            {
                case ukro_equal:
                    return pattern == stringify(k.parse_value());
                case ukro_not_equal:
                    return pattern != stringify(k.parse_value());
                case ukro_less_or_subset:
                    return k.parse_value() < destringify<long>(pattern);
                case ukro_greater:
                    return k.parse_value() > destringify<long>(pattern);
            }

            return false;
        }

        bool visit(const MetadataValueKey<std::shared_ptr<const Choices> > &) const
        {
            return false;
        }

        bool visit(const MetadataValueKey<std::shared_ptr<const PackageID> > & k) const
        {
            switch (op)
            {
                case ukro_equal:
                    return pattern == stringify(*k.parse_value());
                case ukro_not_equal:
                    return pattern != stringify(*k.parse_value());
            }

            return false;
        }

        bool visit(const MetadataSpecTreeKey<DependencySpecTree> & s) const
        {
            switch (op)
            {
                case ukro_equal:
                    return false;
                case ukro_less_or_subset:
                    return s.parse_value()->top()->accept_returning<bool>(SpecTreeSearcher(env, id, pattern));
            }

            return false;
        }

        bool visit(const MetadataSpecTreeKey<PlainTextSpecTree> & s) const
        {
            switch (op)
            {
                case ukro_equal:
                    return false;
                case ukro_less_or_subset:
                    return s.parse_value()->top()->accept_returning<bool>(SpecTreeSearcher(env, id, pattern));
            }

            return false;
        }

        bool visit(const MetadataSpecTreeKey<RequiredUseSpecTree> & s) const
        {
            switch (op)
            {
                case ukro_equal:
                    return false;
                case ukro_less_or_subset:
                    return s.parse_value()->top()->accept_returning<bool>(SpecTreeSearcher(env, id, pattern));
            }

            return false;
        }

        bool visit(const MetadataSpecTreeKey<SimpleURISpecTree> & s) const
        {
            switch (op)
            {
                case ukro_equal:
                    return false;
                case ukro_less_or_subset:
                    return s.parse_value()->top()->accept_returning<bool>(SpecTreeSearcher(env, id, pattern));
            }

            return false;
        }

        bool visit(const MetadataSpecTreeKey<FetchableURISpecTree> & s) const
        {
            switch (op)
            {
                case ukro_equal:
                    return false;
                case ukro_less_or_subset:
                    return s.parse_value()->top()->accept_returning<bool>(SpecTreeSearcher(env, id, pattern));
            }

            return false;
        }

        bool visit(const MetadataSpecTreeKey<LicenseSpecTree> & s) const
        {
            switch (op)
            {
                case ukro_equal:
                    return false;
                case ukro_less_or_subset:
                    return s.parse_value()->top()->accept_returning<bool>(SpecTreeSearcher(env, id, pattern));
            }

            return false;
        }

        bool visit(const MetadataCollectionKey<FSPathSequence> & s) const
        {
            auto v(s.parse_value());
            switch (op)
            {
                case ukro_equal:
                    return pattern == join(v->begin(), v->end(), " ");
                case ukro_less_or_subset:
                    return v->end() != std::find_if(v->begin(), v->end(),
                            StringifyEqual(pattern));
            }

            return false;
        }

        bool visit(const MetadataCollectionKey<PackageIDSequence> & s) const
        {
            auto v(s.parse_value());
            switch (op)
            {
                case ukro_equal:
                    return pattern == join(indirect_iterator(v->begin()), indirect_iterator(v->end()), " ");
                case ukro_less_or_subset:
                    return indirect_iterator(v->end()) != std::find_if(
                            indirect_iterator(v->begin()),
                            indirect_iterator(v->end()),
                            StringifyEqual(pattern));
            }

            return false;
        }

        bool visit(const MetadataCollectionKey<Sequence<std::string> > & s) const
        {
            auto v(s.parse_value());
            switch (op)
            {
                case ukro_equal:
                    return pattern == join(v->begin(), v->end(), " ");
                case ukro_less_or_subset:
                    return v->end() != std::find_if(v->begin(), v->end(),
                            StringifyEqual(pattern));
            }

            return false;
        }

        bool visit(const MetadataCollectionKey<Set<std::string> > & s) const
        {
            auto v(s.parse_value());
            switch (op)
            {
                case ukro_equal:
                    return pattern == join(v->begin(), v->end(), " ");
                case ukro_less_or_subset:
                    return v->end() != std::find_if(v->begin(), v->end(),
                            StringifyEqual(pattern));
            }

            return false;
        }

        bool visit(const MetadataCollectionKey<Map<std::string, std::string> > &) const
        {
            return false;
        }

        bool visit(const MetadataCollectionKey<KeywordNameSet> & s) const
        {
            auto v(s.parse_value());
            switch (op)
            {
                case ukro_equal:
                    return pattern == join(v->begin(), v->end(), " ");
                case ukro_less_or_subset:
                    return v->end() != std::find_if(v->begin(), v->end(),
                            StringifyEqual(pattern));
            }

            return false;
        }

        bool visit(const MetadataCollectionKey<Maintainers> & s) const
        {
            auto v(s.parse_value());
            switch (op)
            {
                case ukro_equal:
                    return pattern == join(v->begin(), v->end(), " ");
                case ukro_less_or_subset:
                    return v->end() != std::find_if(v->begin(), v->end(), StringifyEqual(pattern));
            }

            return false;
        }
    };

    struct AssociatedKeyFinder
    {
        const Environment * const env;
        const std::shared_ptr<const PackageID> id;

        const MetadataKey * const visit(const UserMask &) const
        {
            return nullptr;
        }

        const MetadataKey * const visit(const UnacceptedMask & m) const
        {
            auto k(id->find_metadata(m.unaccepted_key_name()));
            if (k != id->end_metadata())
                return &**k;
            else
                return nullptr;
        }

        const MetadataKey * const visit(const RepositoryMask &) const
        {
            return nullptr;
        }

        const MetadataKey * const visit(const UnsupportedMask &) const
        {
            return nullptr;
        }
    };

    struct MaskChecker
    {
        const std::string key;

        bool visit(const UserMask & m) const
        {
            return key == "*" || key == "user" || m.token() == key;
        }

        bool visit(const UnacceptedMask & m) const
        {
            return key == "*" || key == "unaccepted" || m.unaccepted_key_name() == key;
        }

        bool visit(const RepositoryMask & m) const
        {
            return key == "*" || key == "repository" || m.token() == key;
        }

        bool visit(const UnsupportedMask &) const
        {
            return key == "*" || key == "unsupported";
        }
    };
}

const std::pair<bool, std::string>
UserKeyRequirement::requirement_met(
        const Environment * const env,
        const ChangedChoices * const,
        const std::shared_ptr<const PackageID> & id,
        const std::shared_ptr<const PackageID> & from_id,
        const ChangedChoices * const) const
{
    Context context("When working out whether '" + stringify(*id) + "' matches " + as_raw_string() + ":");

    const MetadataKey * key_requirement(nullptr);
    const Mask * mask_requirement(nullptr);

    auto repo(env->fetch_repository(id->repository_name()));
    if (0 == _imp->key.compare(0, 3, "::$"))
    {
        if (_imp->key == "::$format")
            key_requirement = repo->format_key().get();
        else if (_imp->key == "::$location")
            key_requirement = repo->location_key().get();
        else if (_imp->key == "::$installed_root")
            key_requirement = repo->installed_root_key().get();
        else if (_imp->key == "::$sync_host")
            key_requirement = repo->sync_host_key().get();
    }
    else if (0 == _imp->key.compare(0, 1, "$"))
    {
        if (_imp->key == "$behaviours")
            key_requirement = id->behaviours_key().get();
        else if (_imp->key == "$build_dependencies_target")
            key_requirement = id->build_dependencies_target_key().get();
        else if (_imp->key == "$build_dependencies_host")
            key_requirement = id->build_dependencies_host_key().get();
        else if (_imp->key == "$choices")
            key_requirement = id->choices_key().get();
        else if (_imp->key == "$dependencies")
            key_requirement = id->dependencies_key().get();
        else if (_imp->key == "$fetches")
            key_requirement = id->fetches_key().get();
        else if (_imp->key == "$from_repositories")
            key_requirement = id->from_repositories_key().get();
        else if (_imp->key == "$fs_location")
            key_requirement = id->fs_location_key().get();
        else if (_imp->key == "$homepage")
            key_requirement = id->homepage_key().get();
        else if (_imp->key == "$installed_time")
            key_requirement = id->installed_time_key().get();
        else if (_imp->key == "$keywords")
            key_requirement = id->keywords_key().get();
        else if (_imp->key == "$long_description")
            key_requirement = id->long_description_key().get();
        else if (_imp->key == "$post_dependencies")
            key_requirement = id->post_dependencies_key().get();
        else if (_imp->key == "$run_dependencies")
            key_requirement = id->run_dependencies_key().get();
        else if (_imp->key == "$short_description")
            key_requirement = id->short_description_key().get();
        else if (_imp->key == "$slot")
            key_requirement = id->slot_key().get();
    }
    else if (0 == _imp->key.compare(0, 2, "::"))
    {
        Repository::MetadataConstIterator m(repo->find_metadata(_imp->key.substr(2)));
        if (m != repo->end_metadata())
            key_requirement = m->get();
    }
    else if (0 == _imp->key.compare(0, 1, "(") && ')' == _imp->key.at(_imp->key.length() - 1))
    {
        std::string mask_name(_imp->key.substr(1, _imp->key.length() - 2));
        MaskChecker checker{mask_name};
        for (const auto & mask : id->masks())
            if (mask->accept_returning<bool>(checker))
            {
                mask_requirement = &*mask;
                break;
            }
    }
    else
    {
        PackageID::MetadataConstIterator m(id->find_metadata(_imp->key));
        if (m != id->end_metadata())
            key_requirement = m->get();
    }

    if ((! key_requirement) && (! mask_requirement))
        return std::make_pair(false, as_human_string(from_id));

    if (_imp->op == ukro_exists)
        return std::make_pair(true, as_human_string(from_id));

    if (mask_requirement && ! key_requirement)
        key_requirement = mask_requirement->accept_returning<const MetadataKey *>(AssociatedKeyFinder{env, id});

    if (key_requirement)
    {
        KeyComparator c(env, id, _imp->value, _imp->op);
        return std::make_pair(key_requirement->accept_returning<bool>(c), as_human_string(from_id));
    }
    else
        return std::make_pair(false, as_human_string(from_id));
}

const std::string
UserKeyRequirement::as_human_string(const std::shared_ptr<const PackageID> &) const
{
    std::string key_str;
    if ((! _imp->key.empty()) && (_imp->key.at(0) == '$'))
        key_str = "with role '" + _imp->key.substr(1) + "'";
    else
        key_str = "'" + _imp->key + "'";

    switch (_imp->op)
    {
        case ukro_equal:
            return "Key " + key_str + " has simple string value '" + _imp->value + "'";
        case ukro_not_equal:
            return "Key " + key_str + " has a different string value than'" + _imp->value + "'";
        case ukro_less_or_subset:
            return "Key " + key_str + " contains or is less than '" + _imp->value + "'";
        case ukro_greater:
            return "Key " + key_str + " is greater than '" + _imp->value + "'";
        case ukro_exists:
            return "Key " + key_str + " exists";
        case last_ukro:
            break;
    }

    throw InternalError(PALUDIS_HERE, "unknown op");
}

const std::string
UserKeyRequirement::as_raw_string() const
{
    std::string op_string = "";
    switch (_imp->op)
    {
        case ukro_equal:
            return "[." + _imp->key + "=" + _imp->value + "]";
        case ukro_not_equal:
            return "[." + _imp->key + "!=" + _imp->value + "]";
        case ukro_less_or_subset:
            return "[." + _imp->key + "<" + _imp->value + "]";
        case ukro_greater:
            return "[." + _imp->key + ">" + _imp->value + "]";
        case ukro_exists:
            return "[." + _imp->key + "?" + _imp->value + "]";
        case last_ukro:
            break;
    }

    throw InternalError(PALUDIS_HERE, "unknown op");
}

Tribool
UserKeyRequirement::accumulate_changes_to_make_met(
        const Environment * const,
        const ChangedChoices * const,
        const std::shared_ptr<const PackageID> &,
        const std::shared_ptr<const PackageID> &,
        ChangedChoices &) const
{
    return false;
}

ExcludeRequirement::ExcludeRequirement(const PackageDepSpec & s) :
    _s(s)
{
}

ExcludeRequirement::~ExcludeRequirement() = default;

const std::pair<bool, std::string>
ExcludeRequirement::requirement_met(
        const Environment * const env,
        const ChangedChoices * const,
        const std::shared_ptr<const PackageID> & id,
        const std::shared_ptr<const PackageID> & from_id,
        const ChangedChoices * const) const
{
    Context context("When working out whether '" + stringify(*id) + "' matches " + as_raw_string() + ":");

    return std::make_pair(!match_package(*env, _s, id, from_id, { }), as_human_string(from_id));
}

const std::string
ExcludeRequirement::as_human_string(const std::shared_ptr<const PackageID> &) const
{
    return "Exclude packages matching " + stringify(_s);
}

const std::string
ExcludeRequirement::as_raw_string() const
{
    return "[.!exclude=" + stringify(_s) + "]";
}

Tribool
ExcludeRequirement::accumulate_changes_to_make_met(
        const Environment * const,
        const ChangedChoices * const,
        const std::shared_ptr<const PackageID> &,
        const std::shared_ptr<const PackageID> &,
        ChangedChoices &) const
{
    return false;
}

VersionSpecOptions
paludis::user_version_spec_options()
{
    return { vso_flexible_dashes, vso_flexible_dots,
        vso_ignore_case, vso_letters_anywhere, vso_dotted_suffixes };
}

namespace paludis
{
    template class Pimp<UserKeyRequirement>;
}

