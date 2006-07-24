/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_PIPE_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_PIPE_HH 1

#include <paludis/util/instantiation_policy.hh>

/** \file
 * Declaration for the Pipe class.
 *
 * \ingroup grppipe
 */

namespace paludis
{
    /**
     * Wrapper around pipe file descriptors.
     *
     * \ingroup grppipe
     */
    class Pipe :
        InstantiationPolicy<Pipe, instantiation_method::NonCopyableTag>
    {
        private:
            int _fds[2];

        public:
            Pipe();

            ~Pipe();

            int read_fd() const
            {
                return _fds[0];
            }

            int write_fd() const
            {
                return _fds[1];
            }
    };

}

#endif
