/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_TIMESTAMP_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_TIMESTAMP_HH 1

#include <paludis/util/timestamp-fwd.hh>
#include <paludis/util/operators.hh>
#include <paludis/util/attributes.hh>
#include <sys/stat.h>
#include <sys/time.h>

namespace paludis
{
    /**
     * Wrapper class to simplify dealing with the zillion different ways C has
     * of dealing with timestamps.
     *
     * Most definitely not for use for durations (that is, the difference
     * between two date-times).
     *
     * \since 0.44
     */
    class PALUDIS_VISIBLE Timestamp :
        public relational_operators::HasRelationalOperators
    {
        private:
            time_t _s;
            long _ns;

        public:
            explicit Timestamp(const struct timespec &);
            Timestamp(const time_t, const long);
            Timestamp(const Timestamp &);

            Timestamp & operator= (const Timestamp &);

            bool operator== (const Timestamp &) const PALUDIS_ATTRIBUTE((warn_unused_result));
            bool operator< (const Timestamp &) const PALUDIS_ATTRIBUTE((warn_unused_result));

            time_t seconds() const PALUDIS_ATTRIBUTE((warn_unused_result));
            long nanoseconds() const PALUDIS_ATTRIBUTE((warn_unused_result));

            struct timespec as_timespec() const PALUDIS_ATTRIBUTE((warn_unused_result));
            struct timeval as_timeval() const PALUDIS_ATTRIBUTE((warn_unused_result));

            static Timestamp now() PALUDIS_ATTRIBUTE((warn_unused_result));
    };
}

#endif
