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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_ARROW_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_ARROW_HH 1

#include <paludis/resolver/arrow-fwd.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/reason-fwd.hh>
#include <paludis/resolver/job_id.hh>
#include <paludis/resolver/failure_kinds-fwd.hh>
#include <paludis/serialise-fwd.hh>
#include <paludis/util/named_value.hh>

namespace paludis
{
    namespace n
    {
        typedef Name<struct comes_after_name> comes_after;
        typedef Name<struct failure_kinds_name> failure_kinds;
        typedef Name<struct maybe_reason_name> maybe_reason;
    }

    namespace resolver
    {
        struct Arrow
        {
            NamedValue<n::comes_after, JobID> comes_after;
            NamedValue<n::failure_kinds, FailureKinds> failure_kinds;
            NamedValue<n::maybe_reason, std::tr1::shared_ptr<const Reason> > maybe_reason;

            void serialise(Serialiser &) const;

            static const Arrow deserialise(Deserialisation & d) PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }
}

#endif
