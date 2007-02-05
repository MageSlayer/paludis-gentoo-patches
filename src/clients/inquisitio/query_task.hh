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

#ifndef PALUDIS_GUARD_SRC_CLIENTS_INQUISITIO_QUERY_TASK_HH
#define PALUDIS_GUARD_SRC_CLIENTS_INQUISITIO_QUERY_TASK_HH 1

#include <src/output/console_query_task.hh>

namespace inquisitio
{
    class InquisitioQueryTask :
        public paludis::ConsoleQueryTask
    {
        public:
            InquisitioQueryTask(const paludis::Environment * const);

            virtual bool want_deps() const
            {
                return false;
            }

            virtual bool want_raw() const
            {
                return false;
            }
    };
}

#endif
