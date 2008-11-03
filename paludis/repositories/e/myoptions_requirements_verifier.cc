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

#include <paludis/repositories/e/myoptions_requirements_verifier.hh>
#include <paludis/repositories/e/e_repository_id.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/log.hh>
#include <paludis/util/set.hh>
#include <paludis/choice.hh>
#include <paludis/metadata_key.hh>
#include <algorithm>
#include <list>

using namespace paludis;
using namespace paludis::erepository;

namespace paludis
{
    template <>
    struct Implementation<MyOptionsRequirementsVerifier>
    {
        const std::tr1::shared_ptr<const ERepositoryID> id;

        std::tr1::shared_ptr<Sequence<std::string> > unmet_requirements;
        std::list<ChoicePrefixName> current_prefix_stack;

        Implementation(const std::tr1::shared_ptr<const ERepositoryID> & i) :
            id(i),
            unmet_requirements(new Sequence<std::string>)
        {
            current_prefix_stack.push_front(ChoicePrefixName(""));
        }
    };
}

MyOptionsRequirementsVerifier::MyOptionsRequirementsVerifier(const std::tr1::shared_ptr<const ERepositoryID> & id) :
    PrivateImplementationPattern<MyOptionsRequirementsVerifier>(new Implementation<MyOptionsRequirementsVerifier>(id))
{
}

MyOptionsRequirementsVerifier::~MyOptionsRequirementsVerifier()
{
}

const std::tr1::shared_ptr<const Sequence<std::string> >
MyOptionsRequirementsVerifier::unmet_requirements() const
{
    return _imp->unmet_requirements;
}

void
MyOptionsRequirementsVerifier::visit_leaf(const PlainTextLabelDepSpec & s)
{
    *_imp->current_prefix_stack.begin() = ChoicePrefixName(s.label());
}

void
MyOptionsRequirementsVerifier::visit_leaf(const PlainTextDepSpec & s)
{
    Context context("When verifying requirements for item '" + stringify(s) + "':");

    if (! s.annotations_key())
        return;

    bool active_flag_state(s.text().at(0) != '-');
    ChoiceNameWithPrefix active_flag((
                ! stringify(*_imp->current_prefix_stack.begin()).empty() ? stringify(*_imp->current_prefix_stack.begin()) +
                stringify(_imp->id->eapi()->supported()->choices_options()->use_expand_separator()) : "") +
            stringify(s.text().substr(active_flag_state ? 0 : 1)));

    {
        std::tr1::shared_ptr<const ChoiceValue> choice_value;
        if (_imp->id->choices_key())
            for (Choices::ConstIterator k(_imp->id->choices_key()->value()->begin()),
                    k_end(_imp->id->choices_key()->value()->end()) ;
                    k != k_end && ! choice_value ; ++k)
            {
                if ((*k)->prefix() != *_imp->current_prefix_stack.begin())
                    continue;

                for (Choice::ConstIterator i((*k)->begin()), i_end((*k)->end()) ;
                        i != i_end && ! choice_value ; ++i)
                    if ((*i)->name_with_prefix() == active_flag)
                        choice_value = *i;
            }

        if (choice_value)
        {
            if (choice_value->enabled() != active_flag_state)
                return;
        }
        else
        {
            /* weird ick */
            _imp->unmet_requirements->push_back("Can't work out the state of option '" + stringify(active_flag) + "'");
            return;
        }
    }

    for (MetadataSectionKey::MetadataConstIterator m(s.annotations_key()->begin_metadata()),
            m_end(s.annotations_key()->end_metadata()) ;
            m != m_end ; ++m)
    {
        const MetadataValueKey<std::string> * mm(visitor_cast<const MetadataValueKey<std::string> >(**m));
        if (! mm)
        {
            Log::get_instance()->message("e_key.myoptions.strange_annotation", ll_warning, lc_context)
                << "Don't know how to handle annotation '" << (*m)->raw_name() << "'";
            continue;
        }

        std::string a_key(mm->raw_name()), a_value(mm->value());

        if (a_key == _imp->id->eapi()->supported()->annotations()->myoptions_description())
        {
        }
        else if (a_key == _imp->id->eapi()->supported()->annotations()->myoptions_requires())
        {
            std::list<std::string> tokens;
            tokenise_whitespace(a_value, std::back_inserter(tokens));
            ChoicePrefixName prefix("");
            for (std::list<std::string>::const_iterator t(tokens.begin()), t_end(tokens.end()) ;
                    t != t_end ; ++t)
            {
                if (t->empty())
                    continue;

                if (':' == t->at(t->length() - 1))
                {
                    prefix = ChoicePrefixName(t->substr(0, t->length() - 1));
                    continue;
                }

                bool req_state(true);
                std::string req_flag_s(*t);
                if ('-' == req_flag_s.at(0))
                {
                    req_state = false;
                    req_flag_s.erase(0, 1);
                }

                UnprefixedChoiceName suffix(req_flag_s);

                std::tr1::shared_ptr<const ChoiceValue> choice_value;
                if (_imp->id->choices_key())
                    for (Choices::ConstIterator k(_imp->id->choices_key()->value()->begin()),
                            k_end(_imp->id->choices_key()->value()->end()) ;
                            k != k_end && ! choice_value ; ++k)
                    {
                        if ((*k)->prefix() != prefix)
                            continue;

                        for (Choice::ConstIterator i((*k)->begin()), i_end((*k)->end()) ;
                                i != i_end && ! choice_value ; ++i)
                            if ((*i)->unprefixed_name() == suffix)
                                choice_value = *i;
                    }

                if (choice_value)
                {
                    if (choice_value->enabled() != req_state)
                    {
                        _imp->unmet_requirements->push_back(stringify(active_flag_state ? "Enabling" : "Disabling") +
                                " option '" + stringify(active_flag) + "' requires option '" +
                                stringify(choice_value->name_with_prefix()) + "' to be " +
                                (req_state ? "enabled" : "disabled"));
                    }
                }
                else
                {
                    /* ick */
                    ChoiceNameWithPrefix qualified_flag_name((stringify(prefix).empty() ? "" : stringify(prefix) +
                                stringify(_imp->id->eapi()->supported()->choices_options()->use_expand_separator())) + stringify(suffix));
                    _imp->unmet_requirements->push_back(stringify(active_flag_state ? "Enabling" : "Disabling") +
                            " option '" + stringify(active_flag) + "' requires option '" + stringify(qualified_flag_name) + "' to be " +
                            (req_state ? "enabled" : "disabled") + ", but no such option exists");
                }
            }
        }
        else
            Log::get_instance()->message("e.myoptions_requirements_verifier.unknown_annotation", ll_warning, lc_context)
                << "Unknown annotation '" << a_key << "' = '" << a_value << "' on option '" << s << "'";
    }
}

void
MyOptionsRequirementsVerifier::visit_sequence(const ConditionalDepSpec & spec,
        PlainTextSpecTree::ConstSequenceIterator cur,
        PlainTextSpecTree::ConstSequenceIterator end)
{
    if (spec.condition_met())
    {
        _imp->current_prefix_stack.push_front(*_imp->current_prefix_stack.begin());
        std::for_each(cur, end, accept_visitor(*this));
        _imp->current_prefix_stack.pop_front();
    }
}

void
MyOptionsRequirementsVerifier::visit_sequence(const AllDepSpec &,
        PlainTextSpecTree::ConstSequenceIterator cur,
        PlainTextSpecTree::ConstSequenceIterator end)
{
    _imp->current_prefix_stack.push_front(*_imp->current_prefix_stack.begin());
    std::for_each(cur, end, accept_visitor(*this));
    _imp->current_prefix_stack.pop_front();
}

template class PrivateImplementationPattern<MyOptionsRequirementsVerifier>;

