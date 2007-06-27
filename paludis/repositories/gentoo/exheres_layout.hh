/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/repositories/gentoo/layout.hh>
#include <paludis/util/private_implementation_pattern.hh>

namespace paludis
{
    class PortageRepositoryEntries;

    /**
     * The Exheres tree layout for a PortageRepository.
     *
     * \ingroup grpportagerepository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE ExheresLayout :
        public Layout,
        private PrivateImplementationPattern<ExheresLayout>
    {
        private:
            void need_category_names() const;
            void need_category_names_collection() const;
            void need_package_ids(const QualifiedPackageName &) const;

        public:
            ///\name Basic operations
            ///\{

            ExheresLayout(const PortageRepository * const, const FSEntry &,
                    tr1::shared_ptr<const PortageRepositoryEntries>,
                    tr1::shared_ptr<const FSEntry>);

            virtual ~ExheresLayout();

            ///\}

            virtual bool has_category_named(const CategoryNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool has_package_named(const QualifiedPackageName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const CategoryNamePartCollection> category_names() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const QualifiedPackageNameCollection> package_names(
                    const CategoryNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const PackageIDSequence> package_ids(
                    const QualifiedPackageName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual FSEntry info_packages_file(const FSEntry &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual FSEntry info_variables_file(const FSEntry &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual FSEntry package_directory(const QualifiedPackageName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual FSEntry category_directory(const CategoryNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual FSEntry package_file(const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const FSEntryCollection> arch_list_files() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const FSEntryCollection> repository_mask_files() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const FSEntryCollection> profiles_desc_files() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const FSEntryCollection> mirror_files() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const FSEntryCollection> use_desc_dirs() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool eapi_ebuild_suffix() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual FSEntry profiles_base_dir() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const FSEntryCollection> exlibsdirs(const QualifiedPackageName &) const;
    };
}

#endif
