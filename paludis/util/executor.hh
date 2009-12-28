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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_EXECUTOR_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_EXECUTOR_HH 1

#include <paludis/util/executor-fwd.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <string>
#include <tr1/memory>

namespace paludis
{
    class PALUDIS_VISIBLE Executive
    {
        public:
            virtual ~Executive() = 0;

            virtual std::string queue_name() const = 0;
            virtual std::string unique_id() const = 0;
            virtual bool can_run() const = 0;

            virtual void pre_execute_exclusive() = 0;
            virtual void execute_threaded() = 0;
            virtual void post_execute_exclusive() = 0;
    };

    class PALUDIS_VISIBLE Executor :
        private PrivateImplementationPattern<Executor>
    {
        private:
            void _one(const std::tr1::shared_ptr<Executive>);

        public:
            Executor();
            ~Executor();

            int pending() const;
            int active() const;
            int done() const;

            void add(const std::tr1::shared_ptr<Executive> & x);

            void execute();
    };

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class PrivateImplementationPattern<Executor>;
#endif
}

#endif
