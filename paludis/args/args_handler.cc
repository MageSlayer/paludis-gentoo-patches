/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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
#include <algorithm>
#include <paludis/util/system.hh>

/** \file
 * Implementation for ArgsHandler.
 *
 * \ingroup Args
 */

using namespace paludis::args;

ArgsHandler::ArgsHandler()
{
}

void
ArgsHandler::add(ArgsGroup * const g)
{
    /// \bug Should check for name uniqueness.
    _groups.push_back(g);
}

void
ArgsHandler::run(const int argc, const char * const * const argv)
{
    std::list<std::string> args;

    std::string env_options = paludis::getenv_with_default("PALUDIS_OPTIONS", "");

    std::istringstream iss(env_options);
    std::string option;
    while(iss.good())
    {
        iss >> option;
        if(!option.empty())
            args.push_back(option);
    }

    args.insert(args.end(), &argv[1], &argv[argc]);

    std::list<std::string>::iterator argit = args.begin(), arge = args.end();

    ArgsVisitor visitor(&argit, arge);

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
            std::map<std::string, ArgsOption*>::iterator it = _longopts.find(arg);
            if (it == _longopts.end())
                throw BadArgument("--" + arg);
            (*it).second->accept(&visitor);
        }
        else if (arg[0] == '-')
        {
            arg.erase(0, 1);
            for (std::string::iterator c = arg.begin(); c != arg.end(); ++c)
            {
                std::map<char, ArgsOption*>::iterator it = _shortopts.find(*c);
                if (it == _shortopts.end())
                {
                    throw BadArgument(std::string("-") + *c);
                }
                (*it).second->accept(&visitor);
            }
        }
        else
        {
            _parameters.push_back(arg);
        }
    }

    _parameters.insert(_parameters.end(), argit, args.end());
}

void
ArgsHandler::dump_to_stream(std::ostream & s) const
{
    ArgsDumper dump(s);
    std::list<ArgsGroup *>::const_iterator g(_groups.begin()), g_end(_groups.end());
    for ( ; g != g_end ; ++g)
    {
        s << (*g)->name() << ":" << std::endl;

        std::for_each((*g)->_args_options.begin(), (*g)->_args_options.end(),
                accept_visitor(&dump));

        s << std::endl;
    }
}

#ifndef DOXYGEN
std::ostream &
paludis::args::operator<< (std::ostream & s, const ArgsHandler & h)
{
    h.dump_to_stream(s);
    return s;
}
#endif

