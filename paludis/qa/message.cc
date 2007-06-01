/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/qa/message.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <ostream>

using namespace paludis;
using namespace paludis::qa;

#include <paludis/qa/message-sr.cc>

std::ostream &
paludis::qa::operator<< (std::ostream & s, const Message & m)
{
    s << "(" << m.level << ") " << m.msg;
    return s;
}

std::ostream &
paludis::qa::operator<< (std::ostream & s, const QALevel & l)
{
    switch (l)
    {
        case qal_info:
            s << "info";
            return s;

        case qal_skip:
            s << "skip";
            return s;

        case qal_maybe:
            s << "maybe";
            return s;

        case qal_minor:
            s << "minor";
            return s;

        case qal_major:
            s << "major";
            return s;

        case qal_fatal:
            s << "fatal";
            return s;

        case last_qal:
            ;
    };

    throw InternalError(PALUDIS_HERE, "Bad QA Level");
}
