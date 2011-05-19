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
#include <paludis/environment.hh>
#include <paludis/repository.hh>
#include <paludis/package_id.hh>
#include <paludis/name.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filter_handler.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>

#include <paludis/util/pimp-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/damerau_levenshtein.hh>
#include <paludis/util/options.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/set-impl.hh>
#include <paludis/util/sequence-impl.hh>

#include <list>
#include <algorithm>
#include <set>

#include <cctype>

using namespace paludis;

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

    class FuzzyPackageNameFilterHandler :
        public AllFilterHandlerBase
    {
        private:
            std::string _package;
            DamerauLevenshtein _distance_calculator;
            unsigned _threshold;
            char _first_char;

        public:
            FuzzyPackageNameFilterHandler(const std::string & package) :
                _package(package),
                _distance_calculator(tolower_0_cost(package)),
                _threshold(package.length() <= 4 ? 1 : 2),
                _first_char(tolower(package[0]))
            {
            }

            virtual std::shared_ptr<const QualifiedPackageNameSet> packages(
                    const Environment * const,
                    const std::shared_ptr<const RepositoryNameSet> &,
                    const std::shared_ptr<const QualifiedPackageNameSet> &) const;

            virtual std::string as_string() const
            {
                return "packages fuzzily like " + _package;
            }
    };

    std::shared_ptr<const QualifiedPackageNameSet>
    FuzzyPackageNameFilterHandler::packages(const Environment * const,
            const std::shared_ptr<const RepositoryNameSet> &,
            const std::shared_ptr<const QualifiedPackageNameSet> & pkgs) const
    {
        std::shared_ptr<QualifiedPackageNameSet> result(std::make_shared<QualifiedPackageNameSet>());

        for (QualifiedPackageNameSet::ConstIterator p(pkgs->begin()),
                    p_end(pkgs->end()); p_end != p; ++p)
            if (((_package.length() >= 3) && (std::string::npos != stringify(p->package()).find(_package))) || (
                        tolower(p->package().value()[0]) == _first_char &&
                        _distance_calculator.distance_with(tolower_0_cost(p->package().value())) <= _threshold))
                result->insert(*p);

        return result;
    }

    class FuzzyPackageName :
        public Filter
    {
        public:
            FuzzyPackageName(const std::string & p) :
                Filter(std::make_shared<FuzzyPackageNameFilterHandler>(p))
            {
            }
    };
}

namespace paludis
{
    template <>
    struct Imp<FuzzyCandidatesFinder>
    {
        std::list<QualifiedPackageName> candidates;
    };

    template <>
    struct WrappedForwardIteratorTraits<FuzzyCandidatesFinder::CandidatesConstIteratorTag>
    {
        typedef std::list<QualifiedPackageName>::const_iterator UnderlyingIterator;
    };
}

FuzzyCandidatesFinder::FuzzyCandidatesFinder(const Environment & e, const std::string & name, const Filter & filter) :
    _imp()
{
    Generator g = generator::All();
    std::string package(name);

    if (std::string::npos != name.find('/'))
    {
        PackageDepSpec pds(parse_user_package_dep_spec(name, &e, { }));

        if (pds.package_ptr())
        {
            g = g & generator::Category(pds.package_ptr()->category());
            package = stringify(pds.package_ptr()->package());
        }

        if (pds.in_repository_ptr())
            g = g & generator::InRepository(*pds.in_repository_ptr());

        if (pds.from_repository_ptr())
            g = g & generator::FromRepository(*pds.from_repository_ptr());
    }

    std::shared_ptr<const PackageIDSequence> ids(e[selection::BestVersionOnly(g | FuzzyPackageName(package) | filter)]);

    for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end())
            ; i != i_end ; ++i)
        _imp->candidates.push_back((*i)->name());
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
    struct Imp<FuzzyRepositoriesFinder>
    {
        std::list<RepositoryName> candidates;
    };

    template <>
    struct WrappedForwardIteratorTraits<FuzzyRepositoriesFinder::RepositoriesConstIteratorTag>
    {
        typedef std::list<RepositoryName>::const_iterator UnderlyingIterator;
    };
}

FuzzyRepositoriesFinder::FuzzyRepositoriesFinder(const Environment & e, const std::string & name) :
    _imp()
{
    DamerauLevenshtein distance_calculator(tolower_0_cost(name));

    unsigned threshold(name.length() <= 4 ? 1 : 2);

    if (0 != name.compare(0, 2, std::string("x-")))
    {
        RepositoryName xname(std::string("x-" + name));
        if (e.has_repository_named(xname))
        {
            _imp->candidates.push_back(xname);
            return;
        }
    }

    for (auto r(e.begin_repositories()), r_end(e.end_repositories()) ; r != r_end ; ++r)
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

template class WrappedForwardIterator<FuzzyCandidatesFinder::CandidatesConstIteratorTag, const QualifiedPackageName>;
template class WrappedForwardIterator<FuzzyRepositoriesFinder::RepositoriesConstIteratorTag, const RepositoryName>;

