/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/resolver/sanitised_dependencies.hh>
#include <paludis/resolver/resolver.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/decider.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/save.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/options.hh>
#include <paludis/util/join.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/wrapped_output_iterator-impl.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/log.hh>
#include <paludis/util/map.hh>
#include <paludis/spec_tree.hh>
#include <paludis/slot_requirement.hh>
#include <paludis/metadata_key.hh>
#include <paludis/package_id.hh>
#include <paludis/elike_package_dep_spec.hh>
#include <paludis/serialise-impl.hh>
#include <paludis/environment.hh>
#include <paludis/repository.hh>
#include <algorithm>
#include <set>
#include <list>

using namespace paludis;
using namespace paludis::resolver;

namespace
{
    template <typename T_>
    void list_push_back(std::list<T_> * const l, const T_ & t)
    {
        l->push_back(t);
    }

    struct MakeAnyOfStringVisitor
    {
        const Environment * const env;
        const std::shared_ptr<const PackageID> our_id;

        std::string result;
        const std::shared_ptr<const ChangedChoices> changed_choices;

        void visit(const DependencySpecTree::NodeType<NamedSetDepSpec>::Type & s)
        {
            result.append(" " + stringify(*s.spec()));
        }

        void visit(const DependencySpecTree::NodeType<DependenciesLabelsDepSpec>::Type & s)
        {
            result.append(" " + stringify(*s.spec()));
        }

        void visit(const DependencySpecTree::NodeType<PackageDepSpec>::Type & s)
        {
            result.append(" " + stringify(*s.spec()));
        }

        void visit(const DependencySpecTree::NodeType<BlockDepSpec>::Type & s)
        {
            result.append(" " + stringify(*s.spec()));
        }

        void visit(const DependencySpecTree::NodeType<ConditionalDepSpec>::Type & node)
        {
            if (changed_choices ? node.spec()->condition_would_be_met_when(env, our_id, *changed_choices) : node.spec()->condition_met(env, our_id))
            {
                MakeAnyOfStringVisitor v{env, our_id, "", changed_choices};
                std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(v));
                result.append(" " + stringify(*node.spec()) + " (" + v.result + " )");
            }
        }

        void visit(const DependencySpecTree::NodeType<AnyDepSpec>::Type & node)
        {
            MakeAnyOfStringVisitor v{env, our_id, "", changed_choices};
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(v));
            result.append(" || (" + v.result + " )");
        }

        void visit(const DependencySpecTree::NodeType<AllDepSpec>::Type & node)
        {
            MakeAnyOfStringVisitor v{env, our_id, "", changed_choices};
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(v));
            result.append(" (" + v.result + " )");
        }
    };

    struct AnyDepSpecChildHandler
    {
        const Environment * const env;
        const Decider & decider;
        const std::shared_ptr<const Resolution> our_resolution;
        const std::shared_ptr<const PackageID> our_id;
        const std::shared_ptr<const ChangedChoices> changed_choices;
        const std::function<std::shared_ptr<SanitisedDependency> (const PackageOrBlockDepSpec &)> parent_make_sanitised;

        bool super_complicated, nested;

        std::list<std::list<PackageOrBlockDepSpec> > child_groups;
        std::list<PackageOrBlockDepSpec> * active_sublist;

        bool seen_any;

        AnyDepSpecChildHandler(
                const Environment * const e,
                const Decider & r, const std::shared_ptr<const Resolution> & q,
                const std::shared_ptr<const PackageID> & o,
                const std::shared_ptr<const ChangedChoices> & c,
                const std::function<std::shared_ptr<SanitisedDependency> (const PackageOrBlockDepSpec &)> & f) :
            env(e),
            decider(r),
            our_resolution(q),
            our_id(o),
            changed_choices(c),
            parent_make_sanitised(f),
            super_complicated(false),
            nested(false),
            active_sublist(nullptr),
            seen_any(false)
        {
        }

        void visit_package_or_block_spec(const PackageOrBlockDepSpec & spec)
        {
            seen_any = true;

            if (active_sublist)
                active_sublist->push_back(spec);
            else
            {
                std::list<PackageOrBlockDepSpec> l;
                l.push_back(spec);
                child_groups.push_back(l);
            }
        }

        void visit_package_spec(const PackageDepSpec & spec)
        {
            if (spec.package_ptr())
                visit_package_or_block_spec(PackageOrBlockDepSpec(spec));
            else
                super_complicated = true;
        }

        void visit_block_spec(const BlockDepSpec & spec)
        {
            if (spec.blocking().package_ptr())
                visit_package_or_block_spec(PackageOrBlockDepSpec(spec));
            else
                super_complicated = true;
        }

        void visit(const DependencySpecTree::NodeType<PackageDepSpec>::Type & node)
        {
            visit_package_spec(*node.spec());
        }

        void visit(const DependencySpecTree::NodeType<BlockDepSpec>::Type & node)
        {
            visit_block_spec(*node.spec());
        }

        void visit(const DependencySpecTree::NodeType<ConditionalDepSpec>::Type & node)
        {
            if (changed_choices ? node.spec()->condition_would_be_met_when(env, our_id, *changed_choices) : node.spec()->condition_met(env, our_id))
            {
                nested = true;

                if (active_sublist)
                    std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
                else
                {
                    Save<std::list<PackageOrBlockDepSpec> *> save_active_sublist(&active_sublist, nullptr);
                    active_sublist = &*child_groups.insert(child_groups.end(), std::list<PackageOrBlockDepSpec>());
                    std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
                }
            }
        }

        void visit(const DependencySpecTree::NodeType<AllDepSpec>::Type & node)
        {
            nested = true;

            if (active_sublist)
                std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
            else
            {
                Save<std::list<PackageOrBlockDepSpec> *> save_active_sublist(&active_sublist, nullptr);
                active_sublist = &*child_groups.insert(child_groups.end(), std::list<PackageOrBlockDepSpec>());
                std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
            }
        }

        void visit(const DependencySpecTree::NodeType<AnyDepSpec>::Type & node)
        {
            AnyDepSpecChildHandler h(env, decider, our_resolution, our_id, changed_choices, parent_make_sanitised);
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(h));
            std::list<SanitisedDependency> sanitised_deps;
            h.commit(
                    parent_make_sanitised,
                    std::bind(&list_push_back<SanitisedDependency>, &sanitised_deps, std::placeholders::_1)
                    );

            if (active_sublist)
            {
                for (const auto & i : sanitised_deps)
                    visit_package_or_block_spec(i.spec());
            }
            else
            {
                Save<std::list<PackageOrBlockDepSpec> *> save_active_sublist(&active_sublist, nullptr);
                active_sublist = &*child_groups.insert(child_groups.end(), std::list<PackageOrBlockDepSpec>());
                for (const auto & i : sanitised_deps)
                    visit_package_or_block_spec(i.spec());
            }
        }

        void visit(const DependencySpecTree::NodeType<NamedSetDepSpec>::Type &)
        {
            super_complicated = true;
        }

        void visit(const DependencySpecTree::NodeType<DependenciesLabelsDepSpec>::Type &)
        {
            super_complicated = true;
        }

        void commit(
                const std::function<std::shared_ptr<SanitisedDependency> (const PackageOrBlockDepSpec &)> & maybe_make_sanitised,
                const std::function<void (const SanitisedDependency &)> & apply)
        {
            if (! seen_any)
            {
            }
            else if (super_complicated)
                throw InternalError(PALUDIS_HERE, "Nested || ( ) dependencies too complicated to handle");
            else
            {
                /* we've got a choice of groups of packages. pick the best score, left to right. */
                std::list<std::list<PackageOrBlockDepSpec> >::const_iterator g_best(child_groups.end());
                std::pair<AnyChildScore, OperatorScore> best_score(acs_worse_than_worst, os_worse_than_worst);

                for (std::list<std::list<PackageOrBlockDepSpec> >::const_iterator g(child_groups.begin()),
                        g_end(child_groups.end()) ;
                        g != g_end ; ++g)
                {
                    /* best possible, to get ( ) right */
                    std::pair<AnyChildScore, OperatorScore> worst_score(acs_better_than_best, os_better_than_best);

                    /* score of a group is the score of the worst child. */
                    for (const auto & h : *g)
                    {
                        auto s(maybe_make_sanitised(PackageOrBlockDepSpec(h)));
                        if (s)
                        {
                            auto score(decider.find_any_score(our_resolution, our_id, *s));
                            Log::get_instance()->message("resolver.sanitised_dependencies.any_score", ll_debug, lc_context)
                                << "Scored " << h << " as " << score.first << " " << score.second;

                            if (score < worst_score)
                                worst_score = score;
                        }
                    }

                    if (worst_score > best_score)
                    {
                        best_score = worst_score;
                        g_best = g;
                    }
                }

                if (g_best != child_groups.end())
                {
                    /* might be nothing to do, if no labels are enabled */
                    for (const auto & h : *g_best)
                    {
                        auto s(maybe_make_sanitised(h));
                        if (s)
                            apply(*s);
                    }
                }
            }
        }
    };

    struct Finder
    {
        const Environment * const env;
        const Decider & decider;
        const std::shared_ptr<const Resolution> our_resolution;
        const std::shared_ptr<const PackageID> & our_id;
        const std::shared_ptr<const ChangedChoices> & changed_choices;
        SanitisedDependencies & sanitised_dependencies;
        const std::string raw_name;
        const std::string human_name;
        std::string original_specs_as_string;
        std::list<std::shared_ptr<const DependenciesLabelSequence> > labels_stack;
        std::list<ConditionalDepSpec> conditions_stack;

        Finder(
                const Environment * const e,
                const Decider & r,
                const std::shared_ptr<const Resolution> & q,
                const std::shared_ptr<const PackageID> & f,
                const std::shared_ptr<const ChangedChoices> & c,
                SanitisedDependencies & s,
                const std::shared_ptr<const DependenciesLabelSequence> & l,
                const std::string & rn,
                const std::string & hn,
                const std::string & a) :
            env(e),
            decider(r),
            our_resolution(q),
            our_id(f),
            changed_choices(c),
            sanitised_dependencies(s),
            raw_name(rn),
            human_name(hn),
            original_specs_as_string(a)
        {
            labels_stack.push_front(l);
        }


        void add(const SanitisedDependency & dep)
        {
            sanitised_dependencies.add(dep);
        }

        std::shared_ptr<SanitisedDependency> maybe_make_sanitised(const PackageOrBlockDepSpec & spec)
        {
            std::stringstream adl;
            std::stringstream acs;
            auto classifier_builder(std::make_shared<LabelsClassifierBuilder>(env, our_id));
            for (const auto & i : *(*labels_stack.begin()))
            {
                adl << (adl.str().empty() ? "" : ", ") << stringify(*i);
                i->accept(*classifier_builder);
            }

            auto classifier(classifier_builder->create());
            if (classifier->any_enabled)
            {
                for (auto & c : conditions_stack)
                    acs << (acs.str().empty() ? "" : ", ") << stringify(c);

                return make_shared_copy(make_named_values<SanitisedDependency>(
                            n::active_conditions_as_string() = acs.str(),
                            n::active_dependency_labels() = *labels_stack.begin(),
                            n::active_dependency_labels_as_string() = adl.str(),
                            n::active_dependency_labels_classifier() = classifier,
                            n::from_id() = our_id,
                            n::metadata_key_human_name() = human_name,
                            n::metadata_key_raw_name() = raw_name,
                            n::original_specs_as_string() = original_specs_as_string,
                            n::spec() = spec
                            ));
            }
            else
                return nullptr;
        }

        void visit(const DependencySpecTree::NodeType<PackageDepSpec>::Type & node)
        {
            auto s(maybe_make_sanitised(*node.spec()));
            if (s)
                add(*s);
        }

        void visit(const DependencySpecTree::NodeType<BlockDepSpec>::Type & node)
        {
            auto s(maybe_make_sanitised(*node.spec()));
            if (s)
                add(*s);
        }

        void visit(const DependencySpecTree::NodeType<ConditionalDepSpec>::Type & node)
        {
            if (changed_choices ? node.spec()->condition_would_be_met_when(env, our_id, *changed_choices) : node.spec()->condition_met(env, our_id))
            {
                conditions_stack.push_front(*node.spec());
                labels_stack.push_front(*labels_stack.begin());
                std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
                labels_stack.pop_front();
                conditions_stack.pop_front();
            }
        }

        void visit(const DependencySpecTree::NodeType<AllDepSpec>::Type & node)
        {
            labels_stack.push_front(*labels_stack.begin());
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
            labels_stack.pop_front();
        }

        void visit(const DependencySpecTree::NodeType<AnyDepSpec>::Type & node)
        {
            Save<std::string> save_original_specs_as_string(&original_specs_as_string);

            {
                MakeAnyOfStringVisitor v{env, our_id, "", changed_choices};
                std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(v));
                original_specs_as_string = "|| (" + v.result + " )";
            }

            AnyDepSpecChildHandler h(env, decider, our_resolution, our_id, changed_choices,
                    std::bind(&Finder::maybe_make_sanitised, this, std::placeholders::_1));
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(h));
            h.commit(
                    std::bind(&Finder::maybe_make_sanitised, this, std::placeholders::_1),
                    std::bind(&Finder::add, this, std::placeholders::_1)
                    );
        }

        void visit(const DependencySpecTree::NodeType<NamedSetDepSpec>::Type & node)
        {
            const std::shared_ptr<const SetSpecTree> set(env->set(node.spec()->name()));
            if (set)
                set->top()->accept(*this);
            else
                throw NoSuchSetError(stringify(node.spec()->name()));
        }

        void visit(const DependencySpecTree::NodeType<DependenciesLabelsDepSpec>::Type & node)
        {
            std::shared_ptr<DependenciesLabelSequence> labels(std::make_shared<DependenciesLabelSequence>());
            std::copy(node.spec()->begin(), node.spec()->end(), labels->back_inserter());
            *labels_stack.begin() = labels;
        }
    };
}

namespace paludis
{
    template <>
    struct Imp<SanitisedDependencies>
    {
        std::list<SanitisedDependency> sanitised_dependencies;
    };

    template <>
    struct WrappedForwardIteratorTraits<SanitisedDependencies::ConstIteratorTag>
    {
        typedef std::list<SanitisedDependency>::const_iterator UnderlyingIterator;
    };
}

SanitisedDependencies::SanitisedDependencies() :
    _imp()
{
}

SanitisedDependencies::~SanitisedDependencies() = default;

void
SanitisedDependencies::_populate_one(
        const Environment * const env,
        const Decider & decider,
        const std::shared_ptr<const Resolution> & resolution,
        const std::shared_ptr<const PackageID> & id,
        const std::shared_ptr<const ChangedChoices> & changed,
        const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > (PackageID::* const pmf) () const
        )
{
    Context context("When finding dependencies for '" + stringify(*id) + "' from key '" + ((*id).*pmf)()->raw_name() + "':");

    Finder f(env, decider, resolution, id, changed, *this, ((*id).*pmf)()->initial_labels(), ((*id).*pmf)()->raw_name(),
            ((*id).*pmf)()->human_name(), "");
    ((*id).*pmf)()->parse_value()->top()->accept(f);
}

void
SanitisedDependencies::populate(
        const Environment * const env,
        const Decider & decider,
        const std::shared_ptr<const Resolution> & resolution,
        const std::shared_ptr<const PackageID> & id,
        const std::shared_ptr<const ChangedChoices> & changed)
{
    Context context("When finding dependencies for '" + stringify(*id) + "':");

    if (id->dependencies_key())
        _populate_one(env, decider, resolution, id, changed, &PackageID::dependencies_key);
    else
    {
        if (id->build_dependencies_key())
            _populate_one(env, decider, resolution, id, changed, &PackageID::build_dependencies_key);
        if (id->run_dependencies_key())
            _populate_one(env, decider, resolution, id, changed, &PackageID::run_dependencies_key);
        if (id->post_dependencies_key())
            _populate_one(env, decider, resolution, id, changed, &PackageID::post_dependencies_key);
    }
}

void
SanitisedDependencies::add(const SanitisedDependency & dep)
{
    _imp->sanitised_dependencies.push_back(dep);
}

SanitisedDependencies::ConstIterator
SanitisedDependencies::begin() const
{
    return ConstIterator(_imp->sanitised_dependencies.begin());
}

SanitisedDependencies::ConstIterator
SanitisedDependencies::end() const
{
    return ConstIterator(_imp->sanitised_dependencies.end());
}

void
SanitisedDependency::serialise(Serialiser & s) const
{
    s.object("SanitisedDependency")
        .member(SerialiserFlags<>(), "active_conditions_as_string", active_conditions_as_string())
        .member(SerialiserFlags<>(), "active_dependency_labels_as_string", active_dependency_labels_as_string())
        .member(SerialiserFlags<serialise::might_be_null>(), "active_dependency_labels_classifier", active_dependency_labels_classifier())
        .member(SerialiserFlags<serialise::might_be_null>(), "from_id", from_id())
        .member(SerialiserFlags<>(), "metadata_key_human_name", metadata_key_human_name())
        .member(SerialiserFlags<>(), "metadata_key_raw_name", metadata_key_raw_name())
        .member(SerialiserFlags<>(), "original_specs_as_string", original_specs_as_string())
        .member(SerialiserFlags<>(), "spec", spec())
        ;
}

SanitisedDependency
SanitisedDependency::deserialise(Deserialisation & d, const std::shared_ptr<const PackageID> & from_id)
{
    Context context("When deserialising:");

    Deserialisator v(d, "SanitisedDependency");

    return make_named_values<SanitisedDependency>(
            n::active_conditions_as_string() = v.member<std::string>("active_conditions_as_string"),
            n::active_dependency_labels() = nullptr,
            n::active_dependency_labels_as_string() = v.member<std::string>("active_dependency_labels_as_string"),
            n::active_dependency_labels_classifier() = v.member<std::shared_ptr<const LabelsClassifier> >("active_dependency_labels_classifier"),
            n::from_id() = v.member<std::shared_ptr<const PackageID> >("from_id"),
            n::metadata_key_human_name() = v.member<std::string>("metadata_key_human_name"),
            n::metadata_key_raw_name() = v.member<std::string>("metadata_key_raw_name"),
            n::original_specs_as_string() = v.member<std::string>("original_specs_as_string"),
            n::spec() = PackageOrBlockDepSpec::deserialise(*v.find_remove_member("spec"),
                    from_id)
            );
}

namespace paludis
{
    template class WrappedForwardIterator<SanitisedDependencies::ConstIteratorTag, const SanitisedDependency>;
}
