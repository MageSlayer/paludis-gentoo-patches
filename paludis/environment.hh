/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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
            virtual DepAtom::Pointer local_package_set(const std::string &,
                    const PackageSetOptions & = PackageSetOptions(false)) const
            {
                return DepAtom::Pointer(0);
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
             */
            virtual bool query_use(const UseFlagName &, const PackageDatabaseEntry *) const = 0;

            /**
             * Fetch a list of enabled USE flags that start with a given prefix,
             * for USE_EXPAND.
             */
            virtual UseFlagNameCollection::Pointer query_enabled_use_matching(
                    const std::string & prefix, const PackageDatabaseEntry *) const = 0;

            /**
             * Is the specified KEYWORD accepted?
             */
            virtual bool accept_keyword(const KeywordName &, const PackageDatabaseEntry * const) const = 0;

            /**
             * Is the specified LICENSE accepted?
             */
            virtual bool accept_license(const std::string &, const PackageDatabaseEntry * const) const = 0;

            /**
             * Fetch the masks for a particular package.
             */
            MaskReasons mask_reasons(const PackageDatabaseEntry &) const;

            /**
             * Are there any user masks on a package?
             */
            virtual bool query_user_masks(const PackageDatabaseEntry &) const = 0;

            /**
             * Are there any user unmasks on a package?
             */
            virtual bool query_user_unmasks(const PackageDatabaseEntry &) const = 0;

            /**
             * Fetch our package database.
             */
            PackageDatabase::Pointer package_database() const
            {
                return _package_database;
            }

            /**
             * Our bashrc files.
             */
            virtual std::string bashrc_files() const = 0;

            /**
             * Our hook directories.
             */
            virtual std::string hook_dirs() const = 0;

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
             */
            virtual MirrorIterator begin_mirrors(const std::string & mirror) const = 0;

            /**
             * Iterator to past the end of our mirrors.
             */
            virtual MirrorIterator end_mirrors(const std::string & mirror) const = 0;

            /**
             * Fetch a named package set.
             */
            DepAtom::Pointer package_set(const std::string &,
                    const PackageSetOptions & = PackageSetOptions(false)) const;

            /**
             * Subclass for callbacks used by add_appropriate_to_world and
             * remove_appropriate_from_world.
             *
             * \ingroup grpenvironment
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
             */
            virtual void perform_hook(const Hook & hook) const = 0;
    };
}

#endif
