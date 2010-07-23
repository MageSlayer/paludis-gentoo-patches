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
#include <paludis/util/simple_visitor.hh>
#include <paludis/util/type_list.hh>
#include <paludis/util/pimp.hh>
#include <paludis/output_manager-fwd.hh>
#include <paludis/serialise-fwd.hh>
#include <memory>

namespace paludis
{
    namespace resolver
    {
        class PALUDIS_VISIBLE JobState :
            public virtual DeclareAbstractAcceptMethods<JobState, MakeTypeList<
                JobPendingState, JobActiveState, JobSucceededState, JobFailedState, JobSkippedState>::Type>
        {
            public:
                virtual ~JobState() = 0;

                virtual void serialise(Serialiser &) const = 0;
                static const std::shared_ptr<JobState> deserialise(Deserialisation &) PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class PALUDIS_VISIBLE JobPendingState :
            public JobState,
            public ImplementAcceptMethods<JobState, JobPendingState>
        {
            public:
                virtual void serialise(Serialiser &) const;
                static const std::shared_ptr<JobPendingState> deserialise(Deserialisation &) PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class PALUDIS_VISIBLE JobActiveState :
            private Pimp<JobActiveState>,
            public JobState,
            public ImplementAcceptMethods<JobState, JobActiveState>
        {
            public:
                JobActiveState();
                ~JobActiveState();

                void set_output_manager(const std::shared_ptr<OutputManager> &);
                const std::shared_ptr<JobSucceededState> succeeded() const PALUDIS_ATTRIBUTE((warn_unused_result));
                const std::shared_ptr<JobFailedState> failed() const PALUDIS_ATTRIBUTE((warn_unused_result));

                const std::shared_ptr<OutputManager> output_manager() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual void serialise(Serialiser &) const;
                static const std::shared_ptr<JobActiveState> deserialise(Deserialisation &) PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class PALUDIS_VISIBLE JobSucceededState :
            private Pimp<JobSucceededState>,
            public JobState,
            public ImplementAcceptMethods<JobState, JobSucceededState>
        {
            public:
                JobSucceededState(const std::shared_ptr<OutputManager> &);
                ~JobSucceededState();

                const std::shared_ptr<OutputManager> output_manager() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual void serialise(Serialiser &) const;
                static const std::shared_ptr<JobSucceededState> deserialise(Deserialisation &) PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class PALUDIS_VISIBLE JobFailedState :
            private Pimp<JobFailedState>,
            public JobState,
            public ImplementAcceptMethods<JobState, JobFailedState>
        {
            public:
                JobFailedState(const std::shared_ptr<OutputManager> &);
                ~JobFailedState();

                const std::shared_ptr<OutputManager> output_manager() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual void serialise(Serialiser &) const;
                static const std::shared_ptr<JobFailedState> deserialise(Deserialisation &) PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class PALUDIS_VISIBLE JobSkippedState :
            public JobState,
            public ImplementAcceptMethods<JobState, JobSkippedState>
        {
            public:
                virtual void serialise(Serialiser &) const;
                static const std::shared_ptr<JobSkippedState> deserialise(Deserialisation &) PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }
}

#endif
