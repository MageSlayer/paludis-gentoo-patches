/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/wrapped_output_iterator-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/set-impl.hh>
#include <paludis/util/wrapped_value-impl.hh>
#include <list>

using namespace paludis;

#include <paludis/choice-se.cc>

typedef std::list<std::shared_ptr<const Choice> > ChoicesList;
typedef std::list<std::shared_ptr<const ChoiceValue> > ChoiceList;

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

ChoicePrefixNameError::ChoicePrefixNameError(const std::string & n) noexcept :
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
        }

        switch (s.at(0))
        {
            case ':':
            case '_':
            case '-':
                return false;
        }

        static const std::string allowed_chars(
                "abcdefghijklmnopqrstuvwxyz"
                "0123456789-_+");

        if (std::string::npos != s.find_first_not_of(allowed_chars))
            return false;
    }

    return true;
}

ChoiceNameWithPrefixError::ChoiceNameWithPrefixError(const std::string & n) noexcept :
    NameError(n, "choice name with prefix")
{
}

bool
WrappedValueTraits<ChoiceNameWithPrefixTag>::validate(const std::string & s)
{
    if (s.empty())
        return false;

    switch (s.at(s.length() - 1))
    {
        case ':':
        case '_':
            return false;
    }

    switch (s.at(0))
    {
        case ':':
        case '_':
        case '-':
            return false;
    }

    static const std::string allowed_chars(
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "0123456789-_+@:.");

    if (std::string::npos != s.find_first_not_of(allowed_chars))
        return false;

    return true;
}

UnprefixedChoiceNameError::UnprefixedChoiceNameError(const std::string & n) noexcept :
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
    }

    switch (s.at(0))
    {
        case ':':
        case '_':
        case '-':
            return false;
    }

    static const std::string allowed_chars(
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "0123456789-_+@.");

    if (std::string::npos != s.find_first_not_of(allowed_chars))
        return false;

    return true;
}

namespace paludis
{
    template <>
    struct Imp<Choices>
    {
        ChoicesList choices;
    };
}

Choices::Choices() :
    _imp()
{
}

Choices::~Choices() = default;

void
Choices::add(const std::shared_ptr<const Choice> & c)
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

const std::shared_ptr<const ChoiceValue>
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

    return std::shared_ptr<const ChoiceValue>();
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
    struct Imp<Choice>
    {
        ChoiceList values;
        const ChoiceParams params;

        Imp(const ChoiceParams & p) :
            params(p)
        {
        }
    };
}

Choice::Choice(const ChoiceParams & p) :
    _imp(p)
{
}

Choice::~Choice() = default;

void
Choice::add(const std::shared_ptr<const ChoiceValue> & v)
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
Choice::hide_description() const
{
    return _imp->params.hide_description();
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

ChoiceValue::~ChoiceValue() = default;

namespace paludis
{
    template class Pimp<Choices>;
    template class Pimp<Choice>;

    template class WrappedForwardIterator<Choices::ConstIteratorTag, const std::shared_ptr<const Choice> >;
    template class WrappedForwardIterator<Choice::ConstIteratorTag, const std::shared_ptr<const ChoiceValue> >;

    template class WrappedValue<UnprefixedChoiceNameTag>;
    template class WrappedValue<ChoicePrefixNameTag>;
    template class WrappedValue<ChoiceNameWithPrefixTag>;

    template class Set<UnprefixedChoiceName>;
    template class WrappedForwardIterator<Set<UnprefixedChoiceName>::ConstIteratorTag, const UnprefixedChoiceName>;
    template class WrappedOutputIterator<Set<UnprefixedChoiceName>::InserterTag, UnprefixedChoiceName>;
}

template PALUDIS_VISIBLE std::ostream & paludis::operator<< (std::ostream &, const WrappedValue<UnprefixedChoiceNameTag> &);
template PALUDIS_VISIBLE std::ostream & paludis::operator<< (std::ostream &, const WrappedValue<ChoicePrefixNameTag> &);
template PALUDIS_VISIBLE std::ostream & paludis::operator<< (std::ostream &, const WrappedValue<ChoiceNameWithPrefixTag> &);

