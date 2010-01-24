/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_JOB_ID_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_JOB_ID_HH 1

#include <paludis/resolver/job_id-fwd.hh>
#include <paludis/util/named_value.hh>
#include <paludis/serialise-fwd.hh>
#include <string>

namespace paludis
{
    namespace n
    {
        struct string_id;
    }

    namespace resolver
    {
        struct JobID
        {
            NamedValue<n::string_id, std::string> string_id;

            std::size_t hash() const PALUDIS_ATTRIBUTE((warn_unused_result));
            bool operator< (const JobID &) const PALUDIS_ATTRIBUTE((warn_unused_result));

            void serialise(Serialiser &) const;
            static const JobID deserialise(Deserialisation & d) PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }
}

#endif
