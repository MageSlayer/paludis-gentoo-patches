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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_ARROW_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_ARROW_HH 1

#include <paludis/resolver/arrow-fwd.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/serialise-fwd.hh>
#include <paludis/resolver/reason-fwd.hh>
#include <paludis/util/named_value.hh>

namespace paludis
{
    namespace n
    {
        struct comes_after;
        struct ignorable_pass;
        struct reason;
    }

    namespace resolver
    {
        struct Arrow
        {
            NamedValue<n::comes_after, Resolvent> comes_after;
            NamedValue<n::ignorable_pass, int> ignorable_pass;
            NamedValue<n::reason, std::tr1::shared_ptr<const Reason> > reason;

            void serialise(Serialiser &) const;

            static const std::tr1::shared_ptr<Arrow> deserialise(
                    Deserialisation & d) PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }
}

#endif
