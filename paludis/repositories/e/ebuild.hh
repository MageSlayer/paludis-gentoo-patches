/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011, 2013 Ciaran McCreesh
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

#include <paludis/repositories/e/permitted_directories-fwd.hh>

#include <paludis/util/attributes.hh>
#include <paludis/util/map-fwd.hh>
#include <paludis/util/process-fwd.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/fs_path.hh>

#include <paludis/action-fwd.hh>
#include <paludis/fs_merger-fwd.hh>
#include <paludis/output_manager-fwd.hh>
#include <paludis/repository-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/partitioning-fwd.hh>
#include <paludis/merger.hh>

#include <string>
#include <memory>

/** \file
 * Declarations for the EbuildCommand classes.
 *
 * \ingroup grpebuildinterface
 */

namespace paludis
{
    class Environment;
    class ERepository;

    namespace n
    {
        typedef Name<struct name_a> a;
        typedef Name<struct name_aa> aa;
        typedef Name<struct name_accept_license> accept_license;
        typedef Name<struct name_binary_dist_base> binary_dist_base;
        typedef Name<struct name_binary_distdir> binary_distdir;
        typedef Name<struct name_binary_ebuild_location> binary_ebuild_location;
        typedef Name<struct name_binary_keywords> binary_keywords;
        typedef Name<struct name_binary_uri_extension> binary_uri_extension;
        typedef Name<struct name_builddir> builddir;
        typedef Name<struct name_clearenv> clearenv;
        typedef Name<struct name_commands> commands;
        typedef Name<struct name_config_protect> config_protect;
        typedef Name<struct name_config_protect_mask> config_protect_mask;
        typedef Name<struct name_cross_compile_host> cross_compile_host;
        typedef Name<struct name_destination> destination;
        typedef Name<struct name_destination_repository> destination_repository;
        typedef Name<struct name_distdir> distdir;
        typedef Name<struct name_ebuild_dir> ebuild_dir;
        typedef Name<struct name_ebuild_file> ebuild_file;
        typedef Name<struct name_eclassdirs> eclassdirs;
        typedef Name<struct name_env_unset> env_unset;
        typedef Name<struct name_environment> environment;
        typedef Name<struct name_environment_file> environment_file;
        typedef Name<struct name_exlibsdirs> exlibsdirs;
        typedef Name<struct name_expand_vars> expand_vars;
        typedef Name<struct name_files_dir> files_dir;
        typedef Name<struct name_image> image;
        typedef Name<struct name_info_vars> info_vars;
        typedef Name<struct name_is_from_pbin> is_from_pbin;
        typedef Name<struct name_load_environment> load_environment;
        typedef Name<struct name_loadsaveenv_dir> loadsaveenv_dir;
        typedef Name<struct name_maybe_output_manager> maybe_output_manager;
        typedef Name<struct name_merger_options> merger_options;
        typedef Name<struct name_output_directory> output_directory;
        typedef Name<struct name_package_builddir> package_builddir;
        typedef Name<struct name_package_id> package_id;
        typedef Name<struct name_parts> parts;
        typedef Name<struct name_permitted_directories> permitted_directories;
        typedef Name<struct name_portdir> portdir;
        typedef Name<struct name_profiles> profiles;
        typedef Name<struct name_profiles_with_parents> profiles_with_parents;
        typedef Name<struct name_replaced_by> replaced_by;
        typedef Name<struct name_replacing_ids> replacing_ids;
        typedef Name<struct name_root> root;
        typedef Name<struct name_sandbox> sandbox;
        typedef Name<struct name_sydbox> sydbox;
        typedef Name<struct name_slot> slot;
        typedef Name<struct name_tool_prefix> tool_prefix;
        typedef Name<struct name_unmerge_only> unmerge_only;
        typedef Name<struct name_unmet_requirements> unmet_requirements;
        typedef Name<struct name_use> use;
        typedef Name<struct name_use_ebuild_file> use_ebuild_file;
        typedef Name<struct name_use_expand> use_expand;
        typedef Name<struct name_use_expand_hidden> use_expand_hidden;
        typedef Name<struct name_userpriv> userpriv;
        typedef Name<struct name_volatile_files> volatile_files;
    }

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
        struct EbuildCommandParams
        {
            NamedValue<n::builddir, FSPath> builddir;
            NamedValue<n::clearenv, bool> clearenv;
            NamedValue<n::commands, std::string> commands;
            NamedValue<n::cross_compile_host, std::string> cross_compile_host;
            NamedValue<n::distdir, FSPath> distdir;
            NamedValue<n::ebuild_dir, FSPath> ebuild_dir;
            NamedValue<n::ebuild_file, FSPath> ebuild_file;
            NamedValue<n::eclassdirs, std::shared_ptr<const FSPathSequence> > eclassdirs;
            NamedValue<n::environment, const Environment *> environment;
            NamedValue<n::exlibsdirs, std::shared_ptr<const FSPathSequence> > exlibsdirs;
            NamedValue<n::files_dir, FSPath> files_dir;
            NamedValue<n::maybe_output_manager, std::shared_ptr<OutputManager> > maybe_output_manager;
            NamedValue<n::package_builddir, FSPath> package_builddir;
            NamedValue<n::package_id, std::shared_ptr<const erepository::ERepositoryID> > package_id;
            NamedValue<n::parts, std::shared_ptr<Partitioning> > parts;
            NamedValue<n::permitted_directories, std::shared_ptr<erepository::PermittedDirectories> > permitted_directories;
            NamedValue<n::portdir, FSPath> portdir;
            NamedValue<n::root, std::string> root;
            NamedValue<n::sandbox, bool> sandbox;
            NamedValue<n::sydbox, bool> sydbox;
            NamedValue<n::tool_prefix, std::string> tool_prefix;
            NamedValue<n::userpriv, bool> userpriv;
            NamedValue<n::volatile_files, std::shared_ptr<FSPathSet> > volatile_files;
        };

        /**
         * Parameters for an EbuildNoFetchCommand.
         *
         * \see EbuildNoFetchCommand
         * \ingroup grpebuildinterface
         * \nosubgrouping
         */
        struct EbuildNoFetchCommandParams
        {
            NamedValue<n::a, std::string> a;
            NamedValue<n::aa, std::string> aa;
            NamedValue<n::env_unset, std::string> env_unset;
            NamedValue<n::expand_vars, std::shared_ptr<const Map<std::string, std::string> > > expand_vars;
            NamedValue<n::profiles, std::shared_ptr<const FSPathSequence> > profiles;
            NamedValue<n::profiles_with_parents, std::shared_ptr<const FSPathSequence> > profiles_with_parents;
            NamedValue<n::use, std::string> use;
            NamedValue<n::use_expand, std::string> use_expand;
            NamedValue<n::use_expand_hidden, std::string> use_expand_hidden;
        };

        /**
         * Parameters for an EbuildFetchExtraCommand.
         *
         * \see EbuildFetchExtraCommand
         * \ingroup grpebuildinterface
         * \nosubgrouping
         */
        struct EbuildFetchExtraCommandParams
        {
            NamedValue<n::a, std::string> a;
            NamedValue<n::aa, std::string> aa;
            NamedValue<n::env_unset, std::string> env_unset;
            NamedValue<n::expand_vars, std::shared_ptr<const Map<std::string, std::string> > > expand_vars;
            NamedValue<n::loadsaveenv_dir, FSPath> loadsaveenv_dir;
            NamedValue<n::profiles, std::shared_ptr<const FSPathSequence> > profiles;
            NamedValue<n::profiles_with_parents, std::shared_ptr<const FSPathSequence> > profiles_with_parents;
            NamedValue<n::slot, std::string> slot;
            NamedValue<n::use, std::string> use;
            NamedValue<n::use_expand, std::string> use_expand;
            NamedValue<n::use_expand_hidden, std::string> use_expand_hidden;
        };

        /**
         * Parameters for an EbuildInstallCommand.
         *
         * \see EbuildInstallCommand
         * \ingroup grpebuildinterface
         * \nosubgrouping
         */
        struct EbuildInstallCommandParams
        {
            NamedValue<n::a, std::string> a;
            NamedValue<n::aa, std::string> aa;
            NamedValue<n::accept_license, std::string> accept_license;
            NamedValue<n::config_protect, std::string> config_protect;
            NamedValue<n::config_protect_mask, std::string> config_protect_mask;
            NamedValue<n::cross_compile_host, std::string> cross_compile_host;
            NamedValue<n::destination, std::shared_ptr<Repository> > destination;
            NamedValue<n::env_unset, std::string> env_unset;
            NamedValue<n::expand_vars, std::shared_ptr<const Map<std::string, std::string> > > expand_vars;
            NamedValue<n::is_from_pbin, bool> is_from_pbin;
            NamedValue<n::loadsaveenv_dir, FSPath> loadsaveenv_dir;
            NamedValue<n::profiles, std::shared_ptr<const FSPathSequence> > profiles;
            NamedValue<n::profiles_with_parents, std::shared_ptr<const FSPathSequence> > profiles_with_parents;
            NamedValue<n::replacing_ids, std::shared_ptr<const PackageIDSequence> > replacing_ids;
            NamedValue<n::slot, std::string> slot;
            NamedValue<n::tool_prefix, std::string> tool_prefix;
            NamedValue<n::use, std::string> use;
            NamedValue<n::use_expand, std::string> use_expand;
            NamedValue<n::use_expand_hidden, std::string> use_expand_hidden;
        };

        /**
         * Parameters for an EbuildPretendCommand.
         *
         * \see EbuildPretendCommand
         * \ingroup grpebuildinterface
         * \nosubgrouping
         */
        struct EbuildPretendCommandParams
        {
            NamedValue<n::destination, std::shared_ptr<Repository> > destination;
            NamedValue<n::env_unset, std::string> env_unset;
            NamedValue<n::expand_vars, std::shared_ptr<const Map<std::string, std::string> > > expand_vars;
            NamedValue<n::is_from_pbin, bool> is_from_pbin;
            NamedValue<n::profiles, std::shared_ptr<const FSPathSequence> > profiles;
            NamedValue<n::profiles_with_parents, std::shared_ptr<const FSPathSequence> > profiles_with_parents;
            NamedValue<n::replacing_ids, std::shared_ptr<const PackageIDSequence> > replacing_ids;
            NamedValue<n::use, std::string> use;
            NamedValue<n::use_expand, std::string> use_expand;
            NamedValue<n::use_expand_hidden, std::string> use_expand_hidden;
        };

        /**
         * Parameters for an EbuildBadOptionsCommand.
         *
         * \see EbuildBadOptionsCommand
         * \ingroup grpebuildinterface
         * \nosubgrouping
         */
        struct EbuildBadOptionsCommandParams
        {
            NamedValue<n::env_unset, std::string> env_unset;
            NamedValue<n::expand_vars, std::shared_ptr<const Map<std::string, std::string> > > expand_vars;
            NamedValue<n::profiles, std::shared_ptr<const FSPathSequence> > profiles;
            NamedValue<n::profiles_with_parents, std::shared_ptr<const FSPathSequence> > profiles_with_parents;
            NamedValue<n::unmet_requirements, std::shared_ptr<const Sequence<std::string> > > unmet_requirements;
            NamedValue<n::use, std::string> use;
            NamedValue<n::use_expand, std::string> use_expand;
            NamedValue<n::use_expand_hidden, std::string> use_expand_hidden;
        };

        /**
         * Parameters for an EbuildUninstallCommand.
         *
         * \see EbuildUninstallCommand
         * \ingroup grpebuildinterface
         * \nosubgrouping
         */
        struct EbuildUninstallCommandParams
        {
            NamedValue<n::load_environment, const FSPath *> load_environment;
            NamedValue<n::loadsaveenv_dir, FSPath> loadsaveenv_dir;
            NamedValue<n::replaced_by, std::shared_ptr<const PackageID> > replaced_by;
            NamedValue<n::unmerge_only, bool> unmerge_only;
        };

        /**
         * Parameters for an EbuildConfigCommand.
         *
         * \see EbuildConfigCommand
         * \ingroup grpebuildinterface
         * \nosubgrouping
         */
        struct EbuildConfigCommandParams
        {
            NamedValue<n::load_environment, const FSPath *> load_environment;
        };

        /**
         * Parameters for an EbuildInfoCommand.
         *
         * \see EbuildInfoCommand
         * \ingroup grpebuildinterface
         * \nosubgrouping
         */
        struct EbuildInfoCommandParams
        {
            NamedValue<n::env_unset, std::string> env_unset;
            NamedValue<n::expand_vars, std::shared_ptr<const Map<std::string, std::string> > > expand_vars;
            NamedValue<n::info_vars, std::shared_ptr<const Set<std::string> > > info_vars;
            NamedValue<n::load_environment, const FSPath *> load_environment;
            NamedValue<n::profiles, std::shared_ptr<const FSPathSequence> > profiles;
            NamedValue<n::profiles_with_parents, std::shared_ptr<const FSPathSequence> > profiles_with_parents;
            NamedValue<n::use, std::string> use;
            NamedValue<n::use_ebuild_file, bool> use_ebuild_file;
            NamedValue<n::use_expand, std::string> use_expand;
            NamedValue<n::use_expand_hidden, std::string> use_expand_hidden;
        };

        /**
         * Parameters for writing a VDB entry.
         *
         * \see WriteVDBEntryCommand
         * \ingroup grpebuildinterface
         * \nosubgrouping
         */
        struct WriteVDBEntryParams
        {
            NamedValue<n::environment, const Environment *> environment;
            NamedValue<n::environment_file, FSPath> environment_file;
            NamedValue<n::maybe_output_manager, std::shared_ptr<OutputManager> > maybe_output_manager;
            NamedValue<n::output_directory, FSPath> output_directory;
            NamedValue<n::package_id, std::shared_ptr<const erepository::ERepositoryID> > package_id;
        };

        /**
         * Parameters for writing a binary ebuild.
         *
         * \see WriteBinaryEbuildCommand
         * \ingroup grpebuildinterface
         * \nosubgrouping
         */
        struct WriteBinaryEbuildCommandParams
        {
            NamedValue<n::binary_dist_base, std::string> binary_dist_base;
            NamedValue<n::binary_distdir, FSPath> binary_distdir;
            NamedValue<n::binary_ebuild_location, FSPath> binary_ebuild_location;
            NamedValue<n::binary_keywords, std::string> binary_keywords;
            NamedValue<n::binary_uri_extension, std::string> binary_uri_extension;
            NamedValue<n::builddir, FSPath> builddir;
            NamedValue<n::destination_repository, const ERepository *> destination_repository;
            NamedValue<n::environment, const Environment *> environment;
            NamedValue<n::environment_file, FSPath> environment_file;
            NamedValue<n::image, FSPath> image;
            NamedValue<n::maybe_output_manager, std::shared_ptr<OutputManager> > maybe_output_manager;
            NamedValue<n::merger_options, MergerOptions> merger_options;
            NamedValue<n::package_id, std::shared_ptr<const erepository::ERepositoryID> > package_id;
        };

        /**
         * Parameters for a VDBPostMergeCommand.
         *
         * \see VDBPostMergeCommand
         * \ingroup grpebuildinterface
         * \nosubgrouping
         */
        struct VDBPostMergeUnmergeCommandParams
        {
            NamedValue<n::root, FSPath> root;
        };

        /**
         * An EbuildCommand is the base class from which specific ebuild
         * command interfaces are descended.
         *
         * \ingroup grpebuildinterface
         */
        class EbuildCommand
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
                virtual bool do_run_command(Process &);

                /**
                 * Add Portage emulation vars.
                 */
                virtual void add_portage_vars(Process &) const;

                /**
                 * Extend the command to be run.
                 */
                virtual void extend_command(Process &) = 0;

                /**
                 * Are we generating metadata?
                 */
                virtual bool in_metadata_generation() const;

            public:
                /**
                 * Destructor.
                 */
                virtual ~EbuildCommand();

                EbuildCommand(const EbuildCommand &) = delete;
                EbuildCommand & operator= (const EbuildCommand &) = delete;

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
                std::string commands() const override;

                void extend_command(Process &) override;

                bool do_run_command(Process &) override;

                bool failure() override;

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

                std::string commands() const override;

                bool failure() override PALUDIS_ATTRIBUTE((noreturn));

                void extend_command(Process &) override;

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

                std::string commands() const override;

                bool failure() override PALUDIS_ATTRIBUTE((noreturn));

                void extend_command(Process &) override;

            public:
                /**
                 * Constructor.
                 */
                EbuildInstallCommand(const EbuildCommandParams &, const EbuildInstallCommandParams &);
        };

        /**
         * An EbuildFetchExtraCommand is used to perform extra fetches for an exheres from
         * ERepository. On failure it throws.
         *
         * \ingroup grpebuildinterface
         */
        class EbuildFetchExtraCommand :
            public EbuildCommand
        {
            protected:
                /// Parameters for install.
                const EbuildFetchExtraCommandParams fetch_extra_params;

                std::string commands() const override;

                bool failure() override PALUDIS_ATTRIBUTE((noreturn));

                void extend_command(Process &) override;

            public:
                /**
                 * Constructor.
                 */
                EbuildFetchExtraCommand(const EbuildCommandParams &, const EbuildFetchExtraCommandParams &);
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

                std::string commands() const override;

                bool failure() override PALUDIS_ATTRIBUTE((noreturn));

                void extend_command(Process &) override;

                std::string ebuild_file() const override;

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

                std::string commands() const override;

                bool failure() override PALUDIS_ATTRIBUTE((noreturn));

                void extend_command(Process &) override;

                std::string ebuild_file() const override;

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

                std::string commands() const override;

                bool failure() override;

                void extend_command(Process &) override;

            public:
                /**
                 * Constructor.
                 */
                EbuildPretendCommand(const EbuildCommandParams &, const EbuildPretendCommandParams &);
        };

        /**
         * An EbuildBadOptionsCommand is used to handle unmet MYOPTIONS requirements for
         * a package in ERepository.
         *
         * \ingroup grpebuildinterface
         */
        class EbuildBadOptionsCommand :
            public EbuildCommand
        {
            protected:
                /// Parameters for config.
                const EbuildBadOptionsCommandParams bad_options_params;

                std::string commands() const override;

                bool failure() override;

                void extend_command(Process &) override;

            public:
                /**
                 * Constructor.
                 */
                EbuildBadOptionsCommand(const EbuildCommandParams &, const EbuildBadOptionsCommandParams &);
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

                std::string commands() const override;

                bool failure() override PALUDIS_ATTRIBUTE((noreturn));

                void extend_command(Process &) override;

                std::string ebuild_file() const override;

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
        class WriteVDBEntryCommand
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

                WriteVDBEntryCommand(const WriteVDBEntryCommand &) = delete;
                WriteVDBEntryCommand & operator= (const WriteVDBEntryCommand &) = delete;

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
        class WriteBinaryEbuildCommand
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

                WriteBinaryEbuildCommand(const WriteBinaryEbuildCommand &) = delete;
                WriteBinaryEbuildCommand & operator= (const WriteBinaryEbuildCommand) = delete;

                /**
                 * Run the command.
                 */
                void operator() ();
        };

        /**
         * Command to be run after a VDB merge or unmerge.
         *
         * \ingroup grpebuildinterface
         */
        class VDBPostMergeUnmergeCommand
        {
            private:
                const VDBPostMergeUnmergeCommandParams params;

            public:
                ///\name Basic operations
                ///\{

                VDBPostMergeUnmergeCommand(const VDBPostMergeUnmergeCommandParams &);

                VDBPostMergeUnmergeCommand(const VDBPostMergeUnmergeCommand &) = delete;
                VDBPostMergeUnmergeCommand & operator= (const VDBPostMergeUnmergeCommand &) = delete;

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
                std::shared_ptr<Map<std::string, std::string> > keys;
                std::string captured_stdout;
                std::string captured_stderr;

            public:
                EbuildMetadataCommand(const EbuildCommandParams &);

                ~EbuildMetadataCommand() override;

                std::string commands() const override;

                bool failure() override;

                bool do_run_command(Process &) override;

                bool in_metadata_generation() const override;

                void extend_command(Process &) override;

                void load(const std::shared_ptr<const EbuildID> &);
        };
    }
}

#endif
