/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_QA_MESSAGE_HH
#define PALUDIS_GUARD_PALUDIS_QA_MESSAGE_HH 1

#include <string>
#include <iosfwd>

/** \file
 * Declarations for the Message class.
 *
 * \ingroup QA
 */

namespace paludis
{
    namespace qa
    {
        enum QALevel
        {
            qal_info,
            qal_skip,
            qal_maybe,
            qal_minor,
            qal_major,
            qal_fatal
        };

#include <paludis/qa/message-sr.hh>

        std::ostream & operator<< (std::ostream &, const Message &);
    }
}

#endif
