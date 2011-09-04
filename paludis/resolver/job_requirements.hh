/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_JOB_REQUIREMENTS_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_JOB_REQUIREMENTS_HH 1

#include <paludis/resolver/job_requirements-fwd.hh>
#include <paludis/resolver/job_list-fwd.hh>
#include <paludis/util/options.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/serialise-fwd.hh>

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_job_number> job_number;
        typedef Name<struct name_required_if> required_if;
    }

    namespace resolver
    {
        struct JobRequirement
        {
            NamedValue<n::job_number, JobNumber> job_number;
            NamedValue<n::required_if, JobRequirementIfs> required_if;

            static const JobRequirement deserialise(Deserialisation & d) PALUDIS_ATTRIBUTE((warn_unused_result));
            void serialise(Serialiser &) const;
        };

        struct JobRequirementComparator
        {
            bool operator() (const JobRequirement & a, const JobRequirement & b);
        };
    }

    extern template class Sequence<resolver::JobRequirement>;
    extern template class WrappedForwardIterator<Sequence<resolver::JobRequirement>::ConstIteratorTag, const resolver::JobRequirement>;
}

#endif
