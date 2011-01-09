/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/args/args_option.hh>
#include <paludis/args/args_error.hh>
#include <paludis/args/args_group.hh>
#include <paludis/args/args_section.hh>
#include <paludis/args/args_handler.hh>
#include <paludis/args/escape.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/join.hh>
#include <set>
#include <vector>
#include <algorithm>
#include <list>

using namespace paludis;
using namespace paludis::args;

namespace
{
    struct ArgIs
    {
        const std::string arg;

        ArgIs(const std::string & a) :
            arg(a)
        {
        }

        bool operator() (const std::pair<std::string, std::string> & p) const
        {
            return p.first == arg;
        }

        bool operator() (const AllowedEnumArg & p) const
        {
            return p.long_name() == arg || (p.short_name() && std::string(1, p.short_name()) == arg);
        }
    };
}

ArgsOption::ArgsOption(ArgsGroup * const g, const std::string & our_long_name,
        const char our_short_name, const std::string & our_description) :
    _group(g),
    _long_name(our_long_name),
    _short_name(our_short_name),
    _description(our_description),
    _specified(false)
{
    g->add(this);
    g->section()->handler()->add_option(this, our_long_name, our_short_name);
}

ArgsOption::~ArgsOption()
{
}

void
ArgsOption::remove()
{
    _group->remove(this);
    _group->section()->handler()->remove_option(_long_name, _short_name);
}

SwitchArg::SwitchArg(ArgsGroup * const our_group, const std::string & our_long_name, char our_short_name,
        const std::string & our_description, const bool c) :
    ArgsOption(our_group, our_long_name, our_short_name, our_description),
    _can_be_negated(c)
{
}

SwitchArg::~SwitchArg()
{
}

const std::string
SwitchArg::forwardable_string() const
{
    if (specified())
        return "--" + long_name();
    else
        return "";
}

const std::shared_ptr<Sequence<std::string> >
SwitchArg::forwardable_args() const
{
    std::shared_ptr<Sequence<std::string> > result(std::make_shared<Sequence<std::string>>());
    if (specified())
        result->push_back("--" + long_name());
    return result;
}

AliasArg::AliasArg(ArgsOption * const o, const std::string & our_long_name, bool is_hidden) :
    ArgsOption(o->group(), our_long_name, '\0', "Alias for --" + o->long_name()),
    _other(o), _hidden(is_hidden)
{
}

const std::string
AliasArg::forwardable_string() const
{
    return "";
}

const std::shared_ptr<Sequence<std::string> >
AliasArg::forwardable_args() const
{
    std::shared_ptr<Sequence<std::string> > result(std::make_shared<Sequence<std::string>>());
    return result;
}

StringArg::StringArg(ArgsGroup * const g, const std::string & our_long_name,
        const char our_short_name, const std::string & our_description) :
    ArgsOption(g, our_long_name, our_short_name, our_description),
    _validator(0)
{
}

StringArg::StringArg(ArgsGroup * const g, const std::string & our_long_name,
        const char our_short_name, const std::string & our_description,
        void (* v) (const std::string &)) :
    ArgsOption(g, our_long_name, our_short_name, our_description),
    _validator(v)
{
}

const std::string
StringArg::forwardable_string() const
{
    if (specified())
        return "--" + long_name() + " " + escape(argument());
    else
        return "";
}

const std::shared_ptr<Sequence<std::string> >
StringArg::forwardable_args() const
{
    std::shared_ptr<Sequence<std::string> > result(std::make_shared<Sequence<std::string>>());
    if (specified())
    {
        result->push_back("--" + long_name());
        result->push_back(argument());
    }
    return result;
}

namespace paludis
{
    template<>
    struct Imp<StringSetArg>
    {
        std::set<std::string> args;
        std::vector<std::pair<std::string, std::string> > allowed_args;
    };

    template<>
    struct Imp<StringSetArg::StringSetArgOptions>
    {
        std::vector<std::pair<std::string, std::string> > options;
    };

    template <>
    struct WrappedForwardIteratorTraits<StringSetArg::ConstIteratorTag>
    {
        typedef std::set<std::string>::const_iterator UnderlyingIterator;
    };

    template <>
    struct WrappedForwardIteratorTraits<StringSetArg::AllowedArgConstIteratorTag>
    {
        typedef std::vector<std::pair<std::string, std::string> >::const_iterator UnderlyingIterator;
    };
}

StringSetArg::StringSetArg(ArgsGroup * const g, const std::string & our_long_name,
        const char our_short_name, const std::string & our_description,
        const StringSetArgOptions & opts) :
    ArgsOption(g, our_long_name, our_short_name, our_description),
    _imp(),
    _validator(0)
{
    std::copy(opts._imp->options.begin(), opts._imp->options.end(),
            std::back_inserter(_imp->allowed_args));
}

StringSetArg::StringSetArg(ArgsGroup * const g, const std::string & our_long_name,
        const char our_short_name, const std::string & our_description,
        const StringSetArgOptions & opts, void (* v) (const std::string &)) :
    ArgsOption(g, our_long_name, our_short_name, our_description),
    _imp(),
    _validator(v)
{
    std::copy(opts._imp->options.begin(), opts._imp->options.end(),
            std::back_inserter(_imp->allowed_args));
}

StringSetArg::ConstIterator
StringSetArg::begin_args() const
{
    return ConstIterator(_imp->args.begin());
}

StringSetArg::ConstIterator
StringSetArg::end_args() const
{
    return ConstIterator(_imp->args.end());
}

void
StringSetArg::add_argument(const std::string & arg)
{
    Context context("When handling argument '" + arg + "' for '--" + long_name() + "':");

    if (! _imp->allowed_args.empty())
        if (_imp->allowed_args.end() == std::find_if(_imp->allowed_args.begin(),
                    _imp->allowed_args.end(), ArgIs(arg)))
            throw (BadValue("--" + long_name(), arg));

    if (_validator)
        (*_validator)(arg);

    _imp->args.insert(arg);
}

const std::string
StringSetArg::forwardable_string() const
{
    if (specified())
        return "--" + long_name() + " " + join(begin_args(), end_args(), " --" + long_name() + " ",
                &escape);
    else
        return "";
}

const std::shared_ptr<Sequence<std::string> >
StringSetArg::forwardable_args() const
{
    std::shared_ptr<Sequence<std::string> > result(std::make_shared<Sequence<std::string>>());
    if (specified())
    {
        for (ConstIterator i(begin_args()), i_end(end_args()) ;
                i != i_end ; ++i)
        {
            result->push_back("--" + long_name());
            result->push_back(*i);
        }
    }
    return result;
}

namespace paludis
{
    template<>
    struct Imp<StringSequenceArg>
    {
        std::list<std::string> args;
    };

    template <>
    struct WrappedForwardIteratorTraits<StringSequenceArg::ConstIteratorTag>
    {
        typedef std::list<std::string>::const_iterator UnderlyingIterator;
    };
}

StringSequenceArg::StringSequenceArg(ArgsGroup * const g, const std::string & our_long_name,
        const char our_short_name, const std::string & our_description) :
    ArgsOption(g, our_long_name, our_short_name, our_description),
    _imp()
{
}

StringSequenceArg::~StringSequenceArg()
{
}

StringSequenceArg::ConstIterator
StringSequenceArg::begin_args() const
{
    return ConstIterator(_imp->args.begin());
}

StringSequenceArg::ConstIterator
StringSequenceArg::end_args() const
{
    return ConstIterator(_imp->args.end());
}

void
StringSequenceArg::add_argument(const std::string & arg)
{
    Context context("When handling argument '" + arg + "' for '--" + long_name() + "':");
    _imp->args.push_back(arg);
}

const std::string
StringSequenceArg::forwardable_string() const
{
    if (specified())
        return "--" + long_name() + " " + join(begin_args(), end_args(), " --" + long_name() + " ",
                &escape);
    else
        return "";
}

const std::shared_ptr<Sequence<std::string> >
StringSequenceArg::forwardable_args() const
{
    std::shared_ptr<Sequence<std::string> > result(std::make_shared<Sequence<std::string>>());
    if (specified())
    {
        for (ConstIterator i(begin_args()), i_end(end_args()) ;
                i != i_end ; ++i)
        {
            result->push_back("--" + long_name());
            result->push_back(*i);
        }
    }
    return result;
}

IntegerArg::IntegerArg(ArgsGroup * const our_group, const std::string & our_long_name,
                char our_short_name, const std::string & our_description) :
    ArgsOption(our_group, our_long_name, our_short_name, our_description)
{
}

const std::string
IntegerArg::forwardable_string() const
{
    if (specified())
        return "--" + long_name() + " " + stringify(argument());
    else
        return "";
}

const std::shared_ptr<Sequence<std::string> >
IntegerArg::forwardable_args() const
{
    std::shared_ptr<Sequence<std::string> > result(std::make_shared<Sequence<std::string>>());
    if (specified())
    {
        result->push_back("--" + long_name());
        result->push_back(stringify(argument()));
    }
    return result;
}

namespace paludis
{
    template<>
    struct Imp<EnumArg>
    {
        std::vector<AllowedEnumArg> allowed_args;
    };

    template<>
    struct Imp<EnumArg::EnumArgOptions>
    {
        std::vector<AllowedEnumArg> options;
    };

    template <>
    struct WrappedForwardIteratorTraits<EnumArg::AllowedArgConstIteratorTag>
    {
        typedef std::vector<AllowedEnumArg>::const_iterator UnderlyingIterator;
    };
}

StringSetArg::StringSetArgOptions::StringSetArgOptions(const std::string & opt, const std::string & desc) :
    _imp()
{
    _imp->options.push_back(std::make_pair(opt, desc));
}

StringSetArg::StringSetArgOptions &
StringSetArg::StringSetArgOptions::operator() (const std::string & opt, const std::string & desc)
{
    _imp->options.push_back(std::make_pair(opt, desc));
    return *this;
}

StringSetArg::StringSetArgOptions::StringSetArgOptions(const StringSetArg::StringSetArgOptions & o) :
    _imp()
{
    std::copy(o._imp->options.begin(), o._imp->options.end(),
            std::back_inserter(_imp->options));
}

StringSetArg::StringSetArgOptions::~StringSetArgOptions()
{
}

StringSetArg::StringSetArgOptions::StringSetArgOptions() :
    _imp()
{
}

void
EnumArg::set_argument(const std::string & arg)
{
    Context context("When handling argument '" + arg + "' for '--" + long_name() + "':");

    /* if we're given the short arg, turn it magically into the long one */
    AllowedArgConstIterator i(std::find_if(_imp->allowed_args.begin(), _imp->allowed_args.end(), ArgIs(arg)));
    if (i == _imp->allowed_args.end())
        throw (BadValue("--" + long_name(), arg));

    _argument = i->long_name();
}

void
StringArg::set_argument(const std::string & arg)
{
    Context context("When handling argument '" + arg + "' for '--" + long_name() + "':");

    if (_validator)
        (*_validator)(arg);

    _argument = arg;
}

EnumArg::~EnumArg()
{
}

EnumArg::EnumArgOptions::EnumArgOptions(const std::string & opt, const std::string & desc) :
    _imp()
{
    _imp->options.push_back(make_named_values<AllowedEnumArg>(
                n::description() = desc,
                n::long_name() = opt,
                n::short_name() = '\0'
                ));
}

EnumArg::EnumArgOptions::EnumArgOptions(const std::string & opt, const char s, const std::string & desc) :
    _imp()
{
    _imp->options.push_back(make_named_values<AllowedEnumArg>(
                n::description() = desc,
                n::long_name() = opt,
                n::short_name() = s
                ));
}

EnumArg::EnumArgOptions &
EnumArg::EnumArgOptions::operator() (const std::string & opt, const std::string & desc)
{
    _imp->options.push_back(make_named_values<AllowedEnumArg>(
                n::description() = desc,
                n::long_name() = opt,
                n::short_name() = '\0'
                ));
    return *this;
}

EnumArg::EnumArgOptions &
EnumArg::EnumArgOptions::operator() (const std::string & opt, const char s, const std::string & desc)
{
    _imp->options.push_back(make_named_values<AllowedEnumArg>(
                n::description() = desc,
                n::long_name() = opt,
                n::short_name() = s
                ));
    return *this;
}

EnumArg::EnumArgOptions::~EnumArgOptions()
{
}

EnumArg::EnumArg(ArgsGroup * const our_group, const std::string & our_long_name,
        const char our_short_name, const std::string & our_description,
        const EnumArgOptions & opts, const std::string & our_default_arg) :
    ArgsOption(our_group, our_long_name, our_short_name, our_description),
    _imp(),
    _argument(our_default_arg),
    _default_arg(our_default_arg)
{
    _imp->allowed_args = opts._imp->options;
}

void
EnumArg::set_default_arg(const std::string & arg)
{
    _argument = arg;
    _default_arg = arg;
}

EnumArg::AllowedArgConstIterator
EnumArg::begin_allowed_args() const
{
    return AllowedArgConstIterator(_imp->allowed_args.begin());
}

EnumArg::AllowedArgConstIterator
EnumArg::end_allowed_args() const
{
    return AllowedArgConstIterator(_imp->allowed_args.end());
}

const std::string
EnumArg::forwardable_string() const
{
    if (specified())
        return "--" + long_name() + " " + escape(argument());
    else
        return "";
}

const std::shared_ptr<Sequence<std::string> >
EnumArg::forwardable_args() const
{
    std::shared_ptr<Sequence<std::string> > result(std::make_shared<Sequence<std::string>>());
    if (specified())
    {
        result->push_back("--" + long_name());
        result->push_back(argument());
    }
    return result;
}

StringSetArg::~StringSetArg()
{
}

StringSetArg::AllowedArgConstIterator
StringSetArg::begin_allowed_args() const
{
    return AllowedArgConstIterator(_imp->allowed_args.begin());
}

StringSetArg::AllowedArgConstIterator
StringSetArg::end_allowed_args() const
{
    return AllowedArgConstIterator(_imp->allowed_args.end());
}

bool
AliasArg::can_be_negated() const
{
    return _other->can_be_negated();
}

bool
StringArg::can_be_negated() const
{
    return false;
}

bool
IntegerArg::can_be_negated() const
{
    return false;
}

bool
EnumArg::can_be_negated() const
{
    return false;
}

bool
StringSetArg::can_be_negated() const
{
    return false;
}

bool
StringSequenceArg::can_be_negated() const
{
    return false;
}

bool
SwitchArg::can_be_negated() const
{
    return _can_be_negated;
}

template class WrappedForwardIterator<StringSetArg::ConstIteratorTag, const std::string>;
template class WrappedForwardIterator<StringSetArg::AllowedArgConstIteratorTag,
         const std::pair<std::string, std::string> >;
template class WrappedForwardIterator<EnumArg::AllowedArgConstIteratorTag,
         const AllowedEnumArg>;
template class WrappedForwardIterator<StringSequenceArg::ConstIteratorTag, const std::string>;


