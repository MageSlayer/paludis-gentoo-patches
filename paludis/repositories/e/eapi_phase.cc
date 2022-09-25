/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/repositories/e/eapi_phase.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <set>
#include <algorithm>
#include <list>

using namespace paludis;
using namespace paludis::erepository;

typedef std::list<std::shared_ptr<const EAPIPhase> > EAPIPhasesList;

namespace paludis
{
    template <>
    struct Imp<EAPIPhase>
    {
        std::set<std::string> options;
        std::list<std::string> commands;
    };

    template <>
    struct Imp<EAPIPhases>
    {
        EAPIPhasesList phases;
    };

    template <>
    struct WrappedForwardIteratorTraits<EAPIPhases::ConstIteratorTag>
    {
        typedef IndirectIterator<EAPIPhasesList::const_iterator> UnderlyingIterator;
    };

    template <>
    struct WrappedForwardIteratorTraits<EAPIPhase::ConstIteratorTag>
    {
        typedef std::list<std::string>::const_iterator UnderlyingIterator;
    };
}

EAPIPhase::EAPIPhase(const std::string & s) :
    _imp()
{
    Context c("When parsing EAPI phase '" + s + "'");

    std::list<std::string> tokens;
    tokenise_whitespace(s, std::back_inserter(tokens));

    std::list<std::string>::iterator t(std::find(tokens.begin(), tokens.end(), ":"));
    if (t == tokens.end())
        throw EAPIConfigurationError("EAPI phase '" + s + "' contains no ':'");

    std::copy(tokens.begin(), t, std::inserter(_imp->options, _imp->options.begin()));
    std::copy(next(t), tokens.end(), std::back_inserter(_imp->commands));
}

EAPIPhase::~EAPIPhase() = default;

bool
EAPIPhase::option(const std::string & s) const
{
    return _imp->options.count(s);
}

std::string
EAPIPhase::equal_option(const std::string & s) const
{
    for (const auto & option : _imp->options)
        if (0 == option.compare(0, s.length() + 1, s + "="))
            return option.substr(s.length() + 1);

    return "";
}

EAPIPhase::ConstIterator
EAPIPhase::begin_commands() const
{
    return ConstIterator(_imp->commands.begin());
}

EAPIPhase::ConstIterator
EAPIPhase::end_commands() const
{
    return ConstIterator(_imp->commands.end());
}

EAPIPhases::EAPIPhases(const std::string & s) :
    _imp()
{
    Context c("When parsing EAPI phases '" + s + "'");

    std::list<std::string> tokens;
    tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>(s, ";", "", std::back_inserter(tokens));
    for (const auto & token : tokens)
        _imp->phases.push_back(std::make_shared<EAPIPhase>(token));
}

EAPIPhases::~EAPIPhases() = default;

EAPIPhases::ConstIterator
EAPIPhases::begin_phases() const
{
    return ConstIterator(indirect_iterator(_imp->phases.begin()));
}

EAPIPhases::ConstIterator
EAPIPhases::end_phases() const
{
    return ConstIterator(indirect_iterator(_imp->phases.end()));
}

namespace paludis
{
    template class WrappedForwardIterator<EAPIPhase::ConstIteratorTag, const std::string>;
    template class WrappedForwardIterator<EAPIPhases::ConstIteratorTag, const EAPIPhase>;
}
