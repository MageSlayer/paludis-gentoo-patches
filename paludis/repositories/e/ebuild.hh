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

#ifndef PALUDIS_GUARD_PALUDIS_EBUILD_HH
#define PALUDIS_GUARD_PALUDIS_EBUILD_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/util/map-fwd.hh>
#include <paludis/util/process-fwd.hh>
#include <paludis/package_database.hh>
#include <paludis/action-fwd.hh>
#include <paludis/fs_merger-fwd.hh>
#include <string>

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
        typedef Name<struct a_name> a;
        typedef Name<struct aa_name> aa;
        typedef Name<struct accept_license_name> accept_license;
        typedef Name<struct binary_dist_base_name> binary_dist_base;
        typedef Name<struct binary_distdir_name> binary_distdir;
        typedef Name<struct binary_ebuild_location_name> binary_ebuild_location;
        typedef Name<struct binary_keywords_name> binary_keywords;
        typedef Name<struct builddir_name> builddir;
        typedef Name<struct clearenv_name> clearenv;
        typedef Name<struct commands_name> commands;
        typedef Name<struct config_protect_name> config_protect;
        typedef Name<struct config_protect_mask_name> config_protect_mask;
        typedef Name<struct destination_repository_name> destination_repository;
        typedef Name<struct distdir_name> distdir;
        typedef Name<struct ebuild_dir_name> ebuild_dir;
        typedef Name<struct ebuild_file_name> ebuild_file;
        typedef Name<struct eclassdirs_name> eclassdirs;
        typedef Name<struct environment_name> environment;
        typedef Name<struct environment_file_name> environment_file;
        typedef Name<struct exlibsdirs_name> exlibsdirs;
        typedef Name<struct expand_vars_name> expand_vars;
        typedef Name<struct files_dir_name> files_dir;
        typedef Name<struct image_name> image;
        typedef Name<struct info_vars_name> info_vars;
        typedef Name<struct load_environment_name> load_environment;
        typedef Name<struct loadsaveenv_dir_name> loadsaveenv_dir;
        typedef Name<struct maybe_output_manager_name> maybe_output_manager;
        typedef Name<struct merger_options_name> merger_options;
        typedef Name<struct output_directory_name> output_directory;
        typedef Name<struct package_builddir_name> package_builddir;
        typedef Name<struct package_id_name> package_id;
        typedef Name<struct portdir_name> portdir;
        typedef Name<struct profiles_name> profiles;
        typedef Name<struct profiles_with_parents_name> profiles_with_parents;
        typedef Name<struct replaced_by_name> replaced_by;
        typedef Name<struct replacing_ids_name> replacing_ids;
        typedef Name<struct root_name> root;
        typedef Name<struct sandbox_name> sandbox;
        typedef Name<struct sydbox_name> sydbox;
        typedef Name<struct slot_name> slot;
        typedef Name<struct unmerge_only_name> unmerge_only;
        typedef Name<struct unmet_requirements_name> unmet_requirements;
        typedef Name<struct use_name> use;
        typedef Name<struct use_ebuild_file_name> use_ebuild_file;
        typedef Name<struct use_expand_name> use_expand;
        typedef Name<struct use_expand_hidden_name> use_expand_hidden;
        typedef Name<struct userpriv_name> userpriv;
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
            NamedValue<n::portdir, FSPath> portdir;
            NamedValue<n::root, std::string> root;
            NamedValue<n::sandbox, bool> sandbox;
            NamedValue<n::sydbox, bool> sydbox;
            NamedValue<n::userpriv, bool> userpriv;
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
            NamedValue<n::expand_vars, std::shared_ptr<const Map<std::string, std::string> > > expand_vars;
            NamedValue<n::loadsaveenv_dir, FSPath> loadsaveenv_dir;
            NamedValue<n::profiles, std::shared_ptr<const FSPathSequence> > profiles;
            NamedValue<n::profiles_with_parents, std::shared_ptr<const FSPathSequence> > profiles_with_parents;
            NamedValue<n::replacing_ids, std::shared_ptr<const PackageIDSequence> > replacing_ids;
            NamedValue<n::slot, std::string> slot;
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
            NamedValue<n::expand_vars, std::shared_ptr<const Map<std::string, std::string> > > expand_vars;
            NamedValue<n::profiles, std::shared_ptr<const FSPathSequence> > profiles;
            NamedValue<n::profiles_with_parents, std::shared_ptr<const FSPathSequence> > profiles_with_parents;
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
                virtual std::string commands() const;

                virtual void extend_command(Process &);

                virtual bool do_run_command(Process &);

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

                virtual void extend_command(Process &);

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

                virtual void extend_command(Process &);

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

                virtual std::string commands() const;

                virtual bool failure() PALUDIS_ATTRIBUTE((noreturn));

                virtual void extend_command(Process &);

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

                virtual std::string commands() const;

                virtual bool failure() PALUDIS_ATTRIBUTE((noreturn));

                virtual void extend_command(Process &);

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

                virtual void extend_command(Process &);

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

                virtual void extend_command(Process &);

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

                virtual std::string commands() const;

                virtual bool failure();

                virtual void extend_command(Process &);

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

                virtual std::string commands() const;

                virtual bool failure() PALUDIS_ATTRIBUTE((noreturn));

                virtual void extend_command(Process &);

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

                ~EbuildMetadataCommand();

                std::string commands() const;

                bool failure();

                bool do_run_command(Process &);

                virtual void extend_command(Process &);

                void load(const std::shared_ptr<const EbuildID> &);
        };
    }
}

#endif
