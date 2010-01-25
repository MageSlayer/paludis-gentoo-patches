/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_JOB_STATE_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_JOB_STATE_HH 1

#include <paludis/resolver/job_state-fwd.hh>
#include <paludis/resolver/job-fwd.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/simple_visitor.hh>
#include <paludis/util/type_list.hh>
#include <paludis/output_manager-fwd.hh>
#include <tr1/memory>
#include <string>

namespace paludis
{
    namespace resolver
    {
        class PALUDIS_VISIBLE JobState :
            public virtual DeclareAbstractAcceptMethods<JobState, MakeTypeList<
                JobPendingState, JobSucceededState, JobFailedState, JobSkippedState>::Type>
        {
            public:
                virtual ~JobState() = 0;

                virtual const std::tr1::shared_ptr<const Job> job() const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual const std::string state_name() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
        };

        class PALUDIS_VISIBLE JobPendingState :
            public JobState,
            public ImplementAcceptMethods<JobState, JobPendingState>,
            private PrivateImplementationPattern<JobPendingState>
        {
            public:
                JobPendingState(const std::tr1::shared_ptr<const Job> &);
                ~JobPendingState();

                virtual const std::tr1::shared_ptr<const Job> job() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string state_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class PALUDIS_VISIBLE JobSucceededState :
            public JobState,
            public ImplementAcceptMethods<JobState, JobSucceededState>,
            private PrivateImplementationPattern<JobSucceededState>
        {
            public:
                JobSucceededState(const std::tr1::shared_ptr<const Job> &);
                ~JobSucceededState();

                virtual const std::tr1::shared_ptr<const Job> job() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string state_name() const PALUDIS_ATTRIBUTE((warn_unused_result));

                void add_output_manager(const std::tr1::shared_ptr<OutputManager> &);
        };

        class PALUDIS_VISIBLE JobFailedState :
            public JobState,
            public ImplementAcceptMethods<JobState, JobFailedState>,
            private PrivateImplementationPattern<JobFailedState>
        {
            public:
                JobFailedState(const std::tr1::shared_ptr<const Job> &);
                ~JobFailedState();

                virtual const std::tr1::shared_ptr<const Job> job() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string state_name() const PALUDIS_ATTRIBUTE((warn_unused_result));

                void add_output_manager(const std::tr1::shared_ptr<OutputManager> &);
        };

        class PALUDIS_VISIBLE JobSkippedState :
            public JobState,
            public ImplementAcceptMethods<JobState, JobSkippedState>,
            private PrivateImplementationPattern<JobSkippedState>
        {
            public:
                JobSkippedState(const std::tr1::shared_ptr<const Job> &);
                ~JobSkippedState();

                virtual const std::tr1::shared_ptr<const Job> job() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string state_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class PrivateImplementationPattern<resolver::JobPendingState>;
    extern template class PrivateImplementationPattern<resolver::JobSucceededState>;
    extern template class PrivateImplementationPattern<resolver::JobFailedState>;
    extern template class PrivateImplementationPattern<resolver::JobSkippedState>;
#endif
}

#endif
