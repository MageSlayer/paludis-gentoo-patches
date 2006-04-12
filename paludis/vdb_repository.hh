/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_VDB_REPOSITORY_HH
#define PALUDIS_GUARD_PALUDIS_VDB_REPOSITORY_HH 1

#include <paludis/repository.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/private_implementation_pattern.hh>

namespace paludis
{
    class VDBRepository :
        public Repository,
        public PrivateImplementationPattern<VDBRepository>
    {
        protected:
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

            virtual void do_install(const QualifiedPackageName &,
                    const VersionSpec &) const PALUDIS_ATTRIBUTE((noreturn));

            virtual void do_uninstall(const QualifiedPackageName &, const VersionSpec &) const;

            virtual DepAtom::Pointer do_package_set(const std::string &) const;

            virtual bool do_sync() const;

        public:
            /**
             * Constructor.
             */
            VDBRepository(const Environment * const env,
                    const PackageDatabase * const db,
                    const FSEntry & location,
                    const FSEntry & root);

            /**
             * Virtual constructor.
             */
            static CountedPtr<Repository> make_vdb_repository(
                    const Environment * const env,
                    const PackageDatabase * const db,
                    const std::map<std::string, std::string> &);

            /**
             * Destructor.
             */
            ~VDBRepository();

            virtual bool installed() const
            {
                return true;
            }

            virtual void invalidate() const;
    };

    /**
     * Thrown if invalid parameters are provided for
     * PortageRepository::make_portage_repository.
     *
     * \ingroup Exception
     */
    class VDBRepositoryConfigurationError : public ConfigurationError
    {
        public:
            /**
             * Constructor.
             */
            VDBRepositoryConfigurationError(const std::string & msg) throw ();
    };

    static const RepositoryMaker::RegisterMaker register_vdb_repository(
            "vdb", &VDBRepository::make_vdb_repository);
}

#endif
