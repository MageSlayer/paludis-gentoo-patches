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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_JOB_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_JOB_HH 1

#include <paludis/resolver/job-fwd.hh>
#include <paludis/resolver/job_state-fwd.hh>
#include <paludis/resolver/job_requirements-fwd.hh>
#include <paludis/resolver/destination_types-fwd.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/visitor.hh>
#include <paludis/util/type_list.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/serialise-fwd.hh>
#include <paludis/name-fwd.hh>
#include <memory>

namespace paludis
{
    namespace resolver
    {
        class PALUDIS_VISIBLE PretendJob
        {
            private:
                Pimp<PretendJob> _imp;

            public:
                PretendJob(
                        const PackageDepSpec &,
                        const RepositoryName &,
                        const DestinationType
                        );
                ~PretendJob();

                const PackageDepSpec origin_id_spec() const PALUDIS_ATTRIBUTE((warn_unused_result));
                const RepositoryName destination_repository_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                DestinationType destination_type() const PALUDIS_ATTRIBUTE((warn_unused_result));

                static const std::shared_ptr<PretendJob> deserialise(Deserialisation &) PALUDIS_ATTRIBUTE((warn_unused_result));
                void serialise(Serialiser &) const;
        };

        class PALUDIS_VISIBLE ExecuteJob :
            public virtual DeclareAbstractAcceptMethods<ExecuteJob, MakeTypeList<
                FetchJob, InstallJob, UninstallJob>::Type>
        {
            public:
                virtual ~ExecuteJob() = default;

                virtual const std::shared_ptr<JobState> state() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
                virtual void set_state(const std::shared_ptr<JobState> &) = 0;

                virtual bool was_target() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual const std::shared_ptr<const JobRequirements> requirements() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                static const std::shared_ptr<ExecuteJob> deserialise(Deserialisation &) PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual void serialise(Serialiser &) const = 0;
        };

        class PALUDIS_VISIBLE FetchJob :
            public ExecuteJob,
            public ImplementAcceptMethods<ExecuteJob, FetchJob>
        {
            private:
                Pimp<FetchJob> _imp;

            public:
                FetchJob(
                        const std::shared_ptr<const JobRequirements> &,
                        const PackageDepSpec &,
                        const bool was_target
                        );
                ~FetchJob();

                const PackageDepSpec origin_id_spec() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::shared_ptr<JobState> state() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual void set_state(const std::shared_ptr<JobState> &);

                virtual bool was_target() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::shared_ptr<const JobRequirements> requirements() const PALUDIS_ATTRIBUTE((warn_unused_result));

                static const std::shared_ptr<FetchJob> deserialise(Deserialisation &) PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual void serialise(Serialiser &) const;
        };

        class PALUDIS_VISIBLE InstallJob :
            public ExecuteJob,
            public ImplementAcceptMethods<ExecuteJob, InstallJob>
        {
            private:
                Pimp<InstallJob> _imp;

            public:
                InstallJob(
                        const std::shared_ptr<const JobRequirements> &,
                        const PackageDepSpec &,
                        const RepositoryName &,
                        const DestinationType,
                        const std::shared_ptr<const Sequence<PackageDepSpec> > &,
                        const bool was_target);
                ~InstallJob();

                const PackageDepSpec origin_id_spec() const PALUDIS_ATTRIBUTE((warn_unused_result));
                const RepositoryName destination_repository_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                DestinationType destination_type() const PALUDIS_ATTRIBUTE((warn_unused_result));
                const std::shared_ptr<const Sequence<PackageDepSpec> > replacing_specs() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::shared_ptr<JobState> state() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual void set_state(const std::shared_ptr<JobState> &);

                virtual bool was_target() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::shared_ptr<const JobRequirements> requirements() const PALUDIS_ATTRIBUTE((warn_unused_result));

                static const std::shared_ptr<InstallJob> deserialise(Deserialisation &) PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual void serialise(Serialiser &) const;
        };

        class PALUDIS_VISIBLE UninstallJob :
            public ExecuteJob,
            public ImplementAcceptMethods<ExecuteJob, UninstallJob>
        {
            private:
                Pimp<UninstallJob> _imp;

            public:
                UninstallJob(
                        const std::shared_ptr<const JobRequirements> &,
                        const std::shared_ptr<const Sequence<PackageDepSpec> > &,
                        const bool was_target
                        );
                ~UninstallJob();

                const std::shared_ptr<const Sequence<PackageDepSpec> > ids_to_remove_specs() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::shared_ptr<JobState> state() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual void set_state(const std::shared_ptr<JobState> &);

                virtual bool was_target() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::shared_ptr<const JobRequirements> requirements() const PALUDIS_ATTRIBUTE((warn_unused_result));

                static const std::shared_ptr<UninstallJob> deserialise(Deserialisation &) PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual void serialise(Serialiser &) const;
        };
    }
}

#endif
