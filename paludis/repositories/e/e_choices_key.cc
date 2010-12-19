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

#include <paludis/repositories/e/e_choices_key.hh>
#include <paludis/repositories/e/e_key.hh>
#include <paludis/repositories/e/ebuild_id.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/dep_spec_pretty_printer.hh>
#include <paludis/repositories/e/profile.hh>
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/e/myoption.hh>

#include <paludis/util/pimp-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/log.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/join.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/tribool.hh>
#include <paludis/util/member_iterator-impl.hh>
#include <paludis/util/map-impl.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/set-impl.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/accept_visitor.hh>

#include <paludis/environment.hh>
#include <paludis/stringify_formatter-impl.hh>
#include <paludis/choice.hh>
#include <paludis/elike_choices.hh>
#include <paludis/dep_spec_annotations.hh>

#include <list>
#include <algorithm>
#include <set>
#include <map>

using namespace paludis;
using namespace paludis::erepository;

namespace paludis
{
    template <>
    struct Imp<EChoicesKey>
    {
        mutable Mutex mutex;
        mutable std::shared_ptr<Choices> value;

        const Environment * const env;
        const std::shared_ptr<const ERepositoryID> id;
        const std::shared_ptr<const ERepository> maybe_e_repository;
        const std::shared_ptr<const Map<ChoiceNameWithPrefix, std::string> > maybe_descriptions;

        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;

        Imp(const Environment * const e, const std::shared_ptr<const ERepositoryID> & i,
                const std::shared_ptr<const ERepository> & p,
                const std::shared_ptr<const Map<ChoiceNameWithPrefix, std::string> > & d,
                const std::string & r, const std::string & h, const MetadataKeyType t) :
            env(e),
            id(i),
            maybe_e_repository(p),
            maybe_descriptions(d),
            raw_name(r),
            human_name(h),
            type(t)
        {
        }
    };
}

EChoicesKey::EChoicesKey(
        const Environment * const e,
        const std::shared_ptr<const ERepositoryID> & i,
        const std::string & r, const std::string & h, const MetadataKeyType t,
        const std::shared_ptr<const ERepository> & p,
        const std::shared_ptr<const Map<ChoiceNameWithPrefix, std::string> > & d) :
    Pimp<EChoicesKey>(e, i, p, d, r, h, t)
{
}

EChoicesKey::~EChoicesKey()
{
}

namespace
{
    struct IsExpand
    {
        ChoiceNameWithPrefix flag;
        std::string delim;

        IsExpand(const ChoiceNameWithPrefix & f, const std::string & d) :
            flag(f),
            delim(d)
        {
        }

        bool operator() (const std::string & s) const
        {
            std::string lower_s;
            std::transform(s.begin(), s.end(), std::back_inserter(lower_s), &::tolower);
            lower_s.append(delim);
            return (0 == flag.value().compare(0, lower_s.length(), lower_s, 0, lower_s.length()));
        }
    };

    struct MyOptionsFinder
    {
        typedef std::map<UnprefixedChoiceName, std::string> Descriptions;
        typedef std::map<ChoicePrefixName, Descriptions> Prefixes;

        Prefixes prefixes;
        std::list<ChoicePrefixName> current_prefix_stack;

        MyOptionsFinder()
        {
            current_prefix_stack.push_front(ChoicePrefixName(""));
        }

        void visit(const PlainTextSpecTree::NodeType<PlainTextDepSpec>::Type & node)
        {
            if (node.spec()->text().empty())
                return;

            Context context("When handling item '" + stringify(*node.spec()) + "':");

            Prefixes::iterator p(prefixes.find(*current_prefix_stack.begin()));
            if (p == prefixes.end())
                p = prefixes.insert(std::make_pair(*current_prefix_stack.begin(), Descriptions())).first;

            UnprefixedChoiceName n(parse_myoption(node.spec()->text()).first);
            if (std::string::npos != stringify(n).find(':'))
                throw MyOptionsError("Flag '" + stringify(n) + "' must not contain a ':'");

            if (node.spec()->maybe_annotations())
            {
                auto m_role(node.spec()->maybe_annotations()->find(dsar_general_description));
                if (m_role != node.spec()->maybe_annotations()->end())
                    p->second.insert(std::make_pair(n, m_role->value())).first->second = m_role->value();
                else
                    p->second.insert(std::make_pair(n, ""));
            }
            else
                p->second.insert(std::make_pair(n, ""));
        }

        void visit(const PlainTextSpecTree::NodeType<PlainTextLabelDepSpec>::Type & node)
        {
            if (node.spec()->label() == "build_options")
                throw MyOptionsError("Label 'build_options' is not for package use");

            *current_prefix_stack.begin() = ChoicePrefixName(node.spec()->label());
        }

        void visit(const PlainTextSpecTree::NodeType<ConditionalDepSpec>::Type & node)
        {
            current_prefix_stack.push_front(*current_prefix_stack.begin());
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
            current_prefix_stack.pop_front();
        }

        void visit(const PlainTextSpecTree::NodeType<AllDepSpec>::Type & node)
        {
            current_prefix_stack.push_front(*current_prefix_stack.begin());
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
            current_prefix_stack.pop_front();
        }
    };

    std::shared_ptr<ChoiceValue> make_myoption(
            const std::shared_ptr<const ERepositoryID> & id,
            std::shared_ptr<Choice> & choice,
            const UnprefixedChoiceName & unprefixed,
            const std::string & description,
            const Tribool s,
            const bool b)
    {
        return id->make_choice_value(choice, unprefixed, s, false, b, description, false);
    }

    std::string get_maybe_description(const std::shared_ptr<const Map<ChoiceNameWithPrefix, std::string> > & m,
            const ChoiceNameWithPrefix & k)
    {
        if (m)
        {
            Map<ChoiceNameWithPrefix, std::string>::ConstIterator i(m->find(k));
            if (i != m->end())
                return i->second;
        }
        return "";
    }
}

namespace paludis
{
    namespace n
    {
        typedef Name<struct default_value_name> default_value;
        typedef Name<struct implicit_name> implicit;
    }
}

namespace
{
    struct ChoiceOptions
    {
        NamedValue<n::default_value, Tribool> default_value;
        NamedValue<n::implicit, bool> implicit;
    };

    void add_choice_to_map(std::map<ChoiceNameWithPrefix, ChoiceOptions> & values,
           const std::pair<const ChoiceNameWithPrefix, ChoiceOptions> & flag,
           const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > & key)
    {
        std::map<ChoiceNameWithPrefix, ChoiceOptions>::iterator i(values.find(flag.first));
        if (values.end() == i)
            values.insert(flag);
        else
        {
            i->second.implicit() = i->second.implicit() && flag.second.implicit();

            if (! flag.second.default_value().is_indeterminate())
            {
                if (i->second.default_value().is_indeterminate())
                    i->second.default_value() = flag.second.default_value();
                else if (flag.second.default_value().is_true() != i->second.default_value().is_true())
                {
                    Log::get_instance()->message("e.iuse_key.contradiction", ll_warning, lc_context)
                        << "Flag '" << flag.first << "' is both enabled and disabled by default in "
                        << "'" << key->raw_name() << "', using enabled";
                    i->second.default_value() = true;
                }

            }
        }
    }
}

const std::shared_ptr<const Choices>
EChoicesKey::value() const
{
    Lock l(_imp->mutex);
    if (_imp->value)
        return _imp->value;

    Context context("When making Choices key for '" + stringify(*_imp->id) + "':");

    _imp->value = std::make_shared<Choices>();
    if (! _imp->id->eapi()->supported())
        return _imp->value;

    if (_imp->id->raw_myoptions_key())
        populate_myoptions();
    else
        populate_iuse();

    return _imp->value;
}

void
EChoicesKey::populate_myoptions() const
{
    Context local_context("When using raw_myoptions_key to populate choices:");

    std::shared_ptr<Choice> options(std::make_shared<Choice>(make_named_values<ChoiceParams>(
                    n::consider_added_or_changed() = true,
                    n::contains_every_value() = false,
                    n::hidden() = false,
                    n::human_name() = _imp->id->eapi()->supported()->ebuild_environment_variables()->env_use(),
                    n::prefix() = ChoicePrefixName(""),
                    n::raw_name() = _imp->id->eapi()->supported()->ebuild_environment_variables()->env_use(),
                    n::show_with_no_prefix() = true
                )));
    _imp->value->add(options);

    std::shared_ptr<const Set<std::string> > hidden;
    if (_imp->id->raw_use_expand_hidden_key())
        hidden = _imp->id->raw_use_expand_hidden_key()->value();

    /* yay. myoptions is easy. */
    MyOptionsFinder myoptions;
    _imp->id->raw_myoptions_key()->value()->top()->accept(myoptions);

    if (_imp->id->raw_use_expand_key())
        for (Set<std::string>::ConstIterator u(_imp->id->raw_use_expand_key()->value()->begin()),
                u_end(_imp->id->raw_use_expand_key()->value()->end()) ;
                u != u_end ; ++u)
        {
            Context local_local_context("When using raw_use_expand_key value '" + *u + "' to populate choices:");

            std::string lower_u;
            std::transform(u->begin(), u->end(), std::back_inserter(lower_u), &::tolower);
            std::shared_ptr<Choice> exp(std::make_shared<Choice>(make_named_values<ChoiceParams>(
                            n::consider_added_or_changed() = true,
                            n::contains_every_value() = false,
                            n::hidden() = hidden ? hidden->end() != hidden->find(*u) : false,
                            n::human_name() = lower_u,
                            n::prefix() = ChoicePrefixName(lower_u),
                            n::raw_name() = stringify(*u),
                            n::show_with_no_prefix() = false
                        )));
            _imp->value->add(exp);

            MyOptionsFinder::Prefixes::iterator p(myoptions.prefixes.find(ChoicePrefixName(lower_u)));
            if (myoptions.prefixes.end() != p)
            {
                for (MyOptionsFinder::Descriptions::const_iterator v(p->second.begin()), v_end(p->second.end()) ;
                        v != v_end ; ++v)
                    exp->add(make_myoption(_imp->id, exp, v->first, v->second, indeterminate, true));
                myoptions.prefixes.erase(p);
            }
        }

    MyOptionsFinder::Prefixes::iterator p(myoptions.prefixes.find(ChoicePrefixName("")));
    if (myoptions.prefixes.end() != p)
    {
        Context local_local_context("When using empty prefix to populate choices:");
        for (MyOptionsFinder::Descriptions::const_iterator v(p->second.begin()), v_end(p->second.end()) ;
                v != v_end ; ++v)
            options->add(make_myoption(_imp->id, options, v->first, v->second, indeterminate, true));
        myoptions.prefixes.erase(p);
    }

    if (! myoptions.prefixes.empty())
    {
        Log::get_instance()->message("e.myoptions_key.invalid", ll_warning, lc_context) << "Key '" << raw_name() << "' for '"
            << *_imp->id << "' uses unknown prefixes { '" << join(first_iterator(myoptions.prefixes.begin()),
                    first_iterator(myoptions.prefixes.end()), "', '") << "' }";
    }

    _imp->id->add_build_options(_imp->value);
}

void
EChoicesKey::populate_iuse() const
{
    Context local_context("When using raw_iuse_key and raw_use_key to populate choices:");

    std::shared_ptr<Choice> use(std::make_shared<Choice>(make_named_values<ChoiceParams>(
                    n::consider_added_or_changed() = true,
                    n::contains_every_value() = false,
                    n::hidden() = false,
                    n::human_name() = _imp->id->eapi()->supported()->ebuild_environment_variables()->env_use(),
                    n::prefix() = ChoicePrefixName(""),
                    n::raw_name() = _imp->id->eapi()->supported()->ebuild_environment_variables()->env_use(),
                    n::show_with_no_prefix() = true
                )));
    _imp->value->add(use);

    bool has_fancy_test_flag(false);

    std::shared_ptr<const Set<std::string> > hidden;
    if (_imp->id->raw_use_expand_hidden_key())
        hidden = _imp->id->raw_use_expand_hidden_key()->value();

    /* ugh. iuse and all that mess. */

    std::map<ChoiceNameWithPrefix, ChoiceOptions> i_values;
    std::string delim(1, _imp->id->eapi()->supported()->choices_options()->use_expand_separator());

    if (_imp->id->raw_iuse_key())
    {
        std::set<std::string> iuse_sanitised;

        std::map<ChoiceNameWithPrefix, ChoiceOptions> values;

        std::map<std::string, bool> iuse_with_implicit;
        for (Set<std::string>::ConstIterator u(_imp->id->raw_iuse_key()->value()->begin()), u_end(_imp->id->raw_iuse_key()->value()->end()) ;
                u != u_end ; ++u)
            iuse_with_implicit.insert(std::make_pair(*u, false));

        if (_imp->id->raw_iuse_effective_key())
            for (Set<std::string>::ConstIterator u(_imp->id->raw_iuse_effective_key()->value()->begin()),
                    u_end(_imp->id->raw_iuse_effective_key()->value()->end()) ;
                    u != u_end ; ++u)
                iuse_with_implicit.insert(std::make_pair(*u, true));

        for (std::map<std::string, bool>::const_iterator u(iuse_with_implicit.begin()), u_end(iuse_with_implicit.end()) ;
                u != u_end ; ++u)
        {
            std::pair<ChoiceNameWithPrefix, Tribool> flag(parse_iuse(_imp->id->eapi(), u->first));
            std::pair<ChoiceNameWithPrefix, ChoiceOptions> flag_with_options(flag.first, make_named_values<ChoiceOptions>(
                        n::default_value() = flag.second,
                        n::implicit() = u->second
                        ));

            iuse_sanitised.insert(stringify(flag.first));
            if (_imp->id->raw_use_expand_key() &&
                    _imp->id->raw_use_expand_key()->value()->end() != std::find_if(
                        _imp->id->raw_use_expand_key()->value()->begin(),
                        _imp->id->raw_use_expand_key()->value()->end(),
                        IsExpand(flag.first, delim)))
                add_choice_to_map(i_values, flag_with_options, _imp->id->raw_iuse_key());
            else
            {
                if (stringify(flag.first) == _imp->id->eapi()->supported()->choices_options()->fancy_test_flag())
                    /* have to add this right at the end, after build_options is there */
                    has_fancy_test_flag = true;
                else
                    add_choice_to_map(values, flag_with_options, _imp->id->raw_iuse_key());
            }
        }

        for (std::map<ChoiceNameWithPrefix, ChoiceOptions>::const_iterator it(values.begin()),
                 it_end(values.end()) ; it != it_end ; ++it)
        {
            std::shared_ptr<const ChoiceValue> choice(_imp->id->make_choice_value(
                        use, UnprefixedChoiceName(stringify(it->first)), it->second.default_value(), false,
                        ! it->second.implicit(), get_maybe_description(_imp->maybe_descriptions, it->first), false));
            use->add(choice);
        }

        /* pain in the ass: installed packages with DEPEND="x86? ( blah )" need to work,
         * even if x86 isn't listed in IUSE. */
        if (_imp->id->raw_use_key() && ! _imp->id->eapi()->supported()->choices_options()->profile_iuse_injection())
        {
            for (Set<std::string>::ConstIterator u(_imp->id->raw_use_key()->value()->begin()), u_end(_imp->id->raw_use_key()->value()->end()) ;
                    u != u_end ; ++u)
            {
                if (iuse_sanitised.end() != iuse_sanitised.find(*u))
                    continue;

                std::pair<ChoiceNameWithPrefix, Tribool> flag(ChoiceNameWithPrefix("x"), indeterminate);
                if (0 == u->compare(0, 1, "-", 0, 1))
                    flag = std::make_pair(ChoiceNameWithPrefix(u->substr(1)), false);
                else
                    flag = std::make_pair(ChoiceNameWithPrefix(*u), true);

                if (_imp->id->raw_use_expand_key() &&
                        _imp->id->raw_use_expand_key()->value()->end() != std::find_if(
                            _imp->id->raw_use_expand_key()->value()->begin(),
                            _imp->id->raw_use_expand_key()->value()->end(),
                            IsExpand(flag.first, delim)))
                {
                    /* don't need to worry */
                }
                else
                    use->add(_imp->id->make_choice_value(use, UnprefixedChoiceName(stringify(flag.first)), flag.second, false, false,
                                get_maybe_description(_imp->maybe_descriptions, flag.first), false));
            }
        }
    }

    std::string env_arch(_imp->id->eapi()->supported()->ebuild_environment_variables()->env_arch());
    if ((! env_arch.empty()) && _imp->maybe_e_repository && ! _imp->id->eapi()->supported()->ebuild_options()->require_use_expand_in_iuse())
    {
        std::shared_ptr<Choice> arch(std::make_shared<Choice>(make_named_values<ChoiceParams>(
                        n::consider_added_or_changed() = false,
                        n::contains_every_value() = false,
                        n::hidden() = true,
                        n::human_name() = env_arch,
                        n::prefix() = ChoicePrefixName(""),
                        n::raw_name() = env_arch,
                        n::show_with_no_prefix() = false
                    )));
        _imp->value->add(arch);

        for (Set<UnprefixedChoiceName>::ConstIterator a(_imp->maybe_e_repository->arch_flags()->begin()), a_end(_imp->maybe_e_repository->arch_flags()->end()) ;
                a != a_end ; ++a)
            arch->add(_imp->id->make_choice_value(arch, *a, indeterminate, false, false, "", false));
    }

    if (_imp->id->raw_use_expand_key())
    {
        for (Set<std::string>::ConstIterator u(_imp->id->raw_use_expand_key()->value()->begin()),
                u_end(_imp->id->raw_use_expand_key()->value()->end()) ;
                u != u_end ; ++u)
        {
            std::string lower_u;
            std::transform(u->begin(), u->end(), std::back_inserter(lower_u), &::tolower);
            std::shared_ptr<Choice> exp(std::make_shared<Choice>(make_named_values<ChoiceParams>(
                            n::consider_added_or_changed() = true,
                            n::contains_every_value() = ! _imp->id->eapi()->supported()->ebuild_options()->require_use_expand_in_iuse(),
                            n::hidden() = hidden ? hidden->end() != hidden->find(*u) : false,
                            n::human_name() = lower_u,
                            n::prefix() = ChoicePrefixName(lower_u),
                            n::raw_name() = stringify(*u),
                            n::show_with_no_prefix() = false
                            )));
            _imp->value->add(exp);

            std::set<UnprefixedChoiceName> values;

            if (! _imp->id->eapi()->supported()->ebuild_options()->require_use_expand_in_iuse())
            {
                std::shared_ptr<const Set<UnprefixedChoiceName> > e_values(_imp->env->known_choice_value_names(_imp->id, exp));
                std::copy(e_values->begin(), e_values->end(), std::inserter(values, values.begin()));

                if (_imp->maybe_e_repository)
                {
                    std::shared_ptr<const Set<UnprefixedChoiceName> > r_values(
                        _imp->maybe_e_repository->profile()->known_choice_value_names(_imp->id, exp));
                    std::copy(r_values->begin(), r_values->end(), std::inserter(values, values.begin()));
                }

                if (_imp->id->raw_use_key())
                {
                    for (Set<std::string>::ConstIterator it(_imp->id->raw_use_key()->value()->begin()),
                             it_end(_imp->id->raw_use_key()->value()->end()); it_end != it; ++it)
                    {
                        std::string flag(0 == it->compare(0, 1, "-", 0, 1) ? it->substr(1) : *it);
                        if (IsExpand(ChoiceNameWithPrefix(flag), delim)(*u))
                            values.insert(UnprefixedChoiceName(flag.substr(u->length() + delim.length())));
                    }
                }
            }

            for (std::map<ChoiceNameWithPrefix, ChoiceOptions>::const_iterator i(i_values.begin()), i_end(i_values.end()) ;
                    i != i_end ; ++i)
                if (IsExpand(i->first, delim)(*u))
                    values.insert(UnprefixedChoiceName(i->first.value().substr(u->length() + delim.length())));

            for (std::set<UnprefixedChoiceName>::const_iterator v(values.begin()), v_end(values.end()) ;
                    v != v_end ; ++v)
            {
                std::map<ChoiceNameWithPrefix, ChoiceOptions>::const_iterator i(i_values.find(ChoiceNameWithPrefix(lower_u + delim + stringify(*v))));
                if (i_values.end() != i)
                    exp->add(_imp->id->make_choice_value(exp, *v, i->second.default_value(), false, ! i->second.implicit(),
                                get_maybe_description(_imp->maybe_descriptions, i->first), false));
                else
                    exp->add(_imp->id->make_choice_value(exp, *v, indeterminate, false, false, "", false));
            }
        }
    }

    _imp->id->add_build_options(_imp->value);

    if (has_fancy_test_flag)
    {
        std::shared_ptr<const ChoiceValue> choice;

        choice = _imp->value->find_by_name_with_prefix(ELikeRecommendedTestsChoiceValue::canonical_name_with_prefix());
        if (! choice)
            choice = _imp->value->find_by_name_with_prefix(ELikeOptionalTestsChoiceValue::canonical_name_with_prefix());
        if (choice)
            use->add(_imp->id->make_choice_value(use, UnprefixedChoiceName(_imp->id->eapi()->supported()->choices_options()->fancy_test_flag()),
                        choice->enabled(), true, true, "", true));
        else
        {
            std::string name(_imp->id->eapi()->supported()->choices_options()->fancy_test_flag());
            choice = _imp->id->make_choice_value(
                        use, UnprefixedChoiceName(name), indeterminate, true, true,
                        get_maybe_description(_imp->maybe_descriptions, ChoiceNameWithPrefix(name)), false);
            use->add(choice);
        }
    }
}

const std::string
EChoicesKey::raw_name() const
{
    return _imp->raw_name;
}

const std::string
EChoicesKey::human_name() const
{
    return _imp->human_name;
}

MetadataKeyType
EChoicesKey::type() const
{
    return _imp->type;
}

template class Map<ChoiceNameWithPrefix, std::string>;

