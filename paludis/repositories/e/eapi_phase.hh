/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_GENTOO_EAPI_PHASE_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_GENTOO_EAPI_PHASE_HH 1

#include <paludis/util/private_implementation_pattern.hh>
#include <libwrapiter/libwrapiter_forward_iterator-fwd.hh>
#include <string>

namespace paludis
{
    namespace erepository
    {
        /**
         * Holds a parsed EAPI phase.
         *
         * \nosubgrouping
         * \ingroup grperepository
         * \ingroup grpeapi
         */
        class EAPIPhase :
            private PrivateImplementationPattern<EAPIPhase>
        {
            public:
                ///\name Basic operations
                ///\{

                explicit EAPIPhase(const std::string &);
                ~EAPIPhase();

                ///\}

                ///\name Information about the phase
                ///\{

                bool option(const std::string &) const;
                bool option_contains(const std::string &, const std::string &) const;

                typedef libwrapiter::ForwardIterator<EAPIPhase, const std::string> ConstIterator;
                ConstIterator begin_commands() const;
                ConstIterator end_commands() const;

                ///\}
        };

        /**
         * Holds parsed EAPI phases.
         *
         * \nosubgrouping
         * \ingroup grperepository
         * \ingroup grpeapi
         */
        class EAPIPhases :
            private PrivateImplementationPattern<EAPIPhases>
        {
            public:
                ///\name Basic operations
                ///\{

                explicit EAPIPhases(const std::string &);
                ~EAPIPhases();

                ///\}

                ///\name Information about the phases
                ///\{

                typedef libwrapiter::ForwardIterator<EAPIPhases, const EAPIPhase> ConstIterator;
                ConstIterator begin_phases() const;
                ConstIterator end_phases() const;

                ///\}
        };
    }
}

#endif
