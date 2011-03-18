/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
 * Copyright (c) 2006 Danny van Dyk
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_GENTOO_LAYOUT_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_GENTOO_LAYOUT_HH 1

#include <paludis/name-fwd.hh>
#include <paludis/version_spec-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/fs_path-fwd.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/map-fwd.hh>
#include <paludis/util/singleton.hh>
#include <paludis/repositories/e/use_desc.hh>
#include <paludis/metadata_key-fwd.hh>
#include <memory>

namespace paludis
{
    class ERepository;

    namespace erepository
    {
        /**
         * Manages the layout of a ERepository.
         *
         * \ingroup grperepository
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE Layout
        {
            private:
                const std::shared_ptr<const FSPathSequence> _master_repositories_locations;

            protected:
                ///\name Basic operations
                ///\{

                Layout(const std::shared_ptr<const FSPathSequence> & master_repositories_locations);

                ///\}

            public:
                ///\name Basic operations
                ///\{

                virtual ~Layout() = 0;

                ///\}

                ///\name Configuration information
                ///\{

                const std::shared_ptr<const FSPathSequence> master_repositories_locations() const;

                ///\}

                ///\name Layout operations
                ///\{

                virtual bool has_category_named(const CategoryNamePart &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual bool has_package_named(const QualifiedPackageName &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual FSPath categories_file() const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual std::shared_ptr<const CategoryNamePartSet> category_names() const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual std::shared_ptr<const QualifiedPackageNameSet> package_names(
                        const CategoryNamePart &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual std::shared_ptr<const PackageIDSequence> package_ids(
                        const QualifiedPackageName &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual const std::shared_ptr<const FSPathSequence> info_packages_files() const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual const std::shared_ptr<const FSPathSequence> info_variables_files() const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual FSPath package_directory(const QualifiedPackageName &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual FSPath category_directory(const CategoryNamePart &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual FSPath binary_ebuild_directory(const QualifiedPackageName &) const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual std::shared_ptr<const FSPathSequence> arch_list_files() const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual std::shared_ptr<const FSPathSequence> repository_mask_files() const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual std::shared_ptr<const FSPathSequence> profiles_desc_files() const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual std::shared_ptr<const FSPathSequence> mirror_files() const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual std::shared_ptr<const UseDescFileInfoSequence> use_desc_files() const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual FSPath profiles_base_dir() const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual std::shared_ptr<const FSPathSequence> exlibsdirs(const QualifiedPackageName &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual std::shared_ptr<const FSPathSequence> exlibsdirs_global() const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual std::shared_ptr<const FSPathSequence> exlibsdirs_category(const CategoryNamePart &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual std::shared_ptr<const FSPathSequence> exlibsdirs_package(const QualifiedPackageName &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual std::shared_ptr<const FSPathSequence> licenses_dirs() const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual std::shared_ptr<Map<FSPath, std::string, FSPathComparator> > manifest_files(const QualifiedPackageName &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual std::shared_ptr<MetadataValueKey<FSPath> > accounts_repository_data_location_key() const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual std::shared_ptr<MetadataValueKey<FSPath> > e_updates_location_key() const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual FSPath sync_filter_file() const;

                ///\}
        };

        /**
         * Virtual constructor for Layout.
         *
         * \ingroup grperepository
         */
        class PALUDIS_VISIBLE LayoutFactory :
            public Singleton<LayoutFactory>
        {
            friend class Singleton<LayoutFactory>;

            private:
                LayoutFactory();

            public:
                const std::shared_ptr<Layout> create(
                        const std::string &,
                        const ERepository * const,
                        const FSPath &,
                        const std::shared_ptr<const FSPathSequence> &)
                    const PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }
}

#endif
