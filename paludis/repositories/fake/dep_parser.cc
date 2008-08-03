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

#include <paludis/repositories/fake/dep_parser.hh>
#include <paludis/util/kc.hh>
#include <paludis/util/keys.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/options.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/elike_dep_parser.hh>
#include <paludis/elike_conditional_dep_spec.hh>
#include <paludis/elike_package_dep_spec.hh>
#include <paludis/dep_spec.hh>
#include <paludis/environment.hh>
#include <paludis/repository.hh>
#include <paludis/package_id.hh>
#include <list>

using namespace paludis;
using namespace paludis::fakerepository;

FakeDepParseError::FakeDepParseError(const std::string & s, const std::string & t) throw () :
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
            const std::tr1::shared_ptr<const PackageID> & id)
    {
        PackageDepSpec p(parse_elike_package_dep_spec(s, ELikePackageDepSpecOptions() + epdso_allow_slot_deps
                    + epdso_allow_slot_star_deps + epdso_allow_slot_equal_deps + epdso_allow_repository_deps
                    + epdso_allow_use_deps + epdso_allow_ranged_deps + epdso_allow_tilde_greater_deps
                    + epdso_strict_parsing, id));
        (*h.begin())[k::add_handler()](make_shared_ptr(new TreeLeaf<T_, PackageDepSpec>(make_shared_ptr(new PackageDepSpec(p)))));
    }

    template <typename T_>
    void package_or_block_dep_spec_string_handler(const typename ParseStackTypes<T_>::Stack & h, const std::string & s,
            const std::tr1::shared_ptr<const PackageID> & id)
    {
        if ((! s.empty()) && ('!' == s.at(0)))
        {
            std::tr1::shared_ptr<BlockDepSpec> b(new BlockDepSpec(
                        make_shared_ptr(new PackageDepSpec(parse_elike_package_dep_spec(s.substr(1),
                                    ELikePackageDepSpecOptions() + epdso_allow_slot_deps
                                    + epdso_allow_slot_star_deps + epdso_allow_slot_equal_deps + epdso_allow_repository_deps
                                    + epdso_allow_use_deps + epdso_allow_ranged_deps + epdso_allow_tilde_greater_deps
                                    + epdso_strict_parsing, id)))));
            (*h.begin())[k::add_handler()](make_shared_ptr(new TreeLeaf<T_, BlockDepSpec>(b)));
        }
        else
            package_dep_spec_string_handler<T_>(h, s, id);
    }

    template <typename T_>
    void license_handler(const typename ParseStackTypes<T_>::Stack & h, const std::string & s)
    {
        (*h.begin())[k::add_handler()](make_shared_ptr(new TreeLeaf<T_, LicenseDepSpec>(make_shared_ptr(new LicenseDepSpec(s)))));
    }

    template <typename T_>
    void simple_uri_handler(const typename ParseStackTypes<T_>::Stack & h, const std::string & s)
    {
        (*h.begin())[k::add_handler()](make_shared_ptr(new TreeLeaf<T_, SimpleURIDepSpec>(make_shared_ptr(new SimpleURIDepSpec(s)))));
    }

    template <typename T_>
    void arrow_handler(const typename ParseStackTypes<T_>::Stack & h, const std::string & s, const std::string & t)
    {
        (*h.begin())[k::add_handler()](make_shared_ptr(new TreeLeaf<T_, FetchableURIDepSpec>(make_shared_ptr(
                            new FetchableURIDepSpec(t.empty() ? s : s + " -> " + t)))));
    }

    void any_not_allowed_handler(const std::string & s) PALUDIS_ATTRIBUTE((noreturn));

    void any_not_allowed_handler(const std::string & s)
    {
        throw FakeDepParseError(s, "Any dep specs not allowed here");
    }

    void arrows_not_allowed_handler(const std::string & s, const std::string & f, const std::string & t) PALUDIS_ATTRIBUTE((noreturn));

    void arrows_not_allowed_handler(const std::string & s, const std::string & f, const std::string & t)
    {
        throw FakeDepParseError(s, "Arrow '" + f + " -> " + t + "' not allowed here");
    }

    void error_handler(const std::string & s, const std::string & t) PALUDIS_ATTRIBUTE((noreturn));

    void error_handler(const std::string & s, const std::string & t)
    {
        throw FakeDepParseError(s, t);
    }

    void labels_not_allowed_handler(const std::string & s, const std::string & f) PALUDIS_ATTRIBUTE((noreturn));

    void labels_not_allowed_handler(const std::string & s, const std::string & f)
    {
        throw FakeDepParseError(s, "Label '" + f + "' not allowed here");
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
            throw FakeDepParseError(s, "Too many ')'s");
    }

    template <typename T_>
    void should_be_empty_handler(typename ParseStackTypes<T_>::Stack & stack, const std::string & s)
    {
        if (1 != stack.size())
            throw FakeDepParseError(s, "Nesting error");
    }

    void do_nothing()
    {
    }

    void discard_annotations(const std::tr1::shared_ptr<const Map<std::string, std::string> > &)
    {
    }
}

std::tr1::shared_ptr<DependencySpecTree::ConstItem>
paludis::fakerepository::parse_depend(const std::string & s,
        const Environment * const env, const std::tr1::shared_ptr<const PackageID> & id)
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
            (k::on_string(), std::tr1::bind(&package_or_block_dep_spec_string_handler<DependencySpecTree>, std::tr1::ref(stack), _1, id))
            (k::on_arrow(), std::tr1::bind(&arrows_not_allowed_handler, s, _1, _2))
            (k::on_any(), std::tr1::bind(&any_all_handler<DependencySpecTree, AnyDepSpec>, std::tr1::ref(stack)))
            (k::on_all(), std::tr1::bind(&any_all_handler<DependencySpecTree, AllDepSpec>, std::tr1::ref(stack)))
            (k::on_use(), std::tr1::bind(&use_handler<DependencySpecTree>, std::tr1::ref(stack), _1, env, id))
            (k::on_label(), std::tr1::bind(&labels_not_allowed_handler, s, _1))
            (k::on_pop(), std::tr1::bind(&pop_handler<DependencySpecTree>, std::tr1::ref(stack), s))
            (k::on_error(), std::tr1::bind(&error_handler, s, _1))
            (k::on_should_be_empty(), std::tr1::bind(&should_be_empty_handler<DependencySpecTree>, std::tr1::ref(stack), s))
            (k::on_use_under_any(), &do_nothing)
            (k::on_annotations(), &discard_annotations)
            );

    parse_elike_dependencies(s, callbacks);

    return (*stack.begin())[k::item()];
}

std::tr1::shared_ptr<ProvideSpecTree::ConstItem>
paludis::fakerepository::parse_provide(const std::string & s,
        const Environment * const env, const std::tr1::shared_ptr<const PackageID> & id)
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
            (k::on_string(), std::tr1::bind(&package_dep_spec_string_handler<ProvideSpecTree>, std::tr1::ref(stack), _1, id))
            (k::on_arrow(), std::tr1::bind(&arrows_not_allowed_handler, s, _1, _2))
            (k::on_any(), std::tr1::bind(&any_not_allowed_handler, s))
            (k::on_all(), std::tr1::bind(&any_all_handler<ProvideSpecTree, AllDepSpec>, std::tr1::ref(stack)))
            (k::on_use(), std::tr1::bind(&use_handler<ProvideSpecTree>, std::tr1::ref(stack), _1, env, id))
            (k::on_label(), std::tr1::bind(&labels_not_allowed_handler, s, _1))
            (k::on_pop(), std::tr1::bind(&pop_handler<ProvideSpecTree>, std::tr1::ref(stack), s))
            (k::on_error(), std::tr1::bind(&error_handler, s, _1))
            (k::on_should_be_empty(), std::tr1::bind(&should_be_empty_handler<ProvideSpecTree>, std::tr1::ref(stack), s))
            (k::on_use_under_any(), &do_nothing)
            (k::on_annotations(), &discard_annotations)
            );

    parse_elike_dependencies(s, callbacks);

    return (*stack.begin())[k::item()];
}

std::tr1::shared_ptr<FetchableURISpecTree::ConstItem>
paludis::fakerepository::parse_fetchable_uri(const std::string & s,
        const Environment * const env, const std::tr1::shared_ptr<const PackageID> & id)
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
            (k::on_string(), std::tr1::bind(&arrow_handler<FetchableURISpecTree>, std::tr1::ref(stack), _1, ""))
            (k::on_arrow(), std::tr1::bind(&arrow_handler<FetchableURISpecTree>, std::tr1::ref(stack), _1, _2))
            (k::on_any(), std::tr1::bind(&any_not_allowed_handler, s))
            (k::on_all(), std::tr1::bind(&any_all_handler<FetchableURISpecTree, AllDepSpec>, std::tr1::ref(stack)))
            (k::on_use(), std::tr1::bind(&use_handler<FetchableURISpecTree>, std::tr1::ref(stack), _1, env, id))
            (k::on_label(), std::tr1::bind(&labels_not_allowed_handler, s, _1))
            (k::on_pop(), std::tr1::bind(&pop_handler<FetchableURISpecTree>, std::tr1::ref(stack), s))
            (k::on_error(), std::tr1::bind(&error_handler, s, _1))
            (k::on_should_be_empty(), std::tr1::bind(&should_be_empty_handler<FetchableURISpecTree>, std::tr1::ref(stack), s))
            (k::on_use_under_any(), &do_nothing)
            (k::on_annotations(), &discard_annotations)
            );

    parse_elike_dependencies(s, callbacks);

    return (*stack.begin())[k::item()];
}

std::tr1::shared_ptr<SimpleURISpecTree::ConstItem>
paludis::fakerepository::parse_simple_uri(const std::string & s,
        const Environment * const env, const std::tr1::shared_ptr<const PackageID> & id)
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
paludis::fakerepository::parse_license(const std::string & s,
        const Environment * const env, const std::tr1::shared_ptr<const PackageID> & id)
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
            (k::on_use_under_any(), &do_nothing)
            (k::on_annotations(), &discard_annotations)
            );

    parse_elike_dependencies(s, callbacks);

    return (*stack.begin())[k::item()];
}

