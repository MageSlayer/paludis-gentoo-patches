/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/util/singleton-impl.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/system.hh>
#include <paludis/util/member_iterator-impl.hh>
#include <paludis/util/join.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/env_var_names.hh>

#include <paludis/distribution.hh>
#include <paludis/about.hh>

#include <unordered_map>
#include <list>

#include "config.h"

using namespace paludis;

typedef std::unordered_map<std::string, EnvironmentFactory::CreateFunction> Keys;

namespace paludis
{
    template <>
    struct Imp<EnvironmentFactory>
    {
        Keys keys;
    };

    namespace environment_groups
    {
        ENVIRONMENT_GROUPS_DECLS;
    }

    template <>
    void register_environment<NoType<0u> >(const NoType<0u> * const, EnvironmentFactory * const)
    {
    }
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

EnvironmentFactory::EnvironmentFactory() :
    _imp()
{
    using namespace environment_groups;

    register_environment(static_cast<const TypeOrNoType<ENVIRONMENT_GROUP_IF_dummy>::Type *>(nullptr), this);
    register_environment(static_cast<const TypeOrNoType<ENVIRONMENT_GROUP_IF_paludis>::Type *>(nullptr), this);
    register_environment(static_cast<const TypeOrNoType<ENVIRONMENT_GROUP_IF_portage>::Type *>(nullptr), this);
    register_environment(static_cast<const TypeOrNoType<ENVIRONMENT_GROUP_IF_test>::Type *>(nullptr), this);
}

EnvironmentFactory::~EnvironmentFactory() = default;

const std::shared_ptr<Environment>
EnvironmentFactory::create(const std::string & s) const
{
    Context context("When making environment from specification '" + s + "':");

    std::string key;
    std::string suffix;
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
                getenv_with_default(env_vars::distribution, DEFAULT_DISTRIBUTION))).default_environment();

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
                    getenv_with_default(env_vars::distribution, DEFAULT_DISTRIBUTION))).fallback_environment());
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
EnvironmentFactory::add_environment_format(
        const std::shared_ptr<const Set<std::string> > & formats,
        const CreateFunction & create_function
        )
{
    for (const auto & format : *formats)
    {
        if (! _imp->keys.insert(std::make_pair(format, create_function)).second)
            throw ConfigurationError("Handler for environment format '" + stringify(format) + "' already exists");
    }
}

namespace paludis
{
    template class Singleton<EnvironmentFactory>;
    template class Pimp<EnvironmentFactory>;
}
