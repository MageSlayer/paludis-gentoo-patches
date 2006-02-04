/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include "log.hh"
#include <iostream>

using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<Log> :
        InternalCounted<Implementation<Log> >
    {
        LogLevel log_level;
        std::ostream * stream;
    };
}

Log::Log() :
    PrivateImplementationPattern<Log>(new Implementation<Log>)
{
    _implementation->log_level = initial_ll;
    _implementation->stream = &std::cerr;
}

Log::~Log()
{
}

void
Log::set_log_level(const LogLevel l)
{
    _implementation->log_level = l;
}

void
Log::message(const LogLevel l, const std::string & s)
{
    if (l >= _implementation->log_level)
    {
        do
        {
            switch (l)
            {
                case ll_debug:
                    *_implementation->stream << "[DEBUG] ";
                    continue;

                case ll_qa:
                    *_implementation->stream << "[QA] ";
                    continue;

                case ll_warning:
                    *_implementation->stream << "[WARNING] ";
                    continue;

                case last_ll:
                    break;
            }

            throw InternalError(PALUDIS_HERE, "Bad value for log_level");

        } while (false);

        *_implementation->stream << s << std::endl;
    }
}

void
Log::set_log_stream(std::ostream * const s)
{
    _implementation->stream = s;
}
