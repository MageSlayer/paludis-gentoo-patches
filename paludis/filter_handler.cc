/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
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

#include <paludis/filter_handler.hh>
#include <paludis/util/options.hh>

using namespace paludis;

FilterHandler::~FilterHandler()
{
}

std::shared_ptr<const RepositoryNameSet>
AllFilterHandlerBase::repositories(const Environment * const,
        const std::shared_ptr<const RepositoryNameSet> & s) const
{
    return s;
}

std::shared_ptr<const CategoryNamePartSet>
AllFilterHandlerBase::categories(const Environment * const,
        const std::shared_ptr<const RepositoryNameSet> &,
        const std::shared_ptr<const CategoryNamePartSet> & s) const
{
    return s;
}

std::shared_ptr<const QualifiedPackageNameSet>
AllFilterHandlerBase::packages(const Environment * const,
        const std::shared_ptr<const RepositoryNameSet> &,
        const std::shared_ptr<const QualifiedPackageNameSet> & s) const
{
    return s;
}

std::shared_ptr<const PackageIDSet>
AllFilterHandlerBase::ids(const Environment * const,
        const std::shared_ptr<const PackageIDSet> & s) const
{
    return s;
}

const RepositoryContentMayExcludes
AllFilterHandlerBase::may_excludes() const
{
    return { };
}

