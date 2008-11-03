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

#include <paludis/choice.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/validated.hh>
#include <paludis/util/set-impl.hh>
#include <list>

using namespace paludis;

ChoicePrefixNameError::ChoicePrefixNameError(const std::string & n) throw () :
    NameError(n, "choice prefix name")
{
}

void
ChoicePrefixNameValidator::validate(const std::string & s)
{
    if (! s.empty())
    {
        switch (s.at(s.length() - 1))
        {
            case ':':
            case '_':
                throw ChoicePrefixNameError(s);
                break;
        };

        switch (s.at(0))
        {
            case ':':
            case '_':
            case '-':
                throw ChoicePrefixNameError(s);
                break;
        };

        if (s[0] >= 'A' && s[0] <= 'Z')
            throw ChoicePrefixNameError(s);

        if (std::string::npos != s.find(" \t\r\n"))
            throw ChoicePrefixNameError(s);
    }
}

ChoiceNameWithPrefixError::ChoiceNameWithPrefixError(const std::string & n) throw () :
    NameError(n, "choice name with prefix")
{
}

void
ChoiceNameWithPrefixValidator::validate(const std::string & s)
{
    if (s.empty())
        throw ChoiceNameWithPrefixError(s);

    switch (s.at(s.length() - 1))
    {
        case ':':
        case '_':
            throw ChoiceNameWithPrefixError(s);
            break;
    };

    switch (s.at(0))
    {
        case ':':
        case '_':
        case '-':
            throw ChoiceNameWithPrefixError(s);
            break;
    };

    if (std::string::npos != s.find(" \t\r\n"))
        throw ChoiceNameWithPrefixError(s);
}

UnprefixedChoiceNameError::UnprefixedChoiceNameError(const std::string & n) throw () :
    NameError(n, "unprefixed choice name")
{
}

void
UnprefixedChoiceNameValidator::validate(const std::string & s)
{
    if (s.empty())
        throw ChoiceNameWithPrefixError(s);

    switch (s.at(s.length() - 1))
    {
        case ':':
        case '_':
            throw ChoiceNameWithPrefixError(s);
            break;
    };

    switch (s.at(0))
    {
        case ':':
        case '_':
        case '-':
            throw ChoiceNameWithPrefixError(s);
            break;
    };

    if (std::string::npos != s.find(" \t\r\n"))
        throw ChoiceNameWithPrefixError(s);
}

namespace paludis
{
    template <>
    struct Implementation<Choices>
    {
        std::list<std::tr1::shared_ptr<const Choice> > choices;
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

const std::tr1::shared_ptr<const ChoiceValue>
Choices::find_by_name_with_prefix(const ChoiceNameWithPrefix & f) const
{
    for (ConstIterator i(begin()), i_end(end()) ;
            i != i_end ; ++i)
    {
        if (0 != (*i)->prefix().data().compare(0, (*i)->prefix().data().length(), f.data(), 0, (*i)->prefix().data().length()))
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
        if (0 != (*i)->prefix().data().compare(0, (*i)->prefix().data().length(), f.data(), 0, (*i)->prefix().data().length()))
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
        std::list<std::tr1::shared_ptr<const ChoiceValue> > values;
        const std::string raw_name;
        const std::string human_name;
        const ChoicePrefixName prefix;
        const bool contains_every_value;
        const bool hidden;
        const bool show_with_no_prefix;
        const bool consider_added_or_changed;

        Implementation(const std::string & r, const std::string & h, const ChoicePrefixName & p,
                const bool c, const bool i, const bool s, const bool a) :
            raw_name(r),
            human_name(h),
            prefix(p),
            contains_every_value(c),
            hidden(i),
            show_with_no_prefix(s),
            consider_added_or_changed(a)
        {
        }
    };
}

Choice::Choice(const std::string & r, const std::string & h, const ChoicePrefixName & p,
        const bool c, const bool i, const bool s, const bool a) :
    PrivateImplementationPattern<Choice>(new Implementation<Choice>(r, h, p, c, i, s, a))
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
    return _imp->raw_name;
}

const std::string
Choice::human_name() const
{
    return _imp->human_name;
}

const bool
Choice::contains_every_value() const
{
    return _imp->contains_every_value;
}

const bool
Choice::hidden() const
{
    return _imp->hidden;
}

const bool
Choice::show_with_no_prefix() const
{
    return _imp->show_with_no_prefix;
}

const bool
Choice::consider_added_or_changed() const
{
    return _imp->consider_added_or_changed;
}

const ChoicePrefixName
Choice::prefix() const
{
    return _imp->prefix;
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

template class Validated<std::string, UnprefixedChoiceNameValidator>;
template class Validated<std::string, ChoicePrefixNameValidator>;
template class Validated<std::string, ChoiceNameWithPrefixValidator>;

template class Set<UnprefixedChoiceName>;
template class WrappedForwardIterator<Set<UnprefixedChoiceName>::ConstIteratorTag, const UnprefixedChoiceName>;

