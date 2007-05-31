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

#ifndef PALUDIS_GUARD_PALUDIS_EBUILD_HH
#define PALUDIS_GUARD_PALUDIS_EBUILD_HH 1

#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/collection.hh>
#include <paludis/package_database.hh>
#include <string>

/** \file
 * Declarations for the EbuildCommand classes.
 *
 * \ingroup grpebuildinterface
 */

namespace paludis
{
    class Environment;
    class Command;

#include <paludis/repositories/gentoo/ebuild-sr.hh>

    /**
     * VersionMetadata for an ebuild.
     *
     * \ingroup grpebuildinterface
     * \nosubgrouping
     */
    class EbuildVersionMetadata :
        public VersionMetadata,
        public VersionMetadataEbuildInterface,
        public VersionMetadataDepsInterface,
        public VersionMetadataLicenseInterface,
        public virtual VersionMetadataHasInterfaces
    {
        public:
            ///\name Basic operations
            ///\{

            EbuildVersionMetadata();
            virtual ~EbuildVersionMetadata();

            ///\}

            virtual const VersionMetadata * version_metadata() const
            {
                return this;
            }
    };

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
             * Return our ebuild file.
             */
            virtual std::string ebuild_file() const;

            /**
             * Actions to be taken after a successful command.
             *
             * The return value of this function is used for the return value
             * of operator().
             */
            virtual bool success();

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
            virtual bool do_run_command(const Command &);

            /**
             * Add Portage emulation vars.
             */
            virtual Command add_portage_vars(const Command &) const;

            /**
             * Extend the command to be run.
             */
            virtual Command extend_command(const Command &) = 0;

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
            tr1::shared_ptr<EbuildVersionMetadata> _metadata;

        protected:
            virtual std::string commands() const;

            virtual bool failure();

            virtual Command extend_command(const Command &);

            virtual bool do_run_command(const Command &);

        public:
            /**
             * Constructor.
             */
            EbuildMetadataCommand(const EbuildCommandParams &);

            /**
             * Return a pointer to our generated metadata. If operator() has not
             * yet been called, will be a zero pointer.
             */
            tr1::shared_ptr<EbuildVersionMetadata> metadata() const
            {
                return _metadata;
            }
    };

    /**
     * An EbuildVariableCommand is used to fetch the value of an environment
     * variable for a particular ebuild in a PortageRepository.
     *
     * \ingroup grpebuildinterface
     */
    class EbuildVariableCommand :
        public EbuildCommand
    {
        private:
            std::string _result;
            const std::string _var;

        protected:
            virtual std::string commands() const;

            virtual Command extend_command(const Command &);

            virtual bool do_run_command(const Command &);

            virtual bool failure();

        public:
            /**
             * Constructor.
             */
            EbuildVariableCommand(const EbuildCommandParams &, const std::string &);

            /**
             * Fetch our result.
             */
            std::string result() const
            {
                return _result;
            }
    };

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

            virtual Command extend_command(const Command &);

        public:
            /**
             * Constructor.
             */
            EbuildFetchCommand(const EbuildCommandParams &, const EbuildFetchCommandParams &);
    };

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

            virtual Command extend_command(const Command &);

        public:
            /**
             * Constructor.
             */
            EbuildInstallCommand(const EbuildCommandParams &, const EbuildInstallCommandParams &);
    };

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

            virtual Command extend_command(const Command &);

            virtual std::string ebuild_file() const;

        public:
            /**
             * Constructor.
             */
            EbuildUninstallCommand(const EbuildCommandParams &, const EbuildUninstallCommandParams &);
    };

    /**
     * An EbuildConfigCommand is used to configure a package in a VDBRepository.
     *
     * \ingroup grpebuildinterface
     */
    class EbuildConfigCommand :
        public EbuildCommand
    {
        protected:
            /// Parameters for config.
            const EbuildConfigCommandParams config_params;

            virtual std::string commands() const;

            virtual bool failure() PALUDIS_ATTRIBUTE((noreturn));

            virtual Command extend_command(const Command &);

        public:
            /**
             * Constructor.
             */
            EbuildConfigCommand(const EbuildCommandParams &, const EbuildConfigCommandParams &);
    };

    /**
     * An EbuildPretendCommand is used to configure a package in a VDBRepository.
     *
     * \ingroup grpebuildinterface
     */
    class EbuildPretendCommand :
        public EbuildCommand
    {
        protected:
            /// Parameters for config.
            const EbuildPretendCommandParams pretend_params;

            virtual std::string commands() const;

            virtual bool failure();

            virtual Command extend_command(const Command &);

        public:
            /**
             * Constructor.
             */
            EbuildPretendCommand(const EbuildCommandParams &, const EbuildPretendCommandParams &);
    };

    /**
     * Command for generating VDB entries (not a regular EbuildCommand).
     *
     * \ingroup grpebuildinterface
     */
    class WriteVDBEntryCommand :
        private InstantiationPolicy<WriteVDBEntryCommand, instantiation_method::NonCopyableTag>
    {
        protected:
            /**
             * Our parameters.
             */
            const WriteVDBEntryParams params;

        public:
            /**
             * Constructor.
             */
            WriteVDBEntryCommand(const WriteVDBEntryParams &);

            /**
             * Run the command.
             */
            void operator() ();
    };

    /**
     * Command to be run after a VDB merge.
     *
     * \ingroup grpebuildinterface
     */
    class VDBPostMergeCommand :
        private InstantiationPolicy<VDBPostMergeCommand, instantiation_method::NonCopyableTag>
    {
        private:
            const VDBPostMergeCommandParams params;

        public:
            ///\name Basic operations
            ///\{

            VDBPostMergeCommand(const VDBPostMergeCommandParams &);

            ///\}

            /**
             * Run the command.
             */
            void operator() ();
    };
}

#endif
