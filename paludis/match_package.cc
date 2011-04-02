/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/match_package.hh>
#include <paludis/dep_spec.hh>
#include <paludis/dep_spec_flattener.hh>
#include <paludis/environment.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/action.hh>
#include <paludis/repository.hh>
#include <paludis/additional_package_dep_spec_requirement.hh>
#include <paludis/package_dep_spec_constraint.hh>
#include <paludis/contents.hh>
#include <paludis/version_operator.hh>

#include <paludis/util/set.hh>
#include <paludis/util/options.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/join.hh>

#include <functional>
#include <algorithm>
#include <istream>
#include <ostream>

using namespace paludis;

#include <paludis/match_package-se.cc>

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
        const KeyConstraintOperation op;

        KeyComparator(const Environment * const e, const std::shared_ptr<const PackageID> & i,
                const std::string & p, const KeyConstraintOperation o) :
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
                case kco_equals:
                    return pattern == stringify(k.value().seconds());
                case kco_less_than:
                    return k.value().seconds() < destringify<time_t>(pattern);
                case kco_greater_than:
                    return k.value().seconds() > destringify<time_t>(pattern);
                case kco_question:
                case last_kco:
                    break;
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
                case kco_equals:
                    return pattern == stringify(k.value());
                case kco_less_than:
                    return k.value() < destringify<long>(pattern);
                case kco_greater_than:
                    return k.value() > destringify<long>(pattern);
                case kco_question:
                case last_kco:
                    break;
            }

            return false;
        }

        bool visit(const MetadataValueKey<std::shared_ptr<const Choices> > &) const
        {
            return false;
        }

        bool visit(const MetadataValueKey<std::shared_ptr<const Contents> > & s) const
        {
            switch (op)
            {
                case kco_equals:
                    return pattern == join(indirect_iterator(s.value()->begin()), indirect_iterator(s.value()->end()), " ",
                            stringify_contents_entry);
                case kco_less_than:
                    return indirect_iterator(s.value()->end()) != std::find_if(
                            indirect_iterator(s.value()->begin()),
                            indirect_iterator(s.value()->end()),
                            StringifyEqual(pattern));

                case kco_greater_than:
                case kco_question:
                case last_kco:
                    break;
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
                case kco_equals:
                    return false;
                case kco_less_than:
                    return s.value()->top()->accept_returning<bool>(SpecTreeSearcher(env, id, pattern));

                case kco_greater_than:
                case kco_question:
                case last_kco:
                    break;
            }

            return false;
        }

        bool visit(const MetadataSpecTreeKey<SetSpecTree> & s) const
        {
            switch (op)
            {
                case kco_equals:
                    return false;
                case kco_less_than:
                    return s.value()->top()->accept_returning<bool>(SpecTreeSearcher(env, id, pattern));

                case kco_greater_than:
                case kco_question:
                case last_kco:
                    break;
            }

            return false;
        }

        bool visit(const MetadataSpecTreeKey<PlainTextSpecTree> & s) const
        {
            switch (op)
            {
                case kco_equals:
                    return false;
                case kco_less_than:
                    return s.value()->top()->accept_returning<bool>(SpecTreeSearcher(env, id, pattern));

                case kco_greater_than:
                case kco_question:
                case last_kco:
                    break;
            }

            return false;
        }

        bool visit(const MetadataSpecTreeKey<RequiredUseSpecTree> & s) const
        {
            switch (op)
            {
                case kco_equals:
                    return false;
                case kco_less_than:
                    return s.value()->top()->accept_returning<bool>(SpecTreeSearcher(env, id, pattern));

                case kco_greater_than:
                case kco_question:
                case last_kco:
                    break;
            }

            return false;
        }

        bool visit(const MetadataSpecTreeKey<ProvideSpecTree> & s) const
        {
            switch (op)
            {
                case kco_equals:
                    return false;
                case kco_less_than:
                    return s.value()->top()->accept_returning<bool>(SpecTreeSearcher(env, id, pattern));

                case kco_greater_than:
                case kco_question:
                case last_kco:
                    break;
            }

            return false;
        }

        bool visit(const MetadataSpecTreeKey<SimpleURISpecTree> & s) const
        {
            switch (op)
            {
                case kco_equals:
                    return false;
                case kco_less_than:
                    return s.value()->top()->accept_returning<bool>(SpecTreeSearcher(env, id, pattern));

                case kco_greater_than:
                case kco_question:
                case last_kco:
                    break;
            }

            return false;
        }

        bool visit(const MetadataSpecTreeKey<FetchableURISpecTree> & s) const
        {
            switch (op)
            {
                case kco_equals:
                    return false;
                case kco_less_than:
                    return s.value()->top()->accept_returning<bool>(SpecTreeSearcher(env, id, pattern));

                case kco_greater_than:
                case kco_question:
                case last_kco:
                    break;
            }

            return false;
        }

        bool visit(const MetadataSpecTreeKey<LicenseSpecTree> & s) const
        {
            switch (op)
            {
                case kco_equals:
                    return false;
                case kco_less_than:
                    return s.value()->top()->accept_returning<bool>(SpecTreeSearcher(env, id, pattern));

                case kco_greater_than:
                case kco_question:
                case last_kco:
                    break;
            }

            return false;
        }

        bool visit(const MetadataCollectionKey<FSPathSequence> & s) const
        {
            switch (op)
            {
                case kco_equals:
                    return pattern == join(s.value()->begin(), s.value()->end(), " ");
                case kco_less_than:
                    return s.value()->end() != std::find_if(s.value()->begin(), s.value()->end(),
                            StringifyEqual(pattern));

                case kco_greater_than:
                case kco_question:
                case last_kco:
                    break;
            }

            return false;
        }

        bool visit(const MetadataCollectionKey<PackageIDSequence> & s) const
        {
            switch (op)
            {
                case kco_equals:
                    return pattern == join(indirect_iterator(s.value()->begin()), indirect_iterator(s.value()->end()), " ");
                case kco_less_than:
                    return indirect_iterator(s.value()->end()) != std::find_if(
                            indirect_iterator(s.value()->begin()),
                            indirect_iterator(s.value()->end()),
                            StringifyEqual(pattern));

                case kco_greater_than:
                case kco_question:
                case last_kco:
                    break;
            }

            return false;
        }

        bool visit(const MetadataCollectionKey<Sequence<std::string> > & s) const
        {
            switch (op)
            {
                case kco_equals:
                    return pattern == join(s.value()->begin(), s.value()->end(), " ");
                case kco_less_than:
                    return s.value()->end() != std::find_if(s.value()->begin(), s.value()->end(),
                            StringifyEqual(pattern));

                case kco_greater_than:
                case kco_question:
                case last_kco:
                    break;
            }

            return false;
        }

        bool visit(const MetadataCollectionKey<Set<std::string> > & s) const
        {
            switch (op)
            {
                case kco_equals:
                    return pattern == join(s.value()->begin(), s.value()->end(), " ");
                case kco_less_than:
                    return s.value()->end() != std::find_if(s.value()->begin(), s.value()->end(),
                            StringifyEqual(pattern));

                case kco_greater_than:
                case kco_question:
                case last_kco:
                    break;
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
                case kco_equals:
                    return pattern == join(s.value()->begin(), s.value()->end(), " ");
                case kco_less_than:
                    return s.value()->end() != std::find_if(s.value()->begin(), s.value()->end(),
                            StringifyEqual(pattern));

                case kco_greater_than:
                case kco_question:
                case last_kco:
                    break;
            }

            return false;
        }
    };

    bool
    match_key_constraint(
            const Environment * const env,
            const std::shared_ptr<const PackageID> & id,
            const std::shared_ptr<const PackageID> &,
            const std::shared_ptr<const KeyConstraint> & constraint)
    {
        const MetadataKey * key(0);

        auto repo(env->fetch_repository(id->repository_name()));
        if (0 == constraint->key().compare(0, 3, "::$"))
        {
            if (constraint->key() == "::$format")
                key = repo->format_key().get();
            else if (constraint->key() == "::$location")
                key = repo->location_key().get();
            else if (constraint->key() == "::$installed_root")
                key = repo->installed_root_key().get();
            else if (constraint->key() == "::$accept_keywords")
                key = repo->accept_keywords_key().get();
            else if (constraint->key() == "::$sync_host")
                key = repo->sync_host_key().get();
        }
        else if (0 == constraint->key().compare(0, 1, "$"))
        {
            if (constraint->key() == "$behaviours")
                key = id->behaviours_key().get();
            else if (constraint->key() == "$build_dependencies")
                key = id->build_dependencies_key().get();
            else if (constraint->key() == "$choices")
                key = id->choices_key().get();
            else if (constraint->key() == "$contained_in")
                key = id->contained_in_key().get();
            else if (constraint->key() == "$contains")
                key = id->contains_key().get();
            else if (constraint->key() == "$contents")
                key = id->contents_key().get();
            else if (constraint->key() == "$dependencies")
                key = id->dependencies_key().get();
            else if (constraint->key() == "$fetches")
                key = id->fetches_key().get();
            else if (constraint->key() == "$from_repositories")
                key = id->from_repositories_key().get();
            else if (constraint->key() == "$fs_location")
                key = id->fs_location_key().get();
            else if (constraint->key() == "$homepage")
                key = id->homepage_key().get();
            else if (constraint->key() == "$installed_time")
                key = id->installed_time_key().get();
            else if (constraint->key() == "$keywords")
                key = id->keywords_key().get();
            else if (constraint->key() == "$long_description")
                key = id->long_description_key().get();
            else if (constraint->key() == "$post_dependencies")
                key = id->post_dependencies_key().get();
            else if (constraint->key() == "$provide")
                key = id->provide_key().get();
            else if (constraint->key() == "$run_dependencies")
                key = id->run_dependencies_key().get();
            else if (constraint->key() == "$short_description")
                key = id->short_description_key().get();
            else if (constraint->key() == "$slot")
                key = id->slot_key().get();
            else if (constraint->key() == "$suggested_dependencies")
                key = id->suggested_dependencies_key().get();
            else if (constraint->key() == "$virtual_for")
                key = id->virtual_for_key().get();
        }
        else if (0 == constraint->key().compare(0, 2, "::"))
        {
            Repository::MetadataConstIterator m(repo->find_metadata(constraint->key().substr(2)));
            if (m != repo->end_metadata())
                key = m->get();
        }
        else
        {
            PackageID::MetadataConstIterator m(id->find_metadata(constraint->key()));
            if (m != id->end_metadata())
                key = m->get();
        }

        if (! key)
            return false;

        if (constraint->operation() == kco_question)
            return true;
        else
        {
            KeyComparator c(env, id, constraint->pattern(), constraint->operation());
            return key->accept_returning<bool>(c);
        }
    }
}

bool
paludis::match_package_with_maybe_changes(
        const Environment & env,
        const PackageDepSpec & spec,
        const ChangedChoices * const maybe_changes_to_owner,
        const std::shared_ptr<const PackageID> & id,
        const std::shared_ptr<const PackageID> & from_id,
        const ChangedChoices * const maybe_changes_to_target,
        const MatchPackageOptions & options)
{
    if (spec.package_name_constraint() && spec.package_name_constraint()->name() != id->name())
        return false;

    if (spec.package_name_part_constraint() && spec.package_name_part_constraint()->name_part() != id->name().package())
        return false;

    if (spec.category_name_part_constraint() && spec.category_name_part_constraint()->name_part() != id->name().category())
        return false;

    if (spec.all_version_constraints())
    {
        bool ok(true);

        for (auto r(spec.all_version_constraints()->begin()), r_end(spec.all_version_constraints()->end()) ;
                r != r_end ; ++r)
        {
            bool one((*r)->version_operator().as_version_spec_comparator()(id->version(), (*r)->version_spec()));

            switch ((*r)->combiner())
            {
                case vcc_and:   ok &= one; break;
                case vcc_or:    ok |= one; break;
                case last_vcc:  throw InternalError(PALUDIS_HERE, "Bad vcc");
            }
        }

        if (! ok)
            return false;
    }

    if (spec.in_repository_constraint())
        if (spec.in_repository_constraint()->name() != id->repository_name())
            return false;

    if (spec.from_repository_constraint())
    {
        if (! id->from_repositories_key())
            return false;

        if (id->from_repositories_key()->value()->end() == id->from_repositories_key()->value()->find(
                    stringify(spec.from_repository_constraint()->name())))
            return false;
    }

    if (spec.installed_at_path_constraint())
    {
        auto repo(env.fetch_repository(id->repository_name()));
        if (! repo->installed_root_key())
            return false;
        if (repo->installed_root_key()->value() != spec.installed_at_path_constraint()->path())
            return false;
    }

    if (spec.installable_to_repository_constraint())
    {
        if (! id->supports_action(SupportsActionTest<InstallAction>()))
            return false;
        if (! spec.installable_to_repository_constraint()->include_masked())
            if (id->masked())
                return false;

        const std::shared_ptr<const Repository> dest(env.fetch_repository(
                    spec.installable_to_repository_constraint()->name()));
        if (! dest->destination_interface())
            return false;
        if (! dest->destination_interface()->is_suitable_destination_for(id))
            return false;
    }

    if (spec.installable_to_path_constraint())
    {
        if (! id->supports_action(SupportsActionTest<InstallAction>()))
            return false;
        if (! spec.installable_to_path_constraint()->include_masked())
            if (id->masked())
                return false;

        bool ok(false);
        for (auto d(env.begin_repositories()), d_end(env.end_repositories()) ;
                d != d_end ; ++d)
        {
            if (! (*d)->destination_interface())
                continue;
            if (! (*d)->installed_root_key())
                continue;
            if ((*d)->installed_root_key()->value() != spec.installable_to_path_constraint()->path())
                continue;
            if (! (*d)->destination_interface()->is_suitable_destination_for(id))
                continue;

            ok = true;
            break;
        }

        if (! ok)
            return false;
    }

    if (spec.exact_slot_constraint())
    {
        if ((! id->slot_key()) || (id->slot_key()->value() != spec.exact_slot_constraint()->name()))
            return false;
    }

    if (spec.any_slot_constraint())
    {
        /* don't care */
    }

    if (! options[mpo_ignore_additional_requirements])
    {
        if (spec.additional_requirements_ptr())
        {
            for (AdditionalPackageDepSpecRequirements::ConstIterator u(spec.additional_requirements_ptr()->begin()),
                    u_end(spec.additional_requirements_ptr()->end()) ; u != u_end ; ++u)
                if (! (*u)->requirement_met(&env, maybe_changes_to_owner, id, from_id, maybe_changes_to_target).first)
                    return false;
        }
    }

    if (spec.all_key_constraints())
    {
        for (auto c(spec.all_key_constraints()->begin()), c_end(spec.all_key_constraints()->end()) ;
                c != c_end ; ++c)
        {
            if (! match_key_constraint(&env, id, from_id, *c))
                return false;
        }
    }

    return true;
}

bool
paludis::match_package(
        const Environment & env,
        const PackageDepSpec & spec,
        const std::shared_ptr<const PackageID> & id,
        const std::shared_ptr<const PackageID> & from_id,
        const MatchPackageOptions & options)
{
    return match_package_with_maybe_changes(env, spec, 0, id, from_id, 0, options);
}

bool
paludis::match_package_in_set(
        const Environment & env,
        const SetSpecTree & target,
        const std::shared_ptr<const PackageID> & id,
        const MatchPackageOptions & options)
{
    using namespace std::placeholders;

    DepSpecFlattener<SetSpecTree, PackageDepSpec> f(&env, id);
    target.top()->accept(f);
    return indirect_iterator(f.end()) != std::find_if(
            indirect_iterator(f.begin()), indirect_iterator(f.end()),
            std::bind(&match_package, std::cref(env), _1, std::cref(id), make_null_shared_ptr(), std::cref(options)));
}

