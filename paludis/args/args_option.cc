/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include "bad_value.hh"
#include <set>
#include <vector>

/** \file
 * Implementation for ArgsOption.
 *
 * \ingroup grplibpaludisargs
 */

using namespace paludis::args;

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

SwitchArg::SwitchArg(ArgsGroup * const our_group, std::string our_long_name, char our_short_name,
        std::string our_description) :
    ArgsOption(our_group, our_long_name, our_short_name, our_description)
{
}

SwitchArg::~SwitchArg()
{
}

AliasArg::AliasArg(ArgsOption * const other, const std::string & our_long_name) :
    ArgsOption(other->group(), our_long_name, '\0', "Alias for --" + other->long_name()),
    _other(other)
{
    other->group()->handler()->add_option(other, our_long_name);
}

StringArg::StringArg(ArgsGroup * const g, const std::string & our_long_name,
        const char our_short_name, const std::string & our_description) :
    ArgsOption(g, our_long_name, our_short_name, our_description)
{
}

namespace paludis
{
    /**
     * Implementation data for StringSetArg.
     *
     * \ingroup grplibpaludisargs
     */
    template<>
    struct Implementation<StringSetArg> :
        InternalCounted<Implementation<StringSetArg> >
    {
        std::set<std::string> args;
    };
}

StringSetArg::StringSetArg(ArgsGroup * const g, const std::string & our_long_name,
        const char our_short_name, const std::string & our_description) :
    ArgsOption(g, our_long_name, our_short_name, our_description),
    PrivateImplementationPattern<StringSetArg>(new Implementation<StringSetArg>)
{
}

StringSetArg::Iterator
StringSetArg::args_begin() const
{
    return Iterator(_imp->args.begin());
}

StringSetArg::Iterator
StringSetArg::args_end() const
{
    return Iterator(_imp->args.end());
}

void
StringSetArg::add_argument(const std::string & arg)
{
    _imp->args.insert(arg);
}

IntegerArg::IntegerArg(ArgsGroup * const our_group, const std::string & our_long_name,
                char our_short_name, const std::string & our_description) :
    ArgsOption(our_group, our_long_name, our_short_name, our_description)
{
}

namespace
{
    /**
     * Is an arg a particular value?
     *
     * \ingroup grplibpaludisargs
     */
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

namespace paludis
{
    /**
     * Implementation data for EnumArg.
     *
     * \ingroup grplibpaludisargs
     */
    template<>
    struct Implementation<EnumArg> :
        InternalCounted<Implementation<EnumArg> >
    {
        std::vector<std::pair<std::string, std::string> > allowed_args;
    };

    /**
     * Implementation data for EnumArg::EnumArgOptions.
     *
     * \ingroup grplibpaludisargs
     */
    template<>
    struct Implementation<EnumArg::EnumArgOptions> :
        InternalCounted<Implementation<EnumArg::EnumArgOptions> >
    {
        std::vector<std::pair<std::string, std::string> > options;
    };
}

void EnumArg::set_argument(const std::string & arg)
{
    if (_imp->allowed_args.end() == std::find_if(_imp->allowed_args.begin(),
                _imp->allowed_args.end(), ArgIs(arg)))
        throw (BadValue("--" + long_name(), arg));

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

EnumArg::AllowedArgIterator
EnumArg::begin_allowed_args() const
{
    return AllowedArgIterator(_imp->allowed_args.begin());
}

EnumArg::AllowedArgIterator
EnumArg::end_allowed_args() const
{
    return AllowedArgIterator(_imp->allowed_args.end());
}

StringSetArg::~StringSetArg()
{
}

