/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_FD_HOLDER_HH
#define PALUDIS_GUARD_PALUDIS_FD_HOLDER_HH 1

#include <unistd.h>

namespace paludis
{
    /**
     * RAII holder for a file descriptor.
     */
    class PALUDIS_VISIBLE FDHolder
    {
        private:
            const int _fd;
            const bool _sync;

        public:
            ///\name Basic operations
            ///\{

            FDHolder(const int fd, bool sync = true) :
                _fd(fd),
                _sync(sync)
            {
            }

            ~FDHolder()
            {
                if (-1 != _fd)
                {
                    if (_sync)
                        ::fsync(_fd);
                    ::close(_fd);
                }
            }

            ///\}

            /**
             * Fetch our file descriptor.
             */
            operator int () const
            {
                return _fd;
            }
    };
}

#endif
