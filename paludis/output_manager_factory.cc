/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/output_manager_factory.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/util/member_iterator-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/join.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/set-impl.hh>
#include <paludis/util/system.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/about.hh>
#include <unordered_map>
#include <list>

#include <paludis/buffer_output_manager.hh>
#include <paludis/command_output_manager.hh>
#include <paludis/file_output_manager.hh>
#include <paludis/format_messages_output_manager.hh>
#include <paludis/forward_at_finish_output_manager.hh>
#include <paludis/standard_output_manager.hh>
#include <paludis/tee_output_manager.hh>

using namespace paludis;

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_create_function> create_function;
    }
}

namespace
{
    struct Funcs
    {
        NamedValue<n::create_function, OutputManagerFactory::CreateFunction> create_function;
    };

    typedef std::unordered_map<std::string, Funcs> Keys;

    const Funcs & fetch(const Keys & keys, const std::string & key)
    {
        if (key.empty())
            throw ConfigurationError("Key 'handler' not specified when creating an output manager");

        Keys::const_iterator i(keys.find(key));
        if (i == keys.end())
            throw ConfigurationError("Format '" + key + "' not supported when creating an output manager (known formats are { "
                    + join(first_iterator(keys.begin()), first_iterator(keys.end()), ", ") + " })");

        return i->second;
    }
}

namespace paludis
{
    template <>
    struct Imp<OutputManagerFactory>
    {
        Keys keys;
        std::list<void *> dl_opened;
    };

    template <>
    struct WrappedForwardIteratorTraits<OutputManagerFactory::ConstIteratorTag>
    {
        typedef FirstIteratorTypes<Keys::const_iterator>::Type UnderlyingIterator;
    };
}

OutputManagerFactory::OutputManagerFactory() :
    _imp()
{
    /* we might want to make this plugin loadable at some point */
    add_manager(BufferOutputManager::factory_managers(), BufferOutputManager::factory_create);
    add_manager(CommandOutputManager::factory_managers(), CommandOutputManager::factory_create);
    add_manager(FileOutputManager::factory_managers(), FileOutputManager::factory_create);
    add_manager(FormatMessagesOutputManager::factory_managers(), FormatMessagesOutputManager::factory_create);
    add_manager(ForwardAtFinishOutputManager::factory_managers(), ForwardAtFinishOutputManager::factory_create);
    add_manager(StandardOutputManager::factory_managers(), StandardOutputManager::factory_create);
    add_manager(TeeOutputManager::factory_managers(), TeeOutputManager::factory_create);
}

OutputManagerFactory::~OutputManagerFactory() = default;

const std::shared_ptr<OutputManager>
OutputManagerFactory::create(
        const KeyFunction & key_function,
        const CreateChildFunction & create_child_function,
        const ReplaceVarsFunc & replace_vars_func
        ) const
{
    Context context("When creating output manager:");
    return fetch(_imp->keys, key_function("handler")).create_function()(key_function, create_child_function, replace_vars_func);
}

OutputManagerFactory::ConstIterator
OutputManagerFactory::begin_keys() const
{
    return ConstIterator(first_iterator(_imp->keys.begin()));
}

OutputManagerFactory::ConstIterator
OutputManagerFactory::end_keys() const
{
    return ConstIterator(first_iterator(_imp->keys.end()));
}

void
OutputManagerFactory::add_manager(
        const std::shared_ptr<const Set<std::string> > & formats,
        const CreateFunction & create_function
        )
{
    for (Set<std::string>::ConstIterator f(formats->begin()), f_end(formats->end()) ;
            f != f_end ; ++f)
    {
        if (! _imp->keys.insert(std::make_pair(*f, make_named_values<Funcs>(
                            n::create_function() = create_function
                            ))).second)
            throw ConfigurationError("Handler for output manager format '" + stringify(*f) + "' already exists");
    }
}

namespace paludis
{
    template class Pimp<OutputManagerFactory>;
    template class Singleton<OutputManagerFactory>;
    template class WrappedForwardIterator<OutputManagerFactory::ConstIteratorTag, const std::string>;
}
