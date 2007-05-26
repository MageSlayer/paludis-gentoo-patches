/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include "args_dumper.hh"
#include <paludis/util/system.hh>
#include <paludis/util/join.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/visitor-impl.hh>
#include <algorithm>
#include <sstream>
#include <list>
#include <map>

/** \file
 * Implementation for ArgsHandler.
 *
 * \ingroup grplibpaludisargs
 */

using namespace paludis::args;

namespace paludis
{
    /**
     * Implementation data for ArgsHandler.
     *
     * \ingroup grplibpaludisargs
     */
    template<>
    struct Implementation<ArgsHandler>
    {
        std::list<ArgsGroup *> groups;
        std::list<std::string> parameters;
        std::list<std::string> usage_lines;
        std::list<std::pair<std::string, std::string> > environment_lines;

        std::map<std::string, ArgsOption *> longopts;
        std::map<char, ArgsOption *> shortopts;
    };
}

ArgsHandler::ArgsHandler() :
    PrivateImplementationPattern<ArgsHandler>(new Implementation<ArgsHandler>)
{
}

ArgsHandler::~ArgsHandler()
{
}

void
ArgsHandler::add_usage_line(const std::string & l)
{
    _imp->usage_lines.push_back(l);
}

void
ArgsHandler::add_environment_variable(const std::string & e, const std::string & desc)
{
    _imp->environment_lines.push_back(std::make_pair(e, desc));
}

void
ArgsHandler::add(ArgsGroup * const g)
{
    /// \bug Should check for name uniqueness.
    _imp->groups.push_back(g);
}

void
ArgsHandler::run(const int argc, const char * const * const argv, const std::string & env_var,
        const std::string & env_prefix)
{
    run(argc, argv, "", env_var, env_prefix);
}

void
ArgsHandler::run(const int argc, const char * const * const argv,
        const std::string & client,
        const std::string & env_var,
        const std::string & env_prefix)
{
    std::list<std::string> args;
    std::string env_options;

    setenv("PALUDIS_CLIENT", client.c_str(), 1);

    if (! env_var.empty())
        env_options = paludis::getenv_with_default(env_var, "");

    std::istringstream iss(env_options);
    std::string option;
    while (iss.good())
    {
        iss >> option;
        if (!option.empty())
            args.push_back(option);
    }

    args.insert(args.end(), &argv[1], &argv[argc]);

    libwrapiter::ForwardIterator<ArgsVisitor, std::string> argit(args.begin()), arge(args.end());

    ArgsVisitor visitor(&argit, arge, env_prefix);

    for ( ; argit != arge; ++argit )
    {
        std::string arg = *argit;

        if (arg == "--")
        {
            ++argit;
            break;
        }
        else if (0 == arg.compare(0, 2, "--"))
        {
            arg.erase(0, 2);
            std::map<std::string, ArgsOption *>::iterator it = _imp->longopts.find(arg);
            if (it == _imp->longopts.end())
                throw BadArgument("--" + arg);
            it->second->accept(visitor);
        }
        else if (arg[0] == '-')
        {
            arg.erase(0, 1);
            for (std::string::iterator c = arg.begin(); c != arg.end(); ++c)
            {
                std::map<char, ArgsOption *>::iterator it = _imp->shortopts.find(*c);
                if (it == _imp->shortopts.end())
                {
                    throw BadArgument(std::string("-") + *c);
                }
                it->second->accept(visitor);
            }
        }
        else
        {
            _imp->parameters.push_back(arg);
        }
    }

    _imp->parameters.insert(_imp->parameters.end(),
            argit, libwrapiter::ForwardIterator<ArgsVisitor, std::string>(args.end()));

    if (! env_prefix.empty())
        setenv((env_prefix + "_PARAMS").c_str(), join(_imp->parameters.begin(),
                    _imp->parameters.end(), " ").c_str(), 1);
}

void
ArgsHandler::dump_to_stream(std::ostream & s) const
{
    ArgsDumper dump(s);
    std::list<ArgsGroup *>::const_iterator g(_imp->groups.begin()), g_end(_imp->groups.end());
    for ( ; g != g_end ; ++g)
    {
        s << (*g)->name() << ":" << std::endl;

        std::for_each(indirect_iterator((*g)->begin()), indirect_iterator((*g)->end()), accept_visitor(dump));

        s << std::endl;
    }
}

ArgsHandler::ParametersIterator
ArgsHandler::begin_parameters() const
{
    return ParametersIterator(_imp->parameters.begin());
}

ArgsHandler::ParametersIterator
ArgsHandler::end_parameters() const
{
    return ParametersIterator(_imp->parameters.end());
}

bool
ArgsHandler::empty() const
{
    return _imp->parameters.empty();
}

std::ostream &
paludis::args::operator<< (std::ostream & s, const ArgsHandler & h)
{
    h.dump_to_stream(s);
    return s;
}

void
ArgsHandler::add_option(ArgsOption * const opt, const std::string & long_name,
        const char short_name)
{
    _imp->longopts[long_name] = opt;
    if (short_name != '\0')
        _imp->shortopts[short_name] = opt;
}

ArgsHandler::UsageLineIterator
ArgsHandler::begin_usage_lines() const
{
    return UsageLineIterator(_imp->usage_lines.begin());
}

ArgsHandler::UsageLineIterator
ArgsHandler::end_usage_lines() const
{
    return UsageLineIterator(_imp->usage_lines.end());
}

ArgsHandler::EnvironmentLineIterator
ArgsHandler::begin_environment_lines() const
{
    return EnvironmentLineIterator(_imp->environment_lines.begin());
}

ArgsHandler::EnvironmentLineIterator
ArgsHandler::end_environment_lines() const
{
    return EnvironmentLineIterator(_imp->environment_lines.end());
}

ArgsHandler::ArgsGroupsIterator
ArgsHandler::begin_args_groups() const
{
    return ArgsGroupsIterator(_imp->groups.begin());
}

ArgsHandler::ArgsGroupsIterator
ArgsHandler::end_args_groups() const
{
    return ArgsGroupsIterator(_imp->groups.end());
}

