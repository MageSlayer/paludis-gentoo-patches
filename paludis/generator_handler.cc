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
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/name.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <functional>
#include <algorithm>

using namespace paludis;

GeneratorHandler::~GeneratorHandler()
{
}

std::shared_ptr<const RepositoryNameSet>
AllGeneratorHandlerBase::repositories(
        const Environment * const env) const
{
    using namespace std::placeholders;
    std::shared_ptr<RepositoryNameSet> result(std::make_shared<RepositoryNameSet>());
    std::transform(env->package_database()->begin_repositories(), env->package_database()->end_repositories(),
            result->inserter(), std::bind(&Repository::name, _1));
    return result;
}

std::shared_ptr<const CategoryNamePartSet>
AllGeneratorHandlerBase::categories(
        const Environment * const env,
        const std::shared_ptr<const RepositoryNameSet> & repos) const
{
    std::shared_ptr<CategoryNamePartSet> result(std::make_shared<CategoryNamePartSet>());

    for (RepositoryNameSet::ConstIterator r(repos->begin()), r_end(repos->end()) ;
            r != r_end ; ++r)
    {
        std::shared_ptr<const CategoryNamePartSet> cats(env->package_database()->fetch_repository(*r)->category_names({ }));
        std::copy(cats->begin(), cats->end(), result->inserter());
    }

    return result;
}

std::shared_ptr<const QualifiedPackageNameSet>
AllGeneratorHandlerBase::packages(
        const Environment * const env,
        const std::shared_ptr<const RepositoryNameSet> & repos,
        const std::shared_ptr<const CategoryNamePartSet> & cats) const
{
    std::shared_ptr<QualifiedPackageNameSet> result(std::make_shared<QualifiedPackageNameSet>());

    for (RepositoryNameSet::ConstIterator r(repos->begin()), r_end(repos->end()) ;
            r != r_end ; ++r)
    {
        for (CategoryNamePartSet::ConstIterator c(cats->begin()), c_end(cats->end()) ;
                c != c_end ; ++c)
        {
            std::shared_ptr<const QualifiedPackageNameSet> pkgs(
                    env->package_database()->fetch_repository(*r)->package_names(*c, { }));
            std::copy(pkgs->begin(), pkgs->end(), result->inserter());
        }
    }

    return result;
}

std::shared_ptr<const PackageIDSet>
AllGeneratorHandlerBase::ids(
        const Environment * const env,
        const std::shared_ptr<const RepositoryNameSet> & repos,
        const std::shared_ptr<const QualifiedPackageNameSet> & qpns) const
{
    std::shared_ptr<PackageIDSet> result(std::make_shared<PackageIDSet>());

    for (RepositoryNameSet::ConstIterator r(repos->begin()), r_end(repos->end()) ;
            r != r_end ; ++r)
    {
        for (QualifiedPackageNameSet::ConstIterator q(qpns->begin()), q_end(qpns->end()) ;
                q != q_end ; ++q)
        {
            std::shared_ptr<const PackageIDSequence> i(
                    env->package_database()->fetch_repository(*r)->package_ids(*q, { }));
            std::copy(i->begin(), i->end(), result->inserter());
        }
    }

    return result;
}

