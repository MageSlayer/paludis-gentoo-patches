/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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
#include <paludis/util/fs_entry-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/map-fwd.hh>
#include <paludis/util/kc.hh>
#include <paludis/util/keys.hh>
#include <paludis/package_database.hh>
#include <paludis/action-fwd.hh>
#include <paludis/merger-fwd.hh>
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
    class ERepository;

    namespace erepository
    {
        class EbuildID;
        class ERepositoryID;

        /**
         * Parameters for an EbuildCommand.
         *
         * \see EbuildCommand
         * \ingroup grpebuildinterface
         * \nosubgrouping
         */
        typedef kc::KeyedClass<
            kc::Field<k::environment, const Environment *>,
            kc::Field<k::package_id, std::tr1::shared_ptr<const erepository::ERepositoryID> >,
            kc::Field<k::ebuild_dir, FSEntry>,
            kc::Field<k::ebuild_file, FSEntry>,
            kc::Field<k::files_dir, FSEntry>,
            kc::Field<k::eclassdirs, std::tr1::shared_ptr<const FSEntrySequence> >,
            kc::Field<k::exlibsdirs, std::tr1::shared_ptr<const FSEntrySequence> >,
            kc::Field<k::portdir, FSEntry>,
            kc::Field<k::distdir, FSEntry>,
            kc::Field<k::builddir, FSEntry>,
            kc::Field<k::userpriv, bool>,
            kc::Field<k::sandbox, bool>,
            kc::Field<k::commands, std::string>
                > EbuildCommandParams;

        /**
         * Parameters for an EbuildNoFetchCommand.
         *
         * \see EbuildNoFetchCommand
         * \ingroup grpebuildinterface
         * \nosubgrouping
         */
        typedef kc::KeyedClass<
            kc::Field<k::a, std::string>,
            kc::Field<k::aa, std::string>,
            kc::Field<k::use, std::string>,
            kc::Field<k::use_expand, std::string>,
            kc::Field<k::root, std::string>,
            kc::Field<k::profiles, std::tr1::shared_ptr<const FSEntrySequence> >,
            kc::Field<k::expand_vars, std::tr1::shared_ptr<const Map<std::string, std::string> > >
                > EbuildNoFetchCommandParams;

        /**
         * Parameters for an EbuildInstallCommand.
         *
         * \see EbuildInstallCommand
         * \ingroup grpebuildinterface
         * \nosubgrouping
         */
        typedef kc::KeyedClass<
            kc::Field<k::a, std::string>,
            kc::Field<k::aa, std::string>,
            kc::Field<k::use, std::string>,
            kc::Field<k::use_expand, std::string>,
            kc::Field<k::root, std::string>,
            kc::Field<k::profiles, std::tr1::shared_ptr<const FSEntrySequence> >,
            kc::Field<k::expand_vars, std::tr1::shared_ptr<const Map<std::string, std::string> > >,
            kc::Field<k::disable_cfgpro, bool>,
            kc::Field<k::slot, SlotName>,
            kc::Field<k::config_protect, std::string>,
            kc::Field<k::config_protect_mask, std::string>,
            kc::Field<k::loadsaveenv_dir, FSEntry>
                > EbuildInstallCommandParams;

        /**
         * Parameters for an EbuildPretendCommand.
         *
         * \see EbuildPretendCommand
         * \ingroup grpebuildinterface
         * \nosubgrouping
         */
        typedef kc::KeyedClass<
            kc::Field<k::use, std::string>,
            kc::Field<k::use_expand, std::string>,
            kc::Field<k::root, std::string>,
            kc::Field<k::profiles, std::tr1::shared_ptr<const FSEntrySequence> >,
            kc::Field<k::expand_vars, std::tr1::shared_ptr<const Map<std::string, std::string> > >
                > EbuildPretendCommandParams;

        /**
         * Parameters for an EbuildUninstallCommand.
         *
         * \see EbuildUninstallCommand
         * \ingroup grpebuildinterface
         * \nosubgrouping
         */
        typedef kc::KeyedClass<
            kc::Field<k::root, std::string>,
            kc::Field<k::disable_cfgpro, bool>,
            kc::Field<k::unmerge_only, bool>,
            kc::Field<k::load_environment, const FSEntry *>,
            kc::Field<k::loadsaveenv_dir, FSEntry>
                > EbuildUninstallCommandParams;

        /**
         * Parameters for an EbuildConfigCommand.
         *
         * \see EbuildConfigCommand
         * \ingroup grpebuildinterface
         * \nosubgrouping
         */
        typedef kc::KeyedClass<
            kc::Field<k::root, std::string>,
            kc::Field<k::load_environment, const FSEntry *>
                > EbuildConfigCommandParams;

        /**
         * Parameters for an EbuildInfoCommand.
         *
         * \see EbuildInfoCommand
         * \ingroup grpebuildinterface
         * \nosubgrouping
         */
        typedef kc::KeyedClass<
            kc::Field<k::use, std::string>,
            kc::Field<k::use_expand, std::string>,
            kc::Field<k::root, std::string>,
            kc::Field<k::profiles, std::tr1::shared_ptr<const FSEntrySequence> >,
            kc::Field<k::expand_vars, std::tr1::shared_ptr<const Map<std::string, std::string> > >,
            kc::Field<k::load_environment, const FSEntry *>,
            kc::Field<k::info_vars, FSEntry>,
            kc::Field<k::use_ebuild_file, bool>
                > EbuildInfoCommandParams;

        /**
         * Parameters for writing a VDB entry.
         *
         * \see WriteVDBEntryCommand
         * \ingroup grpebuildinterface
         * \nosubgrouping
         */
        typedef kc::KeyedClass<
            kc::Field<k::environment, const Environment *>,
            kc::Field<k::package_id, std::tr1::shared_ptr<const erepository::ERepositoryID> >,
            kc::Field<k::output_directory, FSEntry>,
            kc::Field<k::environment_file, FSEntry>
                > WriteVDBEntryParams;

        /**
         * Parameters for writing a binary ebuild.
         *
         * \see WriteBinaryEbuildCommand
         * \ingroup grpebuildinterface
         * \nosubgrouping
         */
        typedef kc::KeyedClass<
            kc::Field<k::environment, const Environment *>,
            kc::Field<k::destination_repository, const ERepository *>,
            kc::Field<k::package_id, std::tr1::shared_ptr<const erepository::ERepositoryID> >,
            kc::Field<k::binary_ebuild_location, FSEntry>,
            kc::Field<k::binary_distdir, FSEntry>,
            kc::Field<k::environment_file, FSEntry>,
            kc::Field<k::image, FSEntry>,
            kc::Field<k::merger_options, MergerOptions>,
            kc::Field<k::builddir, FSEntry>
                > WriteBinaryEbuildCommandParams;

        /**
         * Parameters for a VDBPostMergeCommand.
         *
         * \see VDBPostMergeCommand
         * \ingroup grpebuildinterface
         * \nosubgrouping
         */
        typedef kc::KeyedClass<
            kc::Field<k::root, FSEntry>
                > VDBPostMergeCommandParams;

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
         * An EbuildVariableCommand is used to fetch the value of an environment
         * variable for a particular ebuild in a ERepository.
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
         * An EbuildNoFetchCommand is used to download and verify the digests for a
         * particular ebuild in a ERepository. On failure it throws.
         *
         * \ingroup grpebuildinterface
         */
        class EbuildNoFetchCommand :
            public EbuildCommand
        {
            protected:
                /// Parameters for fetch.
                const EbuildNoFetchCommandParams fetch_params;

                virtual std::string commands() const;

                virtual bool failure() PALUDIS_ATTRIBUTE((noreturn));

                virtual Command extend_command(const Command &);

            public:
                /**
                 * Constructor.
                 */
                EbuildNoFetchCommand(const EbuildCommandParams &, const EbuildNoFetchCommandParams &);
        };

        /**
         * An EbuildInstallCommand is used to install an ebuild from a
         * ERepository. On failure it throws.
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

                virtual std::string ebuild_file() const;

            public:
                /**
                 * Constructor.
                 */
                EbuildConfigCommand(const EbuildCommandParams &, const EbuildConfigCommandParams &);
        };

        /**
         * An EbuildPretendCommand is used to pretend a package in ERepository.
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
         * An EbuildInfoCommand is used to obtain information from a package.
         *
         * \ingroup grpebuildinterface
         */
        class EbuildInfoCommand :
            public EbuildCommand
        {
            protected:
                /// Parameters for config.
                const EbuildInfoCommandParams info_params;

                virtual std::string commands() const;

                virtual bool failure() PALUDIS_ATTRIBUTE((noreturn));

                virtual Command extend_command(const Command &);

                virtual std::string ebuild_file() const;

            public:
                /**
                 * Constructor.
                 */
                EbuildInfoCommand(const EbuildCommandParams &, const EbuildInfoCommandParams &);
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
         * Command for generating binary ebuild entries (not a regular EbuildCommand).
         *
         * \ingroup grpebuildinterface
         */
        class WriteBinaryEbuildCommand :
            private InstantiationPolicy<WriteVDBEntryCommand, instantiation_method::NonCopyableTag>
        {
            protected:
                /**
                 * Our parameters.
                 */
                const WriteBinaryEbuildCommandParams params;

            public:
                /**
                 * Constructor.
                 */
                WriteBinaryEbuildCommand(const WriteBinaryEbuildCommandParams &);

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

        class EbuildMetadataCommand :
            public EbuildCommand
        {
            private:
                std::tr1::shared_ptr<Map<std::string, std::string> > keys;

            public:
                EbuildMetadataCommand(const EbuildCommandParams &);

                ~EbuildMetadataCommand();

                std::string commands() const;

                bool failure();

                bool do_run_command(const Command &);

                Command extend_command(const Command &);

                void load(const std::tr1::shared_ptr<const EbuildID> &);
        };
    }
}

#endif
