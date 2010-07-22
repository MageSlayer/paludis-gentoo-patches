/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010 Ciaran McCreesh
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

#include <paludis/generator.hh>
#include <paludis/generator_handler.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/action_names.hh>
#include <paludis/action.hh>
#include <paludis/match_package.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/package_dep_spec_properties.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <algorithm>
#include <functional>

using namespace paludis;

namespace paludis
{
    template <>
    struct Implementation<Generator>
    {
        std::shared_ptr<const GeneratorHandler> handler;

        Implementation(const std::shared_ptr<const GeneratorHandler> & h) :
            handler(h)
        {
        }
    };
}

Generator::Generator(const std::shared_ptr<const GeneratorHandler> & h) :
    PrivateImplementationPattern<Generator>(new Implementation<Generator>(h))
{
}

Generator::Generator(const Generator & other) :
    PrivateImplementationPattern<Generator>(new Implementation<Generator>(other._imp->handler))
{
}

Generator &
Generator::operator= (const Generator & other)
{
    if (this != &other)
        _imp->handler = other._imp->handler;
    return *this;
}

Generator::~Generator()
{
}

Generator::operator FilteredGenerator () const
{
    return FilteredGenerator(*this, filter::All());
}

std::shared_ptr<const RepositoryNameSet>
Generator::repositories(
        const Environment * const env) const
{
    return _imp->handler->repositories(env);
}

std::shared_ptr<const CategoryNamePartSet>
Generator::categories(
        const Environment * const env,
        const std::shared_ptr<const RepositoryNameSet> & r) const
{
    return _imp->handler->categories(env, r);
}

std::shared_ptr<const QualifiedPackageNameSet>
Generator::packages(
        const Environment * const env,
        const std::shared_ptr<const RepositoryNameSet> & r,
        const std::shared_ptr<const CategoryNamePartSet> & c) const
{
    return _imp->handler->packages(env, r, c);
}

std::shared_ptr<const PackageIDSet>
Generator::ids(
        const Environment * const env,
        const std::shared_ptr<const RepositoryNameSet> & r,
        const std::shared_ptr<const QualifiedPackageNameSet> & q) const
{
    return _imp->handler->ids(env, r, q);
}

std::string
Generator::as_string() const
{
    return _imp->handler->as_string();
}

namespace
{
    struct InRepositoryGeneratorHandler :
        AllGeneratorHandlerBase
    {
        const RepositoryName name;

        InRepositoryGeneratorHandler(const RepositoryName & n) :
            name(n)
        {
        }

        virtual std::shared_ptr<const RepositoryNameSet> repositories(
                const Environment * const env) const
        {
            using namespace std::placeholders;
            std::shared_ptr<RepositoryNameSet> result(new RepositoryNameSet);
            if (env->package_database()->has_repository_named(name))
                result->insert(name);
            return result;
        }

        virtual std::string as_string() const
        {
            return "packages with repository " + stringify(name);
        }
    };

    struct FromRepositoryGeneratorHandler :
        AllGeneratorHandlerBase
    {
        const RepositoryName name;

        FromRepositoryGeneratorHandler(const RepositoryName & n) :
            name(n)
        {
        }

        virtual std::shared_ptr<const PackageIDSet> ids(
                const Environment * const env,
                const std::shared_ptr<const RepositoryNameSet> & repos,
                const std::shared_ptr<const QualifiedPackageNameSet> & qpns) const
        {
            std::shared_ptr<PackageIDSet> result(new PackageIDSet);

            for (RepositoryNameSet::ConstIterator r(repos->begin()), r_end(repos->end()) ;
                    r != r_end ; ++r)
            {
                for (QualifiedPackageNameSet::ConstIterator q(qpns->begin()), q_end(qpns->end()) ;
                        q != q_end ; ++q)
                {
                    std::shared_ptr<const PackageIDSequence> id(
                            env->package_database()->fetch_repository(*r)->package_ids(*q));
                    for (PackageIDSequence::ConstIterator i(id->begin()), i_end(id->end()) ;
                            i != i_end ; ++i)
                        if ((*i)->from_repositories_key() && ((*i)->from_repositories_key()->value()->end() !=
                                    (*i)->from_repositories_key()->value()->find(stringify(name))))
                            result->insert(*i);
                }
            }

            return result;
        }

        virtual std::string as_string() const
        {
            return "packages originally from repository " + stringify(name);
        }
    };

    struct CategoryGeneratorHandler :
        AllGeneratorHandlerBase
    {
        const CategoryNamePart name;

        CategoryGeneratorHandler(const CategoryNamePart & n) :
            name(n)
        {
        }

        virtual std::shared_ptr<const CategoryNamePartSet> categories(
                const Environment * const env,
                const std::shared_ptr<const RepositoryNameSet> & repos
                ) const
        {
            std::shared_ptr<CategoryNamePartSet> result(new CategoryNamePartSet);

            for (RepositoryNameSet::ConstIterator r(repos->begin()), r_end(repos->end()) ;
                    r != r_end ; ++r)
                if (env->package_database()->fetch_repository(*r)->has_category_named(name))
                {
                    result->insert(name);
                    break;
                }

            return result;
        }

        virtual std::string as_string() const
        {
            return "packages with category " + stringify(name);
        }
    };

    struct PackageGeneratorHandler :
        AllGeneratorHandlerBase
    {
        const QualifiedPackageName name;

        PackageGeneratorHandler(const QualifiedPackageName & n) :
            name(n)
        {
        }

        virtual std::shared_ptr<const CategoryNamePartSet> categories(
                const Environment * const env,
                const std::shared_ptr<const RepositoryNameSet> & repos
                ) const
        {
            std::shared_ptr<CategoryNamePartSet> result(new CategoryNamePartSet);

            for (RepositoryNameSet::ConstIterator r(repos->begin()), r_end(repos->end()) ;
                    r != r_end ; ++r)
                if (env->package_database()->fetch_repository(*r)->has_category_named(name.category()))
                {
                    result->insert(name.category());
                    break;
                }

            return result;
        }

        virtual std::shared_ptr<const QualifiedPackageNameSet> packages(
                const Environment * const env,
                const std::shared_ptr<const RepositoryNameSet> & repos,
                const std::shared_ptr<const CategoryNamePartSet> & cats) const
        {
            std::shared_ptr<QualifiedPackageNameSet> result(new QualifiedPackageNameSet);
            for (RepositoryNameSet::ConstIterator r(repos->begin()), r_end(repos->end()) ;
                    r != r_end ; ++r)
                for (CategoryNamePartSet::ConstIterator c(cats->begin()), c_end(cats->end()) ;
                        c != c_end ; ++c)
                    if (env->package_database()->fetch_repository(*r)->has_package_named(name))
                        result->insert(name);

            return result;
        }

        virtual std::string as_string() const
        {
            return "packages named " + stringify(name);
        }
    };

    struct MatchesGeneratorHandler :
        AllGeneratorHandlerBase
    {
        const PackageDepSpec spec;
        const MatchPackageOptions options;

        MatchesGeneratorHandler(const PackageDepSpec & s, const MatchPackageOptions & o) :
            spec(s),
            options(o)
        {
        }

        virtual std::shared_ptr<const RepositoryNameSet> repositories(
                const Environment * const env) const
        {
            if (package_dep_spec_has_properties(spec, make_named_values<PackageDepSpecProperties>(
                            n::has_additional_requirements() = indeterminate,
                            n::has_category_name_part() = indeterminate,
                            n::has_from_repository() = indeterminate,
                            n::has_in_repository() = false,
                            n::has_installable_to_path() = indeterminate,
                            n::has_installable_to_repository() = indeterminate,
                            n::has_installed_at_path() = false,
                            n::has_package() = indeterminate,
                            n::has_package_name_part() = indeterminate,
                            n::has_slot_requirement() = indeterminate,
                            n::has_tag() = indeterminate,
                            n::has_version_requirements() = indeterminate
                            )))
                return AllGeneratorHandlerBase::repositories(env);

            std::shared_ptr<RepositoryNameSet> result(new RepositoryNameSet);

            if (spec.in_repository_ptr())
            {
                if (env->package_database()->has_repository_named(*spec.in_repository_ptr()))
                {
                    if (spec.installed_at_path_ptr())
                    {
                        std::shared_ptr<const Repository> repo(env->package_database()->fetch_repository(
                                    *spec.in_repository_ptr()));
                        if (! repo->installed_root_key())
                            return result;
                        if (repo->installed_root_key()->value() != *spec.installed_at_path_ptr())
                            return result;
                    }

                    result->insert(*spec.in_repository_ptr());
                }
            }
            else
            {
                if (spec.installed_at_path_ptr())
                {
                    for (PackageDatabase::RepositoryConstIterator i(env->package_database()->begin_repositories()),
                            i_end(env->package_database()->end_repositories()) ; i != i_end ; ++i)
                    {
                        if (! (*i)->installed_root_key())
                            continue;

                        if ((*i)->installed_root_key()->value() != *spec.installed_at_path_ptr())
                            continue;

                        result->insert((*i)->name());
                    }
                }
                else
                    return AllGeneratorHandlerBase::repositories(env);
            }

            return result;
        }

        virtual std::shared_ptr<const CategoryNamePartSet> categories(
                const Environment * const env,
                const std::shared_ptr<const RepositoryNameSet> & repos) const
        {
            if (spec.category_name_part_ptr())
            {
                std::shared_ptr<CategoryNamePartSet> result(new CategoryNamePartSet);
                for (RepositoryNameSet::ConstIterator r(repos->begin()), r_end(repos->end()) ;
                        r != r_end ; ++r)
                    if (env->package_database()->fetch_repository(*r)->has_category_named(*spec.category_name_part_ptr()))
                    {
                        result->insert(*spec.category_name_part_ptr());
                        break;
                    }

                return result;
            }
            else if (spec.package_name_part_ptr())
            {
                std::shared_ptr<CategoryNamePartSet> result(new CategoryNamePartSet);
                for (RepositoryNameSet::ConstIterator r(repos->begin()), r_end(repos->end()) ;
                        r != r_end ; ++r)
                {
                    std::shared_ptr<const CategoryNamePartSet> cats(
                        env->package_database()->fetch_repository(*r)
                        ->category_names_containing_package(*spec.package_name_part_ptr()));
                    std::copy(cats->begin(), cats->end(), result->inserter());
                }

                return result;
            }
            else if (spec.package_ptr())
            {
                std::shared_ptr<CategoryNamePartSet> result(new CategoryNamePartSet);
                for (RepositoryNameSet::ConstIterator r(repos->begin()), r_end(repos->end()) ;
                        r != r_end ; ++r)
                    if (env->package_database()->fetch_repository(*r)->has_category_named(spec.package_ptr()->category()))
                    {
                        result->insert(spec.package_ptr()->category());
                        break;
                    }

                return result;
            }
            else
                return AllGeneratorHandlerBase::categories(env, repos);
        }

        virtual std::shared_ptr<const QualifiedPackageNameSet> packages(
                const Environment * const env,
                const std::shared_ptr<const RepositoryNameSet> & repos,
                const std::shared_ptr<const CategoryNamePartSet> & cats) const
        {
            if (spec.package_name_part_ptr())
            {
                std::shared_ptr<QualifiedPackageNameSet> result(new QualifiedPackageNameSet);
                for (RepositoryNameSet::ConstIterator r(repos->begin()), r_end(repos->end()) ;
                        r != r_end ; ++r)
                    for (CategoryNamePartSet::ConstIterator c(cats->begin()), c_end(cats->end()) ;
                            c != c_end ; ++c)
                        if (env->package_database()->fetch_repository(*r)->has_package_named(*c + *spec.package_name_part_ptr()))
                            result->insert(*c + *spec.package_name_part_ptr());

                return result;
            }
            else if (spec.package_ptr())
            {
                std::shared_ptr<QualifiedPackageNameSet> result(new QualifiedPackageNameSet);
                for (RepositoryNameSet::ConstIterator r(repos->begin()), r_end(repos->end()) ;
                        r != r_end ; ++r)
                    if (env->package_database()->fetch_repository(*r)->has_package_named(*spec.package_ptr()))
                    {
                        result->insert(*spec.package_ptr());
                        break;
                    }

                return result;
            }
            else
                return AllGeneratorHandlerBase::packages(env, repos, cats);
        }

        virtual std::shared_ptr<const PackageIDSet> ids(
                const Environment * const env,
                const std::shared_ptr<const RepositoryNameSet> & repos,
                const std::shared_ptr<const QualifiedPackageNameSet> & qpns) const
        {
            std::shared_ptr<PackageIDSet> result(new PackageIDSet);

            for (RepositoryNameSet::ConstIterator r(repos->begin()), r_end(repos->end()) ;
                    r != r_end ; ++r)
            {
                for (QualifiedPackageNameSet::ConstIterator q(qpns->begin()), q_end(qpns->end()) ;
                        q != q_end ; ++q)
                {
                    std::shared_ptr<const PackageIDSequence> id(
                            env->package_database()->fetch_repository(*r)->package_ids(*q));
                    for (PackageIDSequence::ConstIterator i(id->begin()), i_end(id->end()) ;
                            i != i_end ; ++i)
                        if (match_package(*env, spec, **i, options))
                            result->insert(*i);
                }
            }

            return result;
        }

        virtual std::string as_string() const
        {
            std::string suffix;
            if (options[mpo_ignore_additional_requirements])
                suffix = " (ignoring additional requirements)";
            return "packages matching " + stringify(spec) + suffix;
        }
    };

    struct IntersectionGeneratorHandler :
        GeneratorHandler
    {
        const Generator g1;
        const Generator g2;

        IntersectionGeneratorHandler(const Generator & h1, const Generator & h2) :
            g1(h1),
            g2(h2)
        {
        }

        virtual std::shared_ptr<const RepositoryNameSet> repositories(
                const Environment * const env) const
        {
            std::shared_ptr<const RepositoryNameSet> r1(g1.repositories(env));
            if (r1->empty())
                return r1;

            std::shared_ptr<const RepositoryNameSet> r2(g2.repositories(env));
            if (r2->empty())
                return r2;

            std::shared_ptr<RepositoryNameSet> result(new RepositoryNameSet);
            std::set_intersection(
                    r1->begin(), r1->end(),
                    r2->begin(), r2->end(),
                    result->inserter());
            return result;
        }

        virtual std::shared_ptr<const CategoryNamePartSet> categories(
                const Environment * const env,
                const std::shared_ptr<const RepositoryNameSet> & repos) const
        {
            std::shared_ptr<const CategoryNamePartSet> c1(g1.categories(env, repos));
            if (c1->empty())
                return c1;

            std::shared_ptr<const CategoryNamePartSet> c2(g2.categories(env, repos));
            if (c2->empty())
                return c2;

            std::shared_ptr<CategoryNamePartSet> result(new CategoryNamePartSet);
            std::set_intersection(
                    c1->begin(), c1->end(),
                    c2->begin(), c2->end(),
                    result->inserter());
            return result;
        }

        virtual std::shared_ptr<const QualifiedPackageNameSet> packages(
                const Environment * const env,
                const std::shared_ptr<const RepositoryNameSet> & repos,
                const std::shared_ptr<const CategoryNamePartSet> & cats) const
        {
            std::shared_ptr<const QualifiedPackageNameSet> q1(g1.packages(env, repos, cats));
            if (q1->empty())
                return q1;

            std::shared_ptr<const QualifiedPackageNameSet> q2(g2.packages(env, repos, cats));
            if (q2->empty())
                return q2;

            std::shared_ptr<QualifiedPackageNameSet> result(new QualifiedPackageNameSet);
            std::set_intersection(
                    q1->begin(), q1->end(),
                    q2->begin(), q2->end(),
                    result->inserter());
            return result;
        }

        virtual std::shared_ptr<const PackageIDSet> ids(
                const Environment * const env,
                const std::shared_ptr<const RepositoryNameSet> & repos,
                const std::shared_ptr<const QualifiedPackageNameSet> & qpns) const
        {
            std::shared_ptr<const PackageIDSet> i1(g1.ids(env, repos, qpns));
            if (i1->empty())
                return i1;

            std::shared_ptr<const PackageIDSet> i2(g2.ids(env, repos, qpns));
            if (i2->empty())
                return i2;

            std::shared_ptr<PackageIDSet> result(new PackageIDSet);
            std::set_intersection(
                    i1->begin(), i1->end(),
                    i2->begin(), i2->end(),
                    result->inserter(),
                    PackageIDSetComparator());
            return result;
        }

        virtual std::string as_string() const
        {
            return stringify(g1) + " intersected with " + stringify(g2);
        }
    };

    struct UnionGeneratorHandler :
        GeneratorHandler
    {
        const Generator g1;
        const Generator g2;

        UnionGeneratorHandler(const Generator & h1, const Generator & h2) :
            g1(h1),
            g2(h2)
        {
        }

        virtual std::shared_ptr<const RepositoryNameSet> repositories(
                const Environment * const env) const
        {
            std::shared_ptr<const RepositoryNameSet> r1(g1.repositories(env));
            std::shared_ptr<const RepositoryNameSet> r2(g2.repositories(env));
            std::shared_ptr<RepositoryNameSet> result(new RepositoryNameSet);
            std::set_union(
                    r1->begin(), r1->end(),
                    r2->begin(), r2->end(),
                    result->inserter());
            return result;
        }

        virtual std::shared_ptr<const CategoryNamePartSet> categories(
                const Environment * const env,
                const std::shared_ptr<const RepositoryNameSet> & repos) const
        {
            std::shared_ptr<const CategoryNamePartSet> c1(g1.categories(env, repos));
            std::shared_ptr<const CategoryNamePartSet> c2(g2.categories(env, repos));
            std::shared_ptr<CategoryNamePartSet> result(new CategoryNamePartSet);
            std::set_union(
                    c1->begin(), c1->end(),
                    c2->begin(), c2->end(),
                    result->inserter());
            return result;
        }

        virtual std::shared_ptr<const QualifiedPackageNameSet> packages(
                const Environment * const env,
                const std::shared_ptr<const RepositoryNameSet> & repos,
                const std::shared_ptr<const CategoryNamePartSet> & cats) const
        {
            std::shared_ptr<const QualifiedPackageNameSet> q1(g1.packages(env, repos, cats));
            std::shared_ptr<const QualifiedPackageNameSet> q2(g2.packages(env, repos, cats));
            std::shared_ptr<QualifiedPackageNameSet> result(new QualifiedPackageNameSet);
            std::set_union(
                    q1->begin(), q1->end(),
                    q2->begin(), q2->end(),
                    result->inserter());
            return result;
        }

        virtual std::shared_ptr<const PackageIDSet> ids(
                const Environment * const env,
                const std::shared_ptr<const RepositoryNameSet> & repos,
                const std::shared_ptr<const QualifiedPackageNameSet> & qpns) const
        {
            std::shared_ptr<const PackageIDSet> i1(g1.ids(env, repos, qpns));
            std::shared_ptr<const PackageIDSet> i2(g2.ids(env, repos, qpns));
            std::shared_ptr<PackageIDSet> result(new PackageIDSet);
            std::set_union(
                    i1->begin(), i1->end(),
                    i2->begin(), i2->end(),
                    result->inserter(),
                    PackageIDSetComparator());
            return result;
        }

        virtual std::string as_string() const
        {
            return stringify(g1) + " unioned with " + stringify(g2);
        }
    };

    struct AllGeneratorHandler :
        AllGeneratorHandlerBase
    {
        virtual std::string as_string() const
        {
            return "all packages";
        }
    };

    template <typename A_>
    struct SomeIDsMightSupportActionGeneratorHandler :
        AllGeneratorHandlerBase
    {
        virtual std::shared_ptr<const RepositoryNameSet> repositories(
                const Environment * const env) const
        {
            std::shared_ptr<RepositoryNameSet> result(new RepositoryNameSet);
            for (PackageDatabase::RepositoryConstIterator i(env->package_database()->begin_repositories()),
                    i_end(env->package_database()->end_repositories()) ; i != i_end ; ++i)
                if ((*i)->some_ids_might_support_action(SupportsActionTest<A_>()))
                    result->insert((*i)->name());

            return result;
        }

        virtual std::string as_string() const
        {
            return "packages that might support action " + stringify(ActionNames<A_>::value);
        }
    };

    struct NothingGeneratorHandler :
        GeneratorHandler
    {
        virtual std::shared_ptr<const RepositoryNameSet> repositories(
                const Environment * const) const
        {
            return std::make_shared<RepositoryNameSet>();
        }

        virtual std::shared_ptr<const CategoryNamePartSet> categories(
                const Environment * const,
                const std::shared_ptr<const RepositoryNameSet> &) const
        {
            return std::make_shared<CategoryNamePartSet>();
        }

        virtual std::shared_ptr<const QualifiedPackageNameSet> packages(
                const Environment * const,
                const std::shared_ptr<const RepositoryNameSet> &,
                const std::shared_ptr<const CategoryNamePartSet> &) const
        {
            return std::make_shared<QualifiedPackageNameSet>();
        }

        virtual std::shared_ptr<const PackageIDSet> ids(
                const Environment * const,
                const std::shared_ptr<const RepositoryNameSet> &,
                const std::shared_ptr<const QualifiedPackageNameSet> &) const
        {
            return std::make_shared<PackageIDSet>();
        }

        virtual std::string as_string() const
        {
            return "no packages";
        }
    };
}

generator::All::All() :
    Generator(std::make_shared<AllGeneratorHandler>())
{
}

generator::InRepository::InRepository(const RepositoryName & n) :
    Generator(std::make_shared<InRepositoryGeneratorHandler>(n))
{
}

generator::FromRepository::FromRepository(const RepositoryName & n) :
    Generator(std::make_shared<FromRepositoryGeneratorHandler>(n))
{
}

generator::Category::Category(const CategoryNamePart & n) :
    Generator(std::make_shared<CategoryGeneratorHandler>(n))
{
}

generator::Package::Package(const QualifiedPackageName & n) :
    Generator(std::make_shared<PackageGeneratorHandler>(n))
{
}

generator::Matches::Matches(const PackageDepSpec & spec, const MatchPackageOptions & o) :
    Generator(std::make_shared<MatchesGeneratorHandler>(spec, o))
{
}

generator::Intersection::Intersection(const Generator & g1, const Generator & g2) :
    Generator(std::make_shared<IntersectionGeneratorHandler>(g1, g2))
{
}

generator::Union::Union(const Generator & g1, const Generator & g2) :
    Generator(std::make_shared<UnionGeneratorHandler>(g1, g2))
{
}

generator::Nothing::Nothing() :
    Generator(std::make_shared<NothingGeneratorHandler>())
{
}

template <typename A_>
generator::SomeIDsMightSupportAction<A_>::SomeIDsMightSupportAction() :
    Generator(std::make_shared<SomeIDsMightSupportActionGeneratorHandler<A_>>())
{
}

Generator
paludis::operator& (const Generator & g1, const Generator & g2)
{
    return generator::Intersection(g1, g2);
}

Generator
paludis::operator+ (const Generator & g1, const Generator & g2)
{
    return generator::Union(g1, g2);
}

std::ostream &
paludis::operator<< (std::ostream & s, const Generator & g)
{
    s << g.as_string();
    return s;
}

template class PrivateImplementationPattern<Generator>;
template class generator::SomeIDsMightSupportAction<InstallAction>;
template class generator::SomeIDsMightSupportAction<UninstallAction>;
template class generator::SomeIDsMightSupportAction<PretendAction>;
template class generator::SomeIDsMightSupportAction<ConfigAction>;
template class generator::SomeIDsMightSupportAction<FetchAction>;
template class generator::SomeIDsMightSupportAction<InfoAction>;
template class generator::SomeIDsMightSupportAction<PretendFetchAction>;

