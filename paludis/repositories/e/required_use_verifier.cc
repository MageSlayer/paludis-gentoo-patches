/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#include <paludis/repositories/e/required_use_verifier.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/save.hh>
#include <paludis/util/log.hh>
#include <paludis/action.hh>
#include <paludis/metadata_key.hh>
#include <paludis/choice.hh>
#include <paludis/unformatted_pretty_printer.hh>
#include <list>
#include <algorithm>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    struct Met
    {
        int number_met;
        bool any_unmet;
    };
}

namespace paludis
{
    template <>
    struct Imp<RequiredUseVerifier>
    {
        const Environment * const env;
        const std::shared_ptr<const ERepositoryID> id;
        std::shared_ptr<Sequence<std::string> > unmet_requirements;

        std::list<Met> stack;
        bool top;

        Imp(
                const Environment * const e,
                const std::shared_ptr<const ERepositoryID> & i) :
            env(e),
            id(i),
            unmet_requirements(std::make_shared<Sequence<std::string>>()),
            top(true)
        {
            stack.push_front(Met{0, false});
        }
    };
}

RequiredUseVerifier::RequiredUseVerifier(
        const Environment * const e,
        const std::shared_ptr<const ERepositoryID> & id) :
    Pimp<RequiredUseVerifier>(e, id)
{
}

RequiredUseVerifier::~RequiredUseVerifier() = default;

bool
RequiredUseVerifier::matches(const std::string & s)
{
    if (s.empty())
        throw ActionFailedError("Could not verify empty use requirement");

    if ('!' == s.at(0))
        return ! matches(s.substr(1));

    if (! _imp->id->choices_key())
    {
        Log::get_instance()->message("e.required_use.no_choices", ll_warning, lc_context)
            << "ID '" << *_imp->id << "' has no choices, so cannot check that required use constraint '" << s << "' matches";
        return false;
    }

    auto c(_imp->id->choices_key()->value()->find_by_name_with_prefix(ChoiceNameWithPrefix(s)));
    if (! c)
    {
        Log::get_instance()->message("e.required_use.no_choice", ll_warning, lc_context)
            << "ID '" << *_imp->id << "' has no choice named '" << s << "'', so cannot check that required use constraint '" << s << "' matches";
        return false;
    }

    return c->enabled();
}

void
RequiredUseVerifier::visit(const RequiredUseSpecTree::NodeType<PlainTextDepSpec>::Type & node)
{
    if (matches(node.spec()->text()))
        ++_imp->stack.begin()->number_met;
    else
        _imp->stack.begin()->any_unmet = true;
}

void
RequiredUseVerifier::visit(const RequiredUseSpecTree::NodeType<AllDepSpec>::Type & node)
{
    _imp->stack.push_front(Met{0, false});
    {
        Save<bool> top(&_imp->top, false);
        std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
    }

    Met met(*_imp->stack.begin());
    _imp->stack.pop_front();

    if (met.any_unmet)
        _imp->stack.begin()->any_unmet = true;
    else
        ++_imp->stack.begin()->number_met;

    if (_imp->top)
    {
        if (_imp->stack.begin()->any_unmet)
            _imp->unmet_requirements->push_back(_imp->id->required_use_key()->pretty_print_value(UnformattedPrettyPrinter(), { }));
    }
}

void
RequiredUseVerifier::visit(const RequiredUseSpecTree::NodeType<AnyDepSpec>::Type & node)
{
    _imp->stack.push_front(Met{0, false});
    std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));

    Met met(*_imp->stack.begin());
    _imp->stack.pop_front();

    if (met.number_met > 0)
        ++_imp->stack.begin()->number_met;
    else if (met.any_unmet)
        _imp->stack.begin()->any_unmet = true;
    else
    {
        /* || ( disabled? ( bar ) ) and || ( ) are true. yay Portage! */
        ++_imp->stack.begin()->number_met;
    }
}

void
RequiredUseVerifier::visit(const RequiredUseSpecTree::NodeType<ExactlyOneDepSpec>::Type & node)
{
    _imp->stack.push_front(Met{0, false});
    std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));

    Met met(*_imp->stack.begin());
    _imp->stack.pop_front();

    if (met.number_met == 1)
        ++_imp->stack.begin()->number_met;
    else if (met.number_met > 1)
        _imp->stack.begin()->any_unmet = true;
    else if (met.any_unmet)
        _imp->stack.begin()->any_unmet = true;
    else
        ++_imp->stack.begin()->number_met;
}

void
RequiredUseVerifier::visit(const RequiredUseSpecTree::NodeType<ConditionalDepSpec>::Type & node)
{
    if (! node.spec()->condition_met(_imp->env, _imp->id))
        return;

    _imp->stack.push_front(Met{0, false});
    std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));

    Met met(*_imp->stack.begin());
    _imp->stack.pop_front();

    if (met.any_unmet)
        _imp->stack.begin()->any_unmet = true;
    else
        ++_imp->stack.begin()->number_met;
}

const std::shared_ptr<const Sequence<std::string> >
RequiredUseVerifier::unmet_requirements() const
{
    return _imp->unmet_requirements;
}

template class Pimp<RequiredUseVerifier>;

