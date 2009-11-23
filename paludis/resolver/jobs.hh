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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_JOBS_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_JOBS_HH 1

#include <paludis/resolver/jobs-fwd.hh>
#include <paludis/resolver/job-fwd.hh>
#include <paludis/resolver/job_id-fwd.hh>
#include <paludis/resolver/resolvent-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/serialise-fwd.hh>
#include <tr1/memory>

namespace paludis
{
    namespace resolver
    {
        class PALUDIS_VISIBLE Jobs :
            private PrivateImplementationPattern<Jobs>
        {
            public:
                Jobs();
                ~Jobs();

                void add(const std::tr1::shared_ptr<Job> &);

                bool have_job_for_installed(const Resolvent &) const PALUDIS_ATTRIBUTE((warn_unused_result));

                const JobID find_id_for_installed(const Resolvent &) const PALUDIS_ATTRIBUTE((warn_unused_result));

                bool have_job_for_usable(const Resolvent &) const PALUDIS_ATTRIBUTE((warn_unused_result));

                const JobID find_id_for_usable(const Resolvent &) const PALUDIS_ATTRIBUTE((warn_unused_result));

                const std::tr1::shared_ptr<Job> fetch(
                        const JobID &) PALUDIS_ATTRIBUTE((warn_unused_result));

                const std::tr1::shared_ptr<const Job> fetch(
                        const JobID &) const PALUDIS_ATTRIBUTE((warn_unused_result));

                void serialise(Serialiser &) const;

                static const std::tr1::shared_ptr<Jobs> deserialise(
                        Deserialisation & d) PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class PrivateImplementationPattern<resolver::Jobs>;
#endif

}
#endif
