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

#include <paludis/repositories/e/myoptions_requirements_verifier.hh>
#include <paludis/repositories/e/e_repository_id.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/myoption.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/log.hh>
#include <paludis/util/set.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/choice.hh>
#include <paludis/metadata_key.hh>
#include <algorithm>
#include <list>

using namespace paludis;
using namespace paludis::erepository;

typedef std::list<std::pair<ChoicePrefixName, std::string> > ChildrenList;

namespace paludis
{
    template <>
    struct Imp<MyOptionsRequirementsVerifier>
    {
        const std::shared_ptr<const ERepositoryID> id;

        std::shared_ptr<Sequence<std::string> > unmet_requirements;
        std::list<ChoicePrefixName> current_prefix_stack;
        std::list<ChildrenList> current_children_stack;
        std::list<int> number_enabled_stack;

        Imp(const std::shared_ptr<const ERepositoryID> & i) :
            id(i),
            unmet_requirements(std::make_shared<Sequence<std::string>>())
        {
            current_prefix_stack.push_front(ChoicePrefixName(""));
            current_children_stack.push_front(ChildrenList());
            number_enabled_stack.push_front(0);
        }
    };
}

MyOptionsRequirementsVerifier::MyOptionsRequirementsVerifier(const std::shared_ptr<const ERepositoryID> & id) :
    Pimp<MyOptionsRequirementsVerifier>(id)
{
}

MyOptionsRequirementsVerifier::~MyOptionsRequirementsVerifier()
{
}

const std::shared_ptr<const Sequence<std::string> >
MyOptionsRequirementsVerifier::unmet_requirements() const
{
    return _imp->unmet_requirements;
}

void
MyOptionsRequirementsVerifier::visit(const PlainTextSpecTree::NodeType<PlainTextLabelDepSpec>::Type & node)
{
    *_imp->current_prefix_stack.begin() = ChoicePrefixName(node.spec()->label());
}

namespace
{
    const std::shared_ptr<const ChoiceValue> find_choice_value(
            const std::shared_ptr<const ERepositoryID> & id,
            const ChoicePrefixName & prefix,
            const ChoiceNameWithPrefix & name_with_prefix)
    {
        if (id->choices_key())
            for (Choices::ConstIterator k(id->choices_key()->value()->begin()),
                    k_end(id->choices_key()->value()->end()) ;
                    k != k_end ; ++k)
            {
                if ((*k)->prefix() != prefix)
                    continue;

                for (Choice::ConstIterator i((*k)->begin()), i_end((*k)->end()) ;
                        i != i_end ; ++i)
                    if ((*i)->name_with_prefix() == name_with_prefix)
                        return *i;
            }

        return make_null_shared_ptr();
    }
}

void
MyOptionsRequirementsVerifier::verify_one(const ChoicePrefixName & spec_prefix,
        const std::string & spec_text, const std::shared_ptr<const MetadataSectionKey> & annotations_key)
{
    std::pair<UnprefixedChoiceName, bool> active_myoption(parse_myoption(spec_text));
    ChoiceNameWithPrefix active_flag((
                ! stringify(spec_prefix).empty() ? stringify(spec_prefix) +
                stringify(_imp->id->eapi()->supported()->choices_options()->use_expand_separator()) : "") +
            stringify(active_myoption.first));

    {
        std::shared_ptr<const ChoiceValue> choice_value(find_choice_value(_imp->id, spec_prefix, active_flag));

        if (choice_value)
        {
            if (choice_value->enabled() != active_myoption.second)
                return;
        }
        else
        {
            /* weird ick */
            _imp->unmet_requirements->push_back("Can't work out the state of option '" + stringify(active_flag) + "'");
            return;
        }
    }

    for (MetadataSectionKey::MetadataConstIterator m(annotations_key->begin_metadata()), m_end(annotations_key->end_metadata()) ;
            m != m_end ; ++m)
    {
        const MetadataValueKey<std::string> * mm(simple_visitor_cast<const MetadataValueKey<std::string> >(**m));
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
        else if (a_key == _imp->id->eapi()->supported()->annotations()->myoptions_number_selected())
        {
        }
        else if (a_key == _imp->id->eapi()->supported()->ebuild_options()->bracket_merged_variables_annotation())
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

                std::shared_ptr<const ChoiceValue> choice_value;
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
                        _imp->unmet_requirements->push_back(stringify(active_myoption.second ? "Enabling" : "Disabling") +
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
                    _imp->unmet_requirements->push_back(stringify(active_myoption.second ? "Enabling" : "Disabling") +
                            " option '" + stringify(active_flag) + "' requires option '" + stringify(qualified_flag_name) + "' to be " +
                            (req_state ? "enabled" : "disabled") + ", but no such option exists");
                }
            }
        }
        else
            Log::get_instance()->message("e.myoptions_requirements_verifier.unknown_annotation", ll_warning, lc_context)
                << "Unknown annotation '" << a_key << "' = '" << a_value << "' on option '" << spec_text << "'";
    }
}

void
MyOptionsRequirementsVerifier::visit(const PlainTextSpecTree::NodeType<PlainTextDepSpec>::Type & node)
{
    Context context("When verifying requirements for item '" + stringify(*node.spec()) + "':");

    for (std::list<ChildrenList>::iterator l(_imp->current_children_stack.begin()), l_end(_imp->current_children_stack.end()) ;
            l != l_end ; ++l)
        l->push_back(std::make_pair(*_imp->current_prefix_stack.begin(), node.spec()->text()));

    {
        Context local_context("When finding associated choice:");

        std::pair<UnprefixedChoiceName, bool> active_myoption(parse_myoption(node.spec()->text()));
        ChoiceNameWithPrefix active_flag((
                    ! stringify(*_imp->current_prefix_stack.begin()).empty() ? stringify(*_imp->current_prefix_stack.begin()) +
                    stringify(_imp->id->eapi()->supported()->choices_options()->use_expand_separator()) : "") +
                stringify(active_myoption.first));
        std::shared_ptr<const ChoiceValue> choice_value(find_choice_value(_imp->id, *_imp->current_prefix_stack.begin(), active_flag));

        if (choice_value && choice_value->enabled() == active_myoption.second)
            for (std::list<int>::iterator l(_imp->number_enabled_stack.begin()), l_end(_imp->number_enabled_stack.end()) ;
                    l != l_end ; ++l)
                ++*l;
    }

    if ((! node.spec()->annotations_key()) || (node.spec()->annotations_key()->begin_metadata() == node.spec()->annotations_key()->end_metadata()))
        return;

    verify_one(*_imp->current_prefix_stack.begin(), node.spec()->text(), node.spec()->annotations_key());
}

void
MyOptionsRequirementsVerifier::visit(const PlainTextSpecTree::NodeType<ConditionalDepSpec>::Type & node)
{
    if (node.spec()->condition_met())
    {
        _imp->current_prefix_stack.push_front(*_imp->current_prefix_stack.begin());
        _imp->current_children_stack.push_front(ChildrenList());
        _imp->number_enabled_stack.push_front(0);

        std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));

        _imp->number_enabled_stack.pop_front();
        _imp->current_children_stack.pop_front();
        _imp->current_prefix_stack.pop_front();
    }
}

void
MyOptionsRequirementsVerifier::visit(const PlainTextSpecTree::NodeType<AllDepSpec>::Type & node)
{
    _imp->current_prefix_stack.push_front(*_imp->current_prefix_stack.begin());
    _imp->current_children_stack.push_front(ChildrenList());
    _imp->number_enabled_stack.push_front(0);

    std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
    if (node.spec()->annotations_key() && (node.spec()->annotations_key()->begin_metadata() != node.spec()->annotations_key()->end_metadata()))
    {
        for (ChildrenList::const_iterator i(_imp->current_children_stack.begin()->begin()),
                i_end(_imp->current_children_stack.begin()->end()) ;
                i != i_end ; ++i)
            verify_one(i->first, i->second, node.spec()->annotations_key());

        for (MetadataSectionKey::MetadataConstIterator m(node.spec()->annotations_key()->begin_metadata()), m_end(node.spec()->annotations_key()->end_metadata()) ;
                m != m_end ; ++m)
        {
            const MetadataValueKey<std::string> * mm(simple_visitor_cast<const MetadataValueKey<std::string> >(**m));
            if (! mm)
            {
                Log::get_instance()->message("e_key.myoptions.strange_annotation", ll_warning, lc_context)
                    << "Don't know how to handle annotation '" << (*m)->raw_name() << "'";
                continue;
            }

            std::string a_key(mm->raw_name()), a_value(mm->value());

            if (a_key == _imp->id->eapi()->supported()->annotations()->myoptions_number_selected())
            {
                std::string children_s;
                for (ChildrenList::const_iterator i(_imp->current_children_stack.begin()->begin()),
                        i_end(_imp->current_children_stack.begin()->end()) ;
                        i != i_end ; ++i)
                {
                    if (! children_s.empty())
                        children_s.append(", ");

                    if (! stringify(i->first).empty())
                    {
                        children_s.append(stringify(i->first));
                        children_s.append(stringify(_imp->id->eapi()->supported()->choices_options()->use_expand_separator()));
                    }
                    children_s.append(stringify(i->second));
                }

                children_s = "( " + children_s + " )";

                if (a_value == _imp->id->eapi()->supported()->annotations()->myoptions_number_selected_at_least_one())
                {
                    if (*_imp->number_enabled_stack.begin() < 1)
                        _imp->unmet_requirements->push_back("At least one of options " + children_s + " must be met");
                }
                else if (a_value == _imp->id->eapi()->supported()->annotations()->myoptions_number_selected_at_most_one())
                {
                    if (*_imp->number_enabled_stack.begin() > 1)
                        _imp->unmet_requirements->push_back("At most one of options " + children_s + " must be met");
                }
                else if (a_value == _imp->id->eapi()->supported()->annotations()->myoptions_number_selected_exactly_one())
                {
                    if (*_imp->number_enabled_stack.begin() != 1)
                        _imp->unmet_requirements->push_back("Exactly one of options " + children_s + " must be met");
                }
                else
                    _imp->unmet_requirements->push_back("Don't know what '" + stringify(a_value) + "' means");
            }
        }
    }
    _imp->number_enabled_stack.pop_front();
    _imp->current_children_stack.pop_front();
    _imp->current_prefix_stack.pop_front();
}

template class Pimp<MyOptionsRequirementsVerifier>;

