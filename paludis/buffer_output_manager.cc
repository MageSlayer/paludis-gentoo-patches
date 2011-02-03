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

#include <paludis/buffer_output_manager.hh>
#include <paludis/util/buffer_output_stream.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/set.hh>
#include <paludis/util/exception.hh>

using namespace paludis;

namespace paludis
{
    template <>
    struct Imp<BufferOutputManager>
    {
        const std::shared_ptr<OutputManager> child;
        BufferOutputStream stdout_stream;
        BufferOutputStream stderr_stream;

        Imp(
                const std::shared_ptr<OutputManager> & c) :
            child(c)
        {
        }
    };
}

BufferOutputManager::BufferOutputManager(
        const std::shared_ptr<OutputManager> & c) :
    _imp(c)
{
}

BufferOutputManager::~BufferOutputManager()
{
    flush();
}

std::ostream &
BufferOutputManager::stdout_stream()
{
    return _imp->stdout_stream;
}

std::ostream &
BufferOutputManager::stderr_stream()
{
    return _imp->stderr_stream;
}

void
BufferOutputManager::succeeded()
{
    _imp->child->succeeded();
}

void
BufferOutputManager::ignore_succeeded()
{
    _imp->child->ignore_succeeded();
}

void
BufferOutputManager::message(const MessageType, const std::string &)
{
}

void
BufferOutputManager::flush()
{
    _imp->stdout_stream.unbuffer(_imp->child->stdout_stream());
    _imp->stderr_stream.unbuffer(_imp->child->stderr_stream());
    _imp->child->flush();
}

bool
BufferOutputManager::want_to_flush() const
{
    return _imp->stdout_stream.anything_to_unbuffer() || _imp->stderr_stream.anything_to_unbuffer();
}

void
BufferOutputManager::nothing_more_to_come()
{
    _imp->child->nothing_more_to_come();
}

const std::shared_ptr<const Set<std::string> >
BufferOutputManager::factory_managers()
{
    std::shared_ptr<Set<std::string> > result(std::make_shared<Set<std::string>>());
    result->insert("buffer");
    return result;
}

const std::shared_ptr<OutputManager>
BufferOutputManager::factory_create(
        const OutputManagerFactory::KeyFunction & key_func,
        const OutputManagerFactory::CreateChildFunction & create_child,
        const OutputManagerFactory::ReplaceVarsFunc &)
{
    std::string child_str(key_func("child"));
    if (child_str.empty())
        throw ConfigurationError("No child specified for BufferOutputManager");
    const std::shared_ptr<OutputManager> child(create_child(child_str));

    return std::make_shared<BufferOutputManager>(child);
}

template class Pimp<BufferOutputManager>;

