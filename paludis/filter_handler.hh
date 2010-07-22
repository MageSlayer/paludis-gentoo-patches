/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
 * Copyright (c) 2008 David Leverton
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

#ifndef PALUDIS_GUARD_PALUDIS_FILTER_HANDLER_HH
#define PALUDIS_GUARD_PALUDIS_FILTER_HANDLER_HH 1

#include <paludis/filter_handler-fwd.hh>
#include <paludis/name-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/util/attributes.hh>
#include <memory>

namespace paludis
{
    class PALUDIS_VISIBLE FilterHandler
    {
        public:
            virtual ~FilterHandler() = 0;

            virtual std::string as_string() const = 0;

            virtual std::shared_ptr<const RepositoryNameSet> repositories(
                    const Environment * const,
                    const std::shared_ptr<const RepositoryNameSet> &) const = 0;

            virtual std::shared_ptr<const CategoryNamePartSet> categories(
                    const Environment * const,
                    const std::shared_ptr<const RepositoryNameSet> &,
                    const std::shared_ptr<const CategoryNamePartSet> &) const = 0;

            virtual std::shared_ptr<const QualifiedPackageNameSet> packages(
                    const Environment * const,
                    const std::shared_ptr<const RepositoryNameSet> &,
                    const std::shared_ptr<const QualifiedPackageNameSet> &) const = 0;

            virtual std::shared_ptr<const PackageIDSet> ids(
                    const Environment * const,
                    const std::shared_ptr<const PackageIDSet> &) const = 0;
    };

    class PALUDIS_VISIBLE AllFilterHandlerBase :
        public FilterHandler
    {
        public:
            virtual std::shared_ptr<const RepositoryNameSet> repositories(
                    const Environment * const,
                    const std::shared_ptr<const RepositoryNameSet> &) const;

            virtual std::shared_ptr<const CategoryNamePartSet> categories(
                    const Environment * const,
                    const std::shared_ptr<const RepositoryNameSet> &,
                    const std::shared_ptr<const CategoryNamePartSet> &) const;

            virtual std::shared_ptr<const QualifiedPackageNameSet> packages(
                    const Environment * const,
                    const std::shared_ptr<const RepositoryNameSet> &,
                    const std::shared_ptr<const QualifiedPackageNameSet> &) const;

            virtual std::shared_ptr<const PackageIDSet> ids(
                    const Environment * const,
                    const std::shared_ptr<const PackageIDSet> &) const;
    };
}

#endif
