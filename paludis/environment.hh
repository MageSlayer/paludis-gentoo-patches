/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/util/counted_ptr.hh>
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
    class Hook
    {
        private:
            std::map<std::string, std::string> _extra_env;

            std::string _name;

        public:
            /// Constructor.
            Hook(const std::string & name);

            /// Perform the hook.
            Hook operator() (const std::string & key, const std::string & value) const;

            /// Iterate over extra environment data.
            typedef std::map<std::string, std::string>::const_iterator Iterator;

            /// Start of extra environment data.
            Iterator begin() const
            {
                return _extra_env.begin();
            }

            /// End of extra environment data.
            Iterator end() const
            {
                return _extra_env.end();
            }

            /// Our name.
            std::string name() const
            {
                return _name;
            }
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
            PackageDatabase::Pointer _package_database;

            mutable bool _has_provide_map;

            mutable std::map<QualifiedPackageName, QualifiedPackageName> _provide_map;

        protected:
            /**
             * Constructor.
             */
            Environment(PackageDatabase::Pointer);

            /**
             * Local package set, or zero.
             */
            virtual CompositeDepAtom::Pointer local_package_set(const SetName &) const
            {
                return AllDepAtom::Pointer(0);
            }

            /**
             * Change our package database.
             */
            void change_package_database(PackageDatabase::Pointer _p)
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
            virtual bool query_use(const UseFlagName &, const PackageDatabaseEntry *) const;

            /**
             * Fetch any known use expand names (excluding prefix) that start with a
             * given prefix.
             *
             * Default behaviour: no names known.
             */
            virtual UseFlagNameCollection::ConstPointer known_use_expand_names(const UseFlagName &,
                    const PackageDatabaseEntry *) const;

            /**
             * Is the specified KEYWORD accepted?
             *
             * Default behaviour: only "*" accepted.
             */
            virtual bool accept_keyword(const KeywordName &, const PackageDatabaseEntry * const) const;

            /**
             * Is the specified LICENSE accepted?
             *
             * Default behaviour: yes.
             */
            virtual bool accept_license(const std::string &, const PackageDatabaseEntry * const) const;

            /**
             * Fetch the masks for a particular package.
             */
            MaskReasons mask_reasons(const PackageDatabaseEntry &) const;

            /**
             * Are there any user masks on a package?
             *
             * Default behaviour: no.
             */
            virtual bool query_user_masks(const PackageDatabaseEntry &) const;

            /**
             * Are there any user unmasks on a package?
             *
             * Default behaviour: no.
             */
            virtual bool query_user_unmasks(const PackageDatabaseEntry &) const;

            /**
             * Fetch our package database.
             */
            PackageDatabase::Pointer package_database() const
            {
                return _package_database;
            }

            /**
             * Our bashrc files.
             *
             * Default behaviour: none.
             */
            virtual std::string bashrc_files() const;

            /**
             * Our hook directories.
             *
             * Default behaviour: none.
             */
            virtual std::string hook_dirs() const;

            /**
             * How to run paludis.
             */
            virtual std::string paludis_command() const = 0;

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
            virtual MirrorIterator begin_mirrors(const std::string & mirror) const;

            /**
             * Iterator to past the end of our mirrors.
             *
             * Default behaviour: no mirrors. If specialising, also do
             * Environment::begin_mirrors.
             */
            virtual MirrorIterator end_mirrors(const std::string & mirror) const;

            /**
             * Fetch a named package set.
             */
            DepAtom::Pointer package_set(const SetName &) const;

            /**
             * Fetch all named sets. Does not include sets from repositories.
             *
             * Default behaviour: no sets.
             */
            virtual SetsCollection::ConstPointer sets_list() const;

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

                    WorldCallbacks()
                    {
                    }

                    ///\}

                public:
                    ///\name Basic operations
                    ///\{

                    virtual ~WorldCallbacks()
                    {
                    }

                    ///\}

                    /**
                     * Called when adding an entry to world.
                     */
                    virtual void add_callback(const PackageDepAtom *)
                    {
                    }

                    /**
                     * Called when skipping adding an entry to world.
                     */
                    virtual void skip_callback(const PackageDepAtom *,
                            const std::string &)
                    {
                    }

                    /**
                     * Called when removing an entry to world.
                     */
                    virtual void remove_callback(const PackageDepAtom *)
                    {
                    }
            };

            /**
             * Add packages to world, if they are not there already, and if they are
             * not a restricted atom.
             */
            void
            add_appropriate_to_world(DepAtom::ConstPointer a, WorldCallbacks * const) const;

            /**
             * Remove packages from world, if they are there.
             */
            void remove_appropriate_from_world(DepAtom::ConstPointer, WorldCallbacks * const) const;

            /**
             * Perform a hook.
             *
             * Default behaviour: nothing happens.
             */
            virtual void perform_hook(const Hook & hook) const;
    };
}

#endif
