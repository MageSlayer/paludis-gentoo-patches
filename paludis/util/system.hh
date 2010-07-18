/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_SYSTEM_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_SYSTEM_HH 1

#include <paludis/util/exception.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/wrapped_forward_iterator-fwd.hh>
#include <tr1/memory>
#include <tr1/functional>
#include <string>
#include <sys/types.h>

/** \file
 * Various system utilities.
 *
 * \ingroup g_system
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    class FSEntry;

    /**
     * Thrown if getenv_or_error fails.
     *
     * \ingroup g_exceptions
     * \ingroup g_system
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE GetenvError : public Exception
    {
        public:
            ///\name Basic operations
            ///\{

            GetenvError(const std::string & key) throw ();

            ///\}
    };

    /**
     * Thrown if fork, wait or chdir fail when running a command.
     *
     * \ingroup g_exceptions
     * \ingroup g_system
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE RunCommandError : public Exception
    {
        public:
            /**
             * Constructor.
             */
            RunCommandError(const std::string & message) throw ();
    };

    /**
     * Fetch the value of environment variable key, or def if the variable is
     * not defined.
     *
     * \ingroup g_system
     */
    std::string getenv_with_default(const std::string & key, const std::string & def) PALUDIS_VISIBLE;

    /**
     * Fetch the value of environment variable key, or throw a GetenvError if
     * the variable is not defined.
     *
     * \ingroup g_system
     */
    std::string getenv_or_error(const std::string & key) PALUDIS_VISIBLE;

    /**
     * Fetch the kernel version, for $KV.
     *
     * \ingroup g_system
     */
    std::string kernel_version() PALUDIS_VISIBLE;

    /**
     * A command to be run.
     *
     * \see PStream
     * \see run_command
     * \ingroup g_system
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE Command :
        private PrivateImplementationPattern<Command>
    {
        public:
            ///\name Basic operations
            ///\{

            Command(const std::string &);
            Command(const char * const);
            Command(const Command &);
            const Command & operator= (const Command &);
            ~Command();

            ///\}

            ///\name Change command execution options
            ///\{

            /**
             * Include a chdir before we run our command.
             */
            Command & with_chdir(const FSEntry &);

            /**
             * Add a setenv before we run our command.
             */
            Command & with_setenv(const std::string &, const std::string &);

            /**
             * Remove (most) existing environment variables before
             * setting those added with with_setenv.
             *
             * \since 0.36
             */
            Command & with_clearenv();

            /**
             * Run our command sandboxed.
             */
            Command & with_sandbox();

            /**
             * Run our command sydboxed
             *
             * \since 0.38
             */
            Command & with_sydbox();

            /**
             * Echo the command to be run to stderr before running it.
             */
            Command & with_echo_to_stderr();

            /**
             * Add a setuid and setgid before running our command.
             */
            Command & with_uid_gid(const uid_t, const gid_t);

            /**
             * Prefix stdout output with this.
             */
            Command & with_stdout_prefix(const std::string &);

            /**
             * Prefix stderr output with this.
             */
            Command & with_stderr_prefix(const std::string &);

            /**
             * If prefixing, and if the output contains only blanks, don't display
             * any output.
             */
            Command & with_prefix_discard_blank_output();

            /**
             * If prefixing, prefix blank lines too.
             */
            Command & with_prefix_blank_lines();

            /**
             * Specify a pipe command handler.
             *
             * \param env_var_prefix has _READ_FD and _WRITE_FD appended to it.
             *     \since 0.44, previously was always PALUDIS_PIPE_COMMAND.
             */
            Command & with_pipe_command_handler(
                    const std::string & env_var_prefix,
                    const std::tr1::function<std::string (const std::string &)> &);

            /**
             * Specify a stream to which stdout is captured and written.
             */
            Command & with_captured_stdout_stream(std::ostream * const);

            /**
             * Specify a stream to which stderr is captured and written.
             *
             * \since 0.30
             */
            Command & with_captured_stderr_stream(std::ostream * const);

            /**
             * Capture input from a particular FD to a stream.
             *
             * May only be called once.
             *
             * \param fd Read to this FD. If -1, pick an unused FD.
             * \param env_var If not empty, put the FD chosen in this env var.
             *
             * \since 0.50
             */
            Command & with_output_stream(
                    std::ostream * const,
                    int fd,
                    const std::string & env_var);

            /**
             * Send the contents of a stream in via a particular FD.
             *
             * May only be called once.
             *
             * \param fd Send the stream to this FD. If -1, pick an unused FD.
             * \param env_var If not empty, put the FD chosen in this env var.
             *
             * \since 0.40
             */
            Command & with_input_stream(
                    std::istream * const,
                    int fd,
                    const std::string & env_var);

            /**
             * Use ptys instead of pipes to capture stdout and/or stderr.
             *
             * \since 0.38
             */
            Command & with_ptys();

            ///\}

            ///\name Fetch command execution options
            ///\{

            /**
             * Our command, as a string.
             */
            std::string command() const;

            /**
             * Where to chdir, as a string.
             */
            std::string chdir() const;

            /**
             * Echo ourself to stderr.
             */
            void echo_to_stderr() const;

            /**
             * The uid for setuid.
             */
            std::tr1::shared_ptr<const uid_t> uid() const;

            /**
             * The gid for setgid.
             */
            std::tr1::shared_ptr<const gid_t> gid() const;

            /**
             * The stdout prefix.
             */
            std::string stdout_prefix() const;

            /**
             * The stderr prefix.
             */
            std::string stderr_prefix() const;

            /**
             * If prefixing, and if the output contains only blanks, don't display
             * any output?
             */
            bool prefix_discard_blank_output() const;

            /**
             * Prefix blank lines?
             */
            bool prefix_blank_lines() const;

            /**
             * The pipe command handler.
             */
            const std::tr1::function<std::string (const std::string &)> & pipe_command_handler() const;

            /**
             * The pipe command env var prefix.
             *
             * \since 0.44
             */
            const std::string pipe_command_env_var_prefix() const;

            /**
             * The captured stdout stream, or null.
             */
            std::ostream * captured_stdout_stream() const;

            /**
             * The captured stderr stream, or null.
             *
             * \since 0.30
             */
            std::ostream * captured_stderr_stream() const;

            /**
             * The input stream, or null.
             *
             * \since 0.40
             */
            std::istream * input_stream() const;

            /**
             * The input FD, if input_stream() not null.
             *
             * \since 0.40
             */
            int input_fd() const;

            /**
             * The input FD env var, if input_stream() not null.
             *
             * \since 0.40
             */
            const std::string input_fd_env_var() const;

            /**
             * The input stream, or null.
             *
             * \since 0.50
             */
            std::ostream * output_stream() const;

            /**
             * The output FD, if output_stream() not null.
             *
             * \since 0.50
             */
            int output_fd() const;

            /**
             * The output FD env var, if output_stream() not null.
             *
             * \since 0.50
             */
            const std::string output_fd_env_var() const;

            /**
             * Uses ptys instead of pipes?
             *
             * \since 0.38
             */
            bool ptys() const;

            /**
             * Should we clear existing environment variables?
             *
             * \since 0.36
             */
            bool clearenv() const;

            ///\}

            ///\name Iterate over our setenvs.
            ///\{

            struct ConstIteratorTag;
            typedef WrappedForwardIterator<ConstIteratorTag, const std::pair<const std::string, std::string> > ConstIterator;
            ConstIterator begin_setenvs() const;
            ConstIterator end_setenvs() const;

            ///\}
    };

    /**
     * Run a command, wait for it to terminate and return its exit status.
     *
     * \ingroup g_system
     */
    int run_command(const Command & cmd) PALUDIS_VISIBLE
        PALUDIS_ATTRIBUTE((warn_unused_result));

    /**
     * Become another command.
     *
     * Actions that change the initial state (uid / gid, env) work, as do input
     * streams, but output redirection does not. Pipe commands don't work, but
     * could be made to.
     *
     * \ingroup g_system
     * \since 0.40.1
     */
    void become_command(const Command & cmd) PALUDIS_VISIBLE PALUDIS_ATTRIBUTE((noreturn));

    /**
     * Set the stderr and close for stdout fds used by run_command and
     * run_command_in_directory.
     *
     * \ingroup g_system
     */
    void set_run_command_stdout_fds(const int, const int) PALUDIS_VISIBLE;

    /**
     * Set the stderr and close for stderr fds used by run_command and
     * run_command_in_directory.
     *
     * \ingroup g_system
     */
    void set_run_command_stderr_fds(const int, const int) PALUDIS_VISIBLE;

    /**
     * Fetch the username for a uid, or the uid as a string if not available.
     *
     * \ingroup g_system
     */
    std::string get_user_name(const uid_t) PALUDIS_VISIBLE;

    /**
     * Fetch the group name for a gid, or the gid as a string if not available.
     *
     * \ingroup g_system
     */
    std::string get_group_name(const gid_t) PALUDIS_VISIBLE;
}

#endif

