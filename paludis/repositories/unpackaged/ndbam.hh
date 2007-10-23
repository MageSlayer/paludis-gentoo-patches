/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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

#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/sequence-fwd.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/contents-fwd.hh>

namespace paludis
{
    namespace unpackaged_repositories
    {

#include <paludis/repositories/unpackaged/ndbam-sr.hh>

        typedef Sequence<tr1::shared_ptr<NDBAMEntry> > NDBAMEntrySequence;

        class NDBAM :
            private PrivateImplementationPattern<NDBAM>
        {
            public:
                NDBAM(const FSEntry &,
                        const tr1::function<bool (const std::string &)> & check_format,
                        const std::string & preferred_format);
                ~NDBAM();

                tr1::shared_ptr<const CategoryNamePartSet> category_names()
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                tr1::shared_ptr<const QualifiedPackageNameSet> package_names(
                        const CategoryNamePart & c)
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                tr1::shared_ptr<const CategoryNamePartSet> category_names_containing_package(
                        const PackageNamePart &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                bool has_package_named(const QualifiedPackageName &)
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                bool has_category_named(const CategoryNamePart &)
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                tr1::shared_ptr<NDBAMEntrySequence> entries(const QualifiedPackageName &)
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                void parse_contents(const PackageID &,
                        const tr1::function<void (const FSEntry &, const std::string & md5, const time_t mtime)> & on_file,
                        const tr1::function<void (const FSEntry &)> & on_dir,
                        const tr1::function<void (const FSEntry &, const std::string & target, const time_t mtime)> & on_sym
                        ) const;

                void index(const QualifiedPackageName &, const std::string &) const;
                void deindex(const QualifiedPackageName &) const;
        };
    }
}

#endif
