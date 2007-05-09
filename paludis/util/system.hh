/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <string>
#include <sys/types.h>

/** \file
 * Various system utilities.
 *
 * \ingroup grpsystem
 */

namespace paludis
{
    class FSEntry;

    /**
     * Thrown if getenv_or_error fails.
     *
     * \ingroup grpexceptions
     * \ingroup grpsystem
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
     * \ingroup grpexceptions
     * \ingroup grpsystem
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
     * \ingroup grpsystem
     */
    std::string getenv_with_default(const std::string & key, const std::string & def) PALUDIS_VISIBLE;

    /**
     * Fetch the value of environment variable key, or throw a GetenvError if
     * the variable is not defined.
     *
     * \ingroup grpsystem
     */
    std::string getenv_or_error(const std::string & key) PALUDIS_VISIBLE;

    /**
     * Fetch the kernel version, for $KV.
     *
     * \ingroup grpsystem
     */
    std::string kernel_version() PALUDIS_VISIBLE;

    /**
     * A command to be run.
     *
     * \see PStream
     * \see run_command
     * \ingroup grpsystem
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
             * Run our command sandboxed.
             */
            Command & with_sandbox();

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

            ///\}

            ///\name Iterate over our setenvs.
            ///\{

            typedef libwrapiter::ForwardIterator<Command, const std::pair<const std::string, std::string> > Iterator;
            Iterator begin_setenvs() const;
            Iterator end_setenvs() const;

            ///\}
    };


    /**
     * Run a command, wait for it to terminate and return its exit status.
     *
     * Use PStream instead if you need to capture stdout.
     *
     * \ingroup grpsystem
     */
    int run_command(const Command & cmd) PALUDIS_VISIBLE
        PALUDIS_ATTRIBUTE((warn_unused_result));

    /**
     * Set the stderr and close for stdout fds used by run_command and
     * run_command_in_directory.
     *
     * \ingroup grpsystem
     */
    void set_run_command_stdout_fds(const int, const int) PALUDIS_VISIBLE;

    /**
     * Set the stderr and close for stderr fds used by run_command and
     * run_command_in_directory.
     *
     * \ingroup grpsystem
     */
    void set_run_command_stderr_fds(const int, const int) PALUDIS_VISIBLE;

    /**
     * Fetch the username for a uid, or the uid as a string if not available.
     *
     * \ingroup grpsystem
     */
    std::string get_user_name(const uid_t) PALUDIS_VISIBLE;

    /**
     * Fetch the group name for a gid, or the gid as a string if not available.
     *
     * \ingroup grpsystem
     */
    std::string get_group_name(const gid_t) PALUDIS_VISIBLE;
}

#endif

