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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_SYSTEM_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_SYSTEM_HH 1

#include <paludis/util/exception.hh>
#include <string>

/** \file
 * Various system utilities.
 *
 * \ingroup grpsystem
 */

namespace paludis
{
    /**
     * Thrown if getenv_or_error fails.
     *
     * \ingroup grpexceptions
     * \ingroup grpsystem
     */
    class GetenvError : public Exception
    {
        public:
            /**
             * Constructor.
             */
            GetenvError(const std::string & key) throw ();
    };

    /**
     * Fetch the value of environment variable key, or def if the variable is
     * not defined.
     *
     * \ingroup grpsystem
     */
    std::string getenv_with_default(const std::string & key, const std::string & def);

    /**
     * Fetch the value of environment variable key, or throw a GetenvError if
     * the variable is not defined.
     *
     * \ingroup grpsystem
     */
    std::string getenv_or_error(const std::string & key);

    /**
     * Fetch the kernel version, for $KV.
     *
     * \ingroup grpsystem
     */
    std::string kernel_version();

    /**
     * Run a command, wait for it to terminate and return its exit status.
     *
     * Use PStream instead if you need to capture stdout.
     *
     * \ingroup grpsystem
     */
    int run_command(const std::string & cmd);

    /**
     * Make a command that's run in a particular environment.
     */
    class MakeEnvCommand
    {
        private:
            std::string cmd;
            std::string args;

        public:
            /**
             * Constructor.
             */
            explicit MakeEnvCommand(const std::string &, const std::string &);

            /**
             * Add some environment.
             */
            MakeEnvCommand operator() (const std::string &, const std::string &) const;

            /**
             * Turn ourself into a command string.
             */
            operator std::string() const;
    };

    /**
     * Make a command, with environment.
     *
     * \ingroup grpsystem
     */
    const MakeEnvCommand make_env_command(const std::string & cmd);

    /**
     * Make a command that is run inside the sandbox, if sandbox is enabled.
     *
     * \ingroup grpsystem
     */
    const std::string make_sandbox_command(const std::string & cmd);
}

#endif

