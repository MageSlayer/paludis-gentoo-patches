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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_JOB_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_JOB_HH 1

#include <paludis/resolver/job-fwd.hh>
#include <paludis/resolver/job_id-fwd.hh>
#include <paludis/resolver/arrow-fwd.hh>
#include <paludis/resolver/resolution-fwd.hh>
#include <paludis/resolver/decision-fwd.hh>
#include <paludis/resolver/sync_point-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/simple_visitor.hh>
#include <paludis/util/type_list.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/serialise-fwd.hh>
#include <tr1/memory>

namespace paludis
{
    namespace resolver
    {
        class PALUDIS_VISIBLE Job :
            public virtual DeclareAbstractAcceptMethods<Job, MakeTypeList<
                UsableJob, SimpleInstallJob, PretendJob, FetchJob, UntakenInstallJob, SyncPointJob>::Type>
        {
            public:
                virtual ~Job() = 0;

                virtual const JobID id()
                    const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual const std::tr1::shared_ptr<ArrowSequence> arrows()
                    const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual void serialise(Serialiser &) const = 0;

                static const std::tr1::shared_ptr<Job> deserialise(
                        Deserialisation & d) PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class PALUDIS_VISIBLE UsableJob :
            public Job,
            public ImplementAcceptMethods<Job, UsableJob>,
            private PrivateImplementationPattern<UsableJob>
        {
            public:
                UsableJob(const std::tr1::shared_ptr<const Resolution> &);
                ~UsableJob();

                virtual const JobID id() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::tr1::shared_ptr<ArrowSequence> arrows()
                    const PALUDIS_ATTRIBUTE((warn_unused_result));

                const std::tr1::shared_ptr<const Resolution> resolution() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual void serialise(Serialiser &) const;
        };

        class PALUDIS_VISIBLE PretendJob :
            public Job,
            public ImplementAcceptMethods<Job, PretendJob>,
            private PrivateImplementationPattern<PretendJob>
        {
            public:
                PretendJob(
                        const std::tr1::shared_ptr<const Resolution> &,
                        const std::tr1::shared_ptr<const ChangesToMakeDecision> &);
                ~PretendJob();

                virtual const JobID id() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::tr1::shared_ptr<ArrowSequence> arrows()
                    const PALUDIS_ATTRIBUTE((warn_unused_result));

                const std::tr1::shared_ptr<const Resolution> resolution() const PALUDIS_ATTRIBUTE((warn_unused_result));
                const std::tr1::shared_ptr<const ChangesToMakeDecision> decision() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual void serialise(Serialiser &) const;
        };

        class PALUDIS_VISIBLE FetchJob :
            public Job,
            public ImplementAcceptMethods<Job, FetchJob>,
            private PrivateImplementationPattern<FetchJob>
        {
            public:
                FetchJob(
                        const std::tr1::shared_ptr<const Resolution> &,
                        const std::tr1::shared_ptr<const ChangesToMakeDecision> &);
                ~FetchJob();

                virtual const JobID id() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::tr1::shared_ptr<ArrowSequence> arrows()
                    const PALUDIS_ATTRIBUTE((warn_unused_result));

                const std::tr1::shared_ptr<const Resolution> resolution() const PALUDIS_ATTRIBUTE((warn_unused_result));
                const std::tr1::shared_ptr<const ChangesToMakeDecision> decision() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual void serialise(Serialiser &) const;
        };

        class PALUDIS_VISIBLE SimpleInstallJob :
            public Job,
            public ImplementAcceptMethods<Job, SimpleInstallJob>,
            private PrivateImplementationPattern<SimpleInstallJob>
        {
            public:
                SimpleInstallJob(
                        const std::tr1::shared_ptr<const Resolution> &,
                        const std::tr1::shared_ptr<const ChangesToMakeDecision> &);
                ~SimpleInstallJob();

                virtual const JobID id() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::tr1::shared_ptr<ArrowSequence> arrows()
                    const PALUDIS_ATTRIBUTE((warn_unused_result));

                const std::tr1::shared_ptr<const Resolution> resolution() const PALUDIS_ATTRIBUTE((warn_unused_result));
                const std::tr1::shared_ptr<const ChangesToMakeDecision> decision() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual void serialise(Serialiser &) const;
        };

        class PALUDIS_VISIBLE SyncPointJob :
            public Job,
            public ImplementAcceptMethods<Job, SyncPointJob>,
            private PrivateImplementationPattern<SyncPointJob>
        {
            public:
                SyncPointJob(const SyncPoint);
                ~SyncPointJob();

                virtual const JobID id() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::tr1::shared_ptr<ArrowSequence> arrows()
                    const PALUDIS_ATTRIBUTE((warn_unused_result));

                SyncPoint sync_point() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual void serialise(Serialiser &) const;
        };

        class PALUDIS_VISIBLE UntakenInstallJob :
            public Job,
            public ImplementAcceptMethods<Job, UntakenInstallJob>,
            private PrivateImplementationPattern<UntakenInstallJob>
        {
            public:
                UntakenInstallJob(const std::tr1::shared_ptr<const Resolution> &);
                ~UntakenInstallJob();

                virtual const JobID id() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::tr1::shared_ptr<ArrowSequence> arrows()
                    const PALUDIS_ATTRIBUTE((warn_unused_result));

                const std::tr1::shared_ptr<const Resolution> resolution() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual void serialise(Serialiser &) const;
        };
    }

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class PrivateImplementationPattern<resolver::UsableJob>;
    extern template class PrivateImplementationPattern<resolver::PretendJob>;
    extern template class PrivateImplementationPattern<resolver::FetchJob>;
    extern template class PrivateImplementationPattern<resolver::SimpleInstallJob>;
    extern template class PrivateImplementationPattern<resolver::SyncPointJob>;
    extern template class PrivateImplementationPattern<resolver::UntakenInstallJob>;
#endif
}

#endif
