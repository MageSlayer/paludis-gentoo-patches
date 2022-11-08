/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2010, 2011 Ciaran McCreesh
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

#include <paludis/generator_handler.hh>
#include <paludis/name.hh>
#include <paludis/environment.hh>
#include <paludis/repository.hh>

#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/wrapped_forward_iterator.hh>

#include <functional>
#include <algorithm>

using namespace paludis;

GeneratorHandler::~GeneratorHandler() = default;

std::shared_ptr<const RepositoryNameSet>
AllGeneratorHandlerBase::repositories(
        const Environment * const env,
        const RepositoryContentMayExcludes &) const
{
    using namespace std::placeholders;
    std::shared_ptr<RepositoryNameSet> result(std::make_shared<RepositoryNameSet>());
    std::transform(env->begin_repositories(), env->end_repositories(),
            result->inserter(), std::bind(&Repository::name, _1));
    return result;
}

std::shared_ptr<const CategoryNamePartSet>
AllGeneratorHandlerBase::categories(
        const Environment * const env,
        const std::shared_ptr<const RepositoryNameSet> & repos,
        const RepositoryContentMayExcludes & may_exclude) const
{
    std::shared_ptr<CategoryNamePartSet> result(std::make_shared<CategoryNamePartSet>());

    for (const auto & r : *repos)
    {
        std::shared_ptr<const CategoryNamePartSet> cats(env->fetch_repository(r)->category_names(may_exclude));
        std::copy(cats->begin(), cats->end(), result->inserter());
    }

    return result;
}

std::shared_ptr<const QualifiedPackageNameSet>
AllGeneratorHandlerBase::packages(
        const Environment * const env,
        const std::shared_ptr<const RepositoryNameSet> & repos,
        const std::shared_ptr<const CategoryNamePartSet> & cats,
        const RepositoryContentMayExcludes & may_exclude) const
{
    std::shared_ptr<QualifiedPackageNameSet> result(std::make_shared<QualifiedPackageNameSet>());

    for (const auto & r : *repos)
    {
        for (const auto & c : *cats)
        {
            std::shared_ptr<const QualifiedPackageNameSet> pkgs(
                    env->fetch_repository(r)->package_names(c, may_exclude));
            std::copy(pkgs->begin(), pkgs->end(), result->inserter());
        }
    }

    return result;
}

std::shared_ptr<const PackageIDSet>
AllGeneratorHandlerBase::ids(
        const Environment * const env,
        const std::shared_ptr<const RepositoryNameSet> & repos,
        const std::shared_ptr<const QualifiedPackageNameSet> & qpns,
        const RepositoryContentMayExcludes & may_exclude) const
{
    std::shared_ptr<PackageIDSet> result(std::make_shared<PackageIDSet>());

    for (const auto & r : *repos)
    {
        for (const auto & q : *qpns)
        {
            std::shared_ptr<const PackageIDSequence> i(env->fetch_repository(r)->package_ids(q, may_exclude));
            std::copy(i->begin(), i->end(), result->inserter());
        }
    }

    return result;
}

