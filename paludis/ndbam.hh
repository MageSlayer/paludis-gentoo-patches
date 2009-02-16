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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNPACKAGED_NDBAM_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNPACKAGED_NDBAM_HH 1

#include <paludis/ndbam-fwd.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/sequence-fwd.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/named_value.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/contents-fwd.hh>
#include <tr1/functional>

namespace paludis
{
    namespace n
    {
        struct fs_location;
        struct magic;
        struct mutex;
        struct name;
        struct package_id;
        struct slot;
        struct version;
    }

    struct NDBAMEntry
    {
        NamedValue<n::fs_location, FSEntry> fs_location;
        NamedValue<n::magic, std::string> magic;
        NamedValue<n::mutex, std::tr1::shared_ptr<Mutex> > mutex;
        NamedValue<n::name, QualifiedPackageName> name;
        NamedValue<n::package_id, std::tr1::shared_ptr<PackageID> > package_id;
        NamedValue<n::slot, SlotName> slot;
        NamedValue<n::version, VersionSpec> version;
    };

    /**
     * NDBAM provides a partial implementation of a Repository for installed packages using
     * a Paludis-defined on-disk format. It is used by unpackaged repositories and exndbam,
     * where the format can be defined by us. NDBAM is designed to reduce unnecessary disk
     * access and to reduce the need for global locking.
     *
     * \ingroup g_ndbam
     * \since 0.26
     */
    class PALUDIS_VISIBLE NDBAM :
        private PrivateImplementationPattern<NDBAM>
    {
        public:
            ///\name Basic operations
            ///\{

            NDBAM(const FSEntry &,
                    const std::tr1::function<bool (const std::string &)> & check_format,
                    const std::string & preferred_format);
            ~NDBAM();

            ///\}

            ///\name Repository method implementations
            ///\{

            std::tr1::shared_ptr<const CategoryNamePartSet> category_names()
                PALUDIS_ATTRIBUTE((warn_unused_result));

            std::tr1::shared_ptr<const QualifiedPackageNameSet> package_names(
                    const CategoryNamePart & c)
                PALUDIS_ATTRIBUTE((warn_unused_result));

            std::tr1::shared_ptr<const CategoryNamePartSet> category_names_containing_package(
                    const PackageNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            bool has_package_named(const QualifiedPackageName &)
                PALUDIS_ATTRIBUTE((warn_unused_result));

            bool has_category_named(const CategoryNamePart &)
                PALUDIS_ATTRIBUTE((warn_unused_result));

            std::tr1::shared_ptr<NDBAMEntrySequence> entries(const QualifiedPackageName &)
                PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}

            /**
             * Parse the contents file for a given ID, using the provided callbacks.
             */
            void parse_contents(const PackageID &,
                    const std::tr1::function<void (const std::tr1::shared_ptr<const ContentsEntry> &)> & on_file,
                    const std::tr1::function<void (const std::tr1::shared_ptr<const ContentsEntry> &)> & on_dir,
                    const std::tr1::function<void (const std::tr1::shared_ptr<const ContentsEntry> &)> & on_sym
                    ) const;

            /**
             * Index a newly added QualifiedPackageName, using the provided data directory
             * name part.
             */
            void index(const QualifiedPackageName &, const std::string &) const;

            /**
             * Deindex a QualifiedPackageName that no longer has any versions installed.
             */
            void deindex(const QualifiedPackageName &) const;
    };
}

#endif
