/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Fernando J. Pereda
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

#include <paludis/fuzzy_finder.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/damerau_levenshtein.hh>
#include <paludis/package_database.hh>
#include <paludis/environment.hh>
#include <paludis/repository.hh>
#include <paludis/package_id.hh>
#include <paludis/name.hh>
#include <paludis/query.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <list>
#include <algorithm>
#include <set>

#include <cctype>

using namespace paludis;

template class WrappedForwardIterator<FuzzyCandidatesFinder::CandidatesConstIteratorTag, const QualifiedPackageName>;
template class WrappedForwardIterator<FuzzyRepositoriesFinder::RepositoriesConstIteratorTag, const RepositoryName>;

namespace
{
    bool char_0_cost(char c)
    {
        return (c == '-' || c == '_');
    }

    std::string tolower_0_cost(const std::string & s)
    {
        std::string res(s);
        std::string::iterator e(std::remove_if(res.begin(), res.end(), char_0_cost));
        std::string res2(res.begin(), e);
        std::transform(res.begin(), e, res2.begin(), ::tolower);
        return res2;
    }
}

namespace paludis
{
    template <>
    struct Implementation<FuzzyCandidatesFinder>
    {
        std::list<QualifiedPackageName> candidates;
    };
}

FuzzyCandidatesFinder::FuzzyCandidatesFinder(const Environment & e, const std::string & name, const Query & generator) :
    PrivateImplementationPattern<FuzzyCandidatesFinder>(new Implementation<FuzzyCandidatesFinder>)
{
    Query real_generator(generator);
    std::string package(name);

    if (std::string::npos != name.find('/'))
    {
        PackageDepSpec pds(name, pds_pm_permissive);

        if (pds.package_ptr())
        {
            real_generator = real_generator & query::Category(pds.package_ptr()->category);
            package = stringify(pds.package_ptr()->package);
        }

        if (pds.repository_ptr())
            real_generator = real_generator & query::Repository(*pds.repository_ptr());
    }

    std::string package_0_cost(tolower_0_cost(package));
    DamerauLevenshtein distance_calculator(package);

    unsigned threshold(package.length() <= 4 ? 1 : 2);

    QualifiedPackageNameSet potential_candidates;

    tr1::shared_ptr<const PackageIDSequence> ids(e.package_database()->query(real_generator, qo_whatever));

    for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end())
            ; i != i_end ; ++i)
    {
        if (tolower(stringify((*i)->name().package)[0]) != tolower(package[0]))
            continue;
        potential_candidates.insert((*i)->name());
    }

    for (QualifiedPackageNameSet::ConstIterator p(potential_candidates.begin()), p_end(potential_candidates.end()) ;
            p != p_end ; ++p)
        if (distance_calculator.distance_with(tolower_0_cost(stringify(p->package))) <= threshold)
            _imp->candidates.push_back(*p);
}

FuzzyCandidatesFinder::~FuzzyCandidatesFinder()
{
}

FuzzyCandidatesFinder::CandidatesConstIterator
FuzzyCandidatesFinder::begin() const
{
    return CandidatesConstIterator(_imp->candidates.begin());
}

FuzzyCandidatesFinder::CandidatesConstIterator
FuzzyCandidatesFinder::end() const
{
    return CandidatesConstIterator(_imp->candidates.end());
}

namespace paludis
{
    template <>
    struct Implementation<FuzzyRepositoriesFinder>
    {
        std::list<RepositoryName> candidates;
    };
}

FuzzyRepositoriesFinder::FuzzyRepositoriesFinder(const Environment & e, const std::string & name) :
    PrivateImplementationPattern<FuzzyRepositoriesFinder>(new Implementation<FuzzyRepositoriesFinder>)
{
    DamerauLevenshtein distance_calculator(tolower_0_cost(name));

    unsigned threshold(name.length() <= 4 ? 1 : 2);

    if (0 != name.compare(0, 2, std::string("x-")))
    {
        RepositoryName xname(std::string("x-" + name));
        if (e.package_database()->has_repository_named(xname))
        {
            _imp->candidates.push_back(xname);
            return;
        }
    }

    for (PackageDatabase::RepositoryConstIterator r(e.package_database()->begin_repositories()),
            r_end(e.package_database()->end_repositories()) ; r != r_end ; ++r)
        if (distance_calculator.distance_with(tolower_0_cost(stringify((*r)->name()))) <= threshold)
            _imp->candidates.push_back((*r)->name());
}

FuzzyRepositoriesFinder::~FuzzyRepositoriesFinder()
{
}

FuzzyRepositoriesFinder::RepositoriesConstIterator
FuzzyRepositoriesFinder::begin() const
{
    return RepositoriesConstIterator(_imp->candidates.begin());
}

FuzzyRepositoriesFinder::RepositoriesConstIterator
FuzzyRepositoriesFinder::end() const
{
    return RepositoriesConstIterator(_imp->candidates.end());
}
