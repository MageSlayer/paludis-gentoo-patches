/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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
#include <paludis/repositories/e/eapi.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/options.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/map.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/return_literal_function.hh>
#include <paludis/util/log.hh>
#include <paludis/elike_annotations.hh>
#include <paludis/elike_dep_parser.hh>
#include <paludis/elike_conditional_dep_spec.hh>
#include <paludis/elike_package_dep_spec.hh>
#include <paludis/dep_spec.hh>
#include <paludis/environment.hh>
#include <paludis/repository.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/action.hh>
#include <paludis/choice.hh>
#include <list>
#include <set>
#include <ostream>

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
        typedef Name<struct item_name> item;
        typedef Name<struct spec_name> spec;
    }
}

namespace
{
    template <typename T_>
    struct ParseStackTypes
    {
        struct Item
        {
            NamedValue<n::item, std::shared_ptr<typename T_::BasicInnerNode> > item;
            NamedValue<n::spec, std::shared_ptr<DepSpec> > spec;
        };

        typedef std::list<Item> Stack;
        typedef std::function<void (const std::shared_ptr<DepSpec> &)> AnnotationsGoHere;
    };

    template <typename T_>
    void package_dep_spec_string_handler(
            const typename ParseStackTypes<T_>::Stack & h,
            const typename ParseStackTypes<T_>::AnnotationsGoHere & annotations_go_here,
            const std::string & s,
            const EAPI & eapi,
            const std::shared_ptr<const PackageID> & id)
    {
        std::shared_ptr<PackageDepSpec> spec(std::make_shared<PackageDepSpec>(
                    parse_elike_package_dep_spec(s, eapi.supported()->package_dep_spec_parse_options(),
                        eapi.supported()->version_spec_options(), id)));
        h.begin()->item()->append(spec);
        annotations_go_here(spec);
    }

    template <typename T_>
    void package_or_block_dep_spec_string_handler(
            const typename ParseStackTypes<T_>::Stack & h,
            const typename ParseStackTypes<T_>::AnnotationsGoHere & annotations_go_here,
            const std::string & s,
            const EAPI & eapi,
            const std::shared_ptr<const PackageID> & id)
    {
        if ((! s.empty()) && ('!' == s.at(0)))
        {
            bool strong(false);
            std::string::size_type specstart(1);
            if (2 <= s.length() && '!' == s.at(1))
            {
                if (! eapi.supported()->dependency_spec_tree_parse_options()[dstpo_double_bang_blocks])
                    throw EDepParseError(s, "Double-! blocks not allowed in this EAPI");
                specstart = 2;
                strong = true;
            }
            else
            {
                if (eapi.supported()->dependency_spec_tree_parse_options()[dstpo_single_bang_block_is_hard])
                    strong = true;
            }

            std::shared_ptr<BlockDepSpec> spec(std::make_shared<BlockDepSpec>(
                        s,
                        parse_elike_package_dep_spec(s.substr(specstart),
                            eapi.supported()->package_dep_spec_parse_options(),
                            eapi.supported()->version_spec_options(),
                            id),
                        strong));
            h.begin()->item()->append(spec);
            annotations_go_here(spec);
        }
        else
            package_dep_spec_string_handler<T_>(h, annotations_go_here, s, eapi, id);
    }

    template <typename T_>
    void license_handler(
            const typename ParseStackTypes<T_>::Stack & h,
            const typename ParseStackTypes<T_>::AnnotationsGoHere & annotations_go_here,
            const std::string & s)
    {
        std::shared_ptr<LicenseDepSpec> spec(std::make_shared<LicenseDepSpec>(s));
        h.begin()->item()->append(spec);
        annotations_go_here(spec);
    }

    template <typename T_>
    void plain_text_handler(
            const typename ParseStackTypes<T_>::Stack & h,
            const typename ParseStackTypes<T_>::AnnotationsGoHere & annotations_go_here,
            const std::string & s)
    {
        std::shared_ptr<PlainTextDepSpec> spec(std::make_shared<PlainTextDepSpec>(s));
        h.begin()->item()->append(spec);
        annotations_go_here(spec);
    }

    template <typename T_>
    void simple_uri_handler(
            const typename ParseStackTypes<T_>::Stack & h,
            const typename ParseStackTypes<T_>::AnnotationsGoHere & annotations_go_here,
            const std::string & s)
    {
        std::shared_ptr<SimpleURIDepSpec> spec(std::make_shared<SimpleURIDepSpec>(s));
        h.begin()->item()->append(spec);
        annotations_go_here(spec);
    }

    template <typename T_>
    void arrow_handler(
            const typename ParseStackTypes<T_>::Stack & h,
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

    template <typename T_>
    void dependency_label_handler(
            const typename ParseStackTypes<T_>::Stack & h,
            const typename ParseStackTypes<T_>::AnnotationsGoHere & annotations_go_here,
            const std::shared_ptr<const PackageID> & id,
            const std::string & s,
            const EAPI & eapi)
    {
        std::shared_ptr<DependenciesLabelsDepSpec> spec(parse_dependency_label(id, s, eapi));
        h.begin()->item()->append(spec);
        annotations_go_here(spec);
    }

    template <typename T_>
    void fetchable_label_handler(
            const typename ParseStackTypes<T_>::Stack & h,
            const typename ParseStackTypes<T_>::AnnotationsGoHere & annotations_go_here,
            const std::string & s,
            const EAPI & eapi)
    {
        std::shared_ptr<URILabelsDepSpec> spec(parse_uri_label(s, eapi));
        h.begin()->item()->append(spec);
        annotations_go_here(spec);
    }

    template <typename T_>
    void plain_text_label_handler(
            const typename ParseStackTypes<T_>::Stack & h,
            const typename ParseStackTypes<T_>::AnnotationsGoHere & annotations_go_here,
            const std::string & s)
    {
        std::shared_ptr<PlainTextLabelDepSpec> spec(parse_plain_text_label(s));
        h.begin()->item()->append(spec);
        annotations_go_here(spec);
    }

    template <typename T_, typename A_>
    void any_all_handler(typename ParseStackTypes<T_>::Stack & stack)
    {
        std::shared_ptr<A_> spec(std::make_shared<A_>());
        stack.push_front(make_named_values<typename ParseStackTypes<T_>::Item>(
                    n::item() = stack.begin()->item()->append(spec),
                    n::spec() = spec
                ));
    }

    template <typename T_>
    void use_handler(
            typename ParseStackTypes<T_>::Stack & stack,
            const std::string & u,
            const Environment * const env,
            const std::shared_ptr<const PackageID> & id)
    {
        std::shared_ptr<ConditionalDepSpec> spec(std::make_shared<ConditionalDepSpec>(parse_elike_conditional_dep_spec(
                        u, env, id, bool(id->repository()->installed_root_key()))));
        stack.push_front(make_named_values<typename ParseStackTypes<T_>::Item>(
                    n::item() = stack.begin()->item()->append(spec),
                    n::spec() = spec
                ));
    }

    template <typename T_>
    void pop_handler(
            typename ParseStackTypes<T_>::Stack & stack,
            const typename ParseStackTypes<T_>::AnnotationsGoHere & annotations_go_here,
            const std::string & s)
    {
        annotations_go_here(stack.begin()->spec());
        stack.pop_front();
        if (stack.empty())
            throw EDepParseError(s, "Too many ')'s");
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

    void set_annotations(std::shared_ptr<DepSpec> & spec, const std::shared_ptr<const Map<std::string, std::string> > & m)
    {
        std::shared_ptr<ELikeAnnotations> key(std::make_shared<ELikeAnnotations>(m));
        spec->set_annotations_key(key);
    }

    void set_thing_to_annotate(std::shared_ptr<DepSpec> & spec, const std::shared_ptr<DepSpec> & s)
    {
        spec = s;
    }
}

std::shared_ptr<DependencySpecTree>
paludis::erepository::parse_depend(const std::string & s,
        const Environment * const env, const std::shared_ptr<const PackageID> & id, const EAPI & eapi)
{
    using namespace std::placeholders;

    ParseStackTypes<DependencySpecTree>::Stack stack;
    std::shared_ptr<AllDepSpec> spec(std::make_shared<AllDepSpec>());
    std::shared_ptr<DepSpec> thing_to_annotate(spec);
    std::shared_ptr<DependencySpecTree> top(std::make_shared<DependencySpecTree>(spec));
    stack.push_front(make_named_values<ParseStackTypes<DependencySpecTree>::Item>(
                n::item() = top->root(),
                n::spec() = spec
            ));

    ELikeDepParserCallbacks callbacks(
            make_named_values<ELikeDepParserCallbacks>(
                n::on_all() = std::bind(&any_all_handler<DependencySpecTree, AllDepSpec>, std::ref(stack)),
                n::on_annotations() = std::bind(&set_annotations, std::ref(thing_to_annotate), _1),
                n::on_any() = std::bind(&any_all_handler<DependencySpecTree, AnyDepSpec>, std::ref(stack)),
                n::on_arrow() = std::bind(&arrows_not_allowed_handler, s, _1, _2),
                n::on_error() = std::bind(&error_handler, s, _1),
                n::on_label() = std::bind(&dependency_label_handler<DependencySpecTree>,
                    std::ref(stack),
                    ParseStackTypes<DependencySpecTree>::AnnotationsGoHere(std::bind(
                            &set_thing_to_annotate, std::ref(thing_to_annotate), _1)),
                    id, _1, eapi),
                n::on_pop() = std::bind(&pop_handler<DependencySpecTree>, std::ref(stack),
                    ParseStackTypes<DependencySpecTree>::AnnotationsGoHere(std::bind(
                            &set_thing_to_annotate, std::ref(thing_to_annotate), _1)), s),
                n::on_should_be_empty() = std::bind(&should_be_empty_handler<DependencySpecTree>, std::ref(stack), s),
                n::on_string() = std::bind(&package_or_block_dep_spec_string_handler<DependencySpecTree>, std::ref(stack),
                    ParseStackTypes<DependencySpecTree>::AnnotationsGoHere(std::bind(
                            &set_thing_to_annotate, std::ref(thing_to_annotate), _1)), _1, eapi, id),
                n::on_use() = std::bind(&use_handler<DependencySpecTree>, std::ref(stack), _1, env, id),
                n::on_use_under_any() = std::bind(&use_under_any_handler, s, eapi)
            ));

    parse_elike_dependencies(s, callbacks);

    return top;
}

std::shared_ptr<ProvideSpecTree>
paludis::erepository::parse_provide(const std::string & s,
        const Environment * const env, const std::shared_ptr<const PackageID> & id, const EAPI & eapi)
{
    using namespace std::placeholders;

    ParseStackTypes<ProvideSpecTree>::Stack stack;
    std::shared_ptr<AllDepSpec> spec(std::make_shared<AllDepSpec>());
    std::shared_ptr<DepSpec> thing_to_annotate(spec);
    std::shared_ptr<ProvideSpecTree> top(std::make_shared<ProvideSpecTree>(spec));
    stack.push_front(make_named_values<ParseStackTypes<ProvideSpecTree>::Item>(
                n::item() = top->root(),
                n::spec() = spec
            ));

    ELikeDepParserCallbacks callbacks(
            make_named_values<ELikeDepParserCallbacks>(
                n::on_all() = std::bind(&any_all_handler<ProvideSpecTree, AllDepSpec>, std::ref(stack)),
                n::on_annotations() = std::bind(&set_annotations, std::ref(thing_to_annotate), _1),
                n::on_any() = std::bind(&any_not_allowed_handler, s),
                n::on_arrow() = std::bind(&arrows_not_allowed_handler, s, _1, _2),
                n::on_error() = std::bind(&error_handler, s, _1),
                n::on_label() = std::bind(&labels_not_allowed_handler, s, _1),
                n::on_pop() = std::bind(&pop_handler<ProvideSpecTree>, std::ref(stack),
                    ParseStackTypes<ProvideSpecTree>::AnnotationsGoHere(std::bind(
                            &set_thing_to_annotate, std::ref(thing_to_annotate), _1)), s),
                n::on_should_be_empty() = std::bind(&should_be_empty_handler<ProvideSpecTree>, std::ref(stack), s),
                n::on_string() = std::bind(&package_dep_spec_string_handler<ProvideSpecTree>, std::ref(stack),
                    ParseStackTypes<DependencySpecTree>::AnnotationsGoHere(std::bind(
                            &set_thing_to_annotate, std::ref(thing_to_annotate), _1)), _1, eapi, id),
                n::on_use() = std::bind(&use_handler<ProvideSpecTree>, std::ref(stack), _1, env, id),
                n::on_use_under_any() = std::bind(&use_under_any_handler, s, eapi)
            ));

    parse_elike_dependencies(s, callbacks);

    return top;
}

std::shared_ptr<FetchableURISpecTree>
paludis::erepository::parse_fetchable_uri(const std::string & s,
        const Environment * const env, const std::shared_ptr<const PackageID> & id, const EAPI & eapi)
{
    using namespace std::placeholders;

    ParseStackTypes<FetchableURISpecTree>::Stack stack;
    std::shared_ptr<AllDepSpec> spec(std::make_shared<AllDepSpec>());
    std::shared_ptr<DepSpec> thing_to_annotate(spec);
    std::shared_ptr<FetchableURISpecTree> top(std::make_shared<FetchableURISpecTree>(spec));
    stack.push_front(make_named_values<ParseStackTypes<FetchableURISpecTree>::Item>(
                n::item() = top->root(),
                n::spec() = spec
            ));

    ELikeDepParserCallbacks callbacks(
            make_named_values<ELikeDepParserCallbacks>(
                n::on_all() = std::bind(&any_all_handler<FetchableURISpecTree, AllDepSpec>, std::ref(stack)),
                n::on_annotations() = std::bind(&set_annotations, std::ref(thing_to_annotate), _1),
                n::on_any() = std::bind(&any_not_allowed_handler, s),
                n::on_arrow() = std::bind(&arrow_handler<FetchableURISpecTree>, std::ref(stack),
                    ParseStackTypes<FetchableURISpecTree>::AnnotationsGoHere(std::bind(
                            &set_thing_to_annotate, std::ref(thing_to_annotate), _1)), s, _1, _2, eapi),
                n::on_error() = std::bind(&error_handler, s, _1),
                n::on_label() = std::bind(&fetchable_label_handler<FetchableURISpecTree>, std::ref(stack),
                    ParseStackTypes<FetchableURISpecTree>::AnnotationsGoHere(std::bind(
                            &set_thing_to_annotate, std::ref(thing_to_annotate), _1)), _1, eapi),
                n::on_pop() = std::bind(&pop_handler<FetchableURISpecTree>, std::ref(stack),
                    ParseStackTypes<FetchableURISpecTree>::AnnotationsGoHere(std::bind(
                            &set_thing_to_annotate, std::ref(thing_to_annotate), _1)), s),
                n::on_should_be_empty() = std::bind(&should_be_empty_handler<FetchableURISpecTree>, std::ref(stack), s),
                n::on_string() = std::bind(&arrow_handler<FetchableURISpecTree>, std::ref(stack),
                    ParseStackTypes<FetchableURISpecTree>::AnnotationsGoHere(std::bind(
                            &set_thing_to_annotate, std::ref(thing_to_annotate), _1)), s, _1, "", eapi),
                n::on_use() = std::bind(&use_handler<FetchableURISpecTree>, std::ref(stack), _1, env, id),
                n::on_use_under_any() = std::bind(&use_under_any_handler, s, eapi)
            ));

    parse_elike_dependencies(s, callbacks);

    return top;
}

std::shared_ptr<SimpleURISpecTree>
paludis::erepository::parse_simple_uri(const std::string & s,
        const Environment * const env, const std::shared_ptr<const PackageID> & id, const EAPI &)
{
    using namespace std::placeholders;

    ParseStackTypes<SimpleURISpecTree>::Stack stack;
    std::shared_ptr<AllDepSpec> spec(std::make_shared<AllDepSpec>());
    std::shared_ptr<DepSpec> thing_to_annotate(spec);
    std::shared_ptr<SimpleURISpecTree> top(std::make_shared<SimpleURISpecTree>(spec));
    stack.push_front(make_named_values<ParseStackTypes<SimpleURISpecTree>::Item>(
                n::item() = top->root(),
                n::spec() = spec
            ));

    ELikeDepParserCallbacks callbacks(
            make_named_values<ELikeDepParserCallbacks>(
                n::on_all() = std::bind(&any_all_handler<SimpleURISpecTree, AllDepSpec>, std::ref(stack)),
                n::on_annotations() = std::bind(&set_annotations, std::ref(thing_to_annotate), _1),
                n::on_any() = std::bind(&any_not_allowed_handler, s),
                n::on_arrow() = std::bind(&arrows_not_allowed_handler, s, _1, _2),
                n::on_error() = std::bind(&error_handler, s, _1),
                n::on_label() = std::bind(&labels_not_allowed_handler, s, _1),
                n::on_pop() = std::bind(&pop_handler<SimpleURISpecTree>, std::ref(stack),
                    ParseStackTypes<DependencySpecTree>::AnnotationsGoHere(std::bind(
                            &set_thing_to_annotate, std::ref(thing_to_annotate), _1)), s),
                n::on_should_be_empty() = std::bind(&should_be_empty_handler<SimpleURISpecTree>, std::ref(stack), s),
                n::on_string() = std::bind(&simple_uri_handler<SimpleURISpecTree>, std::ref(stack),
                    ParseStackTypes<SimpleURISpecTree>::AnnotationsGoHere(std::bind(
                            &set_thing_to_annotate, std::ref(thing_to_annotate), _1)), _1),
                n::on_use() = std::bind(&use_handler<SimpleURISpecTree>, std::ref(stack), _1, env, id),
                n::on_use_under_any() = &do_nothing
            ));

    parse_elike_dependencies(s, callbacks);

    return top;
}

std::shared_ptr<LicenseSpecTree>
paludis::erepository::parse_license(const std::string & s,
        const Environment * const env, const std::shared_ptr<const PackageID> & id, const EAPI & eapi)
{
    using namespace std::placeholders;

    ParseStackTypes<LicenseSpecTree>::Stack stack;
    std::shared_ptr<AllDepSpec> spec(std::make_shared<AllDepSpec>());
    std::shared_ptr<DepSpec> thing_to_annotate(spec);
    std::shared_ptr<LicenseSpecTree> top(std::make_shared<LicenseSpecTree>(spec));
    stack.push_front(make_named_values<ParseStackTypes<LicenseSpecTree>::Item>(
                n::item() = top->root(),
                n::spec() = spec
            ));

    ELikeDepParserCallbacks callbacks(
            make_named_values<ELikeDepParserCallbacks>(
                n::on_all() = std::bind(&any_all_handler<LicenseSpecTree, AllDepSpec>, std::ref(stack)),
                n::on_annotations() = std::bind(&set_annotations, std::ref(thing_to_annotate), _1),
                n::on_any() = std::bind(&any_all_handler<LicenseSpecTree, AnyDepSpec>, std::ref(stack)),
                n::on_arrow() = std::bind(&arrows_not_allowed_handler, s, _1, _2),
                n::on_error() = std::bind(&error_handler, s, _1),
                n::on_label() = std::bind(&labels_not_allowed_handler, s, _1),
                n::on_pop() = std::bind(&pop_handler<LicenseSpecTree>, std::ref(stack),
                    ParseStackTypes<DependencySpecTree>::AnnotationsGoHere(std::bind(
                            &set_thing_to_annotate, std::ref(thing_to_annotate), _1)), s),
                n::on_should_be_empty() = std::bind(&should_be_empty_handler<LicenseSpecTree>, std::ref(stack), s),
                n::on_string() = std::bind(&license_handler<LicenseSpecTree>, std::ref(stack),
                    ParseStackTypes<LicenseSpecTree>::AnnotationsGoHere(std::bind(
                            &set_thing_to_annotate, std::ref(thing_to_annotate), _1)), _1),
                n::on_use() = std::bind(&use_handler<LicenseSpecTree>, std::ref(stack), _1, env, id),
                n::on_use_under_any() = std::bind(&use_under_any_handler, s, eapi)
            ));

    parse_elike_dependencies(s, callbacks);

    return top;
}

std::shared_ptr<PlainTextSpecTree>
paludis::erepository::parse_plain_text(const std::string & s,
        const Environment * const env, const std::shared_ptr<const PackageID> & id, const EAPI &)
{
    using namespace std::placeholders;

    ParseStackTypes<PlainTextSpecTree>::Stack stack;
    std::shared_ptr<AllDepSpec> spec(std::make_shared<AllDepSpec>());
    std::shared_ptr<DepSpec> thing_to_annotate(spec);
    std::shared_ptr<PlainTextSpecTree> top(std::make_shared<PlainTextSpecTree>(spec));
    stack.push_front(make_named_values<ParseStackTypes<PlainTextSpecTree>::Item>(
                n::item() = top->root(),
                n::spec() = spec
            ));

    ELikeDepParserCallbacks callbacks(
            make_named_values<ELikeDepParserCallbacks>(
                n::on_all() = std::bind(&any_all_handler<PlainTextSpecTree, AllDepSpec>, std::ref(stack)),
                n::on_annotations() = std::bind(&set_annotations, std::ref(thing_to_annotate), _1),
                n::on_any() = std::bind(&any_not_allowed_handler, s),
                n::on_arrow() = std::bind(&arrows_not_allowed_handler, s, _1, _2),
                n::on_error() = std::bind(&error_handler, s, _1),
                n::on_label() = std::bind(&labels_not_allowed_handler, s, _1),
                n::on_pop() = std::bind(&pop_handler<PlainTextSpecTree>, std::ref(stack),
                        ParseStackTypes<PlainTextSpecTree>::AnnotationsGoHere(std::bind(
                                &set_thing_to_annotate, std::ref(thing_to_annotate), _1)), s),
                n::on_should_be_empty() = std::bind(&should_be_empty_handler<PlainTextSpecTree>, std::ref(stack), s),
                n::on_string() = std::bind(&plain_text_handler<PlainTextSpecTree>, std::ref(stack),
                        ParseStackTypes<PlainTextSpecTree>::AnnotationsGoHere(std::bind(
                                &set_thing_to_annotate, std::ref(thing_to_annotate), _1)), _1),
                n::on_use() = std::bind(&use_handler<PlainTextSpecTree>, std::ref(stack), _1, env, id),
                n::on_use_under_any() = &do_nothing
            ));

    parse_elike_dependencies(s, callbacks);

    return top;
}

std::shared_ptr<PlainTextSpecTree>
paludis::erepository::parse_myoptions(const std::string & s,
        const Environment * const env, const std::shared_ptr<const PackageID> & id, const EAPI &)
{
    using namespace std::placeholders;

    ParseStackTypes<PlainTextSpecTree>::Stack stack;
    std::shared_ptr<AllDepSpec> spec(std::make_shared<AllDepSpec>());
    std::shared_ptr<DepSpec> thing_to_annotate(spec);
    std::shared_ptr<PlainTextSpecTree> top(std::make_shared<PlainTextSpecTree>(spec));
    stack.push_front(make_named_values<ParseStackTypes<PlainTextSpecTree>::Item>(
                n::item() = top->root(),
                n::spec() = spec
            ));

    ELikeDepParserCallbacks callbacks(
            make_named_values<ELikeDepParserCallbacks>(
                n::on_all() = std::bind(&any_all_handler<PlainTextSpecTree, AllDepSpec>, std::ref(stack)),
                n::on_annotations() = std::bind(&set_annotations, std::ref(thing_to_annotate), _1),
                n::on_any() = std::bind(&any_not_allowed_handler, s),
                n::on_arrow() = std::bind(&arrows_not_allowed_handler, s, _1, _2),
                n::on_error() = std::bind(&error_handler, s, _1),
                n::on_label() = std::bind(&plain_text_label_handler<PlainTextSpecTree>, std::ref(stack),
                        ParseStackTypes<PlainTextSpecTree>::AnnotationsGoHere(std::bind(
                                &set_thing_to_annotate, std::ref(thing_to_annotate), _1)), _1),
                n::on_pop() = std::bind(&pop_handler<PlainTextSpecTree>, std::ref(stack),
                        ParseStackTypes<PlainTextSpecTree>::AnnotationsGoHere(std::bind(
                                &set_thing_to_annotate, std::ref(thing_to_annotate), _1)), s),
                n::on_should_be_empty() = std::bind(&should_be_empty_handler<PlainTextSpecTree>, std::ref(stack), s),
                n::on_string() = std::bind(&plain_text_handler<PlainTextSpecTree>, std::ref(stack),
                        ParseStackTypes<PlainTextSpecTree>::AnnotationsGoHere(std::bind(
                                &set_thing_to_annotate, std::ref(thing_to_annotate), _1)), _1),
                n::on_use() = std::bind(&use_handler<PlainTextSpecTree>, std::ref(stack), _1, env, id),
                n::on_use_under_any() = &do_nothing
            ));

    parse_elike_dependencies(s, callbacks);

    return top;
}

std::shared_ptr<URILabelsDepSpec>
paludis::erepository::parse_uri_label(const std::string & s, const EAPI & e)
{
    Context context("When parsing label string '" + s + "' using EAPI '" + e.name() + "':");

    if (s.empty())
        throw EDepParseError(s, "Empty label");

    std::string c(e.supported()->uri_labels()->class_for_label(s.substr(0, s.length() - 1)));
    if (c.empty())
        throw EDepParseError(s, "Unknown label");

    std::shared_ptr<URILabelsDepSpec> l(std::make_shared<URILabelsDepSpec>());

    if (c == "URIMirrorsThenListedLabel")
        l->add_label(std::make_shared<URIMirrorsThenListedLabel>(s.substr(0, s.length() - 1)));
    else if (c == "URIMirrorsOnlyLabel")
        l->add_label(std::make_shared<URIMirrorsOnlyLabel>(s.substr(0, s.length() - 1)));
    else if (c == "URIListedOnlyLabel")
        l->add_label(std::make_shared<URIListedOnlyLabel>(s.substr(0, s.length() - 1)));
    else if (c == "URIListedThenMirrorsLabel")
        l->add_label(std::make_shared<URIListedThenMirrorsLabel>(s.substr(0, s.length() - 1)));
    else if (c == "URILocalMirrorsOnlyLabel")
        l->add_label(std::make_shared<URILocalMirrorsOnlyLabel>(s.substr(0, s.length() - 1)));
    else if (c == "URIManualOnlyLabel")
        l->add_label(std::make_shared<URIManualOnlyLabel>(s.substr(0, s.length() - 1)));
    else
        throw EDepParseError(s, "Label '" + s + "' maps to unknown class '" + c + "'");

    return l;
}

std::shared_ptr<PlainTextLabelDepSpec>
paludis::erepository::parse_plain_text_label(const std::string & s)
{
    Context context("When parsing label string '" + s + "':");

    if (s.empty())
        throw EDepParseError(s, "Empty label");

    std::string c(s.substr(0, s.length() - 1));
    if (c.empty())
        throw EDepParseError(s, "Unknown label");

    return std::make_shared<PlainTextLabelDepSpec>(s);
}

namespace
{
    bool enabled_if_option(
            const std::shared_ptr<const PackageID> id,
            const std::string label,
            const ChoiceNameWithPrefix n)
    {
        if (id->repository()->installed_root_key())
            return false;

        if (! id->choices_key())
        {
            Log::get_instance()->message("e.dep_parser.label_enabled.no_choices", ll_warning, lc_context)
                << "ID " << *id << " has no choices, so cannot tell whether label '" << label << "' is enabled";
            return false;
        }

        const std::shared_ptr<const ChoiceValue> v(id->choices_key()->value()->find_by_name_with_prefix(n));
        if (! v)
        {
            Log::get_instance()->message("e.dep_parser.label_enabled.no_choice", ll_warning, lc_context)
                << "ID " << *id << " has no choice named '" << n << "', so cannot tell whether label '"
                << label << "' is enabled";
            return false;
        }

        return v->enabled();
    }
}

std::shared_ptr<DependenciesLabelsDepSpec>
paludis::erepository::parse_dependency_label(
        const std::shared_ptr<const PackageID> & id,
        const std::string & s,
        const EAPI & e)
{
    Context context("When parsing label string '" + s + "' using EAPI '" + e.name() + "':");

    if (s.empty())
        throw EDepParseError(s, "Empty label");

    std::set<std::string> labels;
    std::string label(s.substr(0, s.length() - 1));
    tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>(label, "+", "", std::inserter(labels, labels.end()));

    std::shared_ptr<DependenciesLabelsDepSpec> l(std::make_shared<DependenciesLabelsDepSpec>());

    for (std::set<std::string>::iterator it = labels.begin(), it_e = labels.end(); it != it_e; ++it)
    {
        if (std::string::npos != it->find(','))
        {
            Log::get_instance()->message("e.dep_parser.obsolete_label_syntax", ll_warning, lc_context)
                << "Label '" << *it << "' uses commas, which are obsolete, so treating it as a build label instead";
            l->add_label(std::make_shared<DependenciesBuildLabel>(*it, return_literal_function(true)));
            continue;
        }

        std::string c(e.supported()->dependency_labels()->class_for_label(*it)), cc;
        if (c.empty())
            throw EDepParseError(s, "Unknown label '" + *it + "'");

        std::string::size_type p(c.find('/'));
        if (std::string::npos != p)
        {
            cc = c.substr(p + 1);
            c.erase(p);
        }

        if (c == "DependenciesBuildLabel")
            l->add_label(std::make_shared<DependenciesBuildLabel>(*it, return_literal_function(true)));
        else if (c == "DependenciesRunLabel")
            l->add_label(std::make_shared<DependenciesRunLabel>(*it, return_literal_function(true)));
        else if (c == "DependenciesPostLabel")
            l->add_label(std::make_shared<DependenciesPostLabel>(*it, return_literal_function(true)));
        else if (c == "DependenciesInstallLabel")
            l->add_label(std::make_shared<DependenciesInstallLabel>(*it, return_literal_function(true)));
        else if (c == "DependenciesCompileAgainstLabel")
            l->add_label(std::make_shared<DependenciesCompileAgainstLabel>(*it, return_literal_function(true)));
        else if (c == "DependenciesFetchLabel")
            l->add_label(std::make_shared<DependenciesFetchLabel>(*it, return_literal_function(true)));
        else if (c == "DependenciesSuggestionLabel")
            l->add_label(std::make_shared<DependenciesSuggestionLabel>(*it, return_literal_function(true)));
        else if (c == "DependenciesRecommendationLabel")
            l->add_label(std::make_shared<DependenciesRecommendationLabel>(*it, return_literal_function(true)));
        else if (c == "DependenciesTestLabel")
        {
            if (cc.empty())
                l->add_label(std::make_shared<DependenciesTestLabel>(*it, return_literal_function(true)));
            else
                l->add_label(std::make_shared<DependenciesTestLabel>(*it, std::bind(
                                    &enabled_if_option, id, *it, ChoiceNameWithPrefix(cc))));
        }
        else if (c == "WarnAndIgnore")
        {
            Log::get_instance()->message("e.dep_parser.obsolete_label", ll_warning, lc_context)
                << "Label '" << *it << "' no longer exists, pretending it's a build label instead";
            l->add_label(std::make_shared<DependenciesBuildLabel>(*it, return_literal_function(true)));
        }
        else
            throw EDepParseError(s, "Label '" + *it + "' maps to unknown class '" + c + "'");
    }

    return l;
}

