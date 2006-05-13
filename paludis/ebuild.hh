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

#ifndef PALUDIS_GUARD_PALUDIS_EBUILD_HH
#define PALUDIS_GUARD_PALUDIS_EBUILD_HH 1

#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/smart_record.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/attributes.hh>
#include <paludis/package_database.hh>
#include <string>
#include <map>

/** \file
 * Declarations for the EbuildCommand classes.
 *
 * \ingroup grpebuildinterface
 */

namespace paludis
{
    /**
     * Keys for EbuildCommandParams.
     *
     * \see EbuildCommandParams
     *
     * \ingroup grpebuildinterface
     */
    enum EbuildCommandParamsKeys
    {
        ecpk_environment,
        ecpk_db_entry,
        ecpk_ebuild_dir,
        ecpk_files_dir,
        ecpk_eclass_dir,
        ecpk_portdir,
        ecpk_distdir,
        last_ecpk
    };

    class Environment;
    class MakeEnvCommand;

    /**
     * Tag for EbuildCommandParams.
     *
     * \see EbuildCommandParams.
     *
     * \ingroup grpebuildinterface
     */
    struct EbuildCommandParamsTag :
        SmartRecordTag<comparison_mode::NoComparisonTag, void>,
        SmartRecordKeys<EbuildCommandParamsKeys, last_ecpk>,
        SmartRecordKey<ecpk_environment, const Environment *>,
        SmartRecordKey<ecpk_db_entry, const PackageDatabaseEntry *>,
        SmartRecordKey<ecpk_ebuild_dir, const FSEntry>,
        SmartRecordKey<ecpk_files_dir, const FSEntry>,
        SmartRecordKey<ecpk_eclass_dir, const FSEntry>,
        SmartRecordKey<ecpk_portdir, const FSEntry>,
        SmartRecordKey<ecpk_distdir, const FSEntry>
    {
    };

    /**
     * Parameters for EbuildCommand's constructor.
     *
     * \ingroup grpebuildinterface
     */
    typedef MakeSmartRecord<EbuildCommandParamsTag>::Type EbuildCommandParams;

    /**
     * An EbuildCommand is the base class from which specific ebuild
     * command interfaces are descended.
     *
     * \ingroup grpebuildinterface
     */
    class EbuildCommand :
        private InstantiationPolicy<EbuildCommand, instantiation_method::NonCopyableTag>
    {
        protected:
            /**
             * Our parameters.
             */
            const EbuildCommandParams params;

            /**
             * Constructor.
             */
            EbuildCommand(const EbuildCommandParams &);

            /**
             * Override in descendents: which commands (for example, 'prerm
             * unmerge postrm') do we give to ebuild.bash?
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
             * Extend the command to be run.
             */
            virtual MakeEnvCommand extend_command(const MakeEnvCommand &) = 0;

        public:
            /**
             * Destructor.
             */
            virtual ~EbuildCommand();

            /**
             * Run the command.
             */
            virtual bool operator() ();
    };

    /**
     * An EbuildMetadataCommand is used to generate metadata for a particular
     * ebuild in a PortageRepository.
     *
     * \ingroup grpebuildinterface
     */
    class EbuildMetadataCommand :
        public EbuildCommand
    {
        private:
            VersionMetadata::Ebuild::Pointer _metadata;

        protected:
            virtual std::string commands() const;

            virtual bool failure();

            virtual MakeEnvCommand extend_command(const MakeEnvCommand &);

            virtual bool do_run_command(const std::string &);

        public:
            /**
             * Constructor.
             */
            EbuildMetadataCommand(const EbuildCommandParams &);

            /**
             * Return a pointer to our generated metadata. If operator() has not
             * yet been called, will be a zero pointer.
             */
            VersionMetadata::Pointer metadata() const
            {
                return _metadata;
            }
    };

    /**
     * Keys for EbuildFetchCommandParams.
     *
     * \see EbuildFetchCommandParams
     *
     * \ingroup grpebuildinterface
     */
    enum EbuildFetchCommandParamsKeys
    {
        ecfpk_a,
        ecfpk_use,
        ecfpk_use_expand,
        ecfpk_flat_src_uri,
        ecfpk_root,
        ecfpk_profile,
        ecfpk_expand_vars,
        ecfpk_no_fetch,
        last_ecfpk
    };

    /**
     * Tag for EbuildFetchCommandParams.
     *
     * \see EbuildFetchCommandParams
     *
     * \ingroup grpebuildinterface
     */
    struct EbuildFetchCommandParamsTag :
        SmartRecordTag<comparison_mode::NoComparisonTag, void>,
        SmartRecordKeys<EbuildFetchCommandParamsKeys, last_ecfpk>,
        SmartRecordKey<ecfpk_a, std::string>,
        SmartRecordKey<ecfpk_use, std::string>,
        SmartRecordKey<ecfpk_use_expand, std::string>,
        SmartRecordKey<ecfpk_flat_src_uri, std::string>,
        SmartRecordKey<ecfpk_root, std::string>,
        SmartRecordKey<ecfpk_profile, std::string>,
        SmartRecordKey<ecfpk_expand_vars, std::map<std::string, std::string> >,
        SmartRecordKey<ecfpk_no_fetch, bool>
    {
    };

    /**
     * Parameters for EbuildFetchCommand's constructor.
     *
     * \ingroup grpebuildinterface
     */
    typedef MakeSmartRecord<EbuildFetchCommandParamsTag>::Type EbuildFetchCommandParams;

    /**
     * An EbuildFetchCommand is used to download and verify the digests for a
     * particular ebuild in a PortageRepository. On failure it throws.
     *
     * \ingroup grpebuildinterface
     */
    class EbuildFetchCommand :
        public EbuildCommand
    {
        protected:
            /// Parameters for fetch.
            const EbuildFetchCommandParams fetch_params;

            virtual std::string commands() const;

            virtual bool failure() PALUDIS_ATTRIBUTE((noreturn));

            virtual MakeEnvCommand extend_command(const MakeEnvCommand &);

        public:
            /**
             * Constructor.
             */
            EbuildFetchCommand(const EbuildCommandParams &, const EbuildFetchCommandParams &);
    };

    /**
     * Keys for EbuildInstallCommandParams.
     *
     * \see EbuildInstallCommandParams
     *
     * \ingroup grpebuildinterface
     */
    enum EbuildInstallCommandParamsKeys
    {
        ecipk_a,
        ecipk_use,
        ecipk_use_expand,
        ecipk_root,
        ecipk_profile,
        ecipk_expand_vars,
        ecipk_disable_cfgpro,
        ecipk_merge_only,
        ecipk_slot,
        last_ecipk
    };

    /**
     * Tag for EbuildInstallCommandParams.
     *
     * \see EbuildInstallCommandParams
     *
     * \ingroup grpebuildinterface
     */
    struct EbuildInstallCommandParamsTag :
        SmartRecordTag<comparison_mode::NoComparisonTag, void>,
        SmartRecordKeys<EbuildInstallCommandParamsKeys, last_ecipk>,
        SmartRecordKey<ecipk_a, std::string>,
        SmartRecordKey<ecipk_use, std::string>,
        SmartRecordKey<ecipk_use_expand, std::string>,
        SmartRecordKey<ecipk_root, std::string>,
        SmartRecordKey<ecipk_profile, std::string>,
        SmartRecordKey<ecipk_expand_vars, std::map<std::string, std::string> >,
        SmartRecordKey<ecipk_disable_cfgpro, bool>,
        SmartRecordKey<ecipk_merge_only, bool>,
        SmartRecordKey<ecipk_slot, SlotName>
    {
    };

    /**
     * Parameters for EbuildInstallCommand's constructor.
     *
     * \ingroup grpebuildinterface
     */
    typedef MakeSmartRecord<EbuildInstallCommandParamsTag>::Type EbuildInstallCommandParams;

    /**
     * An EbuildInstallCommand is used to install an ebuild from a
     * PortageRepository. On failure it throws.
     *
     * \ingroup grpebuildinterface
     */
    class EbuildInstallCommand :
        public EbuildCommand
    {
        protected:
            /// Parameters for install.
            const EbuildInstallCommandParams install_params;

            virtual std::string commands() const;

            virtual bool failure() PALUDIS_ATTRIBUTE((noreturn));

            virtual MakeEnvCommand extend_command(const MakeEnvCommand &);

        public:
            /**
             * Constructor.
             */
            EbuildInstallCommand(const EbuildCommandParams &, const EbuildInstallCommandParams &);
    };

    /**
     * Keys for EbuildUninstallCommandParams.
     *
     * \see EbuildUninstallCommandParams
     *
     * \ingroup grpebuildinterface
     */
    enum EbuildUninstallCommandParamsKeys
    {
        ecupk_root,
        ecupk_disable_cfgpro,
        ecupk_unmerge_only,
        last_ecupk
    };

    /**
     * Tags for EbuildUninstallCommandParams.
     *
     * \see EbuildUninstallCommandParams
     *
     * \ingroup grpebuildinterface
     */
    struct EbuildUninstallCommandParamsTag :
        SmartRecordTag<comparison_mode::NoComparisonTag, void>,
        SmartRecordKeys<EbuildUninstallCommandParamsKeys, last_ecupk>,
        SmartRecordKey<ecupk_root, std::string>,
        SmartRecordKey<ecupk_disable_cfgpro, bool>,
        SmartRecordKey<ecupk_unmerge_only, bool>
    {
    };

    /**
     * Parameters for EbuildUninstallCommand's constructor.
     *
     * \ingroup grpebuildinterface
     */
    typedef MakeSmartRecord<EbuildUninstallCommandParamsTag>::Type EbuildUninstallCommandParams;

    /**
     * An EbuildUninstallCommand is used to uninstall a package in a VDBRepository.
     *
     * \ingroup grpebuildinterface
     */
    class EbuildUninstallCommand :
        public EbuildCommand
    {
        protected:
            /// Parameters for uninstall.
            const EbuildUninstallCommandParams uninstall_params;

            virtual std::string commands() const;

            virtual bool failure() PALUDIS_ATTRIBUTE((noreturn));

            virtual MakeEnvCommand extend_command(const MakeEnvCommand &);

        public:
            /**
             * Constructor.
             */
            EbuildUninstallCommand(const EbuildCommandParams &, const EbuildUninstallCommandParams &);
    };
}

#endif
