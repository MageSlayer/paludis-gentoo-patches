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
#include <paludis/util/smart_record.hh>
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
    /**
     * Keys for EbinCommandParams.
     *
     * \see EbinCommandParams
     *
     * \ingroup grpebininterface
     */
    enum EbinCommandParamsKeys
    {
        ebcpk_ebin_dir,
        ebcpk_environment,
        ebcpk_db_entry,
        ebcpk_src_repository,
        ebcpk_pkgdir,
        ebcpk_buildroot,
        last_ebcpk
    };

    class Environment;
    class MakeEnvCommand;

    /**
     * Tag for EbinCommandParams.
     *
     * \see EbinCommandParams.
     *
     * \ingroup grpebininterface
     */
    struct EbinCommandParamsTag :
        SmartRecordTag<comparison_mode::NoComparisonTag, void>,
        SmartRecordKeys<EbinCommandParamsKeys, last_ebcpk>,
        SmartRecordKey<ebcpk_ebin_dir, const FSEntry>,
        SmartRecordKey<ebcpk_environment, const Environment *>,
        SmartRecordKey<ebcpk_db_entry, const PackageDatabaseEntry *>,
        SmartRecordKey<ebcpk_src_repository, const RepositoryName>,
        SmartRecordKey<ebcpk_pkgdir, const FSEntry>,
        SmartRecordKey<ebcpk_buildroot, const FSEntry>
    {
    };

    /**
     * Parameters for EbinCommand's constructor.
     *
     * \ingroup grpebininterface
     */
    typedef MakeSmartRecord<EbinCommandParamsTag>::Type EbinCommandParams;

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
     * Keys for EbinFetchCommandParams.
     *
     * \see EbinFetchCommandParams
     *
     * \ingroup grpebininterface
     */
    enum EbinFetchCommandParamsKeys
    {
        ebcfpk_b,
        ebcfpk_flat_bin_uri,
        ebcfpk_root,
        ebcfpk_profiles,
        last_ebcfpk
    };

    /**
     * Tag for EbinFetchCommandParams.
     *
     * \see EbinFetchCommandParams
     *
     * \ingroup grpebininterface
     */
    struct EbinFetchCommandParamsTag :
        SmartRecordTag<comparison_mode::NoComparisonTag, void>,
        SmartRecordKeys<EbinFetchCommandParamsKeys, last_ebcfpk>,
        SmartRecordKey<ebcfpk_b, std::string>,
        SmartRecordKey<ebcfpk_flat_bin_uri, std::string>,
        SmartRecordKey<ebcfpk_root, std::string>,
        SmartRecordKey<ebcfpk_profiles, FSEntryCollection::ConstPointer>
    {
    };

    /**
     * Parameters for EbinFetchCommand's constructor.
     *
     * \ingroup grpebininterface
     */
    typedef MakeSmartRecord<EbinFetchCommandParamsTag>::Type EbinFetchCommandParams;

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
     * Keys for EbinInstallCommandParams.
     *
     * \see EbinInstallCommandParams
     *
     * \ingroup grpebininterface
     */
    enum EbinInstallCommandParamsKeys
    {
        ebcipk_b,
        ebcipk_use,
        ebcipk_use_expand,
        ebcipk_root,
        ebcipk_profiles,
        ebcipk_expand_vars,
        ebcipk_disable_cfgpro,
        ebcipk_merge_only,
        ebcipk_slot,
        last_ebcipk
    };

    /**
     * Tag for EbinInstallCommandParams.
     *
     * \see EbinInstallCommandParams
     *
     * \ingroup grpebininterface
     */
    struct EbinInstallCommandParamsTag :
        SmartRecordTag<comparison_mode::NoComparisonTag, void>,
        SmartRecordKeys<EbinInstallCommandParamsKeys, last_ebcipk>,
        SmartRecordKey<ebcipk_b, std::string>,
        SmartRecordKey<ebcipk_use, std::string>,
        SmartRecordKey<ebcipk_use_expand, std::string>,
        SmartRecordKey<ebcipk_root, std::string>,
        SmartRecordKey<ebcipk_profiles, FSEntryCollection::ConstPointer>,
        SmartRecordKey<ebcipk_expand_vars, AssociativeCollection<std::string, std::string>::ConstPointer>,
        SmartRecordKey<ebcipk_disable_cfgpro, bool>,
        SmartRecordKey<ebcipk_merge_only, bool>,
        SmartRecordKey<ebcipk_slot, SlotName>
    {
    };

    /**
     * Parameters for EbinInstallCommand's constructor.
     *
     * \ingroup grpebininterface
     */
    typedef MakeSmartRecord<EbinInstallCommandParamsTag>::Type EbinInstallCommandParams;

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
