/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009 Ciaran McCreesh
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
#include <paludis/util/private_implementation_pattern.hh>

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

                ExheresLayout(const ERepository * const, const FSEntry &,
                        const std::tr1::shared_ptr<const FSEntrySequence> &);

                virtual ~ExheresLayout();

                ///\}

                virtual bool has_category_named(const CategoryNamePart &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual bool has_package_named(const QualifiedPackageName &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual FSEntry categories_file() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::tr1::shared_ptr<const CategoryNamePartSet> category_names() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::tr1::shared_ptr<const QualifiedPackageNameSet> package_names(
                        const CategoryNamePart &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::tr1::shared_ptr<const PackageIDSequence> package_ids(
                        const QualifiedPackageName &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::tr1::shared_ptr<const FSEntrySequence> info_packages_files() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::tr1::shared_ptr<const FSEntrySequence> info_variables_files() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual FSEntry package_directory(const QualifiedPackageName &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual FSEntry category_directory(const CategoryNamePart &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual FSEntry binary_ebuild_location(const QualifiedPackageName &, const VersionSpec &,
                        const std::string & eapi) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::tr1::shared_ptr<const FSEntrySequence> arch_list_files() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::tr1::shared_ptr<const FSEntrySequence> repository_mask_files() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::tr1::shared_ptr<const FSEntrySequence> profiles_desc_files() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::tr1::shared_ptr<const FSEntrySequence> mirror_files() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::tr1::shared_ptr<const UseDescFileInfoSequence> use_desc_files() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual FSEntry profiles_base_dir() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::tr1::shared_ptr<const FSEntrySequence> exlibsdirs(const QualifiedPackageName &) const;

                virtual std::tr1::shared_ptr<const FSEntrySequence> exlibsdirs_global() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::tr1::shared_ptr<const FSEntrySequence> exlibsdirs_category(const CategoryNamePart &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::tr1::shared_ptr<const FSEntrySequence> exlibsdirs_package(const QualifiedPackageName &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::tr1::shared_ptr<const FSEntrySequence> licenses_dirs() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::tr1::shared_ptr<Map<FSEntry, std::string> > manifest_files(const QualifiedPackageName &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::tr1::shared_ptr<MetadataValueKey<FSEntry> > accounts_repository_data_location_key() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::tr1::shared_ptr<MetadataValueKey<FSEntry> > e_updates_location_key() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual void invalidate_masks();
        };
    }
}

#endif
