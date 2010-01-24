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

#include <paludis/resolver/jobs.hh>
#include <paludis/resolver/job.hh>
#include <paludis/resolver/job_id.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/serialise-impl.hh>
#include <tr1/unordered_map>
#include <list>
#include <map>

using namespace paludis;
using namespace paludis::resolver;

typedef std::list<std::tr1::shared_ptr<Job> > JobsList;
typedef std::multimap<Resolvent, JobsList::const_iterator> JobsListByResolventIndex;
typedef std::tr1::unordered_map<JobID, JobsList::const_iterator, Hash<JobID> > JobsListByJobIDIndex;

namespace paludis
{
    template <>
    struct Implementation<Jobs>
    {
        JobsList jobs;
        JobsListByResolventIndex jobs_list_by_resolvent_index;
        JobsListByJobIDIndex jobs_list_by_job_id_index;
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

        void visit(const UsableGroupJob &)
        {
        }

        void visit(const SimpleInstallJob & j)
        {
            jobs_list_by_resolvent_index.insert(std::make_pair(j.resolution()->resolvent(), i));
        }

        void visit(const ErrorJob & j)
        {
            jobs_list_by_resolvent_index.insert(std::make_pair(j.resolution()->resolvent(), i));
        }

        void visit(const FetchJob & j)
        {
            jobs_list_by_resolvent_index.insert(std::make_pair(j.resolution()->resolvent(), i));
        }
    };
}

void
Jobs::add(const std::tr1::shared_ptr<Job> & j)
{
    JobsList::const_iterator i(_imp->jobs.insert(_imp->jobs.end(), j));
    Indexer x(_imp->jobs_list_by_resolvent_index, i);
    j->accept(x);
    _imp->jobs_list_by_job_id_index.insert(std::make_pair(j->id(), i));
}

namespace
{
    struct InstalledJobChecker
    {
        bool visit(const FetchJob &) const
        {
            return false;
        }

        bool visit(const ErrorJob &) const
        {
            return true;
        }

        bool visit(const UsableJob &) const
        {
            return false;
        }

        bool visit(const UsableGroupJob &) const
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
        bool visit(const FetchJob &) const
        {
            return false;
        }

        bool visit(const ErrorJob &) const
        {
            return true;
        }

        bool visit(const UsableGroupJob &) const
        {
            return false;
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
    JobsListByJobIDIndex::const_iterator i(_imp->jobs_list_by_job_id_index.find(id));
    if (i == _imp->jobs_list_by_job_id_index.end())
        throw InternalError(PALUDIS_HERE, "no job for id");
    return *i->second;
}

const std::tr1::shared_ptr<const Job>
Jobs::fetch(const JobID & id) const
{
    JobsListByJobIDIndex::const_iterator i(_imp->jobs_list_by_job_id_index.find(id));
    if (i == _imp->jobs_list_by_job_id_index.end())
        throw InternalError(PALUDIS_HERE, "no job for id");
    return *i->second;
}

template <typename J_>
const std::tr1::shared_ptr<const J_>
Jobs::fetch_as(const JobID & id) const
{
    JobsListByJobIDIndex::const_iterator i(_imp->jobs_list_by_job_id_index.find(id));
    if (i == _imp->jobs_list_by_job_id_index.end())
        throw InternalError(PALUDIS_HERE, "no job for id");
    if (simple_visitor_cast<const J_>(**i->second))
        return std::tr1::static_pointer_cast<const J_>(*i->second);
    else
        throw InternalError(PALUDIS_HERE, "wrong job type id");
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

template const std::tr1::shared_ptr<const SimpleInstallJob> Jobs::fetch_as<SimpleInstallJob>(const JobID &) const;
template const std::tr1::shared_ptr<const ErrorJob> Jobs::fetch_as<ErrorJob>(const JobID &) const;
template const std::tr1::shared_ptr<const FetchJob> Jobs::fetch_as<FetchJob>(const JobID &) const;
template const std::tr1::shared_ptr<const UsableJob> Jobs::fetch_as<UsableJob>(const JobID &) const;

