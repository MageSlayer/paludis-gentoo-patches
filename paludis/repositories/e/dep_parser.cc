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

#include <paludis/repositories/e/dep_parser.hh>
#include <paludis/repositories/e/parse_dependency_label.hh>
#include <paludis/repositories/e/parse_uri_label.hh>
#include <paludis/repositories/e/parse_plain_text_label.hh>
#include <paludis/repositories/e/parse_annotations.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/options.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/map.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/return_literal_function.hh>
#include <paludis/util/log.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/elike_dep_parser.hh>
#include <paludis/elike_conditional_dep_spec.hh>
#include <paludis/elike_package_dep_spec.hh>
#include <paludis/dep_spec.hh>
#include <paludis/environment.hh>
#include <paludis/repository.hh>
#include <paludis/package_id.hh>
#include <paludis/action.hh>
#include <paludis/choice.hh>
#include <paludis/dep_spec_annotations.hh>
#include <paludis/metadata_key.hh>
#include <paludis/always_enabled_dependency_label.hh>
#include <paludis/elike_blocker.hh>
#include <paludis/elike_use_requirement.hh>
#include <paludis/partially_made_package_dep_spec.hh>
#include <map>
#include <list>
#include <set>
#include <ostream>
#include <algorithm>
#include <functional>

using namespace paludis;
using namespace paludis::erepository;

#include <paludis/repositories/e/dep_parser-se.cc>

EDepParseError::EDepParseError(const std::string & s, const std::string & t) throw () :
    Exception("Error parsing '" + s + "': " + t)
{
}

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_block_children> block_children;
        typedef Name<struct name_children> children;
        typedef Name<struct name_item> item;
        typedef Name<struct name_spec> spec;
    }
}

namespace
{
    enum BlockFixOp
    {
        bfo_explicit_strong,
        bfo_implicit_strong,
        bfo_implicit_weak
    };

    template <typename T_>
    struct ParseStackTypes
    {
        struct Item
        {
            NamedValue<n::block_children, std::list<std::pair<std::shared_ptr<BlockDepSpec>, BlockFixOp> > > block_children;
            NamedValue<n::children, std::list<std::shared_ptr<DepSpec> > > children;
            NamedValue<n::item, std::shared_ptr<typename T_::BasicInnerNode> > item;
            NamedValue<n::spec, std::shared_ptr<DepSpec> > spec;
        };

        typedef std::list<Item> Stack;
        typedef std::function<void (const std::shared_ptr<DepSpec> &)> AnnotationsGoHere;
        typedef std::function<void (const std::list<std::shared_ptr<DepSpec> > &)> StarAnnotationsGoHere;
    };

    template <typename T_>
    void package_dep_spec_string_handler(
            typename ParseStackTypes<T_>::Stack & h,
            const typename ParseStackTypes<T_>::AnnotationsGoHere & annotations_go_here,
            const std::string & s,
            const EAPI & eapi,
            bool add_explicit_choices_requirement)
    {
        auto data(partial_parse_elike_package_dep_spec(s, eapi.supported()->package_dep_spec_parse_options(),
                    eapi.supported()->version_spec_options()));
        if (add_explicit_choices_requirement)
            data.additional_requirement(make_elike_presumed_choices_requirement());
        std::shared_ptr<PackageDepSpec> spec(std::make_shared<PackageDepSpec>(data));
        h.begin()->item()->append(spec);
        h.begin()->children().push_back(spec);
        annotations_go_here(spec);
    }

    template <typename T_>
    void package_or_block_dep_spec_string_handler(
            typename ParseStackTypes<T_>::Stack & h,
            const typename ParseStackTypes<T_>::AnnotationsGoHere & annotations_go_here,
            const std::string & s,
            const EAPI & eapi)
    {
        auto p(split_elike_blocker(s));

        switch (std::get<0>(p))
        {
            case ebk_no_block:
                package_dep_spec_string_handler<T_>(h, annotations_go_here, s, eapi, true);
                break;

            case ebk_bang_question:
                throw EDepParseError(s, "!? blocks not allowed here");

            case ebk_single_bang:
            case ebk_double_bang:
                {
                    BlockFixOp op(bfo_implicit_weak);

                    if (std::get<0>(p) == ebk_double_bang)
                    {
                        if (! eapi.supported()->dependency_spec_tree_parse_options()[dstpo_double_bang_blocks])
                            throw EDepParseError(s, "Double-! blocks not allowed in this EAPI");
                        op = bfo_explicit_strong;
                    }
                    else
                    {
                        if (eapi.supported()->dependency_spec_tree_parse_options()[dstpo_single_bang_block_is_hard])
                            op = bfo_implicit_strong;
                    }

                    std::shared_ptr<BlockDepSpec> spec(std::make_shared<BlockDepSpec>(
                                s,
                                parse_elike_package_dep_spec(std::get<2>(p),
                                    eapi.supported()->package_dep_spec_parse_options(),
                                    eapi.supported()->version_spec_options())));
                    h.begin()->item()->append(spec);
                    h.begin()->block_children().push_back(std::make_pair(spec, op));
                    h.begin()->children().push_back(spec);

                    annotations_go_here(spec);
                }
                break;

            case last_ebk:
                throw InternalError(PALUDIS_HERE, "unhandled ebk");
        }
    }

    template <typename T_>
    void license_handler(
            typename ParseStackTypes<T_>::Stack & h,
            const typename ParseStackTypes<T_>::AnnotationsGoHere & annotations_go_here,
            const std::string & s)
    {
        std::shared_ptr<LicenseDepSpec> spec(std::make_shared<LicenseDepSpec>(s));
        h.begin()->item()->append(spec);
        h.begin()->children().push_back(spec);
        annotations_go_here(spec);
    }

    template <typename T_>
    void plain_text_handler(
            typename ParseStackTypes<T_>::Stack & h,
            const typename ParseStackTypes<T_>::AnnotationsGoHere & annotations_go_here,
            const std::string & s)
    {
        std::shared_ptr<PlainTextDepSpec> spec(std::make_shared<PlainTextDepSpec>(s));
        h.begin()->item()->append(spec);
        h.begin()->children().push_back(spec);
        annotations_go_here(spec);
    }

    template <typename T_>
    void simple_uri_handler(
            typename ParseStackTypes<T_>::Stack & h,
            const typename ParseStackTypes<T_>::AnnotationsGoHere & annotations_go_here,
            const std::string & s)
    {
        std::shared_ptr<SimpleURIDepSpec> spec(std::make_shared<SimpleURIDepSpec>(s));
        h.begin()->item()->append(spec);
        h.begin()->children().push_back(spec);
        annotations_go_here(spec);
    }

    template <typename T_>
    void arrow_handler(
            typename ParseStackTypes<T_>::Stack & h,
            const typename ParseStackTypes<T_>::AnnotationsGoHere & annotations_go_here,
            const std::string & s,
            const std::string & f,
            const std::string & t,
            const EAPI & eapi)
    {
        if (t.empty() || eapi.supported()->dependency_spec_tree_parse_options()[dstpo_uri_supports_arrow])
        {
            std::shared_ptr<FetchableURIDepSpec> spec(std::make_shared<FetchableURIDepSpec>(t.empty() ? f : f + " -> " + t));
            h.begin()->item()->append(spec);
            h.begin()->children().push_back(spec);
            annotations_go_here(spec);
        }
        else
            throw EDepParseError(s, "Arrows not allowed in this EAPI");
    }

    void any_not_allowed_handler(const std::string & s) PALUDIS_ATTRIBUTE((noreturn));

    void any_not_allowed_handler(const std::string & s)
    {
        throw EDepParseError(s, "Any dep specs not allowed here");
    }

    void exactly_one_not_allowed_handler(const std::string & s) PALUDIS_ATTRIBUTE((noreturn));

    void exactly_one_not_allowed_handler(const std::string & s)
    {
        throw EDepParseError(s, "Exactly one dep specs not allowed here");
    }
    void at_most_one_not_allowed_handler(const std::string & s) PALUDIS_ATTRIBUTE((noreturn));

    void at_most_one_not_allowed_handler(const std::string & s)
    {
        throw EDepParseError(s, "At most one dep specs not allowed here");
    }

    void arrows_not_allowed_handler(const std::string & s, const std::string & f, const std::string & t) PALUDIS_ATTRIBUTE((noreturn));

    void arrows_not_allowed_handler(const std::string & s, const std::string & f, const std::string & t)
    {
        throw EDepParseError(s, "Arrow '" + f + " -> " + t + "' not allowed here");
    }

    void error_handler(const std::string & s, const std::string & t) PALUDIS_ATTRIBUTE((noreturn));

    void error_handler(const std::string & s, const std::string & t)
    {
        throw EDepParseError(s, t);
    }

    void labels_not_allowed_handler(const std::string & s, const std::string & f) PALUDIS_ATTRIBUTE((noreturn));

    void labels_not_allowed_handler(const std::string & s, const std::string & f)
    {
        throw EDepParseError(s, "Label '" + f + "' not allowed here");
    }

    void use_not_allowed_handler(const std::string & s, const std::string & f) PALUDIS_ATTRIBUTE((noreturn));

    void use_not_allowed_handler(const std::string & s, const std::string & f)
    {
        throw EDepParseError(s, "Conditional '" + f + "' not allowed here");
    }

    template <typename T_>
    void dependency_label_handler(
            const Environment * const env,
            typename ParseStackTypes<T_>::Stack & h,
            const typename ParseStackTypes<T_>::AnnotationsGoHere & annotations_go_here,
            const std::string & s,
            const EAPI & eapi)
    {
        std::shared_ptr<DependenciesLabelsDepSpec> spec(parse_dependency_label(env, s, eapi));
        h.begin()->item()->append(spec);
        h.begin()->children().push_back(spec);
        annotations_go_here(spec);
    }

    template <typename T_>
    void fetchable_label_handler(
            typename ParseStackTypes<T_>::Stack & h,
            const typename ParseStackTypes<T_>::AnnotationsGoHere & annotations_go_here,
            const std::string & s,
            const EAPI & eapi)
    {
        std::shared_ptr<URILabelsDepSpec> spec(parse_uri_label(s, eapi));
        h.begin()->item()->append(spec);
        h.begin()->children().push_back(spec);
        annotations_go_here(spec);
    }

    template <typename T_>
    void plain_text_label_handler(
            typename ParseStackTypes<T_>::Stack & h,
            const typename ParseStackTypes<T_>::AnnotationsGoHere & annotations_go_here,
            const std::string & s)
    {
        std::shared_ptr<PlainTextLabelDepSpec> spec(parse_plain_text_label(s));
        h.begin()->item()->append(spec);
        h.begin()->children().push_back(spec);
        annotations_go_here(spec);
    }

    template <typename T_, typename A_>
    void any_all_handler(typename ParseStackTypes<T_>::Stack & stack)
    {
        std::shared_ptr<A_> spec(std::make_shared<A_>());
        stack.push_front(make_named_values<typename ParseStackTypes<T_>::Item>(
                    n::block_children() = std::list<std::pair<std::shared_ptr<BlockDepSpec>, BlockFixOp> >(),
                    n::children() = std::list<std::shared_ptr<DepSpec> >(),
                    n::item() = stack.begin()->item()->append(spec),
                    n::spec() = spec
                ));
    }

    template <typename T_>
    void use_handler(
            typename ParseStackTypes<T_>::Stack & stack,
            const std::string & u,
            const Environment * const,
            const EAPI & eapi,
            bool is_installed)
    {
        std::shared_ptr<ConditionalDepSpec> spec(std::make_shared<ConditionalDepSpec>(parse_elike_conditional_dep_spec(
                        u, is_installed || ! eapi.supported()->package_dep_spec_parse_options()[epdso_missing_use_deps_is_qa])));
        stack.push_front(make_named_values<typename ParseStackTypes<T_>::Item>(
                    n::block_children() = std::list<std::pair<std::shared_ptr<BlockDepSpec>, BlockFixOp> >(),
                    n::children() = std::list<std::shared_ptr<DepSpec> >(),
                    n::item() = stack.begin()->item()->append(spec),
                    n::spec() = spec
                ));
    }

    template <typename T_>
    void pop_handler(
            typename ParseStackTypes<T_>::Stack & stack,
            const typename ParseStackTypes<T_>::AnnotationsGoHere & annotations_go_here,
            const typename ParseStackTypes<T_>::StarAnnotationsGoHere & star_annotations_go_here,
            const std::string & s)
    {
        annotations_go_here(stack.begin()->spec());
        star_annotations_go_here(stack.begin()->children());

        auto children(std::move(stack.begin()->children()));
        auto block_children(std::move(stack.begin()->block_children()));

        stack.pop_front();
        if (stack.empty())
            throw EDepParseError(s, "Too many ')'s");

        stack.begin()->children().insert(stack.begin()->children().end(), children.begin(), children.end());
        stack.begin()->block_children().insert(stack.begin()->block_children().end(), block_children.begin(), block_children.end());
    }

    template <typename T_>
    void should_be_empty_handler(const typename ParseStackTypes<T_>::Stack & stack, const std::string & s)
    {
        if (1 != stack.size())
            throw EDepParseError(s, "Nesting error");
    }

    void use_under_any_handler(const std::string & s, const EAPI & eapi)
    {
        if (eapi.supported()->dependency_spec_tree_parse_options()[dstpo_disallow_any_use])
            throw EDepParseError(s, "use? not allowed under || ( ) in this EAPI");
    }

    void do_nothing()
    {
    }

    void set_thing_to_annotate(std::shared_ptr<DepSpec> & spec, const std::shared_ptr<DepSpec> & s)
    {
        spec = s;
    }

    void set_thing_to_star_annotate(std::list<std::shared_ptr<DepSpec> > & spec_list, const std::list<std::shared_ptr<DepSpec> > & s)
    {
        spec_list = s;
    }

    void add_block_annotations(
            std::shared_ptr<DepSpecAnnotations> & annotations,
            const BlockFixOp & block_fix_op)
    {
        DepSpecAnnotationRole current_role(dsar_none);
        if (annotations)
            current_role = find_blocker_role_in_annotations(annotations);
        if (dsar_none != current_role)
            return;

        switch (block_fix_op)
        {
            case bfo_explicit_strong:
                annotations->add(make_named_values<DepSpecAnnotation>(
                            n::key() = "<resolution>",
                            n::kind() = dsak_synthetic,
                            n::role() = dsar_blocker_strong,
                            n::value() = "<explicit-strong>"
                            ));
                break;

            case bfo_implicit_strong:
                annotations->add(make_named_values<DepSpecAnnotation>(
                            n::key() = "<resolution>",
                            n::kind() = dsak_synthetic,
                            n::role() = dsar_blocker_strong,
                            n::value() = "<implicit-strong>"
                            ));
                break;

            case bfo_implicit_weak:
                annotations->add(make_named_values<DepSpecAnnotation>(
                            n::key() = "<resolution>",
                            n::kind() = dsak_synthetic,
                            n::role() = dsar_blocker_weak,
                            n::value() = "<implicit-weak>"
                            ));
                break;
        }
    }

    void add_synthetic_block_annotations(
            const EAPI & eapi,
            const std::shared_ptr<BlockDepSpec> & block_spec,
            const BlockFixOp & block_fix_op)
    {
        auto annotations(std::make_shared<DepSpecAnnotations>());
        if (block_spec->maybe_annotations())
            std::for_each(block_spec->maybe_annotations()->begin(), block_spec->maybe_annotations()->end(),
                    std::bind(&DepSpecAnnotations::add, annotations, std::placeholders::_1));

        add_block_annotations(annotations, block_fix_op);
        if (eapi.supported()->dependency_spec_tree_parse_options()[dstpo_no_self_block])
            annotations->add(make_named_values<DepSpecAnnotation>(
                        n::key() = "<match_package>",
                        n::kind() = dsak_synthetic,
                        n::role() = dsar_no_self_match,
                        n::value() = "<no_self_block>"
                        ));

        block_spec->set_annotations(annotations);
    }

    void add_expanded_annotation(
            const EAPI &,
            const std::shared_ptr<DepSpec> & spec,
            const DepSpecAnnotation & expandable)
    {
        auto annotations(std::make_shared<DepSpecAnnotations>());
        if (spec->maybe_annotations())
            std::for_each(spec->maybe_annotations()->begin(), spec->maybe_annotations()->end(),
                    std::bind(&DepSpecAnnotations::add, annotations, std::placeholders::_1));

        annotations->add(make_named_values<DepSpecAnnotation>(
                    n::key() = expandable.key().substr(1),
                    n::kind() = dsak_expanded,
                    n::role() = expandable.role(),
                    n::value() = expandable.value()));

        spec->set_annotations(annotations);
    }

    void apply_annotations(
            const EAPI & eapi,
            const std::shared_ptr<DepSpec> & spec,
            const std::list<std::shared_ptr<DepSpec> > & children,
            const std::shared_ptr<const Map<std::string, std::string> > & m)
    {
        spec->set_annotations(parse_annotations(eapi, m));

        for (auto a(spec->maybe_annotations()->begin()), a_end(spec->maybe_annotations()->end()) ;
                a != a_end ; ++a)
            switch (a->kind())
            {
                case dsak_synthetic:
                case dsak_literal:
                case dsak_expanded:
                    continue;

                case dsak_expandable:
                    for (auto c(children.begin()), c_end(children.end()) ;
                            c != c_end ; ++c)
                        add_expanded_annotation(eapi, *c, *a);
                    continue;

                case last_dsak:
                    break;
            }
    }
}

std::shared_ptr<DependencySpecTree>
paludis::erepository::parse_depend(const std::string & s, const Environment * const env, const EAPI & eapi, const bool is_installed)
{
    using namespace std::placeholders;

    ParseStackTypes<DependencySpecTree>::Stack stack;
    std::shared_ptr<AllDepSpec> spec(std::make_shared<AllDepSpec>());
    std::shared_ptr<DepSpec> thing_to_annotate(spec);
    std::list<std::shared_ptr<DepSpec> > thing_to_star_annotate;
    std::shared_ptr<DependencySpecTree> top(std::make_shared<DependencySpecTree>(spec));
    stack.push_front(make_named_values<ParseStackTypes<DependencySpecTree>::Item>(
                n::block_children() = std::list<std::pair<std::shared_ptr<BlockDepSpec>, BlockFixOp> >(),
                n::children() = std::list<std::shared_ptr<DepSpec> >(),
                n::item() = top->top(),
                n::spec() = spec
            ));

    ELikeDepParserCallbacks callbacks(
            make_named_values<ELikeDepParserCallbacks>(
                n::on_all() = std::bind(&any_all_handler<DependencySpecTree, AllDepSpec>, std::ref(stack)),
                n::on_annotations() = std::bind(&apply_annotations, std::cref(eapi), std::ref(thing_to_annotate), std::cref(thing_to_star_annotate), _1),
                n::on_any() = std::bind(&any_all_handler<DependencySpecTree, AnyDepSpec>, std::ref(stack)),
                n::on_arrow() = std::bind(&arrows_not_allowed_handler, s, _1, _2),
                n::on_at_most_one() = std::bind(&at_most_one_not_allowed_handler, s),
                n::on_error() = std::bind(&error_handler, s, _1),
                n::on_exactly_one() = std::bind(&exactly_one_not_allowed_handler, s),
                n::on_label() = std::bind(&dependency_label_handler<DependencySpecTree>, env,
                    std::ref(stack),
                    ParseStackTypes<DependencySpecTree>::AnnotationsGoHere(std::bind(
                            &set_thing_to_annotate, std::ref(thing_to_annotate), _1)),
                    _1, std::cref(eapi)),
                n::on_no_annotations() = &do_nothing,
                n::on_pop() = std::bind(&pop_handler<DependencySpecTree>, std::ref(stack),
                    ParseStackTypes<DependencySpecTree>::AnnotationsGoHere(std::bind(
                            &set_thing_to_annotate, std::ref(thing_to_annotate), _1)),
                    ParseStackTypes<DependencySpecTree>::StarAnnotationsGoHere(std::bind(
                            &set_thing_to_star_annotate, std::ref(thing_to_star_annotate), _1)),
                    s),
                n::on_should_be_empty() = std::bind(&should_be_empty_handler<DependencySpecTree>, std::ref(stack), s),
                n::on_string() = std::bind(&package_or_block_dep_spec_string_handler<DependencySpecTree>, std::ref(stack),
                    ParseStackTypes<DependencySpecTree>::AnnotationsGoHere(std::bind(
                            &set_thing_to_annotate, std::ref(thing_to_annotate), _1)), _1, eapi),
                n::on_use() = std::bind(&use_handler<DependencySpecTree>, std::ref(stack), _1, env, std::cref(eapi), is_installed),
                n::on_use_under_any() = std::bind(&use_under_any_handler, s, std::cref(eapi))
            ));

    parse_elike_dependencies(s, callbacks, { });

    for (auto b(stack.begin()->block_children().begin()), b_end(stack.begin()->block_children().end()) ;
            b != b_end ; ++b)
        add_synthetic_block_annotations(eapi, b->first, b->second);

    return top;
}

std::shared_ptr<SetSpecTree>
paludis::erepository::parse_commented_set(const std::string & s, const Environment * const, const EAPI & eapi)
{
    using namespace std::placeholders;

    ParseStackTypes<SetSpecTree>::Stack stack;
    std::shared_ptr<AllDepSpec> spec(std::make_shared<AllDepSpec>());
    std::shared_ptr<DepSpec> thing_to_annotate(spec);
    std::list<std::shared_ptr<DepSpec> > thing_to_star_annotate;
    std::shared_ptr<SetSpecTree> top(std::make_shared<SetSpecTree>(spec));
    stack.push_front(make_named_values<ParseStackTypes<SetSpecTree>::Item>(
                n::block_children() = std::list<std::pair<std::shared_ptr<BlockDepSpec>, BlockFixOp> >(),
                n::children() = std::list<std::shared_ptr<DepSpec> >(),
                n::item() = top->top(),
                n::spec() = spec
            ));

    ELikeDepParserCallbacks callbacks(
            make_named_values<ELikeDepParserCallbacks>(
                n::on_all() = std::bind(&any_all_handler<SetSpecTree, AllDepSpec>, std::ref(stack)),
                n::on_annotations() = std::bind(&apply_annotations, std::cref(eapi), std::ref(thing_to_annotate), std::cref(thing_to_star_annotate), _1),
                n::on_any() = std::bind(&any_not_allowed_handler, s),
                n::on_arrow() = std::bind(&arrows_not_allowed_handler, s, _1, _2),
                n::on_at_most_one() = std::bind(&at_most_one_not_allowed_handler, s),
                n::on_error() = std::bind(&error_handler, s, _1),
                n::on_exactly_one() = std::bind(&exactly_one_not_allowed_handler, s),
                n::on_label() = std::bind(&labels_not_allowed_handler, s, _1),
                n::on_no_annotations() = &do_nothing,
                n::on_pop() = std::bind(&pop_handler<SetSpecTree>, std::ref(stack),
                    ParseStackTypes<SetSpecTree>::AnnotationsGoHere(std::bind(
                            &set_thing_to_annotate, std::ref(thing_to_annotate), _1)),
                    ParseStackTypes<SetSpecTree>::StarAnnotationsGoHere(std::bind(
                            &set_thing_to_star_annotate, std::ref(thing_to_star_annotate), _1)),
                    s),
                n::on_should_be_empty() = std::bind(&should_be_empty_handler<SetSpecTree>, std::ref(stack), s),
                n::on_string() = std::bind(&package_dep_spec_string_handler<SetSpecTree>, std::ref(stack),
                    ParseStackTypes<SetSpecTree>::AnnotationsGoHere(std::bind(
                            &set_thing_to_annotate, std::ref(thing_to_annotate), _1)), _1, eapi, false),
                n::on_use() = std::bind(&use_not_allowed_handler, s, _1),
                n::on_use_under_any() = std::bind(&use_under_any_handler, s, std::cref(eapi))
            ));

    parse_elike_dependencies(s, callbacks, { edpo_allow_embedded_comments });

    for (auto b(stack.begin()->block_children().begin()), b_end(stack.begin()->block_children().end()) ;
            b != b_end ; ++b)
        add_synthetic_block_annotations(eapi, b->first, b->second);

    return top;
}

std::shared_ptr<FetchableURISpecTree>
paludis::erepository::parse_fetchable_uri(const std::string & s, const Environment * const env, const EAPI & eapi, const bool is_installed)
{
    using namespace std::placeholders;

    ParseStackTypes<FetchableURISpecTree>::Stack stack;
    std::shared_ptr<AllDepSpec> spec(std::make_shared<AllDepSpec>());
    std::shared_ptr<DepSpec> thing_to_annotate(spec);
    std::list<std::shared_ptr<DepSpec> > thing_to_star_annotate;
    std::shared_ptr<FetchableURISpecTree> top(std::make_shared<FetchableURISpecTree>(spec));
    stack.push_front(make_named_values<ParseStackTypes<FetchableURISpecTree>::Item>(
                n::block_children() = std::list<std::pair<std::shared_ptr<BlockDepSpec>, BlockFixOp> >(),
                n::children() = std::list<std::shared_ptr<DepSpec> >(),
                n::item() = top->top(),
                n::spec() = spec
            ));

    ELikeDepParserCallbacks callbacks(
            make_named_values<ELikeDepParserCallbacks>(
                n::on_all() = std::bind(&any_all_handler<FetchableURISpecTree, AllDepSpec>, std::ref(stack)),
                n::on_annotations() = std::bind(&apply_annotations, std::cref(eapi), std::ref(thing_to_annotate), std::cref(thing_to_star_annotate), _1),
                n::on_any() = std::bind(&any_not_allowed_handler, s),
                n::on_arrow() = std::bind(&arrow_handler<FetchableURISpecTree>, std::ref(stack),
                    ParseStackTypes<FetchableURISpecTree>::AnnotationsGoHere(std::bind(
                            &set_thing_to_annotate, std::ref(thing_to_annotate), _1)), s, _1, _2, std::cref(eapi)),
                n::on_at_most_one() = std::bind(&at_most_one_not_allowed_handler, s),
                n::on_error() = std::bind(&error_handler, s, _1),
                n::on_exactly_one() = std::bind(&exactly_one_not_allowed_handler, s),
                n::on_label() = std::bind(&fetchable_label_handler<FetchableURISpecTree>, std::ref(stack),
                    ParseStackTypes<FetchableURISpecTree>::AnnotationsGoHere(std::bind(
                            &set_thing_to_annotate, std::ref(thing_to_annotate), _1)), _1, std::cref(eapi)),
                n::on_no_annotations() = &do_nothing,
                n::on_pop() = std::bind(&pop_handler<FetchableURISpecTree>, std::ref(stack),
                    ParseStackTypes<FetchableURISpecTree>::AnnotationsGoHere(std::bind(
                            &set_thing_to_annotate, std::ref(thing_to_annotate), _1)),
                    ParseStackTypes<FetchableURISpecTree>::StarAnnotationsGoHere(std::bind(
                            &set_thing_to_star_annotate, std::ref(thing_to_star_annotate), _1)),
                    s),
                n::on_should_be_empty() = std::bind(&should_be_empty_handler<FetchableURISpecTree>, std::ref(stack), s),
                n::on_string() = std::bind(&arrow_handler<FetchableURISpecTree>, std::ref(stack),
                    ParseStackTypes<FetchableURISpecTree>::AnnotationsGoHere(std::bind(
                            &set_thing_to_annotate, std::ref(thing_to_annotate), _1)), s, _1, "", std::cref(eapi)),
                n::on_use() = std::bind(&use_handler<FetchableURISpecTree>, std::ref(stack), _1, env, std::cref(eapi), is_installed),
                n::on_use_under_any() = std::bind(&use_under_any_handler, s, std::cref(eapi))
            ));

    parse_elike_dependencies(s, callbacks, { });

    return top;
}

std::shared_ptr<SimpleURISpecTree>
paludis::erepository::parse_simple_uri(const std::string & s, const Environment * const env, const EAPI & eapi, const bool is_installed)
{
    using namespace std::placeholders;

    ParseStackTypes<SimpleURISpecTree>::Stack stack;
    std::shared_ptr<AllDepSpec> spec(std::make_shared<AllDepSpec>());
    std::shared_ptr<DepSpec> thing_to_annotate(spec);
    std::list<std::shared_ptr<DepSpec> > thing_to_star_annotate;
    std::shared_ptr<SimpleURISpecTree> top(std::make_shared<SimpleURISpecTree>(spec));
    stack.push_front(make_named_values<ParseStackTypes<SimpleURISpecTree>::Item>(
                n::block_children() = std::list<std::pair<std::shared_ptr<BlockDepSpec>, BlockFixOp> >(),
                n::children() = std::list<std::shared_ptr<DepSpec> >(),
                n::item() = top->top(),
                n::spec() = spec
            ));

    ELikeDepParserCallbacks callbacks(
            make_named_values<ELikeDepParserCallbacks>(
                n::on_all() = std::bind(&any_all_handler<SimpleURISpecTree, AllDepSpec>, std::ref(stack)),
                n::on_annotations() = std::bind(&apply_annotations, std::cref(eapi), std::ref(thing_to_annotate), std::cref(thing_to_star_annotate), _1),
                n::on_any() = std::bind(&any_not_allowed_handler, s),
                n::on_arrow() = std::bind(&arrows_not_allowed_handler, s, _1, _2),
                n::on_at_most_one() = std::bind(&at_most_one_not_allowed_handler, s),
                n::on_error() = std::bind(&error_handler, s, _1),
                n::on_exactly_one() = std::bind(&exactly_one_not_allowed_handler, s),
                n::on_label() = std::bind(&labels_not_allowed_handler, s, _1),
                n::on_no_annotations() = &do_nothing,
                n::on_pop() = std::bind(&pop_handler<SimpleURISpecTree>, std::ref(stack),
                    ParseStackTypes<SimpleURISpecTree>::AnnotationsGoHere(std::bind(
                            &set_thing_to_annotate, std::ref(thing_to_annotate), _1)),
                    ParseStackTypes<SimpleURISpecTree>::StarAnnotationsGoHere(std::bind(
                            &set_thing_to_star_annotate, std::ref(thing_to_star_annotate), _1)),
                    s),
                n::on_should_be_empty() = std::bind(&should_be_empty_handler<SimpleURISpecTree>, std::ref(stack), s),
                n::on_string() = std::bind(&simple_uri_handler<SimpleURISpecTree>, std::ref(stack),
                    ParseStackTypes<SimpleURISpecTree>::AnnotationsGoHere(std::bind(
                            &set_thing_to_annotate, std::ref(thing_to_annotate), _1)), _1),
                n::on_use() = std::bind(&use_handler<SimpleURISpecTree>, std::ref(stack), _1, env, std::cref(eapi), is_installed),
                n::on_use_under_any() = &do_nothing
            ));

    parse_elike_dependencies(s, callbacks, { });

    return top;
}

std::shared_ptr<LicenseSpecTree>
paludis::erepository::parse_license(const std::string & s, const Environment * const env, const EAPI & eapi, const bool is_installed)
{
    using namespace std::placeholders;

    ParseStackTypes<LicenseSpecTree>::Stack stack;
    std::shared_ptr<AllDepSpec> spec(std::make_shared<AllDepSpec>());
    std::shared_ptr<DepSpec> thing_to_annotate(spec);
    std::list<std::shared_ptr<DepSpec> > thing_to_star_annotate;
    std::shared_ptr<LicenseSpecTree> top(std::make_shared<LicenseSpecTree>(spec));
    stack.push_front(make_named_values<ParseStackTypes<LicenseSpecTree>::Item>(
                n::block_children() = std::list<std::pair<std::shared_ptr<BlockDepSpec>, BlockFixOp> >(),
                n::children() = std::list<std::shared_ptr<DepSpec> >(),
                n::item() = top->top(),
                n::spec() = spec
            ));

    ELikeDepParserCallbacks callbacks(
            make_named_values<ELikeDepParserCallbacks>(
                n::on_all() = std::bind(&any_all_handler<LicenseSpecTree, AllDepSpec>, std::ref(stack)),
                n::on_annotations() = std::bind(&apply_annotations, std::cref(eapi), std::ref(thing_to_annotate), std::cref(thing_to_star_annotate), _1),
                n::on_any() = std::bind(&any_all_handler<LicenseSpecTree, AnyDepSpec>, std::ref(stack)),
                n::on_arrow() = std::bind(&arrows_not_allowed_handler, s, _1, _2),
                n::on_at_most_one() = std::bind(&at_most_one_not_allowed_handler, s),
                n::on_error() = std::bind(&error_handler, s, _1),
                n::on_exactly_one() = std::bind(&exactly_one_not_allowed_handler, s),
                n::on_label() = std::bind(&labels_not_allowed_handler, s, _1),
                n::on_no_annotations() = &do_nothing,
                n::on_pop() = std::bind(&pop_handler<LicenseSpecTree>, std::ref(stack),
                    ParseStackTypes<LicenseSpecTree>::AnnotationsGoHere(std::bind(
                            &set_thing_to_annotate, std::ref(thing_to_annotate), _1)),
                    ParseStackTypes<LicenseSpecTree>::StarAnnotationsGoHere(std::bind(
                            &set_thing_to_star_annotate, std::ref(thing_to_star_annotate), _1)),
                    s),
                n::on_should_be_empty() = std::bind(&should_be_empty_handler<LicenseSpecTree>, std::ref(stack), s),
                n::on_string() = std::bind(&license_handler<LicenseSpecTree>, std::ref(stack),
                    ParseStackTypes<LicenseSpecTree>::AnnotationsGoHere(std::bind(
                            &set_thing_to_annotate, std::ref(thing_to_annotate), _1)), _1),
                n::on_use() = std::bind(&use_handler<LicenseSpecTree>, std::ref(stack), _1, env, std::cref(eapi), is_installed),
                n::on_use_under_any() = std::bind(&use_under_any_handler, s, std::cref(eapi))
            ));

    parse_elike_dependencies(s, callbacks, { });

    return top;
}

std::shared_ptr<PlainTextSpecTree>
paludis::erepository::parse_plain_text(const std::string & s, const Environment * const env, const EAPI & eapi, const bool is_installed)
{
    using namespace std::placeholders;

    ParseStackTypes<PlainTextSpecTree>::Stack stack;
    std::shared_ptr<AllDepSpec> spec(std::make_shared<AllDepSpec>());
    std::shared_ptr<DepSpec> thing_to_annotate(spec);
    std::list<std::shared_ptr<DepSpec> > thing_to_star_annotate;
    std::shared_ptr<PlainTextSpecTree> top(std::make_shared<PlainTextSpecTree>(spec));
    stack.push_front(make_named_values<ParseStackTypes<PlainTextSpecTree>::Item>(
                n::block_children() = std::list<std::pair<std::shared_ptr<BlockDepSpec>, BlockFixOp> >(),
                n::children() = std::list<std::shared_ptr<DepSpec> >(),
                n::item() = top->top(),
                n::spec() = spec
            ));

    ELikeDepParserCallbacks callbacks(
            make_named_values<ELikeDepParserCallbacks>(
                n::on_all() = std::bind(&any_all_handler<PlainTextSpecTree, AllDepSpec>, std::ref(stack)),
                n::on_annotations() = std::bind(&apply_annotations, std::cref(eapi), std::ref(thing_to_annotate), std::cref(thing_to_star_annotate), _1),
                n::on_any() = std::bind(&any_not_allowed_handler, s),
                n::on_arrow() = std::bind(&arrows_not_allowed_handler, s, _1, _2),
                n::on_at_most_one() = std::bind(&at_most_one_not_allowed_handler, s),
                n::on_error() = std::bind(&error_handler, s, _1),
                n::on_exactly_one() = std::bind(&exactly_one_not_allowed_handler, s),
                n::on_label() = std::bind(&labels_not_allowed_handler, s, _1),
                n::on_no_annotations() = &do_nothing,
                n::on_pop() = std::bind(&pop_handler<PlainTextSpecTree>, std::ref(stack),
                    ParseStackTypes<PlainTextSpecTree>::AnnotationsGoHere(std::bind(
                            &set_thing_to_annotate, std::ref(thing_to_annotate), _1)),
                    ParseStackTypes<PlainTextSpecTree>::StarAnnotationsGoHere(std::bind(
                            &set_thing_to_star_annotate, std::ref(thing_to_star_annotate), _1)),
                    s),
                n::on_should_be_empty() = std::bind(&should_be_empty_handler<PlainTextSpecTree>, std::ref(stack), s),
                n::on_string() = std::bind(&plain_text_handler<PlainTextSpecTree>, std::ref(stack),
                        ParseStackTypes<PlainTextSpecTree>::AnnotationsGoHere(std::bind(
                                &set_thing_to_annotate, std::ref(thing_to_annotate), _1)), _1),
                n::on_use() = std::bind(&use_handler<PlainTextSpecTree>, std::ref(stack), _1, env, std::cref(eapi), is_installed),
                n::on_use_under_any() = &do_nothing
            ));

    parse_elike_dependencies(s, callbacks, { });

    return top;
}

std::shared_ptr<PlainTextSpecTree>
paludis::erepository::parse_myoptions(const std::string & s, const Environment * const env, const EAPI & eapi, const bool is_installed)
{
    using namespace std::placeholders;

    ParseStackTypes<PlainTextSpecTree>::Stack stack;
    std::shared_ptr<AllDepSpec> spec(std::make_shared<AllDepSpec>());
    std::shared_ptr<DepSpec> thing_to_annotate(spec);
    std::list<std::shared_ptr<DepSpec> > thing_to_star_annotate;
    std::shared_ptr<PlainTextSpecTree> top(std::make_shared<PlainTextSpecTree>(spec));
    stack.push_front(make_named_values<ParseStackTypes<PlainTextSpecTree>::Item>(
                n::block_children() = std::list<std::pair<std::shared_ptr<BlockDepSpec>, BlockFixOp> >(),
                n::children() = std::list<std::shared_ptr<DepSpec> >(),
                n::item() = top->top(),
                n::spec() = spec
            ));

    ELikeDepParserCallbacks callbacks(
            make_named_values<ELikeDepParserCallbacks>(
                n::on_all() = std::bind(&any_all_handler<PlainTextSpecTree, AllDepSpec>, std::ref(stack)),
                n::on_annotations() = std::bind(&apply_annotations, std::cref(eapi), std::ref(thing_to_annotate), std::cref(thing_to_star_annotate), _1),
                n::on_any() = std::bind(&any_not_allowed_handler, s),
                n::on_arrow() = std::bind(&arrows_not_allowed_handler, s, _1, _2),
                n::on_at_most_one() = std::bind(&at_most_one_not_allowed_handler, s),
                n::on_error() = std::bind(&error_handler, s, _1),
                n::on_exactly_one() = std::bind(&exactly_one_not_allowed_handler, s),
                n::on_label() = std::bind(&plain_text_label_handler<PlainTextSpecTree>, std::ref(stack),
                        ParseStackTypes<PlainTextSpecTree>::AnnotationsGoHere(std::bind(
                                &set_thing_to_annotate, std::ref(thing_to_annotate), _1)), _1),
                n::on_no_annotations() = &do_nothing,
                n::on_pop() = std::bind(&pop_handler<PlainTextSpecTree>, std::ref(stack),
                    ParseStackTypes<PlainTextSpecTree>::AnnotationsGoHere(std::bind(
                            &set_thing_to_annotate, std::ref(thing_to_annotate), _1)),
                    ParseStackTypes<PlainTextSpecTree>::StarAnnotationsGoHere(std::bind(
                            &set_thing_to_star_annotate, std::ref(thing_to_star_annotate), _1)),
                    s),
                n::on_should_be_empty() = std::bind(&should_be_empty_handler<PlainTextSpecTree>, std::ref(stack), s),
                n::on_string() = std::bind(&plain_text_handler<PlainTextSpecTree>, std::ref(stack),
                        ParseStackTypes<PlainTextSpecTree>::AnnotationsGoHere(std::bind(
                                &set_thing_to_annotate, std::ref(thing_to_annotate), _1)), _1),
                n::on_use() = std::bind(&use_handler<PlainTextSpecTree>, std::ref(stack), _1, env, std::cref(eapi), is_installed),
                n::on_use_under_any() = &do_nothing
            ));

    parse_elike_dependencies(s, callbacks, { });

    return top;
}

std::shared_ptr<RequiredUseSpecTree>
paludis::erepository::parse_required_use(const std::string & s, const Environment * const env, const EAPI & eapi, const bool is_installed)
{
    using namespace std::placeholders;

    ParseStackTypes<RequiredUseSpecTree>::Stack stack;
    std::shared_ptr<AllDepSpec> spec(std::make_shared<AllDepSpec>());
    std::shared_ptr<DepSpec> thing_to_annotate(spec);
    std::list<std::shared_ptr<DepSpec> > thing_to_star_annotate;
    std::shared_ptr<RequiredUseSpecTree> top(std::make_shared<RequiredUseSpecTree>(spec));
    stack.push_front(make_named_values<ParseStackTypes<RequiredUseSpecTree>::Item>(
                n::block_children() = std::list<std::pair<std::shared_ptr<BlockDepSpec>, BlockFixOp> >(),
                n::children() = std::list<std::shared_ptr<DepSpec> >(),
                n::item() = top->top(),
                n::spec() = spec
            ));

    ELikeDepParserCallbacks callbacks(
            make_named_values<ELikeDepParserCallbacks>(
                n::on_all() = std::bind(&any_all_handler<RequiredUseSpecTree, AllDepSpec>, std::ref(stack)),
                n::on_annotations() = std::bind(&apply_annotations, std::cref(eapi), std::ref(thing_to_annotate), std::cref(thing_to_star_annotate), _1),
                n::on_any() = std::bind(&any_all_handler<RequiredUseSpecTree, AnyDepSpec>, std::ref(stack)),
                n::on_arrow() = std::bind(&arrows_not_allowed_handler, s, _1, _2),
                n::on_at_most_one() =
                    eapi.supported()->dependency_spec_tree_parse_options()[dstpo_allow_at_most_one] ?
                    ELikeDepParserAtMostOneFunction(std::bind(&any_all_handler<RequiredUseSpecTree, AtMostOneDepSpec>, std::ref(stack))) :
                    ELikeDepParserAtMostOneFunction(std::bind(&at_most_one_not_allowed_handler, s)),
                n::on_error() = std::bind(&error_handler, s, _1),
                n::on_exactly_one() = std::bind(&any_all_handler<RequiredUseSpecTree, ExactlyOneDepSpec>, std::ref(stack)),
                n::on_label() = std::bind(&labels_not_allowed_handler, s, _1),
                n::on_no_annotations() = &do_nothing,
                n::on_pop() = std::bind(&pop_handler<RequiredUseSpecTree>, std::ref(stack),
                    ParseStackTypes<RequiredUseSpecTree>::AnnotationsGoHere(std::bind(
                            &set_thing_to_annotate, std::ref(thing_to_annotate), _1)),
                    ParseStackTypes<RequiredUseSpecTree>::StarAnnotationsGoHere(std::bind(
                            &set_thing_to_star_annotate, std::ref(thing_to_star_annotate), _1)),
                    s),
                n::on_should_be_empty() = std::bind(&should_be_empty_handler<RequiredUseSpecTree>, std::ref(stack), s),
                n::on_string() = std::bind(&plain_text_handler<RequiredUseSpecTree>, std::ref(stack),
                        ParseStackTypes<RequiredUseSpecTree>::AnnotationsGoHere(std::bind(
                                &set_thing_to_annotate, std::ref(thing_to_annotate), _1)), _1),
                n::on_use() = std::bind(&use_handler<RequiredUseSpecTree>, std::ref(stack), _1, env, std::cref(eapi), is_installed),
                n::on_use_under_any() = &do_nothing
            ));

    parse_elike_dependencies(s, callbacks, { });

    return top;
}

