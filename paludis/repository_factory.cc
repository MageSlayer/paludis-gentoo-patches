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

#include <paludis/repository_factory.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/instantiation_policy-impl.hh>
#include <paludis/util/member_iterator-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/join.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/set-impl.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/system.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/name.hh>
#include <paludis/about.hh>
#include <tr1/unordered_map>
#include <list>
#include "config.h"

using namespace paludis;

namespace paludis
{
    namespace n
    {
        struct create_function;
        struct dependencies_function;
        struct importance_function;
        struct name_function;
    }
}

namespace
{
    struct Funcs
    {
        NamedValue<n::create_function, RepositoryFactory::CreateFunction> create_function;
        NamedValue<n::dependencies_function, RepositoryFactory::DependenciesFunction> dependencies_function;
        NamedValue<n::importance_function, RepositoryFactory::ImportanceFunction> importance_function;
        NamedValue<n::name_function, RepositoryFactory::NameFunction> name_function;
    };

    typedef std::tr1::unordered_map<std::string, Funcs> Keys;

    const Funcs & fetch(const Keys & keys, const std::string & key)
    {
        if (key.empty())
            throw ConfigurationError("Key 'format' not specified when creating a repository");

        Keys::const_iterator i(keys.find(key));
        if (i == keys.end())
            throw ConfigurationError("Format '" + key + "' not supported when creating a repository (known formats are { "
                    + join(first_iterator(keys.begin()), first_iterator(keys.end()), ", ") + "})");

        return i->second;
    }
}

namespace paludis
{
    template <>
    struct Implementation<RepositoryFactory>
    {
        Keys keys;
    };

    namespace repository_groups
    {
        REPOSITORY_GROUPS_DECLS;
    }

    template <>
    void register_repositories<NoType<0u> >(const NoType<0u> *, RepositoryFactory * const)
    {
    }

    template <>
    struct WrappedForwardIteratorTraits<RepositoryFactory::ConstIteratorTag>
    {
        typedef FirstIteratorTypes<Keys::const_iterator>::Type UnderlyingIterator;
    };
}

namespace
{
    /**
     * Alas, fefault template types for functions only works with 0x.
     */
    template <typename T_ = NoType<0u> >
    struct TypeOrNoType
    {
        typedef T_ Type;
    };
}

RepositoryFactory::RepositoryFactory() :
    PrivateImplementationPattern<RepositoryFactory>(new Implementation<RepositoryFactory>)
{
    using namespace repository_groups;

    register_repositories(static_cast<const TypeOrNoType<REPOSITORY_GROUP_IF_accounts>::Type *>(0), this);
    register_repositories(static_cast<const TypeOrNoType<REPOSITORY_GROUP_IF_cran>::Type *>(0), this);
    register_repositories(static_cast<const TypeOrNoType<REPOSITORY_GROUP_IF_dummy>::Type *>(0), this);
    register_repositories(static_cast<const TypeOrNoType<REPOSITORY_GROUP_IF_e>::Type *>(0), this);
    register_repositories(static_cast<const TypeOrNoType<REPOSITORY_GROUP_IF_gems>::Type *>(0), this);
    register_repositories(static_cast<const TypeOrNoType<REPOSITORY_GROUP_IF_fake>::Type *>(0), this);
    register_repositories(static_cast<const TypeOrNoType<REPOSITORY_GROUP_IF_repository>::Type *>(0), this);
    register_repositories(static_cast<const TypeOrNoType<REPOSITORY_GROUP_IF_unavailable>::Type *>(0), this);
    register_repositories(static_cast<const TypeOrNoType<REPOSITORY_GROUP_IF_unwritten>::Type *>(0), this);
    register_repositories(static_cast<const TypeOrNoType<REPOSITORY_GROUP_IF_unpackaged>::Type *>(0), this);
    register_repositories(static_cast<const TypeOrNoType<REPOSITORY_GROUP_IF_virtuals>::Type *>(0), this);
}

RepositoryFactory::~RepositoryFactory()
{
}

const std::tr1::shared_ptr<Repository>
RepositoryFactory::create(
        Environment * const env,
        const KeyFunction & key_function
        ) const
{
    Context context("When creating repository" + (key_function("repo_file").empty() ? ":" :
                " from file '" + key_function("repo_file") + ":"));
    return fetch(_imp->keys, key_function("format")).create_function()(env, key_function);
}

const std::tr1::shared_ptr<const RepositoryNameSet>
RepositoryFactory::dependencies(
        const Environment * const env,
        const KeyFunction & key_function
        ) const
{
    Context context("When working out dependencies for repository" + (key_function("repo_file").empty() ? ":" :
                " from file '" + key_function("repo_file") + ":"));
    return fetch(_imp->keys, key_function("format")).dependencies_function()(env, key_function);
}

const RepositoryName
RepositoryFactory::name(
        const Environment * const env,
        const KeyFunction & key_function
        ) const
{
    Context context("When working out name for repository" + (key_function("repo_file").empty() ? ":" :
                " from file '" + key_function("repo_file") + ":"));
    return fetch(_imp->keys, key_function("format")).name_function()(env, key_function);
}

int
RepositoryFactory::importance(
        const Environment * const env,
        const KeyFunction & key_function
        ) const
{
    Context context("When working out importance for repository" + (key_function("repo_file").empty() ? ":" :
                " from file '" + key_function("repo_file") + ":"));
    return fetch(_imp->keys, key_function("format")).importance_function()(env, key_function);
}

RepositoryFactory::ConstIterator
RepositoryFactory::begin_keys() const
{
    return ConstIterator(first_iterator(_imp->keys.begin()));
}

RepositoryFactory::ConstIterator
RepositoryFactory::end_keys() const
{
    return ConstIterator(first_iterator(_imp->keys.end()));
}

void
RepositoryFactory::add_repository_format(
        const std::tr1::shared_ptr<const Set<std::string> > & formats,
        const NameFunction & name_function,
        const ImportanceFunction & importance_function,
        const CreateFunction & create_function,
        const DependenciesFunction & dependencies_function
        )
{
    for (Set<std::string>::ConstIterator f(formats->begin()), f_end(formats->end()) ;
            f != f_end ; ++f)
    {
        if (! _imp->keys.insert(std::make_pair(*f, make_named_values<Funcs>(
                            value_for<n::create_function>(create_function),
                            value_for<n::dependencies_function>(dependencies_function),
                            value_for<n::importance_function>(importance_function),
                            value_for<n::name_function>(name_function)
                            ))).second)
            throw ConfigurationError("Handler for repository format '" + stringify(*f) + "' already exists");
    }
}

template class PrivateImplementationPattern<RepositoryFactory>;
template class InstantiationPolicy<RepositoryFactory, instantiation_method::SingletonTag>;
template class WrappedForwardIterator<RepositoryFactory::ConstIteratorTag, const std::string>;

