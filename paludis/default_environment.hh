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

#ifndef PALUDIS_GUARD_PALUDIS_DEFAULT_ENVIRONMENT_HH
#define PALUDIS_GUARD_PALUDIS_DEFAULT_ENVIRONMENT_HH 1

#include <paludis/package_database.hh>
#include <paludis/environment.hh>

/** \file
 * Declarations for the DefaultEnvironment class.
 *
 * \ingroup grpdefaultenvironment
 */

namespace paludis
{
    /**
     * The DefaultEnvironment is an Environment that corresponds to the normal
     * operating evironment.
     *
     * \ingroup grpdefaultenvironment
     */
    class DefaultEnvironment :
        public Environment,
        public InstantiationPolicy<DefaultEnvironment, instantiation_method::SingletonAsNeededTag>
    {
        friend class InstantiationPolicy<DefaultEnvironment, instantiation_method::SingletonAsNeededTag>;

        private:
            DefaultEnvironment();

            ~DefaultEnvironment();

        protected:
            CompositeDepAtom::Pointer local_package_set(const std::string &,
                    const PackageSetOptions & = PackageSetOptions(false)) const;

        public:
            virtual bool query_use(const UseFlagName &, const PackageDatabaseEntry *) const;

            virtual bool accept_keyword(const KeywordName &, const PackageDatabaseEntry * const) const;

            virtual bool accept_license(const std::string &, const PackageDatabaseEntry * const) const;

            virtual bool query_user_masks(const PackageDatabaseEntry &) const;

            virtual bool query_user_unmasks(const PackageDatabaseEntry &) const;

            virtual std::string bashrc_files() const;

            virtual std::string hook_dirs() const;

            virtual std::string paludis_command() const;

            virtual UseFlagNameCollection::Pointer query_enabled_use_matching(
                    const std::string & prefix, const PackageDatabaseEntry *) const;

            virtual void perform_hook(const Hook & hook) const;

            virtual MirrorIterator begin_mirrors(const std::string & mirror) const;

            virtual MirrorIterator end_mirrors(const std::string & mirror) const;
    };
}
#endif
