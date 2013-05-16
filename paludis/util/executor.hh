/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011, 2012 Ciaran McCreesh
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
#include <paludis/util/pimp.hh>
#include <paludis/util/attributes.hh>
#include <memory>
#include <mutex>
#include <string>

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
            virtual void flush_threaded() = 0;
            virtual void post_execute_exclusive() = 0;
    };

    class PALUDIS_VISIBLE Executor
    {
        private:
            Pimp<Executor> _imp;

            void _one(const std::shared_ptr<Executive>);

        public:
            explicit Executor(const int ms_update_interval = 1000);
            ~Executor();

            int pending() const;
            int active() const;
            int done() const;

            void add(const std::shared_ptr<Executive> & x);

            void execute();

            std::mutex & exclusivity_mutex() PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    extern template class Pimp<Executor>;
}

#endif
