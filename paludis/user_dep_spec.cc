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
#include <paludis/version_requirements.hh>
#include <paludis/package_database.hh>
#include <paludis/filter.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/dep_label.hh>
#include <paludis/partially_made_package_dep_spec.hh>
#include <paludis/util/options.hh>
#include <paludis/util/log.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/tribool.hh>
#include <paludis/util/make_null_shared_ptr.hh>
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
            result.package(env->package_database()->fetch_unique_qualified_package_name(PackageNamePart(s),
                filter::And(filter, filter::Matches(result, make_null_shared_ptr(), { }))));
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
                                vop = vo_nice_equal_star;
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
                    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req(std::make_shared<UserKeyRequirement>(flag.substr(1)));
                    result.additional_requirement(req);
                }
                break;

            default:
                {
                    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req(parse_elike_use_requirement(flag, { }));
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
        std::string::size_type slot_p(s.rfind(':'));
        if (std::string::npos != slot_p)
        {
            result.slot_requirement(std::make_shared<UserSlotExactRequirement>(SlotName(s.substr(slot_p + 1))));
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

namespace paludis
{
    template <>
    struct Imp<UserKeyRequirement>
    {
        std::string key;
        std::string value;
        char op;

        Imp(const std::string & s)
        {
            std::string::size_type p(s.find_first_of("=<>?"));
            if (std::string::npos == p)
                throw PackageDepSpecError("Expected an =, a <, a > or a ? inside '[." + s + "]'");

            key = s.substr(0, p);
            value = s.substr(p + 1);
            op = s.at(p);

            if (op == '?' && ! value.empty())
                throw PackageDepSpecError("Operator '?' takes no value inside '[." + s + "]'");
        }
    };
}

UserKeyRequirement::UserKeyRequirement(const std::string & s) :
    Pimp<UserKeyRequirement>(s)
{
}

UserKeyRequirement::~UserKeyRequirement()
{
}

namespace
{
    std::string stringify_contents_entry(const ContentsEntry & e)
    {
        return stringify(e.location_key()->value());
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
                case '=':
                    return pattern == stringify(k.value().seconds());
                case '<':
                    return k.value().seconds() < destringify<time_t>(pattern);
                case '>':
                    return k.value().seconds() > destringify<time_t>(pattern);
            }

            return false;
        }

        bool visit(const MetadataValueKey<std::string> & k) const
        {
            return pattern == stringify(k.value());
        }

        bool visit(const MetadataValueKey<SlotName> & k) const
        {
            return pattern == stringify(k.value());
        }

        bool visit(const MetadataValueKey<FSPath> & k) const
        {
            return pattern == stringify(k.value());
        }

        bool visit(const MetadataValueKey<bool> & k) const
        {
            return pattern == stringify(k.value());
        }

        bool visit(const MetadataValueKey<long> & k) const
        {
            switch (op)
            {
                case '=':
                    return pattern == stringify(k.value());
                case '<':
                    return k.value() < destringify<long>(pattern);
                case '>':
                    return k.value() > destringify<long>(pattern);
            }

            return false;
        }

        bool visit(const MetadataValueKey<std::shared_ptr<const Choices> > &) const
        {
            return false;
        }

        bool visit(const MetadataValueKey<std::shared_ptr<const RepositoryMaskInfo> > &) const
        {
            return false;
        }

        bool visit(const MetadataValueKey<std::shared_ptr<const Contents> > & s) const
        {
            switch (op)
            {
                case '=':
                    return pattern == join(indirect_iterator(s.value()->begin()), indirect_iterator(s.value()->end()), " ",
                            stringify_contents_entry);
                case '<':
                    return indirect_iterator(s.value()->end()) != std::find_if(
                            indirect_iterator(s.value()->begin()),
                            indirect_iterator(s.value()->end()),
                            StringifyEqual(pattern));
            }

            return false;
        }

        bool visit(const MetadataValueKey<std::shared_ptr<const PackageID> > & k) const
        {
            return pattern == stringify(*k.value());
        }

        bool visit(const MetadataSpecTreeKey<DependencySpecTree> & s) const
        {
            switch (op)
            {
                case '=':
                    return false;
                case '<':
                    return s.value()->top()->accept_returning<bool>(SpecTreeSearcher(env, id, pattern));
            }

            return false;
        }

        bool visit(const MetadataSpecTreeKey<SetSpecTree> & s) const
        {
            switch (op)
            {
                case '=':
                    return false;
                case '<':
                    return s.value()->top()->accept_returning<bool>(SpecTreeSearcher(env, id, pattern));
            }

            return false;
        }

        bool visit(const MetadataSpecTreeKey<PlainTextSpecTree> & s) const
        {
            switch (op)
            {
                case '=':
                    return false;
                case '<':
                    return s.value()->top()->accept_returning<bool>(SpecTreeSearcher(env, id, pattern));
            }

            return false;
        }

        bool visit(const MetadataSpecTreeKey<RequiredUseSpecTree> & s) const
        {
            switch (op)
            {
                case '=':
                    return false;
                case '<':
                    return s.value()->top()->accept_returning<bool>(SpecTreeSearcher(env, id, pattern));
            }

            return false;
        }

        bool visit(const MetadataSpecTreeKey<ProvideSpecTree> & s) const
        {
            switch (op)
            {
                case '=':
                    return false;
                case '<':
                    return s.value()->top()->accept_returning<bool>(SpecTreeSearcher(env, id, pattern));
            }

            return false;
        }

        bool visit(const MetadataSpecTreeKey<SimpleURISpecTree> & s) const
        {
            switch (op)
            {
                case '=':
                    return false;
                case '<':
                    return s.value()->top()->accept_returning<bool>(SpecTreeSearcher(env, id, pattern));
            }

            return false;
        }

        bool visit(const MetadataSpecTreeKey<FetchableURISpecTree> & s) const
        {
            switch (op)
            {
                case '=':
                    return false;
                case '<':
                    return s.value()->top()->accept_returning<bool>(SpecTreeSearcher(env, id, pattern));
            }

            return false;
        }

        bool visit(const MetadataSpecTreeKey<LicenseSpecTree> & s) const
        {
            switch (op)
            {
                case '=':
                    return false;
                case '<':
                    return s.value()->top()->accept_returning<bool>(SpecTreeSearcher(env, id, pattern));
            }

            return false;
        }

        bool visit(const MetadataCollectionKey<FSPathSequence> & s) const
        {
            switch (op)
            {
                case '=':
                    return pattern == join(s.value()->begin(), s.value()->end(), " ");
                case '<':
                    return s.value()->end() != std::find_if(s.value()->begin(), s.value()->end(),
                            StringifyEqual(pattern));
            }

            return false;
        }

        bool visit(const MetadataCollectionKey<PackageIDSequence> & s) const
        {
            switch (op)
            {
                case '=':
                    return pattern == join(indirect_iterator(s.value()->begin()), indirect_iterator(s.value()->end()), " ");
                case '<':
                    return indirect_iterator(s.value()->end()) != std::find_if(
                            indirect_iterator(s.value()->begin()),
                            indirect_iterator(s.value()->end()),
                            StringifyEqual(pattern));
            }

            return false;
        }

        bool visit(const MetadataCollectionKey<Sequence<std::string> > & s) const
        {
            switch (op)
            {
                case '=':
                    return pattern == join(s.value()->begin(), s.value()->end(), " ");
                case '<':
                    return s.value()->end() != std::find_if(s.value()->begin(), s.value()->end(),
                            StringifyEqual(pattern));
            }

            return false;
        }

        bool visit(const MetadataCollectionKey<Set<std::string> > & s) const
        {
            switch (op)
            {
                case '=':
                    return pattern == join(s.value()->begin(), s.value()->end(), " ");
                case '<':
                    return s.value()->end() != std::find_if(s.value()->begin(), s.value()->end(),
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
            switch (op)
            {
                case '=':
                    return pattern == join(s.value()->begin(), s.value()->end(), " ");
                case '<':
                    return s.value()->end() != std::find_if(s.value()->begin(), s.value()->end(),
                            StringifyEqual(pattern));
            }

            return false;
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

    const MetadataKey * key(0);

    auto repo(env->package_database()->fetch_repository(id->repository_name()));
    if (0 == _imp->key.compare(0, 3, "::$"))
    {
        if (_imp->key == "::$format")
            key = repo->format_key().get();
        else if (_imp->key == "::$location")
            key = repo->location_key().get();
        else if (_imp->key == "::$installed_root")
            key = repo->installed_root_key().get();
        else if (_imp->key == "::$accept_keywords")
            key = repo->accept_keywords_key().get();
        else if (_imp->key == "::$sync_host")
            key = repo->sync_host_key().get();
    }
    else if (0 == _imp->key.compare(0, 1, "$"))
    {
        if (_imp->key == "$behaviours")
            key = id->behaviours_key().get();
        else if (_imp->key == "$build_dependencies")
            key = id->build_dependencies_key().get();
        else if (_imp->key == "$choices")
            key = id->choices_key().get();
        else if (_imp->key == "$contained_in")
            key = id->contained_in_key().get();
        else if (_imp->key == "$contains")
            key = id->contains_key().get();
        else if (_imp->key == "$contents")
            key = id->contents_key().get();
        else if (_imp->key == "$dependencies")
            key = id->dependencies_key().get();
        else if (_imp->key == "$fetches")
            key = id->fetches_key().get();
        else if (_imp->key == "$from_repositories")
            key = id->from_repositories_key().get();
        else if (_imp->key == "$fs_location")
            key = id->fs_location_key().get();
        else if (_imp->key == "$homepage")
            key = id->homepage_key().get();
        else if (_imp->key == "$installed_time")
            key = id->installed_time_key().get();
        else if (_imp->key == "$keywords")
            key = id->keywords_key().get();
        else if (_imp->key == "$long_description")
            key = id->long_description_key().get();
        else if (_imp->key == "$post_dependencies")
            key = id->post_dependencies_key().get();
        else if (_imp->key == "$provide")
            key = id->provide_key().get();
        else if (_imp->key == "$run_dependencies")
            key = id->run_dependencies_key().get();
        else if (_imp->key == "$short_description")
            key = id->short_description_key().get();
        else if (_imp->key == "$slot")
            key = id->slot_key().get();
        else if (_imp->key == "$suggested_dependencies")
            key = id->suggested_dependencies_key().get();
        else if (_imp->key == "$virtual_for")
            key = id->virtual_for_key().get();
    }
    else if (0 == _imp->key.compare(0, 2, "::"))
    {
        Repository::MetadataConstIterator m(repo->find_metadata(_imp->key.substr(2)));
        if (m != repo->end_metadata())
            key = m->get();
    }
    else
    {
        PackageID::MetadataConstIterator m(id->find_metadata(_imp->key));
        if (m != id->end_metadata())
            key = m->get();
    }

    if (! key)
        return std::make_pair(false, as_human_string(from_id));

    if (_imp->op == '?')
        return std::make_pair(true, as_human_string(from_id));
    else
    {
        KeyComparator c(env, id, _imp->value, _imp->op);
        return std::make_pair(key->accept_returning<bool>(c), as_human_string(from_id));
    }
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
        case '=':
            return "Key " + key_str + " has simple string value '" + _imp->value + "'";
        case '<':
            return "Key " + key_str + " contains or is less than '" + _imp->value + "'";
        case '>':
            return "Key " + key_str + " is greater than '" + _imp->value + "'";
        case '?':
            return "Key " + key_str + " exists";
    }

    throw InternalError(PALUDIS_HERE, "unknown op");
}

const std::string
UserKeyRequirement::as_raw_string() const
{
    return "[." + _imp->key + std::string(1, _imp->op) + _imp->value + "]";
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

VersionSpecOptions
paludis::user_version_spec_options()
{
    return { vso_flexible_dashes, vso_flexible_dots,
        vso_ignore_case, vso_letters_anywhere, vso_dotted_suffixes };
}

template class Pimp<UserKeyRequirement>;

