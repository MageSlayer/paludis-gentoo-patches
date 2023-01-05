/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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
#include <paludis/util/fs_path-fwd.hh>

#include <string>
#include <iosfwd>
#include <memory>
#include <functional>
#include <initializer_list>
#include <vector>

#include <sys/types.h>
#include <unistd.h>

namespace paludis
{
    typedef std::function<std::string (const std::string &)> ProcessPipeCommandFunction;

    class PALUDIS_VISIBLE ProcessError :
        public Exception
    {
        public:
            ProcessError(const std::string &) noexcept;
            ProcessError(int error, const std::string &) noexcept;
    };

    class PALUDIS_VISIBLE ProcessCommand
    {
        private:
            Pimp<ProcessCommand> _imp;

        public:
            /**
             * List of arguments, one string per argv value.
             **/
            explicit ProcessCommand(const std::initializer_list<std::string> &);

            /**
             * List of arguments, passed to sh -c.
             */
            explicit ProcessCommand(const std::string &);

            ProcessCommand(ProcessCommand &&);
            ~ProcessCommand();

            ProcessCommand(const ProcessCommand &) = delete;
            ProcessCommand & operator= (const ProcessCommand &) = delete;

            void prepend_args(const std::initializer_list<std::string> &);
            void append_args(const std::initializer_list<std::string> &);

            void echo_command_to(std::ostream &);

            ProcessCommand & setenv(const std::string &, const std::string &);
            ProcessCommand & clearenv();
            ProcessCommand & chdir(const FSPath &);
            ProcessCommand & setuid_setgid(uid_t, gid_t);

            void exec_prepare();
            void exec(int err_fd = -1) PALUDIS_ATTRIBUTE((noreturn));

            const std::vector<std::string>& get_args();
            const std::string& get_args_string();
    };

    class PALUDIS_VISIBLE Process
    {
        private:
            Pimp<Process> _imp;

        public:
            explicit Process(ProcessCommand &&);
            ~Process();

            Process(const Process &) = delete;
            Process & operator= (const Process &) = delete;

            RunningProcessHandle run() PALUDIS_ATTRIBUTE((warn_unused_result));

            Process & setenv(const std::string &, const std::string &);
            Process & clearenv();
            Process & chdir(const FSPath &);
            Process & setuid_setgid(uid_t, gid_t);

            Process & capture_stdout(std::ostream &);
            Process & capture_stderr(std::ostream &);
            Process & capture_output_to_fd(std::ostream &, int fd_or_minus_one, const std::string & env_var_with_fd);
            Process & send_input_to_fd(std::istream &, int fd_or_minus_one, const std::string & env_var_with_fd);
            Process & set_stdin_fd(int);
            Process & pipe_command_handler(const std::string &, const ProcessPipeCommandFunction &);

            Process & use_ptys();
            Process & echo_command_to(std::ostream &);

            Process & prefix_stdout(const std::string &);
            Process & prefix_stderr(const std::string &);
            Process & extra_newlines_if_any_output_exists();

            Process & sandbox();
            Process & sydbox();

            /* NOTE: Do not use this functionality together with a
             *       multi-threaded process. Not only will all but the
             *       executing thread disappear, but locks the other
             *       threads held may be still locked or data protected by
             *       them might be in an inconsistent state. */
            Process & as_main_process();
    };

    class PALUDIS_VISIBLE RunningProcessHandle
    {
        private:
            Pimp<RunningProcessHandle> _imp;

        public:
            RunningProcessHandle(
                    const pid_t,
                    std::unique_ptr<RunningProcessThread> &&);

            ~RunningProcessHandle() noexcept(false);
            RunningProcessHandle(RunningProcessHandle &&);

            RunningProcessHandle(const RunningProcessHandle &) = delete;
            RunningProcessHandle & operator= (const RunningProcessHandle &) = delete;

            int wait() PALUDIS_ATTRIBUTE((warn_unused_result));
    };
}

#endif
