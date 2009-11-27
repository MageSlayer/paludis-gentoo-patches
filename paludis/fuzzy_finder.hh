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

#ifndef PALUDIS_GUARD_PALUDIS_FUZZY_FINDER_HH
#define PALUDIS_GUARD_PALUDIS_FUZZY_FINDER_HH 1

#include <paludis/environment-fwd.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/name.hh>
#include <paludis/filter-fwd.hh>
#include <string>

/** \file
 * Declarations for paludis::FuzzyCandidatesFinder and paludis::FuzzyRepositoriesFinder
 *
 * \ingroup g_package_database
 */

namespace paludis
{
    /**
     * Object to find package names to a given name
     *
     * \ingroup g_package_database
     */
    class PALUDIS_VISIBLE FuzzyCandidatesFinder :
        private PrivateImplementationPattern<FuzzyCandidatesFinder>
    {
        public:
            ///\name Basic Operations
            ///\{

            FuzzyCandidatesFinder(const Environment & e, const std::string & name, const Filter & filter);
            ~FuzzyCandidatesFinder();

            ///\}

            ///\name Iterate over the candidates
            ///\{

            struct CandidatesConstIteratorTag;
            typedef WrappedForwardIterator<CandidatesConstIteratorTag, const QualifiedPackageName>
                CandidatesConstIterator;

            CandidatesConstIterator begin() const;
            CandidatesConstIterator end() const;

            ///\}
    };

    /**
     * Object to find repositories similar to a given name
     *
     * \ingroup g_package_database
     */
    class PALUDIS_VISIBLE FuzzyRepositoriesFinder :
        private PrivateImplementationPattern<FuzzyRepositoriesFinder>
    {
        public:
            ///\name Basic Operations
            ///\{

            FuzzyRepositoriesFinder(const Environment & e, const std::string & name);
            ~FuzzyRepositoriesFinder();

            ///\}

            ///\name Iterate over the candidates
            ///\{

            struct RepositoriesConstIteratorTag;
            typedef WrappedForwardIterator<RepositoriesConstIteratorTag, const RepositoryName>
                RepositoriesConstIterator;

            RepositoriesConstIterator begin() const;
            RepositoriesConstIterator end() const;

            ///\}
    };

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class WrappedForwardIterator<FuzzyCandidatesFinder::CandidatesConstIteratorTag, const QualifiedPackageName>;
    extern template class WrappedForwardIterator<FuzzyRepositoriesFinder::RepositoriesConstIteratorTag, const RepositoryName>;
#endif

}

#endif
