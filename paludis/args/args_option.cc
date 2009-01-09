/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008 Ciaran McCreesh
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

#include "args.hh"
#include "args_error.hh"
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <set>
#include <vector>
#include <algorithm>
#include <list>

using namespace paludis;
using namespace paludis::args;

template class WrappedForwardIterator<StringSetArg::ConstIteratorTag, const std::string>;
template class WrappedForwardIterator<StringSetArg::AllowedArgConstIteratorTag,
         const std::pair<std::string, std::string> >;
template class WrappedForwardIterator<EnumArg::AllowedArgConstIteratorTag,
         const std::pair<std::string, std::string> >;
template class WrappedForwardIterator<StringSequenceArg::ConstIteratorTag, const std::string>;

namespace
{
    struct ArgIs
    {
        /// The argument.
        const std::string arg;

        /// Constructor.
        ArgIs(const std::string & a) :
            arg(a)
        {
        }

        /// Comparator.
        bool operator() (const std::pair<std::string, std::string> & p) const
        {
            return p.first == arg;
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
    g->handler()->add_option(this, our_long_name, our_short_name);
}

ArgsOption::~ArgsOption()
{
}

void
ArgsOption::remove()
{
    _group->remove(this);
    _group->handler()->remove_option(_long_name, _short_name);
}

SwitchArg::SwitchArg(ArgsGroup * const our_group, std::string our_long_name, char our_short_name,
        std::string our_description, const bool c) :
    ArgsOption(our_group, our_long_name, our_short_name, our_description),
    _can_be_negated(c)
{
}

SwitchArg::~SwitchArg()
{
}

AliasArg::AliasArg(ArgsOption * const o, const std::string & our_long_name, bool is_hidden) :
    ArgsOption(o->group(), our_long_name, '\0', "Alias for --" + o->long_name()),
    _other(o), _hidden(is_hidden)
{
    o->group()->handler()->add_option(o, our_long_name);
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

namespace paludis
{
    template<>
    struct Implementation<StringSetArg>
    {
        std::set<std::string> args;
        std::vector<std::pair<std::string, std::string> > allowed_args;
    };

    template<>
    struct Implementation<StringSetArg::StringSetArgOptions>
    {
        std::vector<std::pair<std::string, std::string> > options;
    };
}

StringSetArg::StringSetArg(ArgsGroup * const g, const std::string & our_long_name,
        const char our_short_name, const std::string & our_description,
        const StringSetArgOptions & opts) :
    ArgsOption(g, our_long_name, our_short_name, our_description),
    PrivateImplementationPattern<StringSetArg>(new Implementation<StringSetArg>),
    _validator(0)
{
    std::copy(opts._imp->options.begin(), opts._imp->options.end(),
            std::back_inserter(_imp->allowed_args));
}

StringSetArg::StringSetArg(ArgsGroup * const g, const std::string & our_long_name,
        const char our_short_name, const std::string & our_description,
        const StringSetArgOptions & opts, void (* v) (const std::string &)) :
    ArgsOption(g, our_long_name, our_short_name, our_description),
    PrivateImplementationPattern<StringSetArg>(new Implementation<StringSetArg>),
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

namespace paludis
{
    template<>
    struct Implementation<StringSequenceArg>
    {
        std::list<std::string> args;
    };
}

StringSequenceArg::StringSequenceArg(ArgsGroup * const g, const std::string & our_long_name,
        const char our_short_name, const std::string & our_description) :
    ArgsOption(g, our_long_name, our_short_name, our_description),
    PrivateImplementationPattern<StringSequenceArg>(new Implementation<StringSequenceArg>)
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

IntegerArg::IntegerArg(ArgsGroup * const our_group, const std::string & our_long_name,
                char our_short_name, const std::string & our_description) :
    ArgsOption(our_group, our_long_name, our_short_name, our_description)
{
}

namespace paludis
{
    template<>
    struct Implementation<EnumArg>
    {
        std::vector<std::pair<std::string, std::string> > allowed_args;
    };

    template<>
    struct Implementation<EnumArg::EnumArgOptions>
    {
        std::vector<std::pair<std::string, std::string> > options;
    };
}

StringSetArg::StringSetArgOptions::StringSetArgOptions(std::string opt, std::string desc) :
    PrivateImplementationPattern<StringSetArgOptions>(new Implementation<StringSetArgOptions>)
{
    _imp->options.push_back(std::make_pair(opt, desc));
}

StringSetArg::StringSetArgOptions &
StringSetArg::StringSetArgOptions::operator() (std::string opt, std::string desc)
{
    _imp->options.push_back(std::make_pair(opt, desc));
    return *this;
}

StringSetArg::StringSetArgOptions::StringSetArgOptions(const StringSetArg::StringSetArgOptions & o) :
    PrivateImplementationPattern<StringSetArgOptions>(new Implementation<StringSetArgOptions>)
{
    std::copy(o._imp->options.begin(), o._imp->options.end(),
            std::back_inserter(_imp->options));
}

StringSetArg::StringSetArgOptions::~StringSetArgOptions()
{
}

StringSetArg::StringSetArgOptions::StringSetArgOptions() :
    PrivateImplementationPattern<StringSetArgOptions>(new Implementation<StringSetArgOptions>)
{
}

void
EnumArg::set_argument(const std::string & arg)
{
    Context context("When handling argument '" + arg + "' for '--" + long_name() + "':");

    if (_imp->allowed_args.end() == std::find_if(_imp->allowed_args.begin(),
                _imp->allowed_args.end(), ArgIs(arg)))
        throw (BadValue("--" + long_name(), arg));

    _argument = arg;
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

EnumArg::EnumArgOptions::EnumArgOptions(std::string opt, std::string desc) :
    PrivateImplementationPattern<EnumArgOptions>(new Implementation<EnumArgOptions>)
{
    _imp->options.push_back(std::make_pair(opt, desc));
}

EnumArg::EnumArgOptions & EnumArg::EnumArgOptions::operator() (std::string opt, std::string desc)
{
    _imp->options.push_back(std::make_pair(opt, desc));
    return *this;
}

EnumArg::EnumArgOptions::~EnumArgOptions()
{
}

EnumArg::EnumArg(ArgsGroup * const our_group, const std::string & our_long_name,
        const char our_short_name, const std::string & our_description,
        const EnumArgOptions & opts, const std::string & our_default_arg) :
    ArgsOption(our_group, our_long_name, our_short_name, our_description),
    PrivateImplementationPattern<EnumArg>(new Implementation<EnumArg>),
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

