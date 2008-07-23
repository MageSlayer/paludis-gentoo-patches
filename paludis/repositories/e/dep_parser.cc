/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008 Ciaran McCreesh
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
#include <paludis/util/kc.hh>
#include <paludis/util/keys.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/options.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/elike_dep_parser.hh>
#include <paludis/elike_conditional_dep_spec.hh>
#include <paludis/elike_package_dep_spec.hh>
#include <paludis/dep_spec.hh>
#include <paludis/environment.hh>
#include <paludis/repository.hh>
#include <paludis/package_id.hh>
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

namespace
{
    template <typename T_>
    struct ParseStackTypes
    {
        typedef std::tr1::function<void (const std::tr1::shared_ptr<const typename T_::ConstItem> &)> AddHandler;

        typedef kc::KeyedClass<
            kc::Field<k::add_handler, AddHandler>,
            kc::Field<k::item, const std::tr1::shared_ptr<const typename T_::ConstItem> >
                > Item;

        typedef std::list<Item> Stack;
    };

    template <typename T_>
    void package_dep_spec_string_handler(const typename ParseStackTypes<T_>::Stack & h, const std::string & s,
            const EAPI & eapi, const std::tr1::shared_ptr<const PackageID> & id)
    {
        PackageDepSpec p(parse_elike_package_dep_spec(s, eapi.supported()->package_dep_spec_parse_options(), id));
        (*h.begin())[k::add_handler()](make_shared_ptr(new TreeLeaf<T_, PackageDepSpec>(make_shared_ptr(new PackageDepSpec(p)))));
    }

    template <typename T_>
    void package_or_block_dep_spec_string_handler(const typename ParseStackTypes<T_>::Stack & h, const std::string & s,
            const EAPI & eapi, const std::tr1::shared_ptr<const PackageID> & id)
    {
        if ((! s.empty()) && ('!' == s.at(0)))
        {
            std::tr1::shared_ptr<BlockDepSpec> b(new BlockDepSpec(
                        make_shared_ptr(new PackageDepSpec(parse_elike_package_dep_spec(s.substr(1),
                                    eapi.supported()->package_dep_spec_parse_options(), id)))));
            (*h.begin())[k::add_handler()](make_shared_ptr(new TreeLeaf<T_, BlockDepSpec>(b)));
        }
        else
            package_dep_spec_string_handler<T_>(h, s, eapi, id);
    }

    template <typename T_>
    void license_handler(const typename ParseStackTypes<T_>::Stack & h, const std::string & s)
    {
        (*h.begin())[k::add_handler()](make_shared_ptr(new TreeLeaf<T_, LicenseDepSpec>(make_shared_ptr(new LicenseDepSpec(s)))));
    }

    template <typename T_>
    void restrict_handler(const typename ParseStackTypes<T_>::Stack & h, const std::string & s)
    {
        (*h.begin())[k::add_handler()](make_shared_ptr(new TreeLeaf<T_, PlainTextDepSpec>(make_shared_ptr(new PlainTextDepSpec(s)))));
    }

    template <typename T_>
    void simple_uri_handler(const typename ParseStackTypes<T_>::Stack & h, const std::string & s)
    {
        (*h.begin())[k::add_handler()](make_shared_ptr(new TreeLeaf<T_, SimpleURIDepSpec>(make_shared_ptr(new SimpleURIDepSpec(s)))));
    }

    template <typename T_>
    void arrow_handler(const typename ParseStackTypes<T_>::Stack & h, const std::string & s, const std::string & f, const std::string & t,
            const EAPI & eapi)
    {
        if (t.empty() || eapi.supported()->dependency_spec_tree_parse_options()[dstpo_uri_supports_arrow])
            (*h.begin())[k::add_handler()](make_shared_ptr(new TreeLeaf<T_, FetchableURIDepSpec>(make_shared_ptr(
                                new FetchableURIDepSpec(t.empty() ? f : f + " -> " + t)))));
        else
            throw EDepParseError(s, "arrows in this EAPI");
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
    void dependency_label_handler(const typename ParseStackTypes<T_>::Stack & h, const std::string & s,
            const EAPI & eapi)
    {
        (*h.begin())[k::add_handler()](make_shared_ptr(new TreeLeaf<T_, DependencyLabelsDepSpec>(
                        parse_dependency_label(s, eapi))));
    }

    template <typename T_>
    void fetchable_label_handler(const typename ParseStackTypes<T_>::Stack & h, const std::string & s,
            const EAPI & eapi)
    {
        (*h.begin())[k::add_handler()](make_shared_ptr(new TreeLeaf<T_, URILabelsDepSpec>(
                        parse_uri_label(s, eapi))));
    }

    template <typename T_, typename A_>
    void any_all_handler(typename ParseStackTypes<T_>::Stack & stack)
    {
        using namespace std::tr1::placeholders;
        std::tr1::shared_ptr<ConstTreeSequence<T_, A_> > item(
                new ConstTreeSequence<T_, A_>(make_shared_ptr(new A_)));
        (*stack.begin())[k::add_handler()](item);
        stack.push_front(ParseStackTypes<T_>::Item::named_create()
                (k::add_handler(), std::tr1::bind(&ConstTreeSequence<T_, A_>::add, item.get(), _1))
                (k::item(), item)
                );
    }

    template <typename T_>
    void use_handler(typename ParseStackTypes<T_>::Stack & stack, const std::string & u,
            const Environment * const env, const std::tr1::shared_ptr<const PackageID> & id)
    {
        using namespace std::tr1::placeholders;
        std::tr1::shared_ptr<ConstTreeSequence<T_, ConditionalDepSpec> > item(
                new ConstTreeSequence<T_, ConditionalDepSpec>(make_shared_ptr(new ConditionalDepSpec(
                            parse_elike_conditional_dep_spec(u, env, id)))));
        (*stack.begin())[k::add_handler()](item);
        stack.push_front(ParseStackTypes<T_>::Item::named_create()
                (k::add_handler(), std::tr1::bind(&ConstTreeSequence<T_, ConditionalDepSpec>::add, item.get(), _1))
                (k::item(), item)
                );
    }

    template <typename T_>
    void pop_handler(typename ParseStackTypes<T_>::Stack & stack, const std::string & s)
    {
        stack.pop_front();
        if (stack.empty())
            throw EDepParseError(s, "Too many ')'s");
    }

    template <typename T_>
    void should_be_empty_handler(typename ParseStackTypes<T_>::Stack & stack, const std::string & s)
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

    void discard_annotations(const std::tr1::shared_ptr<const Map<std::string, std::string> > &)
    {
    }
}

std::tr1::shared_ptr<DependencySpecTree::ConstItem>
paludis::erepository::parse_depend(const std::string & s,
        const Environment * const env, const std::tr1::shared_ptr<const PackageID> & id, const EAPI & eapi)
{
    using namespace std::tr1::placeholders;

    ParseStackTypes<DependencySpecTree>::Stack stack;
    std::tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> > top(
            new ConstTreeSequence<DependencySpecTree, AllDepSpec>(make_shared_ptr(new AllDepSpec)));
    stack.push_front(ParseStackTypes<DependencySpecTree>::Item::named_create()
            (k::add_handler(), std::tr1::bind(&ConstTreeSequence<DependencySpecTree, AllDepSpec>::add, top.get(), _1))
            (k::item(), top)
            );

    ELikeDepParserCallbacks callbacks(
            ELikeDepParserCallbacks::named_create()
            (k::on_string(), std::tr1::bind(&package_or_block_dep_spec_string_handler<DependencySpecTree>, std::tr1::ref(stack), _1, eapi, id))
            (k::on_arrow(), std::tr1::bind(&arrows_not_allowed_handler, s, _1, _2))
            (k::on_any(), std::tr1::bind(&any_all_handler<DependencySpecTree, AnyDepSpec>, std::tr1::ref(stack)))
            (k::on_all(), std::tr1::bind(&any_all_handler<DependencySpecTree, AllDepSpec>, std::tr1::ref(stack)))
            (k::on_use(), std::tr1::bind(&use_handler<DependencySpecTree>, std::tr1::ref(stack), _1, env, id))
            (k::on_label(), std::tr1::bind(&dependency_label_handler<DependencySpecTree>, std::tr1::ref(stack), _1, eapi))
            (k::on_pop(), std::tr1::bind(&pop_handler<DependencySpecTree>, std::tr1::ref(stack), s))
            (k::on_error(), std::tr1::bind(&error_handler, s, _1))
            (k::on_should_be_empty(), std::tr1::bind(&should_be_empty_handler<DependencySpecTree>, std::tr1::ref(stack), s))
            (k::on_use_under_any(), std::tr1::bind(&use_under_any_handler, s, eapi))
            (k::on_annotations(), &discard_annotations)
            );

    parse_elike_dependencies(s, callbacks);

    return (*stack.begin())[k::item()];
}

std::tr1::shared_ptr<ProvideSpecTree::ConstItem>
paludis::erepository::parse_provide(const std::string & s,
        const Environment * const env, const std::tr1::shared_ptr<const PackageID> & id, const EAPI & eapi)
{
    using namespace std::tr1::placeholders;

    ParseStackTypes<ProvideSpecTree>::Stack stack;
    std::tr1::shared_ptr<ConstTreeSequence<ProvideSpecTree, AllDepSpec> > top(
            new ConstTreeSequence<ProvideSpecTree, AllDepSpec>(make_shared_ptr(new AllDepSpec)));
    stack.push_front(ParseStackTypes<ProvideSpecTree>::Item::named_create()
            (k::add_handler(), std::tr1::bind(&ConstTreeSequence<ProvideSpecTree, AllDepSpec>::add, top.get(), _1))
            (k::item(), top)
            );

    ELikeDepParserCallbacks callbacks(
            ELikeDepParserCallbacks::named_create()
            (k::on_string(), std::tr1::bind(&package_dep_spec_string_handler<ProvideSpecTree>, std::tr1::ref(stack), _1, eapi, id))
            (k::on_arrow(), std::tr1::bind(&arrows_not_allowed_handler, s, _1, _2))
            (k::on_any(), std::tr1::bind(&any_not_allowed_handler, s))
            (k::on_all(), std::tr1::bind(&any_all_handler<ProvideSpecTree, AllDepSpec>, std::tr1::ref(stack)))
            (k::on_use(), std::tr1::bind(&use_handler<ProvideSpecTree>, std::tr1::ref(stack), _1, env, id))
            (k::on_label(), std::tr1::bind(&labels_not_allowed_handler, s, _1))
            (k::on_pop(), std::tr1::bind(&pop_handler<ProvideSpecTree>, std::tr1::ref(stack), s))
            (k::on_error(), std::tr1::bind(&error_handler, s, _1))
            (k::on_should_be_empty(), std::tr1::bind(&should_be_empty_handler<ProvideSpecTree>, std::tr1::ref(stack), s))
            (k::on_use_under_any(), std::tr1::bind(&use_under_any_handler, s, eapi))
            (k::on_annotations(), &discard_annotations)
            );

    parse_elike_dependencies(s, callbacks);

    return (*stack.begin())[k::item()];
}

std::tr1::shared_ptr<FetchableURISpecTree::ConstItem>
paludis::erepository::parse_fetchable_uri(const std::string & s,
        const Environment * const env, const std::tr1::shared_ptr<const PackageID> & id, const EAPI & eapi)
{
    using namespace std::tr1::placeholders;

    ParseStackTypes<FetchableURISpecTree>::Stack stack;
    std::tr1::shared_ptr<ConstTreeSequence<FetchableURISpecTree, AllDepSpec> > top(
            new ConstTreeSequence<FetchableURISpecTree, AllDepSpec>(make_shared_ptr(new AllDepSpec)));
    stack.push_front(ParseStackTypes<FetchableURISpecTree>::Item::named_create()
            (k::add_handler(), std::tr1::bind(&ConstTreeSequence<FetchableURISpecTree, AllDepSpec>::add, top.get(), _1))
            (k::item(), top)
            );

    ELikeDepParserCallbacks callbacks(
            ELikeDepParserCallbacks::named_create()
            (k::on_string(), std::tr1::bind(&arrow_handler<FetchableURISpecTree>, std::tr1::ref(stack), s, _1, "", eapi))
            (k::on_arrow(), std::tr1::bind(&arrow_handler<FetchableURISpecTree>, std::tr1::ref(stack), s, _1, _2, eapi))
            (k::on_any(), std::tr1::bind(&any_not_allowed_handler, s))
            (k::on_all(), std::tr1::bind(&any_all_handler<FetchableURISpecTree, AllDepSpec>, std::tr1::ref(stack)))
            (k::on_use(), std::tr1::bind(&use_handler<FetchableURISpecTree>, std::tr1::ref(stack), _1, env, id))
            (k::on_label(), std::tr1::bind(&fetchable_label_handler<FetchableURISpecTree>, std::tr1::ref(stack), _1, eapi))
            (k::on_pop(), std::tr1::bind(&pop_handler<FetchableURISpecTree>, std::tr1::ref(stack), s))
            (k::on_error(), std::tr1::bind(&error_handler, s, _1))
            (k::on_should_be_empty(), std::tr1::bind(&should_be_empty_handler<FetchableURISpecTree>, std::tr1::ref(stack), s))
            (k::on_use_under_any(), std::tr1::bind(&use_under_any_handler, s, eapi))
            (k::on_annotations(), &discard_annotations)
            );

    parse_elike_dependencies(s, callbacks);

    return (*stack.begin())[k::item()];
}

std::tr1::shared_ptr<SimpleURISpecTree::ConstItem>
paludis::erepository::parse_simple_uri(const std::string & s,
        const Environment * const env, const std::tr1::shared_ptr<const PackageID> & id, const EAPI &)
{
    using namespace std::tr1::placeholders;

    ParseStackTypes<SimpleURISpecTree>::Stack stack;
    std::tr1::shared_ptr<ConstTreeSequence<SimpleURISpecTree, AllDepSpec> > top(
            new ConstTreeSequence<SimpleURISpecTree, AllDepSpec>(make_shared_ptr(new AllDepSpec)));
    stack.push_front(ParseStackTypes<SimpleURISpecTree>::Item::named_create()
            (k::add_handler(), std::tr1::bind(&ConstTreeSequence<SimpleURISpecTree, AllDepSpec>::add, top.get(), _1))
            (k::item(), top)
            );

    ELikeDepParserCallbacks callbacks(
            ELikeDepParserCallbacks::named_create()
            (k::on_string(), std::tr1::bind(&simple_uri_handler<SimpleURISpecTree>, std::tr1::ref(stack), _1))
            (k::on_arrow(), std::tr1::bind(&arrows_not_allowed_handler, s, _1, _2))
            (k::on_any(), std::tr1::bind(&any_not_allowed_handler, s))
            (k::on_all(), std::tr1::bind(&any_all_handler<SimpleURISpecTree, AllDepSpec>, std::tr1::ref(stack)))
            (k::on_use(), std::tr1::bind(&use_handler<SimpleURISpecTree>, std::tr1::ref(stack), _1, env, id))
            (k::on_label(), std::tr1::bind(&labels_not_allowed_handler, s, _1))
            (k::on_pop(), std::tr1::bind(&pop_handler<SimpleURISpecTree>, std::tr1::ref(stack), s))
            (k::on_error(), std::tr1::bind(&error_handler, s, _1))
            (k::on_should_be_empty(), std::tr1::bind(&should_be_empty_handler<SimpleURISpecTree>, std::tr1::ref(stack), s))
            (k::on_use_under_any(), &do_nothing)
            (k::on_annotations(), &discard_annotations)
            );

    parse_elike_dependencies(s, callbacks);

    return (*stack.begin())[k::item()];
}

std::tr1::shared_ptr<LicenseSpecTree::ConstItem>
paludis::erepository::parse_license(const std::string & s,
        const Environment * const env, const std::tr1::shared_ptr<const PackageID> & id, const EAPI & eapi)
{
    using namespace std::tr1::placeholders;

    ParseStackTypes<LicenseSpecTree>::Stack stack;
    std::tr1::shared_ptr<ConstTreeSequence<LicenseSpecTree, AllDepSpec> > top(
            new ConstTreeSequence<LicenseSpecTree, AllDepSpec>(make_shared_ptr(new AllDepSpec)));
    stack.push_front(ParseStackTypes<LicenseSpecTree>::Item::named_create()
            (k::add_handler(), std::tr1::bind(&ConstTreeSequence<LicenseSpecTree, AllDepSpec>::add, top.get(), _1))
            (k::item(), top)
            );

    ELikeDepParserCallbacks callbacks(
            ELikeDepParserCallbacks::named_create()
            (k::on_string(), std::tr1::bind(&license_handler<LicenseSpecTree>, std::tr1::ref(stack), _1))
            (k::on_arrow(), std::tr1::bind(&arrows_not_allowed_handler, s, _1, _2))
            (k::on_any(), std::tr1::bind(&any_all_handler<LicenseSpecTree, AnyDepSpec>, std::tr1::ref(stack)))
            (k::on_all(), std::tr1::bind(&any_all_handler<LicenseSpecTree, AllDepSpec>, std::tr1::ref(stack)))
            (k::on_use(), std::tr1::bind(&use_handler<LicenseSpecTree>, std::tr1::ref(stack), _1, env, id))
            (k::on_label(), std::tr1::bind(&labels_not_allowed_handler, s, _1))
            (k::on_pop(), std::tr1::bind(&pop_handler<LicenseSpecTree>, std::tr1::ref(stack), s))
            (k::on_error(), std::tr1::bind(&error_handler, s, _1))
            (k::on_should_be_empty(), std::tr1::bind(&should_be_empty_handler<LicenseSpecTree>, std::tr1::ref(stack), s))
            (k::on_use_under_any(), std::tr1::bind(&use_under_any_handler, s, eapi))
            (k::on_annotations(), &discard_annotations)
            );

    parse_elike_dependencies(s, callbacks);

    return (*stack.begin())[k::item()];
}

std::tr1::shared_ptr<RestrictSpecTree::ConstItem>
paludis::erepository::parse_restrict(const std::string & s,
        const Environment * const env, const std::tr1::shared_ptr<const PackageID> & id, const EAPI &)
{
    using namespace std::tr1::placeholders;

    ParseStackTypes<RestrictSpecTree>::Stack stack;
    std::tr1::shared_ptr<ConstTreeSequence<RestrictSpecTree, AllDepSpec> > top(
            new ConstTreeSequence<RestrictSpecTree, AllDepSpec>(make_shared_ptr(new AllDepSpec)));
    stack.push_front(ParseStackTypes<RestrictSpecTree>::Item::named_create()
            (k::add_handler(), std::tr1::bind(&ConstTreeSequence<RestrictSpecTree, AllDepSpec>::add, top.get(), _1))
            (k::item(), top)
            );

    ELikeDepParserCallbacks callbacks(
            ELikeDepParserCallbacks::named_create()
            (k::on_string(), std::tr1::bind(&restrict_handler<RestrictSpecTree>, std::tr1::ref(stack), _1))
            (k::on_arrow(), std::tr1::bind(&arrows_not_allowed_handler, s, _1, _2))
            (k::on_any(), std::tr1::bind(&any_not_allowed_handler, s))
            (k::on_all(), std::tr1::bind(&any_all_handler<RestrictSpecTree, AllDepSpec>, std::tr1::ref(stack)))
            (k::on_use(), std::tr1::bind(&use_handler<RestrictSpecTree>, std::tr1::ref(stack), _1, env, id))
            (k::on_label(), std::tr1::bind(&labels_not_allowed_handler, s, _1))
            (k::on_pop(), std::tr1::bind(&pop_handler<RestrictSpecTree>, std::tr1::ref(stack), s))
            (k::on_error(), std::tr1::bind(&error_handler, s, _1))
            (k::on_should_be_empty(), std::tr1::bind(&should_be_empty_handler<RestrictSpecTree>, std::tr1::ref(stack), s))
            (k::on_use_under_any(), &do_nothing)
            (k::on_annotations(), &discard_annotations)
            );

    parse_elike_dependencies(s, callbacks);

    return (*stack.begin())[k::item()];
}

std::tr1::shared_ptr<URILabelsDepSpec>
paludis::erepository::parse_uri_label(const std::string & s, const EAPI & e)
{
    Context context("When parsing label string '" + s + "' using EAPI '" + e.name() + "':");

    if (s.empty())
        throw EDepParseError(s, "Empty label");

    std::string c(e.supported()->uri_labels()->class_for_label(s.substr(0, s.length() - 1)));
    if (c.empty())
        throw EDepParseError(s, "Unknown label");

    std::tr1::shared_ptr<URILabelsDepSpec> l(new LabelsDepSpec<URILabelVisitorTypes>);

    if (c == "URIMirrorsThenListedLabel")
        l->add_label(make_shared_ptr(new URIMirrorsThenListedLabel(s.substr(0, s.length() - 1))));
    else if (c == "URIMirrorsOnlyLabel")
        l->add_label(make_shared_ptr(new URIMirrorsOnlyLabel(s.substr(0, s.length() - 1))));
    else if (c == "URIListedOnlyLabel")
        l->add_label(make_shared_ptr(new URIListedOnlyLabel(s.substr(0, s.length() - 1))));
    else if (c == "URIListedThenMirrorsLabel")
        l->add_label(make_shared_ptr(new URIListedThenMirrorsLabel(s.substr(0, s.length() - 1))));
    else if (c == "URILocalMirrorsOnlyLabel")
        l->add_label(make_shared_ptr(new URILocalMirrorsOnlyLabel(s.substr(0, s.length() - 1))));
    else if (c == "URIManualOnlyLabel")
        l->add_label(make_shared_ptr(new URIManualOnlyLabel(s.substr(0, s.length() - 1))));
    else
        throw EDepParseError(s, "Label '" + s + "' maps to unknown class '" + c + "'");

    return l;
}

std::tr1::shared_ptr<DependencyLabelsDepSpec>
paludis::erepository::parse_dependency_label(const std::string & s, const EAPI & e)
{
    Context context("When parsing label string '" + s + "' using EAPI '" + e.name() + "':");

    if (s.empty())
        throw EDepParseError(s, "Empty label");

    std::set<std::string> labels;
    std::string label(s.substr(0, s.length() - 1));
    tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>(label, ",+", "", std::inserter(labels, labels.end()));

    std::tr1::shared_ptr<DependencyLabelsDepSpec> l(new DependencyLabelsDepSpec);

    for (std::set<std::string>::iterator it = labels.begin(), it_e = labels.end(); it != it_e; ++it)
    {
        std::string c(e.supported()->dependency_labels()->class_for_label(*it));
        if (c.empty())
            throw EDepParseError(s, "Unknown label '" + *it + "'");

        if (c == "DependencyHostLabel")
            l->add_label(make_shared_ptr(new DependencyHostLabel(*it)));
        else if (c == "DependencyTargetLabel")
            l->add_label(make_shared_ptr(new DependencyTargetLabel(*it)));
        else if (c == "DependencyBuildLabel")
            l->add_label(make_shared_ptr(new DependencyBuildLabel(*it)));
        else if (c == "DependencyRunLabel")
            l->add_label(make_shared_ptr(new DependencyRunLabel(*it)));
        else if (c == "DependencyPostLabel")
            l->add_label(make_shared_ptr(new DependencyPostLabel(*it)));
        else if (c == "DependencyInstallLabel")
            l->add_label(make_shared_ptr(new DependencyInstallLabel(*it)));
        else if (c == "DependencyCompileLabel")
            l->add_label(make_shared_ptr(new DependencyCompileLabel(*it)));
        else if (c == "DependencySuggestedLabel")
            l->add_label(make_shared_ptr(new DependencySuggestedLabel(*it)));
        else if (c == "DependencyRecommendedLabel")
            l->add_label(make_shared_ptr(new DependencyRecommendedLabel(*it)));
        else if (c == "DependencyRequiredLabel")
            l->add_label(make_shared_ptr(new DependencyRequiredLabel(*it)));
        else if (c == "DependencyAnyLabel")
            l->add_label(make_shared_ptr(new DependencyAnyLabel(*it)));
        else if (c == "DependencyMineLabel")
            l->add_label(make_shared_ptr(new DependencyMineLabel(*it)));
        else if (c == "DependencyPrimaryLabel")
            l->add_label(make_shared_ptr(new DependencyPrimaryLabel(*it)));
        else if (c == "DependencyABILabel")
            l->add_label(make_shared_ptr(new DependencyABILabel(*it)));
        else
            throw EDepParseError(s, "Label '" + *it + "' maps to unknown class '" + c + "'");
    }

    return l;
}
