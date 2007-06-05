/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
 * Copyright (c) 2006 Danny van Dyk <kugelfang@gentoo.org>
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
#include <paludis/util/attributes.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/virtual_constructor.hh>
#include <paludis/util/tr1_memory.hh>

namespace paludis
{
    class PortageRepositoryEntries;

    /**
     * Manages the layout of a PortageRepository.
     *
     * \ingroup grpportagerepository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE Layout
    {
        private:
            tr1::shared_ptr<const FSEntry> _master_repository_location;

        protected:
            ///\name Basic operations
            ///\{

            Layout(tr1::shared_ptr<const FSEntry> master_repository_location);

            ///\}

        public:
            ///\name Basic operations
            ///\{

            virtual ~Layout() = 0;

            ///\}

            ///\name Configuration information
            ///\{

            tr1::shared_ptr<const FSEntry> master_repository_location() const;

            ///\}

            ///\name Layout operations
            ///\{

            virtual bool has_category_named(const CategoryNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual bool has_package_named(const QualifiedPackageName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual tr1::shared_ptr<const CategoryNamePartCollection> category_names() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual tr1::shared_ptr<const QualifiedPackageNameCollection> package_names(
                    const CategoryNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual tr1::shared_ptr<const VersionSpecCollection> version_specs(
                    const QualifiedPackageName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual bool has_version(const QualifiedPackageName &, const VersionSpec &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual FSEntry info_packages_file(const FSEntry &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual FSEntry info_variables_file(const FSEntry &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual FSEntry package_directory(const QualifiedPackageName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual FSEntry category_directory(const CategoryNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual FSEntry package_file(const QualifiedPackageName &, const VersionSpec &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual std::string eapi_string_if_known(const QualifiedPackageName &, const VersionSpec &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual tr1::shared_ptr<const FSEntryCollection> arch_list_files() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual tr1::shared_ptr<const FSEntryCollection> repository_mask_files() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual tr1::shared_ptr<const FSEntryCollection> profiles_desc_files() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual tr1::shared_ptr<const FSEntryCollection> mirror_files() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual tr1::shared_ptr<const FSEntryCollection> use_desc_dirs() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual bool eapi_ebuild_suffix() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual FSEntry profiles_base_dir() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            ///\}
    };

    /**
     * Thrown if a layout of the specified type does not exist.
     *
     * \ingroup grpexceptions
     * \ingroup grpportagerepository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE NoSuchLayoutType :
        public ConfigurationError
    {
        public:
            /**
             * Constructor.
             */
            NoSuchLayoutType(const std::string & format) throw ();
    };

    /**
     * Virtual constructor for Layout.
     *
     * \ingroup grpportagerepository
     */
    class PALUDIS_VISIBLE LayoutMaker :
        public VirtualConstructor<std::string,
            tr1::shared_ptr<Layout> (*) (const RepositoryName &, const FSEntry &,
                    tr1::shared_ptr<const PortageRepositoryEntries>,
                    tr1::shared_ptr<const FSEntry>),
            virtual_constructor_not_found::ThrowException<NoSuchLayoutType> >,
        public InstantiationPolicy<LayoutMaker, instantiation_method::SingletonTag>
    {
        friend class InstantiationPolicy<LayoutMaker, instantiation_method::SingletonTag>;

        private:
            LayoutMaker();
    };
}

#endif
