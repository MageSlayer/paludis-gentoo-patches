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

#include <paludis/choice.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/wrapped_output_iterator-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/set-impl.hh>
#include <paludis/util/wrapped_value-impl.hh>
#include <list>

using namespace paludis;

typedef std::list<std::tr1::shared_ptr<const Choice> > ChoicesList;
typedef std::list<std::tr1::shared_ptr<const ChoiceValue> > ChoiceList;

namespace paludis
{
    template <>
    struct WrappedForwardIteratorTraits<Choices::ConstIteratorTag>
    {
        typedef ChoicesList::const_iterator UnderlyingIterator;
    };

    template <>
    struct WrappedForwardIteratorTraits<Choice::ConstIteratorTag>
    {
        typedef ChoiceList::const_iterator UnderlyingIterator;
    };
}

ChoicePrefixNameError::ChoicePrefixNameError(const std::string & n) throw () :
    NameError(n, "choice prefix name")
{
}

bool
WrappedValueTraits<ChoicePrefixNameTag>::validate(const std::string & s)
{
    if (! s.empty())
    {
        switch (s.at(s.length() - 1))
        {
            case ':':
            case '_':
                return false;
        };

        switch (s.at(0))
        {
            case ':':
            case '_':
            case '-':
                return false;
        };

        if (s[0] >= 'A' && s[0] <= 'Z')
            return false;

        if (std::string::npos != s.find(" \t\r\n()"))
            return false;
    }

    return true;
}

ChoiceNameWithPrefixError::ChoiceNameWithPrefixError(const std::string & n) throw () :
    NameError(n, "choice name with prefix")
{
}

bool
WrappedValueTraits<ChoiceNameWithPrefixTag>::validate(const std::string & s)
{
    if (s.empty())
        throw ChoiceNameWithPrefixError(s);

    switch (s.at(s.length() - 1))
    {
        case ':':
        case '_':
            return false;
    };

    switch (s.at(0))
    {
        case ':':
        case '_':
        case '-':
            return false;
    };

    if (std::string::npos != s.find(" \t\r\n()"))
        return false;

    return true;
}

UnprefixedChoiceNameError::UnprefixedChoiceNameError(const std::string & n) throw () :
    NameError(n, "unprefixed choice name")
{
}

bool
WrappedValueTraits<UnprefixedChoiceNameTag>::validate(const std::string & s)
{
    if (s.empty())
        return false;

    switch (s.at(s.length() - 1))
    {
        case ':':
        case '_':
            return false;
            break;
    };

    switch (s.at(0))
    {
        case ':':
        case '_':
        case '-':
            return false;
            break;
    };

    if (std::string::npos != s.find(" \t\r\n()"))
        return false;

    return true;
}

namespace paludis
{
    template <>
    struct Implementation<Choices>
    {
        ChoicesList choices;
    };
}

Choices::Choices() :
    PrivateImplementationPattern<Choices>(new Implementation<Choices>)
{
}

Choices::~Choices()
{
}

void
Choices::add(const std::tr1::shared_ptr<const Choice> & c)
{
    _imp->choices.push_back(c);
}

Choices::ConstIterator
Choices::begin() const
{
    return ConstIterator(_imp->choices.begin());
}

Choices::ConstIterator
Choices::end() const
{
    return ConstIterator(_imp->choices.end());
}

Choices::ConstIterator
Choices::find(const ChoicePrefixName & p) const
{
    for (ConstIterator i(begin()), i_end(end()) ;
            i != i_end ; ++i)
        if ((*i)->prefix() == p)
            return i;
    return end();
}

const std::tr1::shared_ptr<const ChoiceValue>
Choices::find_by_name_with_prefix(const ChoiceNameWithPrefix & f) const
{
    for (ConstIterator i(begin()), i_end(end()) ;
            i != i_end ; ++i)
    {
        if (0 != (*i)->prefix().value().compare(0, (*i)->prefix().value().length(), f.value(), 0, (*i)->prefix().value().length()))
            continue;

        for (Choice::ConstIterator j((*i)->begin()), j_end((*i)->end()) ;
                j != j_end ; ++j)
            if ((*j)->name_with_prefix() == f)
                return *j;
    }

    return std::tr1::shared_ptr<const ChoiceValue>();
}

bool
Choices::has_matching_contains_every_value_prefix(const ChoiceNameWithPrefix & f) const
{
    for (ConstIterator i(begin()), i_end(end()) ;
            i != i_end ; ++i)
    {
        if (0 != (*i)->prefix().value().compare(0, (*i)->prefix().value().length(), f.value(), 0, (*i)->prefix().value().length()))
            continue;

        if ((*i)->contains_every_value())
            return true;
    }

    return false;
}

namespace paludis
{
    template <>
    struct Implementation<Choice>
    {
        ChoiceList values;
        const ChoiceParams params;

        Implementation(const ChoiceParams & p) :
            params(p)
        {
        }
    };
}

Choice::Choice(const ChoiceParams & p) :
    PrivateImplementationPattern<Choice>(new Implementation<Choice>(p))
{
}

Choice::~Choice()
{
}

void
Choice::add(const std::tr1::shared_ptr<const ChoiceValue> & v)
{
    _imp->values.push_back(v);
}

const std::string
Choice::raw_name() const
{
    return _imp->params.raw_name();
}

const std::string
Choice::human_name() const
{
    return _imp->params.human_name();
}

bool
Choice::contains_every_value() const
{
    return _imp->params.contains_every_value();
}

bool
Choice::hidden() const
{
    return _imp->params.hidden();
}

bool
Choice::show_with_no_prefix() const
{
    return _imp->params.show_with_no_prefix();
}

bool
Choice::consider_added_or_changed() const
{
    return _imp->params.consider_added_or_changed();
}

const ChoicePrefixName
Choice::prefix() const
{
    return _imp->params.prefix();
}

Choice::ConstIterator
Choice::begin() const
{
    return ConstIterator(_imp->values.begin());
}

Choice::ConstIterator
Choice::end() const
{
    return ConstIterator(_imp->values.end());
}

ChoiceValue::~ChoiceValue()
{
}

template class PrivateImplementationPattern<Choices>;
template class PrivateImplementationPattern<Choice>;

template class WrappedForwardIterator<Choices::ConstIteratorTag, const std::tr1::shared_ptr<const Choice> >;
template class WrappedForwardIterator<Choice::ConstIteratorTag, const std::tr1::shared_ptr<const ChoiceValue> >;

template class WrappedValue<UnprefixedChoiceNameTag>;
template std::ostream & paludis::operator<< (std::ostream &, const WrappedValue<UnprefixedChoiceNameTag> &);

template class WrappedValue<ChoicePrefixNameTag>;
template std::ostream & paludis::operator<< (std::ostream &, const WrappedValue<ChoicePrefixNameTag> &);

template class WrappedValue<ChoiceNameWithPrefixTag>;
template std::ostream & paludis::operator<< (std::ostream &, const WrappedValue<ChoiceNameWithPrefixTag> &);

template class Set<UnprefixedChoiceName>;
template class WrappedForwardIterator<Set<UnprefixedChoiceName>::ConstIteratorTag, const UnprefixedChoiceName>;
template class WrappedOutputIterator<Set<UnprefixedChoiceName>::InserterTag, UnprefixedChoiceName>;

