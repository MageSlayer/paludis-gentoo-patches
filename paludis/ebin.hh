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

#ifndef PALUDIS_GUARD_PALUDIS_EBIN_HH
#define PALUDIS_GUARD_PALUDIS_EBIN_HH 1

#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/sr.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/collection.hh>
#include <paludis/package_database.hh>
#include <string>

/** \file
 * Declarations for the EbinCommand classes.
 *
 * \ingroup grpebininterface
 */

namespace paludis
{
    class Environment;
    class MakeEnvCommand;

#include <paludis/ebin-sr.hh>

    /**
     * An EbinCommand is the base class from which specific ebin
     * command interfaces are descended.
     *
     * \ingroup grpebininterface
     */
    class EbinCommand :
        private InstantiationPolicy<EbinCommand, instantiation_method::NonCopyableTag>
    {
        protected:
            /**
             * Our parameters.
             */
            const EbinCommandParams params;

            /**
             * Constructor.
             */
            EbinCommand(const EbinCommandParams &);

            /**
             * Override in descendents: which commands (for example, 'prerm
             * unmerge postrm') do we give to ebin.bash?
             */
            virtual std::string commands() const = 0;

            /**
             * Actions to be taken after a successful command.
             *
             * The return value of this function is used for the return value
             * of operator().
             */
            virtual bool success();

            /**
             * Should the sandbox, if available, be used?
             */
            virtual bool use_sandbox() const;

            /**
             * Actions to be taken after a failed command.
             *
             * The return value of this function is used for the return value
             * of operator(). In some descendents, this function throws and
             * does not return.
             */
            virtual bool failure() = 0;

            /**
             * Run the specified command. Can be overridden if, for example,
             * the command output needs to be captured.
             *
             * \return Whether the command succeeded.
             */
            virtual bool do_run_command(const std::string &);

            /**
             * Add Portage emulation vars.
             */
            virtual MakeEnvCommand add_portage_vars(const MakeEnvCommand &) const;

            /**
             * Extend the command to be run.
             */
            virtual MakeEnvCommand extend_command(const MakeEnvCommand &) = 0;

        public:
            /**
             * Destructor.
             */
            virtual ~EbinCommand();

            /**
             * Run the command.
             */
            virtual bool operator() ();
    };

    /**
     * An EbinFetchCommand is used to download and verify the digests for a
     * particular ebin in a PortageRepository. On failure it throws.
     *
     * \ingroup grpebininterface
     */
    class EbinFetchCommand :
        public EbinCommand
    {
        protected:
            /// Parameters for fetch.
            const EbinFetchCommandParams fetch_params;

            virtual std::string commands() const;

            virtual bool failure() PALUDIS_ATTRIBUTE((noreturn));

            virtual MakeEnvCommand extend_command(const MakeEnvCommand &);

        public:
            /**
             * Constructor.
             */
            EbinFetchCommand(const EbinCommandParams &, const EbinFetchCommandParams &);
    };

    /**
     * An EbinInstallCommand is used to install an ebin from a
     * PortageRepository. On failure it throws.
     *
     * \ingroup grpebininterface
     */
    class EbinInstallCommand :
        public EbinCommand
    {
        protected:
            /// Parameters for install.
            const EbinInstallCommandParams install_params;

            virtual std::string commands() const;

            virtual bool failure() PALUDIS_ATTRIBUTE((noreturn));

            virtual MakeEnvCommand extend_command(const MakeEnvCommand &);

        public:
            /**
             * Constructor.
             */
            EbinInstallCommand(const EbinCommandParams &, const EbinInstallCommandParams &);
    };
}

#endif
