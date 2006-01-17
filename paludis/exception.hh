/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_EXCEPTION_HH
#define PALUDIS_GUARD_PALUDIS_EXCEPTION_HH 1

#include <paludis/attributes.hh>

#include <string>
#include <exception>
#include <libebt/libebt.hh>

/** \file
 * Declaration for the Exception base class.
 */

namespace paludis
{
    /**
     * Context tag for libebt.
     */
    struct PaludisBacktraceTag
    {
    };

    /**
     * Backtrace context class.
     */
    typedef libebt::BacktraceContext<PaludisBacktraceTag> Context;

    /**
     * Base exception class.
     */
    class Exception : public std::exception,
                      public libebt::Backtraceable<PaludisBacktraceTag>
    {
        private:
            const std::string _message;

        protected:
            /**
             * Constructor.
             */
            Exception(const std::string & message) throw ();

        public:
            /**
             * Destructor.
             */
            virtual ~Exception() throw () PALUDIS_ATTRIBUTE((nothrow));

            /**
             * Return our descriptive error message.
             */
            const std::string & message() const throw () PALUDIS_ATTRIBUTE((nothrow));
    };
}

#endif
