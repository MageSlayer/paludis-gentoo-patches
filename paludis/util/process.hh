/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_PROCESS_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_PROCESS_HH 1

#include <paludis/util/process-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/fs_entry-fwd.hh>

#include <string>
#include <iosfwd>
#include <memory>
#include <initializer_list>

#include <sys/types.h>
#include <unistd.h>

namespace paludis
{
    class PALUDIS_VISIBLE ProcessError :
        public Exception
    {
        public:
            ProcessError(const std::string &) throw ();
    };

    class PALUDIS_VISIBLE ProcessCommand :
        private Pimp<ProcessCommand>
    {
        public:
            /**
             * List of arguments, one string per argv value.
             **/
            explicit ProcessCommand(const std::initializer_list<std::string> &);

            ProcessCommand(ProcessCommand &&);
            ~ProcessCommand();

            ProcessCommand(const ProcessCommand &) = delete;
            ProcessCommand & operator= (const ProcessCommand &) = delete;

            void exec() PALUDIS_ATTRIBUTE((noreturn));
    };

    class PALUDIS_VISIBLE Process :
        private Pimp<Process>
    {
        public:
            explicit Process(ProcessCommand &&);
            ~Process();

            Process(const Process &) = delete;
            Process & operator= (const Process &) = delete;

            RunningProcessHandle run() PALUDIS_ATTRIBUTE((warn_unused_result));

            Process & capture_stdout(std::ostream &);
            Process & capture_stderr(std::ostream &);
            Process & setenv(const std::string &, const std::string &);
            Process & chdir(const FSEntry &);
            Process & use_ptys();
            Process & setuid_setgid(uid_t, gid_t);
    };

    class PALUDIS_VISIBLE RunningProcessHandle :
        private Pimp<RunningProcessHandle>
    {
        public:
            RunningProcessHandle(
                    const pid_t,
                    std::unique_ptr<RunningProcessThread> &&);

            ~RunningProcessHandle();
            RunningProcessHandle(RunningProcessHandle &&);

            RunningProcessHandle(const RunningProcessHandle &) = delete;
            RunningProcessHandle & operator= (const RunningProcessHandle &) = delete;

            int wait() PALUDIS_ATTRIBUTE((warn_unused_result));
    };
}

#endif
