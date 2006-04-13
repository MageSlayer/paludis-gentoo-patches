/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_PORTAGE_REPOSITORY_HH
#define PALUDIS_GUARD_PALUDIS_PORTAGE_REPOSITORY_HH 1

#include <map>
#include <paludis/repository.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <string>

/** \file
 * Declaration for the PortageRepository class.
 */

namespace paludis
{
    class PackageDatabase;

    /**
     * A PortageRepository is a Repository that handles the layout used by
     * Portage for the main Gentoo tree.
     */
    class PortageRepository : public Repository,
                              private PrivateImplementationPattern<PortageRepository>
    {
        private:
            void need_category_names() const;
            void need_version_names(const QualifiedPackageName &) const;
            void need_virtual_names() const;

        protected:
            /**
             * Try to get the repository name for a particular repository.
             */
            static RepositoryName fetch_repo_name(const std::string & location);

            virtual bool do_has_category_named(const CategoryNamePart &) const;

            virtual bool do_has_package_named(const CategoryNamePart &,
                    const PackageNamePart &) const;

            virtual CategoryNamePartCollection::ConstPointer do_category_names() const;

            virtual QualifiedPackageNameCollection::ConstPointer do_package_names(
                    const CategoryNamePart &) const;

            virtual VersionSpecCollection::ConstPointer do_version_specs(
                    const QualifiedPackageName &) const;

            virtual bool do_has_version(const CategoryNamePart &,
                    const PackageNamePart &, const VersionSpec &) const;

            virtual VersionMetadata::ConstPointer do_version_metadata(
                    const CategoryNamePart &, const PackageNamePart &,
                    const VersionSpec &) const;

            virtual bool do_query_repository_masks(const CategoryNamePart &,
                    const PackageNamePart &, const VersionSpec &) const;

            virtual bool do_query_profile_masks(const CategoryNamePart &,
                    const PackageNamePart &, const VersionSpec &) const;

            virtual UseFlagState do_query_use(const UseFlagName &) const;

            virtual bool do_query_use_mask(const UseFlagName &) const;

            virtual bool do_is_arch_flag(const UseFlagName &) const;

            virtual bool do_is_expand_flag(const UseFlagName &) const;

            virtual bool do_is_licence(const std::string &) const;

            virtual bool do_is_mirror(const std::string &) const;

            virtual void do_install(const QualifiedPackageName &, const VersionSpec &) const;

            virtual void do_uninstall(const QualifiedPackageName &,
                    const VersionSpec &) const PALUDIS_ATTRIBUTE((noreturn));

            virtual DepAtom::Pointer do_package_set(const std::string & s) const;

            virtual bool do_sync() const;

        public:
            /**
             * Constructor.
             */
            PortageRepository(const Environment * const env,
                    const PackageDatabase * const db,
                    const FSEntry & location, const FSEntry & profile,
                    const FSEntry & cache, const FSEntry & distdir,
                    const FSEntry & eclassdir, const std::string & sync,
                    const FSEntry & root);

            /**
             * Virtual constructor.
             */
            static CountedPtr<Repository> make_portage_repository(
                    const Environment * const env,
                    const PackageDatabase * const db,
                    const std::map<std::string, std::string> &);

            /**
             * Destructor.
             */
            ~PortageRepository();

            virtual bool installed() const
            {
                return false;
            }

            virtual void invalidate() const;

            virtual ProvideMapIterator begin_provide_map() const;

            virtual ProvideMapIterator end_provide_map() const;

            virtual void add_to_world(const QualifiedPackageName &) const
            {
            }

            virtual void remove_from_world(const QualifiedPackageName &) const
            {
            }
    };

    /**
     * Thrown if invalid parameters are provided for
     * PortageRepository::make_portage_repository.
     *
     * \ingroup Exception
     */
    class PortageRepositoryConfigurationError : public ConfigurationError
    {
        public:
            /**
             * Constructor.
             */
            PortageRepositoryConfigurationError(const std::string & msg) throw ();
    };

    static const RepositoryMaker::RegisterMaker register_portage_repository(
            "portage", &PortageRepository::make_portage_repository);

}

#endif
