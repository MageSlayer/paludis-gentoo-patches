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
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <string>

namespace paludis
{
    namespace erepository
    {
        class EAPIPhase :
            private PrivateImplementationPattern<EAPIPhase>
        {
            public:
                explicit EAPIPhase(const std::string &);
                ~EAPIPhase();

                bool option(const std::string &) const;

                typedef libwrapiter::ForwardIterator<EAPIPhase, const std::string> Iterator;
                Iterator begin_commands() const;
                Iterator end_commands() const;
        };

        class EAPIPhases :
            private PrivateImplementationPattern<EAPIPhases>
        {
            public:
                explicit EAPIPhases(const std::string &);
                ~EAPIPhases();

                typedef libwrapiter::ForwardIterator<EAPIPhases, const EAPIPhase> Iterator;
                Iterator begin_phases() const;
                Iterator end_phases() const;
        };
    }
}

#endif
