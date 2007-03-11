/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_ENVIRONMENT_HH
#define PALUDIS_GUARD_PALUDIS_ENVIRONMENT_HH 1

#include <paludis/mask_reasons.hh>
#include <paludis/name.hh>
#include <paludis/package_database.hh>
#include <paludis/util/instantiation_policy.hh>

/** \file
 * Declarations for the Environment class.
 *
 * \ingroup grpenvironment
 */

namespace paludis
{
    struct EnvironmentMirrorIteratorTag;

    /**
     * Iterate over environment mirrors.
     *
     * \see Environment
     * \ingroup grpenvironment
     */
    typedef libwrapiter::ForwardIterator<EnvironmentMirrorIteratorTag,
            const std::pair<const std::string, std::string> > EnvironmentMirrorIterator;

    /**
     * Represents the data for an Environment hook call.
     *
     * \ingroup grpenvironment
     */
    class Hook :
        private PrivateImplementationPattern<Hook>
    {
        public:
            /// Constructor.
            Hook(const std::string & name);

            /// Copy constructor.
            Hook(const Hook &);

            /// Destructor.
            ~Hook();

            /// Add data to the hook.
            Hook operator() (const std::string & key, const std::string & value) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /// Iterate over extra environment data.
            typedef libwrapiter::ForwardIterator<Hook, const std::pair<const std::string, std::string> > Iterator;

            /// Start of extra environment data.
            Iterator begin() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /// End of extra environment data.
            Iterator end() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /// Our name.
            std::string name() const
                PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * Represents a working environment, which contains an available packages
     * database and provides various methods for querying package visibility
     * and options.
     *
     * \ingroup grpenvironment
     */
    class Environment :
        private InstantiationPolicy<Environment, instantiation_method::NonCopyableTag>
    {
        private:
            std::tr1::shared_ptr<PackageDatabase> _package_database;

        protected:
            /**
             * Constructor.
             */
            Environment(std::tr1::shared_ptr<PackageDatabase>);

            /**
             * Local package set, or zero.
             */
            virtual std::tr1::shared_ptr<CompositeDepSpec> local_package_set(const SetName &) const
            {
                return std::tr1::shared_ptr<AllDepSpec>();
            }

            /**
             * Change our package database.
             */
            void change_package_database(std::tr1::shared_ptr<PackageDatabase> _p)
            {
                _package_database = _p;
            }

        public:
            /**
             * Does the user want the specified USE flag set for a
             * particular package?
             *
             * Default behaviour: all USE flags turned off, unless overridden by
             * the repository for the pde.
             */
            virtual bool query_use(const UseFlagName &, const PackageDatabaseEntry *) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Force a use flag.
             */
            virtual void force_use(std::tr1::shared_ptr<const PackageDepSpec>, const UseFlagName &,
                    const UseFlagState) = 0;

            /**
             * Clear forced use flags.
             */
            virtual void clear_forced_use() = 0;

            /**
             * Fetch any known use expand names (excluding prefix) that start with a
             * given prefix.
             *
             * Default behaviour: no names known.
             */
            virtual std::tr1::shared_ptr<const UseFlagNameCollection> known_use_expand_names(const UseFlagName &,
                    const PackageDatabaseEntry *) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Is the specified KEYWORD accepted?
             *
             * Default behaviour: only "*" accepted.
             */
            virtual bool accept_keyword(const KeywordName &, const PackageDatabaseEntry * const,
                    const bool override_tilde_keywords = false) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Is the specified EAPI accepted?
             *
             * Default behaviour: known EAPIs accepted.
             */
            virtual bool accept_eapi(const std::string &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Is the specified LICENSE accepted?
             *
             * Default behaviour: yes.
             */
            virtual bool accept_license(const std::string &, const PackageDatabaseEntry * const) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Accept packages that break Portage?
             *
             * Default behaviour: yes.
             */
            virtual bool accept_breaks_portage() const;

            /**
             * Accept interactive packages?
             *
             * Default behaviour: no.
             */
            virtual bool accept_interactive() const;

            /**
             * Fetch the masks for a particular package.
             */
            MaskReasons mask_reasons(const PackageDatabaseEntry &,
                    const bool override_tilde_keywords = false,
                    const bool override_unkeyworded = false) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Are there any user masks on a package?
             *
             * Default behaviour: no.
             */
            virtual bool query_user_masks(const PackageDatabaseEntry &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Are there any user unmasks on a package?
             *
             * Default behaviour: no.
             */
            virtual bool query_user_unmasks(const PackageDatabaseEntry &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Fetch our package database.
             */
            std::tr1::shared_ptr<const PackageDatabase> package_database() const
            {
                return _package_database;
            }

            /**
             * Fetch our package database.
             */
            std::tr1::shared_ptr<PackageDatabase> package_database()
            {
                return _package_database;
            }

            /**
             * Our bashrc files.
             *
             * Default behaviour: none.
             */
            virtual std::string bashrc_files() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Our hook directories.
             *
             * Default behaviour: none.
             */
            virtual std::string hook_dirs() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Our fetchers directories.
             *
             * Default behaviour: user then paludis fetcher dirs.
             */
            virtual std::string fetchers_dirs() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Our syncers directories.
             *
             * Default behaviour: user then paludis syncer dirs.
             */
            virtual std::string syncers_dirs() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * How to run paludis.
             */
            virtual std::string paludis_command() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Change how to run paludis.
             */
            virtual void set_paludis_command(const std::string &) = 0;

            /**
             * Destructor.
             */
            virtual ~Environment();

            /**
             * Iterator over named mirror entries.
             */
            typedef EnvironmentMirrorIterator MirrorIterator;

            /**
             * Iterator to the start of our mirrors.
             *
             * Default behaviour: no mirrors. If specialising, also do
             * Environment::end_mirrors.
             */
            virtual MirrorIterator begin_mirrors(const std::string & mirror) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Iterator to past the end of our mirrors.
             *
             * Default behaviour: no mirrors. If specialising, also do
             * Environment::begin_mirrors.
             */
            virtual MirrorIterator end_mirrors(const std::string & mirror) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Fetch a named package set.
             */
            std::tr1::shared_ptr<DepSpec> package_set(const SetName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Fetch all named sets. Does not include sets from repositories.
             *
             * Default behaviour: no sets.
             */
            virtual std::tr1::shared_ptr<const SetsCollection> sets_list() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Subclass for callbacks used by add_appropriate_to_world and
             * remove_appropriate_from_world.
             *
             * \ingroup grpenvironment
             * \nosubgrouping
             */
            class WorldCallbacks
            {
                protected:
                    ///\name Basic operations
                    ///\{

                    WorldCallbacks();

                    ///\}

                public:
                    ///\name Basic operations
                    ///\{

                    virtual ~WorldCallbacks();

                    ///\}

                    /**
                     * Called when adding an entry to world.
                     */
                    virtual void add_callback(const PackageDepSpec &);

                    /**
                     * Called when skipping adding an entry to world.
                     */
                    virtual void skip_callback(const PackageDepSpec &, const std::string &);

                    /**
                     * Called when removing an entry to world.
                     */
                    virtual void remove_callback(const PackageDepSpec &);

                    /**
                     * Called when adding an entry to world.
                     */
                    virtual void add_callback(const SetName &);

                    /**
                     * Called when skipping adding an entry to world.
                     */
                    virtual void skip_callback(const SetName &, const std::string &);

                    /**
                     * Called when removing an entry from world.
                     */
                    virtual void remove_callback(const SetName &);
            };

            /**
             * Add packages to world, if they are not there already, and if they are
             * not a restricted spec.
             */
            void add_appropriate_to_world(std::tr1::shared_ptr<const DepSpec> a, WorldCallbacks *) const;

            /**
             * Remove packages from world, if they are there.
             */
            void remove_appropriate_from_world(std::tr1::shared_ptr<const DepSpec>, WorldCallbacks *) const;

            /**
             * Add a set to world, if it's not inappropriate.
             */
            void add_set_to_world(const SetName &, WorldCallbacks *) const;

            /**
             * Remove a set from world.
             */
            void remove_set_from_world(const SetName &, WorldCallbacks *) const;

            /**
             * Perform a hook.  Returns the highest exit status of all
             * hooks called (usually, only zero or non-zero is
             * meaningful).
             *
             * Default behaviour: nothing happens.
             */
            virtual int perform_hook(const Hook & hook) const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Default root location.
             *
             * Default: /.
             */
            virtual FSEntry root() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Default destinations.
             *
             * Default: all repositories that provide RepositoryDestinationInterface and mark themselves
             * as a default destination.
             */
            virtual std::tr1::shared_ptr<const DestinationsCollection> default_destinations() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * uid to use for operations that don't reqiure root privs.
             *
             * Should return the current uid unless we are root. Default: always return
             * the current uid.
             */
            virtual uid_t reduced_uid() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * gid to use for operations that don't reqiure root privs.
             *
             * Should return the current gid unless we are root. Default: always return
             * the current gid.
             */
            virtual gid_t reduced_gid() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Test whether a particular package will break Portage.
             */
            bool breaks_portage(const PackageDatabaseEntry &, const VersionMetadata &) const;
    };
}

#endif
