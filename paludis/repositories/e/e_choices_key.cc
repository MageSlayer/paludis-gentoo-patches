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

#include <paludis/repositories/e/e_choices_key.hh>
#include <paludis/repositories/e/e_key.hh>
#include <paludis/repositories/e/ebuild_id.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/dep_spec_pretty_printer.hh>
#include <paludis/repositories/e/e_repository_profile.hh>
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/e/myoption.hh>

#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/log.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/join.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/tribool.hh>
#include <paludis/util/member_iterator-impl.hh>
#include <paludis/util/map.hh>

#include <paludis/environment.hh>
#include <paludis/stringify_formatter-impl.hh>
#include <paludis/choice.hh>
#include <paludis/elike_choices.hh>

#include <algorithm>
#include <set>
#include <map>

using namespace paludis;
using namespace paludis::erepository;

namespace paludis
{
    template <>
    struct Implementation<EChoicesKey>
    {
        mutable Mutex mutex;
        mutable std::tr1::shared_ptr<Choices> value;

        const Environment * const env;
        const std::tr1::shared_ptr<const ERepositoryID> id;
        const std::tr1::shared_ptr<const ERepository> maybe_e_repository;
        const std::tr1::shared_ptr<const Map<ChoiceNameWithPrefix, std::string> > maybe_descriptions;

        Implementation(const Environment * const e, const std::tr1::shared_ptr<const ERepositoryID> & i,
                const std::tr1::shared_ptr<const ERepository> & p,
                const std::tr1::shared_ptr<const Map<ChoiceNameWithPrefix, std::string> > & d) :
            env(e),
            id(i),
            maybe_e_repository(p),
            maybe_descriptions(d)
        {
        }
    };
}

EChoicesKey::EChoicesKey(
        const Environment * const e,
        const std::tr1::shared_ptr<const ERepositoryID> & i,
        const std::string & r, const std::string & h, const MetadataKeyType t,
        const std::tr1::shared_ptr<const ERepository> & p,
        const std::tr1::shared_ptr<const Map<ChoiceNameWithPrefix, std::string> > & d) :
    MetadataValueKey<std::tr1::shared_ptr<const Choices> > (r, h, t),
    PrivateImplementationPattern<EChoicesKey>(new Implementation<EChoicesKey>(e, i, p, d)),
    _imp(PrivateImplementationPattern<EChoicesKey>::_imp)
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
            return (0 == flag.data().compare(0, lower_s.length(), lower_s, 0, lower_s.length()));
        }
    };

    struct MyOptionsFinder :
        ConstVisitor<PlainTextSpecTree>
    {
        typedef std::map<std::string, std::string> Annotations;
        typedef std::map<UnprefixedChoiceName, Annotations> Values;
        typedef std::map<ChoicePrefixName, Values> Prefixes;

        Prefixes prefixes;
        std::list<ChoicePrefixName> current_prefix_stack;

        MyOptionsFinder()
        {
            current_prefix_stack.push_front(ChoicePrefixName(""));
        }

        void visit_leaf(const PlainTextDepSpec & s)
        {
            if (s.text().empty())
                return;

            Context context("When handling item '" + stringify(s) + "':");

            Prefixes::iterator p(prefixes.find(*current_prefix_stack.begin()));
            if (p == prefixes.end())
                p = prefixes.insert(std::make_pair(*current_prefix_stack.begin(), Values())).first;

            UnprefixedChoiceName n(parse_myoption(s.text()).first);
            Values::iterator v(p->second.find(n));
            if (v == p->second.end())
                v = p->second.insert(std::make_pair(n, Annotations())).first;

            if (s.annotations_key())
            {
                for (MetadataSectionKey::MetadataConstIterator m(s.annotations_key()->begin_metadata()),
                        m_end(s.annotations_key()->end_metadata()) ;
                        m != m_end ; ++m)
                {
                    const MetadataValueKey<std::string> * mm(simple_visitor_cast<const MetadataValueKey<std::string> >(**m));
                    if (! mm)
                    {
                        Log::get_instance()->message("e_key.myoptions.strange_annotation", ll_warning, lc_context)
                            << "Don't know how to handle annotation '" << (*m)->raw_name() << "'";
                        continue;
                    }

                    Annotations::iterator a(v->second.find(mm->raw_name()));
                    v->second.insert(make_pair(mm->raw_name(), mm->value()));
                }
            }
        }

        void visit_leaf(const PlainTextLabelDepSpec & s)
        {
            *current_prefix_stack.begin() = ChoicePrefixName(s.label());
        }

        void visit_sequence(const ConditionalDepSpec &,
                PlainTextSpecTree::ConstSequenceIterator cur,
                PlainTextSpecTree::ConstSequenceIterator end)
        {
            current_prefix_stack.push_front(*current_prefix_stack.begin());
            std::for_each(cur, end, accept_visitor(*this));
            current_prefix_stack.pop_front();
        }

        void visit_sequence(const AllDepSpec &,
                PlainTextSpecTree::ConstSequenceIterator cur,
                PlainTextSpecTree::ConstSequenceIterator end)
        {
            current_prefix_stack.push_front(*current_prefix_stack.begin());
            std::for_each(cur, end, accept_visitor(*this));
            current_prefix_stack.pop_front();
        }
    };

    std::tr1::shared_ptr<ChoiceValue> make_myoption(
            const std::tr1::shared_ptr<const ERepositoryID> & id,
            std::tr1::shared_ptr<Choice> & choice,
            MyOptionsFinder::Values::const_iterator & v,
            const Tribool s,
            const bool b)
    {
        std::string description;
        for (MyOptionsFinder::Annotations::const_iterator a(v->second.begin()), a_end(v->second.end()) ;
                a != a_end ; ++a)
        {
            if (a->first == id->eapi()->supported()->annotations()->myoptions_description())
                description = a->second;
        }
        return id->make_choice_value(choice, v->first, s, b, description, false);
    }

    std::string get_maybe_description(const std::tr1::shared_ptr<const Map<ChoiceNameWithPrefix, std::string> > & m,
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

const std::tr1::shared_ptr<const Choices>
EChoicesKey::value() const
{
    Lock l(_imp->mutex);
    if (_imp->value)
        return _imp->value;

    Context context("When making Choices key for '" + stringify(*_imp->id) + "':");

    _imp->value.reset(new Choices);
    if (! _imp->id->eapi()->supported())
        return _imp->value;

    std::tr1::shared_ptr<Choice> use(new Choice(
                _imp->id->eapi()->supported()->ebuild_environment_variables()->env_use(),
                _imp->id->eapi()->supported()->ebuild_environment_variables()->env_use(),
                ChoicePrefixName(""),
                false,
                false,
                true,
                true));
    _imp->value->add(use);

    bool has_fancy_test_flag(false);
    std::tr1::shared_ptr<const ChoiceValue> unfancy_test_choice;

    std::tr1::shared_ptr<const Set<std::string> > hidden;
    if (_imp->id->raw_use_expand_hidden_key())
        hidden = _imp->id->raw_use_expand_hidden_key()->value();

    if (_imp->id->raw_myoptions_key())
    {
        Context local_context("When using raw_myoptions_key to populate choices:");

        /* yay. myoptions is easy. */
        MyOptionsFinder myoptions;
        _imp->id->raw_myoptions_key()->value()->accept(myoptions);

        if (_imp->id->raw_use_expand_key())
            for (Set<std::string>::ConstIterator u(_imp->id->raw_use_expand_key()->value()->begin()),
                    u_end(_imp->id->raw_use_expand_key()->value()->end()) ;
                    u != u_end ; ++u)
            {
                Context local_local_context("When using raw_use_expand_key value '" + *u + "' to populate choices:");

                std::string lower_u;
                std::transform(u->begin(), u->end(), std::back_inserter(lower_u), &::tolower);
                std::tr1::shared_ptr<Choice> exp(new Choice(stringify(*u), lower_u, ChoicePrefixName(lower_u),
                            false, hidden ? hidden->end() != hidden->find(*u) : false, false, true));
                _imp->value->add(exp);

                MyOptionsFinder::Prefixes::iterator p(myoptions.prefixes.find(ChoicePrefixName(lower_u)));
                if (myoptions.prefixes.end() != p)
                {
                    for (MyOptionsFinder::Values::const_iterator v(p->second.begin()), v_end(p->second.end()) ;
                            v != v_end ; ++v)
                        exp->add(make_myoption(_imp->id, exp, v, indeterminate, true));
                    myoptions.prefixes.erase(p);
                }
            }

        MyOptionsFinder::Prefixes::iterator p(myoptions.prefixes.find(ChoicePrefixName("")));
        if (myoptions.prefixes.end() != p)
        {
            Context local_local_context("When using empty prefix to populate choices:");
            for (MyOptionsFinder::Values::const_iterator v(p->second.begin()), v_end(p->second.end()) ;
                    v != v_end ; ++v)
                use->add(make_myoption(_imp->id, use, v, indeterminate, true));
            myoptions.prefixes.erase(p);
        }

        if (! myoptions.prefixes.empty())
        {
            Log::get_instance()->message("e.myoptions_key.invalid", ll_warning, lc_context) << "Key '" << raw_name() << "' for '"
                << *_imp->id << "' uses unknown prefixes { '" << join(first_iterator(myoptions.prefixes.begin()),
                        first_iterator(myoptions.prefixes.end()), "', '") << "' }";
        }
    }
    else
    {
        /* ugh. iuse and all that mess. */
        Context local_context("When using raw_iuse_key and raw_use_key to populate choices:");

        std::map<ChoiceNameWithPrefix, Tribool> i_values;
        std::string delim(1, _imp->id->eapi()->supported()->choices_options()->use_expand_separator());

        if (_imp->id->raw_iuse_key())
        {
            for (Set<std::string>::ConstIterator u(_imp->id->raw_iuse_key()->value()->begin()), u_end(_imp->id->raw_iuse_key()->value()->end()) ;
                    u != u_end ; ++u)
            {
                std::pair<ChoiceNameWithPrefix, Tribool> flag(parse_iuse(_imp->id->eapi(), *u));
                if (_imp->id->raw_use_expand_key() &&
                        _imp->id->raw_use_expand_key()->value()->end() != std::find_if(
                            _imp->id->raw_use_expand_key()->value()->begin(),
                            _imp->id->raw_use_expand_key()->value()->end(),
                            IsExpand(flag.first, delim)))
                    i_values.insert(flag);
                else
                {
                    std::tr1::shared_ptr<const ChoiceValue> choice(_imp->id->make_choice_value(
                                use, UnprefixedChoiceName(stringify(flag.first)), flag.second, true,
                                get_maybe_description(_imp->maybe_descriptions, flag.first), false));
                    if (stringify(flag.first) == _imp->id->eapi()->supported()->choices_options()->fancy_test_flag())
                    {
                        /* have to add this right at the end, after build_options is there */
                        has_fancy_test_flag = true;
                        unfancy_test_choice = choice;
                    }
                    else
                        use->add(choice);
                }
            }

            /* pain in the ass: installed packages with DEPEND="x86? ( blah )" need to work,
             * even if x86 isn't listed in IUSE. */
            if (_imp->id->raw_use_key())
            {
                for (Set<std::string>::ConstIterator u(_imp->id->raw_use_key()->value()->begin()), u_end(_imp->id->raw_use_key()->value()->end()) ;
                        u != u_end ; ++u)
                {
                    if (_imp->id->raw_iuse_key()->value()->end() != _imp->id->raw_iuse_key()->value()->find(*u))
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
                        use->add(_imp->id->make_choice_value(use, UnprefixedChoiceName(stringify(flag.first)), flag.second, false,
                                    get_maybe_description(_imp->maybe_descriptions, flag.first), false));
                }
            }
        }

        std::string env_arch(_imp->id->eapi()->supported()->ebuild_environment_variables()->env_arch());
        if ((! env_arch.empty()) && _imp->maybe_e_repository)
        {
            std::tr1::shared_ptr<Choice> arch(new Choice(env_arch, env_arch, ChoicePrefixName(""), false, true, false, false));
            _imp->value->add(arch);

            for (Set<UnprefixedChoiceName>::ConstIterator a(_imp->maybe_e_repository->arch_flags()->begin()), a_end(_imp->maybe_e_repository->arch_flags()->end()) ;
                    a != a_end ; ++a)
                arch->add(_imp->id->make_choice_value(arch, *a, indeterminate, false, "", false));
        }

        if (_imp->id->raw_use_expand_key())
        {
            for (Set<std::string>::ConstIterator u(_imp->id->raw_use_expand_key()->value()->begin()),
                    u_end(_imp->id->raw_use_expand_key()->value()->end()) ;
                    u != u_end ; ++u)
            {
                std::string lower_u;
                std::transform(u->begin(), u->end(), std::back_inserter(lower_u), &::tolower);
                std::tr1::shared_ptr<Choice> exp(new Choice(stringify(*u), lower_u, ChoicePrefixName(lower_u),
                            ! _imp->id->eapi()->supported()->ebuild_options()->require_use_expand_in_iuse(),
                            hidden ? hidden->end() != hidden->find(*u) : false, false, true));
                _imp->value->add(exp);

                std::set<UnprefixedChoiceName> values;

                if (! _imp->id->eapi()->supported()->ebuild_options()->require_use_expand_in_iuse())
                {
                    std::tr1::shared_ptr<const Set<UnprefixedChoiceName> > e_values(_imp->env->known_choice_value_names(_imp->id, exp));
                    std::copy(e_values->begin(), e_values->end(), std::inserter(values, values.begin()));

                    if (_imp->maybe_e_repository)
                    {
                        std::tr1::shared_ptr<const Set<UnprefixedChoiceName> > r_values(
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

                for (std::map<ChoiceNameWithPrefix, Tribool>::const_iterator i(i_values.begin()), i_end(i_values.end()) ;
                        i != i_end ; ++i)
                    if (IsExpand(i->first, delim)(*u))
                        values.insert(UnprefixedChoiceName(i->first.data().substr(u->length() + delim.length())));

                for (std::set<UnprefixedChoiceName>::const_iterator v(values.begin()), v_end(values.end()) ;
                        v != v_end ; ++v)
                {
                    std::map<ChoiceNameWithPrefix, Tribool>::const_iterator i(i_values.find(ChoiceNameWithPrefix(lower_u + delim + stringify(*v))));
                    if (i_values.end() != i)
                        exp->add(_imp->id->make_choice_value(exp, *v, i->second, true,
                                    get_maybe_description(_imp->maybe_descriptions, i->first), false));
                    else
                        exp->add(_imp->id->make_choice_value(exp, *v, indeterminate, false, "", false));
                }
            }
        }
    }

    _imp->id->add_build_options(_imp->value);

    if (has_fancy_test_flag)
    {
        std::tr1::shared_ptr<const ChoiceValue> choice;

        choice = _imp->value->find_by_name_with_prefix(ELikeRecommendedTestsChoiceValue::canonical_name_with_prefix());
        if (! choice)
            choice = _imp->value->find_by_name_with_prefix(ELikeOptionalTestsChoiceValue::canonical_name_with_prefix());
        if (choice)
            use->add(_imp->id->make_choice_value(use, UnprefixedChoiceName(_imp->id->eapi()->supported()->choices_options()->fancy_test_flag()),
                        choice->enabled(), true, "", true));
        else if (unfancy_test_choice)
            use->add(unfancy_test_choice);
    }

    return _imp->value;
}

