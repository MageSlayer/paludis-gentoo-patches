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

#include <paludis/environment_factory.hh>
#include <paludis/util/instantiation_policy-impl.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/system.hh>
#include <paludis/util/member_iterator-impl.hh>
#include <paludis/util/join.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/distribution.hh>
#include <paludis/about.hh>
#include <tr1/unordered_map>
#include <dlfcn.h>
#include <stdint.h>
#include <list>
#include "config.h"

using namespace paludis;

typedef std::tr1::unordered_map<std::string, EnvironmentFactory::CreateFunction> Keys;

namespace paludis
{
    template <>
    struct Implementation<EnvironmentFactory>
    {
        Keys keys;
        std::list<void *> dl_opened;
    };
}

EnvironmentFactory::EnvironmentFactory() :
    PrivateImplementationPattern<EnvironmentFactory>(new Implementation<EnvironmentFactory>)
{
    FSEntry so_dir(getenv_with_default("PALUDIS_ENVIRONMENT_SO_DIR", LIBDIR "/paludis/environments"));
    if (! so_dir.is_directory())
        throw InternalError(PALUDIS_HERE, "PALUDIS_ENVIRONMENT_SO_DIR '" + stringify(so_dir) + "' not a directory");
    _load_dir(so_dir);
}

EnvironmentFactory::~EnvironmentFactory()
{
}

const std::tr1::shared_ptr<Environment>
EnvironmentFactory::create(const std::string & s) const
{
    Context context("When making environment from specification '" + s + "':");

    std::string key, suffix;
    std::string::size_type p(s.find(':'));

    if (std::string::npos == p)
        key = s;
    else
    {
        key = s.substr(0, p);
        suffix = s.substr(p + 1);
    }

    if (key.empty())
        key = (*DistributionData::get_instance()->distribution_from_string(
                getenv_with_default("PALUDIS_DISTRIBUTION", DEFAULT_DISTRIBUTION))).default_environment();

    try
    {
        Keys::const_iterator i(_imp->keys.find(key));
        if (_imp->keys.end() == i)
            throw ConfigurationError("Format '" + key + "' not supported when creating an environment (known formats are { "
                    + join(first_iterator(_imp->keys.begin()), first_iterator(_imp->keys.end()), ", ") + "})");
        return i->second(suffix);
    }
    catch (const FallBackToAnotherFormatError &)
    {
        std::string f((*DistributionData::get_instance()->distribution_from_string(
                    getenv_with_default("PALUDIS_DISTRIBUTION", DEFAULT_DISTRIBUTION))).fallback_environment());
        if (s.empty() && ! f.empty())
        {
            Keys::const_iterator i(_imp->keys.find(f));
            if (_imp->keys.end() == i)
                throw;
            else
                return i->second(suffix);
        }
        else
            throw;
    }
}

void
EnvironmentFactory::_load_dir(const FSEntry & so_dir)
{
    for (DirIterator d(so_dir), d_end ; d != d_end ; ++d)
    {
        if (d->is_directory())
            _load_dir(*d);

        if (! is_file_with_extension(*d, ".so." + stringify(100 * PALUDIS_VERSION_MAJOR + PALUDIS_VERSION_MINOR),
                    IsFileWithOptions()))
            continue;

        /* don't use RTLD_LOCAL, g++ is over happy about template instantiations, and it
         * can lead to multiple singleton instances. */
        void * dl(dlopen(stringify(*d).c_str(), RTLD_GLOBAL | RTLD_NOW));

        if (dl)
        {
            _imp->dl_opened.push_back(dl);

            void * reg(dlsym(dl, "paludis_initialise_environment_so"));
            if (reg)
            {
                reinterpret_cast<void (*)(EnvironmentFactory * const)>(reinterpret_cast<uintptr_t>(reg))(this);
            }
            else
                throw InternalError(PALUDIS_HERE, "No paludis_initialise_environment_so function defined in '" + stringify(*d) + "'");
        }
        else
            throw InternalError(PALUDIS_HERE, "Couldn't dlopen '" + stringify(*d) + "': " + stringify(dlerror()));
    }

    if ((so_dir / ".libs").is_directory())
        _load_dir(so_dir / ".libs");
}

void
EnvironmentFactory::add_environment_format(
        const std::tr1::shared_ptr<const Set<std::string> > & formats,
        const CreateFunction & create_function
        )
{
    for (Set<std::string>::ConstIterator f(formats->begin()), f_end(formats->end()) ;
            f != f_end ; ++f)
    {
        if (! _imp->keys.insert(std::make_pair(*f, create_function)).second)
            throw ConfigurationError("Handler for environment format '" + stringify(*f) + "' already exists");
    }
}

template class InstantiationPolicy<EnvironmentFactory, instantiation_method::SingletonTag>;
template class PrivateImplementationPattern<EnvironmentFactory>;

