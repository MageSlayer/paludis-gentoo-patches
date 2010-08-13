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

#include <paludis/repositories/fake/dep_parser.hh>
#include <paludis/util/options.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/elike_dep_parser.hh>
#include <paludis/elike_conditional_dep_spec.hh>
#include <paludis/elike_package_dep_spec.hh>
#include <paludis/dep_spec.hh>
#include <paludis/environment.hh>
#include <paludis/repository.hh>
#include <paludis/package_id.hh>
#include <paludis/user_dep_spec.hh>
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
        typedef std::list<std::shared_ptr<typename T_::BasicInnerNode> > Stack;
    };

    template <typename T_>
    void package_dep_spec_string_handler(const typename ParseStackTypes<T_>::Stack & h, const std::string & s,
            const std::shared_ptr<const PackageID> & id)
    {
        PackageDepSpec p(parse_elike_package_dep_spec(s, ELikePackageDepSpecOptions() + epdso_allow_slot_deps
                    + epdso_allow_slot_star_deps + epdso_allow_slot_equal_deps + epdso_allow_repository_deps
                    + epdso_allow_use_deps + epdso_allow_ranged_deps + epdso_allow_tilde_greater_deps
                    + epdso_strict_parsing,
                    user_version_spec_options(),
                    id));
        (*h.begin())->append(std::make_shared<PackageDepSpec>(p));
    }

    template <typename T_>
    void package_or_block_dep_spec_string_handler(const typename ParseStackTypes<T_>::Stack & h, const std::string & s,
            const std::shared_ptr<const PackageID> & id)
    {
        if ((! s.empty()) && ('!' == s.at(0)))
        {
            (*h.begin())->append(std::make_shared<BlockDepSpec>(s,
                            parse_elike_package_dep_spec(s.substr(1),
                                ELikePackageDepSpecOptions() + epdso_allow_slot_deps
                                + epdso_allow_slot_star_deps + epdso_allow_slot_equal_deps + epdso_allow_repository_deps
                                + epdso_allow_use_deps + epdso_allow_ranged_deps + epdso_allow_tilde_greater_deps
                                + epdso_strict_parsing,
                                user_version_spec_options(),
                                id), false));
        }
        else
            package_dep_spec_string_handler<T_>(h, s, id);
    }

    template <typename T_>
    void license_handler(const typename ParseStackTypes<T_>::Stack & h, const std::string & s)
    {
        (*h.begin())->append(std::make_shared<LicenseDepSpec>(s));
    }

    template <typename T_>
    void simple_uri_handler(const typename ParseStackTypes<T_>::Stack & h, const std::string & s)
    {
        (*h.begin())->append(std::make_shared<SimpleURIDepSpec>(s));
    }

    template <typename T_>
    void arrow_handler(const typename ParseStackTypes<T_>::Stack & h, const std::string & s, const std::string & t)
    {
        (*h.begin())->append(std::make_shared<FetchableURIDepSpec>(t.empty() ? s : s + " -> " + t));
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
        stack.push_front((*stack.begin())->append(std::make_shared<A_>()));
    }

    template <typename T_>
    void use_handler(typename ParseStackTypes<T_>::Stack & stack, const std::string & u,
            const Environment * const env, const std::shared_ptr<const PackageID> & id)
    {
        stack.push_front((*stack.begin())->append(
                    std::make_shared<ConditionalDepSpec>(parse_elike_conditional_dep_spec(u, env, id, false))));
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

    void discard_annotations(const std::shared_ptr<const Map<std::string, std::string> > &)
    {
    }
}

std::shared_ptr<DependencySpecTree>
paludis::fakerepository::parse_depend(const std::string & s,
        const Environment * const env, const std::shared_ptr<const PackageID> & id)
{
    using namespace std::placeholders;

    ParseStackTypes<DependencySpecTree>::Stack stack;
    std::shared_ptr<DependencySpecTree> top(std::make_shared<DependencySpecTree>(std::make_shared<AllDepSpec>()));
    stack.push_front(top->top());

    ELikeDepParserCallbacks callbacks(
            make_named_values<ELikeDepParserCallbacks>(
                n::on_all() = std::bind(&any_all_handler<DependencySpecTree, AllDepSpec>, std::ref(stack)),
                n::on_annotations() = &discard_annotations,
                n::on_any() = std::bind(&any_all_handler<DependencySpecTree, AnyDepSpec>, std::ref(stack)),
                n::on_arrow() = std::bind(&arrows_not_allowed_handler, s, _1, _2),
                n::on_error() = std::bind(&error_handler, s, _1),
                n::on_label() = std::bind(&labels_not_allowed_handler, s, _1),
                n::on_pop() = std::bind(&pop_handler<DependencySpecTree>, std::ref(stack), s),
                n::on_should_be_empty() = std::bind(&should_be_empty_handler<DependencySpecTree>, std::ref(stack), s),
                n::on_string() = std::bind(&package_or_block_dep_spec_string_handler<DependencySpecTree>, std::ref(stack), _1, id),
                n::on_use() = std::bind(&use_handler<DependencySpecTree>, std::ref(stack), _1, env, id),
                n::on_use_under_any() = &do_nothing
                ));

    parse_elike_dependencies(s, callbacks);

    return top;
}

std::shared_ptr<ProvideSpecTree>
paludis::fakerepository::parse_provide(const std::string & s,
        const Environment * const env, const std::shared_ptr<const PackageID> & id)
{
    using namespace std::placeholders;

    ParseStackTypes<ProvideSpecTree>::Stack stack;
    std::shared_ptr<ProvideSpecTree> top(std::make_shared<ProvideSpecTree>(std::make_shared<AllDepSpec>()));
    stack.push_front(top->top());

    ELikeDepParserCallbacks callbacks(
            make_named_values<ELikeDepParserCallbacks>(
                n::on_all() = std::bind(&any_all_handler<ProvideSpecTree, AllDepSpec>, std::ref(stack)),
                n::on_annotations() = &discard_annotations,
                n::on_any() = std::bind(&any_not_allowed_handler, s),
                n::on_arrow() = std::bind(&arrows_not_allowed_handler, s, _1, _2),
                n::on_error() = std::bind(&error_handler, s, _1),
                n::on_label() = std::bind(&labels_not_allowed_handler, s, _1),
                n::on_pop() = std::bind(&pop_handler<ProvideSpecTree>, std::ref(stack), s),
                n::on_should_be_empty() = std::bind(&should_be_empty_handler<ProvideSpecTree>, std::ref(stack), s),
                n::on_string() = std::bind(&package_dep_spec_string_handler<ProvideSpecTree>, std::ref(stack), _1, id),
                n::on_use() = std::bind(&use_handler<ProvideSpecTree>, std::ref(stack), _1, env, id),
                n::on_use_under_any() = &do_nothing
                ));

    parse_elike_dependencies(s, callbacks);

    return top;
}

std::shared_ptr<FetchableURISpecTree>
paludis::fakerepository::parse_fetchable_uri(const std::string & s,
        const Environment * const env, const std::shared_ptr<const PackageID> & id)
{
    using namespace std::placeholders;

    ParseStackTypes<FetchableURISpecTree>::Stack stack;
    std::shared_ptr<FetchableURISpecTree> top(std::make_shared<FetchableURISpecTree>(std::make_shared<AllDepSpec>()));
    stack.push_front(top->top());

    ELikeDepParserCallbacks callbacks(
            make_named_values<ELikeDepParserCallbacks>(
                n::on_all() = std::bind(&any_all_handler<FetchableURISpecTree, AllDepSpec>, std::ref(stack)),
                n::on_annotations() = &discard_annotations,
                n::on_any() = std::bind(&any_not_allowed_handler, s),
                n::on_arrow() = std::bind(&arrow_handler<FetchableURISpecTree>, std::ref(stack), _1, _2),
                n::on_error() = std::bind(&error_handler, s, _1),
                n::on_label() = std::bind(&labels_not_allowed_handler, s, _1),
                n::on_pop() = std::bind(&pop_handler<FetchableURISpecTree>, std::ref(stack), s),
                n::on_should_be_empty() = std::bind(&should_be_empty_handler<FetchableURISpecTree>, std::ref(stack), s),
                n::on_string() = std::bind(&arrow_handler<FetchableURISpecTree>, std::ref(stack), _1, ""),
                n::on_use() = std::bind(&use_handler<FetchableURISpecTree>, std::ref(stack), _1, env, id),
                n::on_use_under_any() = &do_nothing
                ));

    parse_elike_dependencies(s, callbacks);

    return top;
}

std::shared_ptr<SimpleURISpecTree>
paludis::fakerepository::parse_simple_uri(const std::string & s,
        const Environment * const env, const std::shared_ptr<const PackageID> & id)
{
    using namespace std::placeholders;

    ParseStackTypes<SimpleURISpecTree>::Stack stack;
    std::shared_ptr<SimpleURISpecTree> top(std::make_shared<SimpleURISpecTree>(std::make_shared<AllDepSpec>()));
    stack.push_front(top->top());

    ELikeDepParserCallbacks callbacks(
            make_named_values<ELikeDepParserCallbacks>(
                n::on_all() = std::bind(&any_all_handler<SimpleURISpecTree, AllDepSpec>, std::ref(stack)),
                n::on_annotations() = &discard_annotations,
                n::on_any() = std::bind(&any_not_allowed_handler, s),
                n::on_arrow() = std::bind(&arrows_not_allowed_handler, s, _1, _2),
                n::on_error() = std::bind(&error_handler, s, _1),
                n::on_label() = std::bind(&labels_not_allowed_handler, s, _1),
                n::on_pop() = std::bind(&pop_handler<SimpleURISpecTree>, std::ref(stack), s),
                n::on_should_be_empty() = std::bind(&should_be_empty_handler<SimpleURISpecTree>, std::ref(stack), s),
                n::on_string() = std::bind(&simple_uri_handler<SimpleURISpecTree>, std::ref(stack), _1),
                n::on_use() = std::bind(&use_handler<SimpleURISpecTree>, std::ref(stack), _1, env, id),
                n::on_use_under_any() = &do_nothing
                ));

    parse_elike_dependencies(s, callbacks);

    return top;
}

std::shared_ptr<LicenseSpecTree>
paludis::fakerepository::parse_license(const std::string & s,
        const Environment * const env, const std::shared_ptr<const PackageID> & id)
{
    using namespace std::placeholders;

    ParseStackTypes<LicenseSpecTree>::Stack stack;
    std::shared_ptr<LicenseSpecTree> top(std::make_shared<LicenseSpecTree>(std::make_shared<AllDepSpec>()));
    stack.push_front(top->top());

    ELikeDepParserCallbacks callbacks(
            make_named_values<ELikeDepParserCallbacks>(
            n::on_all() = std::bind(&any_all_handler<LicenseSpecTree, AllDepSpec>, std::ref(stack)),
            n::on_annotations() = &discard_annotations,
            n::on_any() = std::bind(&any_all_handler<LicenseSpecTree, AnyDepSpec>, std::ref(stack)),
            n::on_arrow() = std::bind(&arrows_not_allowed_handler, s, _1, _2),
            n::on_error() = std::bind(&error_handler, s, _1),
            n::on_label() = std::bind(&labels_not_allowed_handler, s, _1),
            n::on_pop() = std::bind(&pop_handler<LicenseSpecTree>, std::ref(stack), s),
            n::on_should_be_empty() = std::bind(&should_be_empty_handler<LicenseSpecTree>, std::ref(stack), s),
            n::on_string() = std::bind(&license_handler<LicenseSpecTree>, std::ref(stack), _1),
            n::on_use() = std::bind(&use_handler<LicenseSpecTree>, std::ref(stack), _1, env, id),
            n::on_use_under_any() = &do_nothing
            ));

    parse_elike_dependencies(s, callbacks);

    return top;
}

