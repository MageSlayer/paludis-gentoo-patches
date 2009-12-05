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

#include <paludis/resolver/jobs.hh>
#include <paludis/resolver/job.hh>
#include <paludis/resolver/job_id.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/exception.hh>
#include <paludis/serialise-impl.hh>
#include <list>
#include <map>

using namespace paludis;
using namespace paludis::resolver;

typedef std::list<std::tr1::shared_ptr<Job> > JobsList;
typedef std::multimap<Resolvent, JobsList::const_iterator> JobsListByResolventIndex;

namespace paludis
{
    template <>
    struct Implementation<Jobs>
    {
        JobsList jobs;
        JobsListByResolventIndex jobs_list_by_resolvent_index;
    };
}

Jobs::Jobs() :
    PrivateImplementationPattern<Jobs>(new Implementation<Jobs>)
{
}

Jobs::~Jobs()
{
}

namespace
{
    struct Indexer
    {
        JobsListByResolventIndex & jobs_list_by_resolvent_index;
        JobsList::const_iterator i;

        Indexer(JobsListByResolventIndex & j, const JobsList::const_iterator & c) :
            jobs_list_by_resolvent_index(j),
            i(c)
        {
        }

        void visit(const UsableJob & j)
        {
            jobs_list_by_resolvent_index.insert(std::make_pair(j.resolution()->resolvent(), i));
        }

        void visit(const SimpleInstallJob & j)
        {
            jobs_list_by_resolvent_index.insert(std::make_pair(j.resolution()->resolvent(), i));
        }

        void visit(const UntakenInstallJob & j)
        {
            jobs_list_by_resolvent_index.insert(std::make_pair(j.resolution()->resolvent(), i));
        }

        void visit(const PretendJob & j)
        {
            jobs_list_by_resolvent_index.insert(std::make_pair(j.resolution()->resolvent(), i));
        }

        void visit(const FetchJob & j)
        {
            jobs_list_by_resolvent_index.insert(std::make_pair(j.resolution()->resolvent(), i));
        }

        void visit(const SyncPointJob &)
        {
        }
    };
}

void
Jobs::add(const std::tr1::shared_ptr<Job> & j)
{
    JobsList::const_iterator i(_imp->jobs.insert(_imp->jobs.end(), j));
    Indexer x(_imp->jobs_list_by_resolvent_index, i);
    j->accept(x);
}

namespace
{
    struct InstalledJobChecker
    {
        bool visit(const PretendJob &) const
        {
            return false;
        }

        bool visit(const SyncPointJob &) const
        {
            return false;
        }

        bool visit(const FetchJob &) const
        {
            return false;
        }

        bool visit(const UntakenInstallJob &) const
        {
            return true;
        }

        bool visit(const UsableJob &) const
        {
            return false;
        }

        bool visit(const SimpleInstallJob &) const
        {
            return true;
        }
    };

    struct UsableJobChecker
    {
        bool visit(const PretendJob &) const
        {
            return false;
        }

        bool visit(const SyncPointJob &) const
        {
            return false;
        }

        bool visit(const FetchJob &) const
        {
            return false;
        }

        bool visit(const UntakenInstallJob &) const
        {
            return true;
        }

        bool visit(const UsableJob &) const
        {
            return true;
        }

        bool visit(const SimpleInstallJob &) const
        {
            return false;
        }
    };
}

const JobID
Jobs::find_id_for_installed(const Resolvent & r) const
{
    std::pair<JobsListByResolventIndex::const_iterator, JobsListByResolventIndex::const_iterator> candidates(
                _imp->jobs_list_by_resolvent_index.equal_range(r));

    for ( ; candidates.first != candidates.second ; ++candidates.first)
    {
        if ((*candidates.first->second)->accept_returning<bool>(InstalledJobChecker()))
            return (*candidates.first->second)->id();
    }

    return find_id_for_usable(r);
}

const JobID
Jobs::find_id_for_usable(const Resolvent & r) const
{
    std::pair<JobsListByResolventIndex::const_iterator, JobsListByResolventIndex::const_iterator> candidates(
                _imp->jobs_list_by_resolvent_index.equal_range(r));

    for ( ; candidates.first != candidates.second ; ++candidates.first)
    {
        if ((*candidates.first->second)->accept_returning<bool>(UsableJobChecker()))
            return (*candidates.first->second)->id();
    }

    throw InternalError(PALUDIS_HERE, "no build job for " + stringify(r));
}

bool
Jobs::have_job_for_installed(const Resolvent & r) const
{
    std::pair<JobsListByResolventIndex::const_iterator, JobsListByResolventIndex::const_iterator> candidates(
                _imp->jobs_list_by_resolvent_index.equal_range(r));

    for ( ; candidates.first != candidates.second ; ++candidates.first)
    {
        if ((*candidates.first->second)->accept_returning<bool>(InstalledJobChecker()))
            return true;
    }

    return have_job_for_usable(r);
}

bool
Jobs::have_job_for_usable(const Resolvent & r) const
{
    std::pair<JobsListByResolventIndex::const_iterator, JobsListByResolventIndex::const_iterator> candidates(
                _imp->jobs_list_by_resolvent_index.equal_range(r));

    for ( ; candidates.first != candidates.second ; ++candidates.first)
    {
        if ((*candidates.first->second)->accept_returning<bool>(UsableJobChecker()))
            return true;
    }

    return false;
}

const std::tr1::shared_ptr<Job>
Jobs::fetch(const JobID & id)
{
    for (JobsList::const_iterator j(_imp->jobs.begin()), j_end(_imp->jobs.end()) ;
            j != j_end ; ++j)
        if ((*j)->id() == id)
            return *j;

    throw InternalError(PALUDIS_HERE, "no job for id");
}

const std::tr1::shared_ptr<const Job>
Jobs::fetch(const JobID & id) const
{
    for (JobsList::const_iterator j(_imp->jobs.begin()), j_end(_imp->jobs.end()) ;
            j != j_end ; ++j)
        if ((*j)->id() == id)
            return *j;

    throw InternalError(PALUDIS_HERE, "no job for id");
}

void
Jobs::serialise(Serialiser & s) const
{
    s.object("Jobs")
        .member(SerialiserFlags<serialise::container>(), "items", _imp->jobs)
        ;
}

const std::tr1::shared_ptr<Jobs>
Jobs::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "Jobs");
    Deserialisator vv(*v.find_remove_member("items"), "c");
    std::tr1::shared_ptr<Jobs> result(new Jobs);
    for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
        result->add(vv.member<std::tr1::shared_ptr<Job> >(stringify(n)));
    return result;
}

template class PrivateImplementationPattern<resolver::Jobs>;

