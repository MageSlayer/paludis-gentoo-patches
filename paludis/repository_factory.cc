/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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
#include <paludis/util/set.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/system.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/name.hh>
#include <paludis/about.hh>
#include <tr1/unordered_map>
#include <list>
#include <dlfcn.h>
#include <stdint.h>

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
        std::list<void *> dl_opened;
    };
}

RepositoryFactory::RepositoryFactory() :
    PrivateImplementationPattern<RepositoryFactory>(new Implementation<RepositoryFactory>)
{
    FSEntry so_dir(getenv_with_default("PALUDIS_REPOSITORY_SO_DIR", LIBDIR "/paludis/repositories"));
    if (! so_dir.is_directory())
        throw InternalError(PALUDIS_HERE, "PALUDIS_REPOSITORY_SO_DIR '" + stringify(so_dir) + "' not a directory");
    _load_dir(so_dir);
}

RepositoryFactory::~RepositoryFactory()
{
}

void
RepositoryFactory::_load_dir(const FSEntry & so_dir)
{
    for (DirIterator d(so_dir), d_end ; d != d_end ; ++d)
    {
        if (d->is_directory())
            _load_dir(*d);

        if (! is_file_with_extension(*d, "_" + stringify(PALUDIS_PC_SLOT) + ".so." + stringify(100 * PALUDIS_VERSION_MAJOR + PALUDIS_VERSION_MINOR),
                    IsFileWithOptions()))
            continue;

        /* don't use RTLD_LOCAL, g++ is over happy about template instantiations, and it
         * can lead to multiple singleton instances. */
        void * dl(dlopen(stringify(*d).c_str(), RTLD_GLOBAL | RTLD_NOW));

        if (dl)
        {
            _imp->dl_opened.push_back(dl);

            void * reg(dlsym(dl, "paludis_initialise_repository_so"));
            if (reg)
            {
                reinterpret_cast<void (*)(RepositoryFactory * const)>(reinterpret_cast<uintptr_t>(reg))(this);
            }
            else
                throw InternalError(PALUDIS_HERE, "No paludis_initialise_repository_so function defined in '" + stringify(*d) + "'");
        }
        else
            throw InternalError(PALUDIS_HERE, "Couldn't dlopen '" + stringify(*d) + "': " + stringify(dlerror()));
    }

    if ((so_dir / ".libs").is_directory())
        _load_dir(so_dir / ".libs");
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
    return first_iterator(_imp->keys.begin());
}

RepositoryFactory::ConstIterator
RepositoryFactory::end_keys() const
{
    return first_iterator(_imp->keys.end());
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

