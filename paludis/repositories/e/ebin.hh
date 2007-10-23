/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_PALUDIS_REPOSITORIES_GENTOO_EBIN_HH
#define PALUDIS_GUARD_PALUDIS_PALUDIS_REPOSITORIES_GENTOO_EBIN_HH 1

#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/attributes.hh>
#include <paludis/package_database.hh>
#include <paludis/action-fwd.hh>
#include <string>

namespace paludis
{
    class Command;

#include <paludis/repositories/e/ebin-se.hh>
#include <paludis/repositories/e/ebin-sr.hh>

    /**
     * A command related to a .ebin file.
     *
     * \ingroup grpebininterface
     * \nosubgrouping
     */
    class EbinCommand :
        private InstantiationPolicy<EbinCommand, instantiation_method::NonCopyableTag>
    {
        protected:
            /**
             * Our parameters.
             */
            EbinCommandParams params;

            ///\name Basic operations
            ///\{

            EbinCommand(const EbinCommandParams &);

            ///\}

            ///\name Command options
            ///\{

            /**
             * What commands should be run?
             */
            virtual std::string commands() const = 0;

            /**
             * Was our execution a success?
             */
            virtual bool success();

            /**
             * Should we use sandbox?
             */
            virtual bool use_sandbox() const;

            /**
             * Handle a failure.
             */
            virtual bool failure() = 0;

            /**
             * Run the command specified.
             */
            virtual bool do_run_command(const Command &);

            /**
             * Extend the basic command.
             */
            virtual Command extend_command(const Command &) = 0;

            /**
             * Add Portage emulation variables.
             */
            virtual Command add_portage_vars(const Command &) const;

            ///\}

        public:
            ///\name Basic operations
            ///\{

            virtual ~EbinCommand();

            ///\}

            /**
             * Run the command.
             */
            virtual bool operator() ();
    };

    /**
     * Fetch a .ebin's bin_uri contents.
     *
     * \ingroup grpebininterface
     * \nosubgrouping
     */
    class EbinFetchCommand :
        public EbinCommand
    {
        protected:
            /// Our parameters.
            const EbinFetchCommandParams fetch_params;

            virtual std::string commands() const;

            virtual bool failure() PALUDIS_ATTRIBUTE((noreturn));

            virtual Command extend_command(const Command &);

        public:
            ///\name Basic operations
            ///\{

            EbinFetchCommand(const EbinCommandParams &, const EbinFetchCommandParams &);

            ///\}
    };

    /**
     * Perform a part of a .ebin install sequence.
     *
     * \ingroup grpebininterface
     * \nosubgrouping
     */
    class EbinInstallCommand :
        public EbinCommand
    {
        protected:
            /// Our parameters.
            const EbinInstallCommandParams install_params;

            virtual std::string commands() const;

            virtual bool failure() PALUDIS_ATTRIBUTE((noreturn));

            virtual Command extend_command(const Command &);

        public:
            ///\name Basic operations
            ///\{

            EbinInstallCommand(const EbinCommandParams &, const EbinInstallCommandParams &);

            ///\}
    };

    /**
     * Merge to a .ebin repository.
     *
     * \ingroup grpebininterface
     * \nosubgrouping
     */
    class EbinMergeCommand
    {
        protected:
            ///\name Parameters
            ///\{

            const EbinCommandParams params;
            const EbinMergeCommandParams merge_params;

            ///\}

        public:
            ///\name Basic operations
            ///\{

            EbinMergeCommand(const EbinCommandParams &, const EbinMergeCommandParams &);

            ///\}

            /**
             * Perform the merge.
             */
            void operator() ();
    };
}

#endif

