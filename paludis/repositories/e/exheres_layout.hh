/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_GENTOO_EXHERES_LAYOUT_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_GENTOO_EXHERES_LAYOUT_HH 1

#include <paludis/repositories/e/layout.hh>
#include <paludis/util/pimp.hh>

namespace paludis
{
    namespace erepository
    {
        /**
         * The Exheres tree layout for a ERepository.
         *
         * \ingroup grperepository
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE ExheresLayout :
            public Layout
        {
            private:
                Pimp<ExheresLayout> _imp;

                void need_category_names() const;
                void need_category_names_collection() const;
                void need_package_ids(const QualifiedPackageName &) const;

            public:
                ///\name Basic operations
                ///\{

                ExheresLayout(
                        const Environment * const,
                        const ERepository * const, const FSPath &,
                        const std::shared_ptr<const FSPathSequence> &);

                ~ExheresLayout() override;

                ///\}

                bool has_category_named(const CategoryNamePart &) const
                    override PALUDIS_ATTRIBUTE((warn_unused_result));

                bool has_package_named(const QualifiedPackageName &) const
                    override PALUDIS_ATTRIBUTE((warn_unused_result));

                FSPath categories_file() const
                    override PALUDIS_ATTRIBUTE((warn_unused_result));

                std::shared_ptr<const CategoryNamePartSet> category_names() const
                    override PALUDIS_ATTRIBUTE((warn_unused_result));

                std::shared_ptr<const QualifiedPackageNameSet> package_names(
                        const CategoryNamePart &) const
                    override PALUDIS_ATTRIBUTE((warn_unused_result));

                std::shared_ptr<const PackageIDSequence> package_ids(
                        const QualifiedPackageName &) const
                    override PALUDIS_ATTRIBUTE((warn_unused_result));

                const std::shared_ptr<const FSPathSequence> info_packages_files() const
                    override PALUDIS_ATTRIBUTE((warn_unused_result));

                const std::shared_ptr<const FSPathSequence> info_variables_files() const
                    override PALUDIS_ATTRIBUTE((warn_unused_result));

                FSPath package_directory(const QualifiedPackageName &) const
                    override PALUDIS_ATTRIBUTE((warn_unused_result));

                FSPath category_directory(const CategoryNamePart &) const
                    override PALUDIS_ATTRIBUTE((warn_unused_result));

                FSPath binary_ebuild_directory(const QualifiedPackageName &) const override PALUDIS_ATTRIBUTE((warn_unused_result));

                std::shared_ptr<const FSPathSequence> arch_list_files() const
                    override PALUDIS_ATTRIBUTE((warn_unused_result));

                std::shared_ptr<const FSPathSequence> repository_mask_files() const
                    override PALUDIS_ATTRIBUTE((warn_unused_result));

                std::shared_ptr<const FSPathSequence> profiles_desc_files() const
                    override PALUDIS_ATTRIBUTE((warn_unused_result));

                std::shared_ptr<const FSPathSequence> mirror_files() const
                    override PALUDIS_ATTRIBUTE((warn_unused_result));

                std::shared_ptr<const UseDescFileInfoSequence> use_desc_files() const
                    override PALUDIS_ATTRIBUTE((warn_unused_result));

                FSPath profiles_base_dir() const
                    override PALUDIS_ATTRIBUTE((warn_unused_result));

                std::shared_ptr<const FSPathSequence> exlibsdirs(const QualifiedPackageName &) const override;

                std::shared_ptr<const FSPathSequence> exlibsdirs_global() const
                    override PALUDIS_ATTRIBUTE((warn_unused_result));

                std::shared_ptr<const FSPathSequence> exlibsdirs_category(const CategoryNamePart &) const
                    override PALUDIS_ATTRIBUTE((warn_unused_result));

                std::shared_ptr<const FSPathSequence> exlibsdirs_package(const QualifiedPackageName &) const
                    override PALUDIS_ATTRIBUTE((warn_unused_result));

                std::shared_ptr<const FSPathSequence> licenses_dirs() const
                    override PALUDIS_ATTRIBUTE((warn_unused_result));

                std::shared_ptr<Map<FSPath, std::string, FSPathComparator> > manifest_files(
                        const QualifiedPackageName &, const FSPath &) const
                    override PALUDIS_ATTRIBUTE((warn_unused_result));

                std::shared_ptr<MetadataValueKey<FSPath> > accounts_repository_data_location_key() const
                    override PALUDIS_ATTRIBUTE((warn_unused_result));

                std::shared_ptr<MetadataValueKey<FSPath> > e_updates_location_key() const
                    override PALUDIS_ATTRIBUTE((warn_unused_result));

                std::shared_ptr<MetadataValueKey<FSPath> > licence_groups_location_key() const
                    override PALUDIS_ATTRIBUTE((warn_unused_result));

                std::shared_ptr<const MasksInfo> repository_masks(const std::shared_ptr<const PackageID> &) const
                    override PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }
}

#endif
